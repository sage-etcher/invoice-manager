
#ifndef MALLOC_WRAPPER_HEADER 
#define MALLOC_WRAPPER_HEADER 

#include <stddef.h>


/* realloc free-on-error */
void *realloc_foe (void *ptr, size_t n);

#endif /* header guard */
/* end of file */