
#include "logging.h"

#include <stdio.h>


/* declaration */
FILE *g_info    = NULL;
FILE *g_verbose = NULL;
FILE *g_debug   = NULL;
FILE *g_warning = NULL;
FILE *g_error   = NULL;


/* initialize the logging variables */
void
logging_init (void)
{
    g_info    = stdout;
    g_verbose = stdout;
    g_debug   = stderr;
    g_warning = stderr;
    g_error   = stderr;
    
    return;
}


void
logging_quit (void)
{
    g_info    = NULL;
    g_verbose = NULL;
    g_debug   = NULL;
    g_warning = NULL;
    g_error   = NULL;
}


void
log_file (const char *filename, char *msg)
{
    FILE *fp = NULL;
    fp = fopen (filename, "w+");
    if (!fp) return;

    (void)fprintf (fp, "%s\n", msg);

    fflush (fp);
    fclose (fp); 
    fp = NULL;
    return;
}
