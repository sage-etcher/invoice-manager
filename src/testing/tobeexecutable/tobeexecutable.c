
#include "tobelibrary.h"
#include <stdio.h>


static int test_tobeexecutable (void);

int
main (int argc, char **argv)
{
    (void)test_tobeexecutable ();    
    (void)test_tobelibrary ();

    return 0;
}


static int
test_tobeexecutable (void)
{
    return printf ("test: tobeexecutable\n");
}
