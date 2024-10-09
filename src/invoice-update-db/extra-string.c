
#include "extra-string.h"

#include <ctype.h>
#include "logging.h"
#include <string.h>


char *
find_replace_char (char *src, char find, char replace)
{
    char *iter = src;

    while ((iter = strchr (iter, find)) != NULL)
    {
        *iter = replace;
    }

    return src;
}


char *
trim_preceeding (char *src)
{
    if (src == NULL) return NULL;
    for (; ((*src != '\0') && (isspace (*src))); src++) {}
    return src;
}

char *
trim_trailing (char *src)
{
    if (src == NULL) return NULL;
    char *end = strchr (src, '\0') - 1;
    for (; ((end >= src) && (isspace (*end))); end--) {}
    *(end + 1) = '\0';
    return src;
}


char *
trim_whitespace (char *src)
{
    char *result = src;

    result = trim_preceeding (result);
    result = trim_trailing   (result);

    return result;
}


int
is_empty (char *src)
{
    return (*src == '\0');
}


char *
string_join (char **array, size_t n, char *seperator)
{
    char *dest = NULL;
    size_t dest_len = 0;

    if ((NULL == array) || (NULL == seperator) || (0 == n))
    {
        errno = EINVAL;
        return NULL;
    }

    for (size_t i = 0; i < n; i++)
    {
        if (NULL == array[i])
        {
            errno = EINVAL;
            return NULL;
        }
        dest_len += strlen (array[i]);
    }

    dest_len += (strlen (seperator) * (n - 1));

    dest = calloc (dest_len + 1, sizeof (char));
    if (NULL == dest)
    {
        return NULL;
    }

    for (size_t i = 0; i < n; i++)
    {
        (void)strcat (dest, array[i]);

        if ((i + 1) < n) (void)strcat (dest, seperator);
    }

    return dest;
}
