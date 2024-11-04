
#ifndef INVOICE_LOGGING_HEADER
#define INVOICE_LOGGING_HEADER


#include <stdio.h>

/* definition */
/* enable with init_logging() */
extern FILE *g_info;
extern FILE *g_verbose;
extern FILE *g_debug;
extern FILE *g_warning;
extern FILE *g_error;

/* logging mode definitions  */
typedef enum
{
    LOG_UNINITIALIZED,
    LOG_SILENT,
    LOG_ERRORS_ONLY,
    LOG_TERSE,
    LOG_VERBOSE,
    LOG_DEBUG,
} logging_mode_t;


/* format should always be an implicit c compile time constant for the 
 * "warning: " and "error: " prefixes to work properly */
#define log_info(...)    (void)((!g_info)    || (fprintf (g_info, __VA_ARGS__)))
#define log_verbose(...) (void)((!g_verbose) || (fprintf (g_verbose, "verbose: " __VA_ARGS__)))
#define log_debug(...)   (void)((!g_debug)   || (fprintf (g_debug, "debug: " __VA_ARGS__)))
#define log_warning(...) (void)((!g_warning) || (fprintf (g_warning, "warning: " __VA_ARGS__)))
#define log_error(...)   (void)((!g_error)   || (fprintf (g_error, "error: " __VA_ARGS__)))

#define log_unimplemented() { \
    log_debug ("%s:%d: %s(): unimplemented\n", __FILE__, __LINE__, __FUNCTION__); \
    abort (); \
}




void logging_init (logging_mode_t mode, char *logfilepath);
void logging_quit (void);
logging_mode_t logging_get_mode (void);

int   logging_init_file (char *logfilepath);
int   logging_quit_file (void);
char *logging_get_filepath (void);

void log_file (char *msg);
void log_implicit_file (const char *filefilepath, char *msg);



#endif /* header guard */
/* end of file */