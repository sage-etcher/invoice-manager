#ifndef INVOICE_UPDATE_SETTINGS_HEADER
#define INVOICE_UPDATE_SETTINGS_HEADER


extern int g_set_logging_mode;
extern int g_set_ignore_cached;
extern int g_set_dryrun;

extern char *g_set_database;
extern char *g_set_badfilelog;


void settings_load_defaults (void);


#endif