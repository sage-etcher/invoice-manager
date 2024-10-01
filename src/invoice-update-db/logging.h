
#ifndef LOGGING_HEADER
#define LOGGING_HEADER


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
#define log_info(...) ((void)fprintf (g_info, __VA_ARGS__))
#define log_verbose(...) ((void)fprintf (g_verbose, __VA_ARGS__))
#define log_debug(...) ((void)fprintf (g_debug, __VA_ARGS__))
#define log_warning(...) ((void)fprintf (g_warning, "warning: " __VA_ARGS__))
#define log_error(...) ((void)fprintf (g_error, "error: " __VA_ARGS__))

#define log_unimplemented() { \
    log_debug ("%s:%d: %s(): unimplemented\n", __FILE__, __LINE__, __FUNCTION__); \
    abort (); \
}


/* initialize loggging streams for custom printf macros */
void logging_init (void);


#endif /* header guard */
/* end of file */