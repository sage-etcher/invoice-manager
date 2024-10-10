
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


/* initialize loggging streams for custom printf macros */
void logging_init (void);
void logging_quit (void);

void log_file (const char *filename, char *msg);

#endif /* header guard */
/* end of file */