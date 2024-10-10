#include "settings.h"

#include <logging-lib/logging.h>
#include "config.h"



int g_set_logging_mode;
int g_set_ignore_cached;
int g_set_dryrun;

char *g_set_database;
char *g_set_badfilelog;


void
settings_load_defaults (void)
{
    g_set_logging_mode  = DEFAULT_LOGGING_MODE;
    g_set_ignore_cached = DEFAULT_IGNORE_CACHED;
    g_set_dryrun        = DEFAULT_DRYRUN;
    g_set_database      = DEFAULT_DATABASE;
    g_set_badfilelog    = DEFAULT_BADFILELOG;

    return;
}