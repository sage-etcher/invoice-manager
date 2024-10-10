#include "parser.h"

#include "logging-lib/logging.h"
#include "myfileio-lib/myfileio.h"
#include "mystring-lib/mystring.h"
#define PCRE2_STATIC
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


enum
{
    RE_INVOICE_GROUP_YYYY_MMDD,
    RE_INVOICE_GROUP,
    RE_COUNT,
};
const char *G_PATTERNS_RAW[RE_COUNT] = {
    [RE_INVOICE_GROUP_YYYY_MMDD] = "^(.*?)(\\d{4}).*?(\\d{2})(\\d{2}).*\\.pdf",
    [RE_INVOICE_GROUP] = "^(.*?)(\\d{2})(\\d{2}).*?(\\d{2})(\\d{2}).*\\.pdf",
};
static pcre2_code *g_re_patterns[RE_COUNT];


typedef struct 
{
    int year;
    int month;
    int day;
} date_tuple_t;


static void destroy_pattern_arr (pcre2_code **compiled_arr, size_t n);
static int compile_pattern_arr (const char **pattern_text_arr, size_t n, pcre2_code **compiled_out);
static void re_group (char *dst, size_t n, PCRE2_SPTR subject, PCRE2_SIZE *ovector, int i);

static date_tuple_t groups_as_ddmm_yyyy (int a, int b, int c, int d);
static date_tuple_t groups_as_mmdd_yyyy (int a, int b, int c, int d);
static date_tuple_t groups_as_yyyy_mmdd (int a, int b, int c, int d);
static date_tuple_t guess_date_format (char *a, char *b, char *c, char *d);
static int validate_date (int year, int month, int day);


int
parser_init (void)
{
    (void)compile_pattern_arr (G_PATTERNS_RAW, RE_COUNT, g_re_patterns);

    return 0;
}


void
parser_quit (void)
{
    destroy_pattern_arr (g_re_patterns, RE_COUNT);

    return;
}


static int
compile_pattern_arr (const char **pattern_text_arr, size_t n, pcre2_code **compiled_out)
{
    int errornumber;
    PCRE2_SIZE erroroffset;

    for (size_t i = 0; i < n; i++)
    {
        compiled_out[i] = pcre2_compile (
                (PCRE2_SPTR)pattern_text_arr[i],
                PCRE2_ZERO_TERMINATED,
                0,
                &errornumber,
                &erroroffset,
                NULL);

        if (compiled_out[i] == NULL)
        {
            PCRE2_UCHAR buffer[256];
            pcre2_get_error_message (errornumber, buffer, sizeof (buffer));
            log_verbose ("PCRE2: %d: %s\n", errornumber, buffer);
            log_error ("PCRE2: %zu: statement compililation failed\n", i);
            destroy_pattern_arr (compiled_out, i);
            return 1;
        }
    }

    return 0;
}


static void
destroy_pattern_arr (pcre2_code **compiled_arr, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        pcre2_code_free (compiled_arr[i]);
        compiled_arr[i] = NULL;
    }

    return;
}


static parsed_t *
preform_regex_match (char *filename)
{
    int retcode;

    static parsed_t s_result;

    pcre2_code *re = NULL;
    PCRE2_SIZE *ovector = NULL;
    pcre2_match_data *match_data = NULL;

    /* null guard */
    if (filename == NULL) return NULL;

    re = g_re_patterns[RE_INVOICE_GROUP];
    enum 
    {
        MATCH,
        GROUP_NAME,
        GROUP_DATE_A,
        GROUP_DATE_B,
        GROUP_DATE_C,
        GROUP_DATE_D,
    };
    match_data = pcre2_match_data_create_from_pattern (re, NULL);
    retcode = pcre2_match (
            re,
            (PCRE2_SPTR)filename,
            strlen (filename),
            0,
            0,
            match_data,
            NULL);

    if (retcode < 0)
    {
        switch (retcode)
        {
        case PCRE2_ERROR_NOMATCH:
            log_warning ("PCRE2: no matches found\n");
            break;

        default:
            log_error ("PCRE2: %d: unknown error\n", retcode);
            break;
        }

        pcre2_match_data_free (match_data);
        return NULL;
    }

    ovector = pcre2_get_ovector_pointer (match_data);    

    /* not enough room in match_data */
    if (retcode == 0)
    {
        log_debug ("PCRE2: ovector too small\n");

        pcre2_match_data_free (match_data);
        return NULL;
    }

    re_group (s_result.name_raw, MAX_PARSED_NAME, (PCRE2_SPTR8)filename, ovector, GROUP_NAME);
    re_group (s_result.group_a, 2, (PCRE2_SPTR8)filename, ovector, GROUP_DATE_A);
    re_group (s_result.group_b, 2, (PCRE2_SPTR8)filename, ovector, GROUP_DATE_B);
    re_group (s_result.group_c, 2, (PCRE2_SPTR8)filename, ovector, GROUP_DATE_C);
    re_group (s_result.group_d, 2, (PCRE2_SPTR8)filename, ovector, GROUP_DATE_D);
    
    pcre2_match_data_free (match_data);
    return &s_result;
}


