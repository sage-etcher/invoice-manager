#include "parser.h"

#include "extra-fileio.h"
#include "logging.h"
#define PCRE2_STATIC
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


enum
{
    RE_INVOICE_GROUP,
    RE_COUNT,
};

const char *G_PATTERNS_RAW[RE_COUNT] = 
{
    [RE_INVOICE_GROUP] = "^(.*?)(\\d{4}).*?(\\d{2})(\\d{2}).*\\.pdf",
};

static pcre2_code *g_re_patterns[RE_COUNT] = { NULL };



static void destroy_pattern_arr (pcre2_code **compiled_arr, size_t n);
static int compile_pattern_arr (const char **pattern_text_arr, size_t n, pcre2_code **compiled_out);
static void re_group (char *dst, size_t n, PCRE2_SPTR subject, PCRE2_SIZE *ovector, int i);


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

    for (size_t i = 0; i < RE_COUNT; i++)
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
            log_error ("PCRE2 compilation of statement %zu failed at offset %d: %s\n", 
                    i, (int)erroroffset, buffer);
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


parsed_t *
parse_path (char *filepath)
{
    int retcode;

    static parsed_t s_result;
    char *filename = NULL;

    pcre2_code *re = NULL;
    PCRE2_SIZE erroroffset = 0;
    PCRE2_SIZE *ovector = NULL;
    pcre2_match_data *match_data = NULL;

    /* null guard */
    if (filepath == NULL) return NULL;

    /* do stuff with stuff */
    filename = basename (filepath);

    re = g_re_patterns[RE_INVOICE_GROUP];
    enum 
    {
        MATCH,
        GROUP_NAME,
        GROUP_YEAR,
        GROUP_MONTH,
        GROUP_DAY,
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
            log_warning ("Bad Filename: '%s'\n", filepath);
            break;

        default:
            log_error ("%d: Regex Matching Failed in filename: '%s'\n", 
                    retcode, filepath);
            break;
        }

        pcre2_match_data_free (match_data);
        return NULL;
    }

    ovector = pcre2_get_ovector_pointer (match_data);    

    /* not enough room in match_data */
    if (retcode == 0)
    {
        log_error ("ovector was not big enough for all the captured substring: '%s'\n", 
                filepath);

        pcre2_match_data_free (match_data);
        return NULL;
    }

    s_result.filepath = filepath;
    re_group (s_result.name,  MAX_PARSED_NAME,  filename, ovector, GROUP_NAME);
    re_group (s_result.year,  MAX_PARSED_YEAR,  filename, ovector, GROUP_YEAR);
    re_group (s_result.month, MAX_PARSED_MONTH, filename, ovector, GROUP_MONTH);
    re_group (s_result.day,   MAX_PARSED_DAY,   filename, ovector, GROUP_DAY);
    
    pcre2_match_data_free (match_data);
    return &s_result;
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


int
validate_date (struct tm *tm, int year, int month, int day)
{
    const int DAYS_IN_MONTH[12]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* future dates are not allowed */
    if (!((year  < (tm->tm_year + 1900)) ||
          (month < (tm->tm_mon  + 1)) ||
          (day   < (tm->tm_mday))))
    {
        log_debug ("date set into the future: %d/%d/%d\n", month, day, year);
        return 0;
    }

    /* too old/invalid range */
    if (year < 1900) 
    {
        log_debug ("year too old: %d\n", year);
        return 0;
    }

    if ((month > 12) || (month < 1)) 
    {
        log_debug ("invalid month: %d\n", month);
        return 0;
    }

    if ((day > DAYS_IN_MONTH[month - 1]) || (day < 1)) 
    {
        log_debug ("invalid day: %d\n", day);
        return 0;
    }

    /* true */
    return 1;
}