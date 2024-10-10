
#include "myfileio.h"

#include <errno.h>
#include <logging-lib/logging.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* designed to work on any stream */
char *
readline (FILE *stream)
{
    /* note: s_result IS a memory leak, BUT ITS FAST THOO! :/
     *       it should probably be declared outside the function scope, so it 
     *       can be freed later. */
    static char *s_result = NULL;       /* reuse buffer between calls */
    static size_t s_alloc = 0;          /* reuse alloc between calls */
    const size_t DEFAULT_ALLOC = 2;     /* initial buffer length */

    int c;              /* character */
    size_t i = 0;       /* iterator/length */

    void *tmp = NULL;   /* realloc overhead */

    
    /* null guard */
    if (stream == NULL) return NULL;

    /* if stream is at end of file, return NULL */
    if (feof (stream)) return NULL;

    /* first time initialize */
    if (s_result == NULL)
    {
        s_result = malloc (DEFAULT_ALLOC + 1);
        if (!s_result) return NULL;
        s_alloc = DEFAULT_ALLOC;
    }

    /* append characters until buffer is empty or newline is found */
    while (c = fgetc (stream), !feof(stream))
    {
        if ((c == '\n') || (c == '\r') || (c == EOF)) break;

        if (i >= s_alloc)
        {
            /* handle size_t overflow */
            if (SIZE_MAX / 2 < s_alloc)
            {
                errno = ERANGE;
                return NULL;
            }

            /* extend s_result */
            tmp = realloc (s_result, s_alloc * 2 + 1);
            if (tmp == NULL) return NULL;
            s_result = tmp;
            s_alloc *= 2;
        }

        /* store the character in our buffer */
        s_result[i] = (char)c;

        /* update the iterator */
        i++; 
    }

    /* append the null terminator */
    s_result[i] = '\0';

    return s_result;
}


/* designed to work specifically on file streams, NOT on stdout/stdin/stderr 
 * or the like */
char *
freadline (FILE *stream)
{
    int start, end;
    size_t length = 0;
    char *line = NULL;


    /* null guard */
    if (stream == NULL) return NULL;

    start = ftell (stream);             /* save the initial position */
    end   = ffindc ('\n', stream);      /* find the next newline character */
    log_debug ("(start, end) => (%i, %i)\n", start, end);
    fseek (stream, start, SEEK_SET);    /* restore position */

    /* find the offset from start to end, and convert it to (size_t)length */
    if (end < start)
    {
        errno = ERANGE;
        return NULL;
    }
    length = (size_t)(end - start); 
    /* allocate the line buffer on the heap */
    line = (char *)malloc (length + 1);
    if (!line) 
    {
        return NULL;             /* validate allocation */
    }

    /* get the line, and terminate it */
    fgets (line, (int)length, stream);
    line[length] = '\0';

    return line;
}


int
ffindc (int character, FILE *stream)
{
    int c;

    /* null guard */
    if (stream == NULL) 
    {
        errno = EINVAL;
        return 0;
    }

    /* pop characters off the stream, until either EOF is reached, or 
     * character is read */
    while (!(feof (stream)) && 
           ((c = fgetc (stream)) != character)) {}

    /* return the current position */
    return ftell (stream);
}


/* string utils */
char *
basename (char *filepath)
{
    char *iter = NULL;

    if (filepath == NULL) return NULL;

    /* start on last character of the string */
    iter = strchr (filepath, '\0');
    iter--;

    /* loop until we reach the origin */
    while (iter >= filepath)
    {
        /* if directory character is reached we found the basename */
        if ((*iter == '/') ||
            (*iter == '\\'))
        {
            break;      /* stop looping */
        }

        iter--;         /* step backwards through the string */
    }

    /* return the cursor (either following '/' or at filepath)*/
    iter++;
    return iter;
}

/* end of file */