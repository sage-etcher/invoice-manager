
#include "logging.h"

#include <stdio.h>


/* variable declarations */
FILE *g_info    = NULL;
FILE *g_verbose = NULL;
FILE *g_debug   = NULL;
FILE *g_warning = NULL;
FILE *g_error   = NULL;


/* file static variables */
static logging_mode_t s_log_mode = LOG_UNINITIALIZED;

static char *s_log_filepath = NULL;
static FILE *s_log_file = NULL;


/* file static function prototypes */
static void logging_clear (void);
static void logging_set (logging_mode_t mode);


/* console logging */
static void
logging_set (logging_mode_t mode)
{
    switch (mode)
    {
    case LOG_DEBUG:
        g_debug = stderr;
    case LOG_VERBOSE:
        g_verbose = stdout;
        g_warning = stderr;
    case LOG_TERSE:
        g_info = stdout;
    case LOG_ERRORS_ONLY:
        g_error = stderr;
    case LOG_SILENT:
    default:
        break;
    }

    s_log_mode = mode;
}


static void
logging_clear (void)
{
    g_info    = NULL;
    g_verbose = NULL;
    g_debug   = NULL;
    g_warning = NULL;
    g_error   = NULL;
}


/* initialize the logging variables */
void
logging_init (logging_mode_t mode, char *logfilepath)
{
    logging_clear ();    
    logging_set (mode);

    if (logfilepath != NULL) (void)logging_init_file (logfilepath);

    return;
}


void
logging_quit (void)
{
    logging_clear ();
    s_log_mode = LOG_UNINITIALIZED;

    (void)logging_quit_file ();

    return;
}


logging_mode_t
logging_get_mode (void)
{
    return s_log_mode;
}


/* file logging */
int
logging_init_file (char *logfilepath)
{
    FILE *fp = NULL;

    /* only init if not already initialized */
    if (s_log_file != NULL) return 0;

    /* open the provided file */
    (void)fopen_s (&fp, logfilepath, "a+");
    if (!fp) return 0;

    /* assign file ptr to static var */
    s_log_file = fp;
    s_log_filepath = logfilepath;

    return 1;
}


int
logging_quit_file (void)
{
    int retcode;

    if (s_log_file == NULL) return 1;

    /* flush the buffer and close the stream */
    (void)fflush (s_log_file);
    retcode = fclose (s_log_file);
    if (retcode) return retcode;

    /* set static variables */
    s_log_file = NULL;
    s_log_filepath = NULL;

    return 0;
}


void
log_file (char *msg)
{
    if (!s_log_file) return;

    (void)fprintf (s_log_file, "%s\n", msg);

    return;
}


void
log_implicit_file (const char *logfilepath, char *msg)
{
    FILE *fp = NULL;
    (void)fopen_s (&fp, logfilepath, "a+");
    if (!fp) return;

    (void)fprintf (fp, "%s\n", msg);

    fflush (fp);
    fclose (fp); 
    fp = NULL;
    return;
}


/* end of file */