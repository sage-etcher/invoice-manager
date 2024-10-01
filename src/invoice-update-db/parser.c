#include "parser.h"


parsed_t *
parse_path (char *filepath)
{
    static parsed_t *s_result = NULL;

    char *filename = NULL;
    static parsed_t *s_result = NULL;

    /* null guard */
    if (filepath == NULL) return NULL;

    /* initial run */
    if (s_result == NULL)
    {
        s_result = malloc (sizeof (parsed_t));
        if (!s_result) return NULL;
    }

    //filename = file_basename (filepath);
    filename = "test_2025-2035202406222-4422.pdf";

    




}