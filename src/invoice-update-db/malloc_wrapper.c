#include "malloc_wrapper.h"

#include <stdlib.h>


void *
realloc_foe (void *ptr, size_t n)
{
    void *new_ptr = realloc (ptr, n);
    if (new_ptr == NULL)
    {
        free (ptr); ptr = NULL;
    }

    return new_ptr;
}