parsed_t *
parse_path (char *filepath)
{
    /* null guard */
    if (filepath == NULL) return NULL;

    /* search the filename for information */
    char *filename = basename (filepath);
    parsed_t *parsed = preform_regex_match (filename);
    if (parsed == NULL) return NULL;

    /* get filepath */
    parsed->filepath = filepath;

    /* get customer name */
    (void)find_replace_char (parsed->name_raw, '_', ' ');
    parsed->name = trim_whitespace (parsed->name_raw);

    /* validate the date */
    date_tuple_t date = guess_date_format (parsed->group_a, parsed->group_b, 
                                           parsed->group_c, parsed->group_d);

    parsed->year  = date.year;
    parsed->month = date.month;
    parsed->day   = date.day;

    return parsed;
}


static void
re_group (char *dst, size_t n, PCRE2_SPTR subject, PCRE2_SIZE *ovector, int i)
{
    PCRE2_SPTR substring_start = subject + ovector[2*i];
    PCRE2_SIZE substring_length = ovector[2*i+1] - ovector[2*i];

    size_t min_length = (substring_length < n ? substring_length : n);

    memcpy (dst, (char *)substring_start, (size_t)min_length);
    dst[min_length] = '\0';

    return;
}


static int
validate_date (int year, int month, int day)
{
    time_t t = time (NULL);
    struct tm *tm = localtime (&t);

    const int DAYS_IN_MONTH[12]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* future dates are not allowed */
    if (!((year  < (tm->tm_year + 1900)) ||
          (month < (tm->tm_mon  + 1)) ||
          (day   < (tm->tm_mday))))
    {
        return 0;
    }

    /* too old/invalid range */
    if (year < 1900) 
    {
        return 0;
    }

    if ((month > 12) || (month < 1)) 
    {
        return 0;
    }

    if ((day > DAYS_IN_MONTH[month - 1]) || (day < 1)) 
    {
        return 0;
    }

    /* true */
    return 1;
}


static date_tuple_t
guess_date_format (char *a, char *b, char *c, char *d)
{
    int ia = atoi (a);
    int ib = atoi (b);
    int ic = atoi (c);
    int id = atoi (d);

    date_tuple_t date;

    date = groups_as_yyyy_mmdd (ia, ib, ic, id);
    if (validate_date (date.year, date.month, date.day)) return date;

    date = groups_as_mmdd_yyyy (ia, ib, ic, id);
    if (validate_date (date.year, date.month, date.day)) return date;

    /*
    date = groups_as_ddmm_yyyy (ia, ib, ic, id);
    if (validate_date (date.year, date.month, date.day)) return date;
    */

    return (date_tuple_t){ .year = 0, .month = 0, .day = 0 };
}

static date_tuple_t
groups_as_yyyy_mmdd (int a, int b, int c, int d)
{
    return (date_tuple_t){
        .year  = ((a * 100) + (b * 1)),
        .month = (c),
        .day   = (d),
    };
}

static date_tuple_t
groups_as_mmdd_yyyy (int a, int b, int c, int d)
{
    return (date_tuple_t){
        .month = (a),
        .day   = (b),
        .year  = ((c * 100) + (d * 1)),
    };
}

static date_tuple_t
groups_as_ddmm_yyyy (int a, int b, int c, int d)
{
    return (date_tuple_t){
        .day   = (a),
        .month = (b),
        .year  = ((c * 100) + (d * 1)),
    };
}
