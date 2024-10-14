#include "settings.h"

#include <logging-lib/logging.h>
#include "config.h"


int g_set_logging_mode;
char *g_set_database;
char *g_set_fmtfile;
char *g_set_secname;
char *g_set_sqlquery;
char *g_set_output_file;


void
settings_load_defaults (void)
{
    g_set_logging_mode = DEFAULT_LOGGING_MODE;
    g_set_database     = DEFAULT_DATABASE;
    g_set_fmtfile      = DEFAULT_FORMAT_FILE;
    g_set_secname      = DEAFULT_SECTION_NAME;
    g_set_sqlquery     = DEFAULT_SQLQUERY;
    g_set_output_file  = NULL;

    return;
}