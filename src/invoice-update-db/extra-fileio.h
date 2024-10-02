
#ifndef EXTRA_FILEIO_HEADER 
#define EXTRA_FILEIO_HEADER

#include <stdio.h>

/* stdin safe */
char *readline (FILE *stream);

/* file only */
char *freadline (FILE *stream);
int ffindc (int character, FILE *stream);

char *basename (char *filepath);

#endif /* header guard */
/* end of file */