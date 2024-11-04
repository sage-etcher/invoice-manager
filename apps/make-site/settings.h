#ifndef INVOICE_UPDATE_SETTINGS_HEADER
#define INVOICE_UPDATE_SETTINGS_HEADER


extern int g_set_logging_mode;
extern char *g_set_database;
extern char *g_set_fmt_file;
extern char *g_set_secname;
extern char *g_set_sqlquery;
extern char *g_set_output_file;


void settings_load_defaults (void);


#endif