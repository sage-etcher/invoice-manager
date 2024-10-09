#include <stdio.h>


/* defined properly in SQLite3 library */
#define STATIC NULL

int bind_int  (void *, int index, int value);
int bind_text (void *, int index, char *value, int n, void(*)(void *));
int bind_null (void *, int index);
int bind_parameter_index (void *, char *name);


/* my C11 macro shenanegans */
#define BIND_N(stmt, index, value, n) (_Generic((value),                      \
       int: bind_int  ((stmt), (index), (int)(value)),                        \
    char *: bind_text ((stmt), (index), (char*)(value), (n), (STATIC)),       \
    void *: bind_null ((stmt), (index))))

#define BIND(stmt, index, value)                                              \
    (BIND_N ((stmt), (index), (value), -1))

#define BIND_NAME(stmt, name, value)                                          \
    (BIND ((stmt), bind_parameter_index ((stmt), (name)), (value)))

#define BIND_NAME_OR_NULL(stmt, name, value)                                  \
    ((value) ? BIND_NAME((stmt), (name), (value)) :                           \
               BIND_NAME((stmt), (name), NULL)) 


/* test usage */
int main (void)
{
    void *stmt = NULL;

    char *value_one = "value";  /* should use bind_text */
    int value_two = 2;          /* should use bind_int  */
    int value_three = 0;        /* should use bind_null */

    /* warnings for the next 3 lines */
    (void)BIND_NAME (stmt, ":NAME_ONE", value_one);
    (void)BIND_NAME (stmt, ":NAME_TWO", value_two);
    (void)BIND_NAME_OR_NULL (stmt, ":NAME_THREE", value_three);

    return 0;
}


/* test definitions for the "SQLite3 library" functions */
int 
bind_int  (void *s, int index, int value)
{
    printf ("bind_int: {%p, %d, %d}\n", s, index, value);
    return 0;
}


int 
bind_text (void *s, int index, char *value, int n, void(*callback)(void *))
{
    printf ("bind_text: {%p, %d, '%s', %d, %p}\n", s, index, value, n, callback);
    return 0;
}


int 
bind_null (void *s, int index)
{
    printf ("bind_null: {%p, %d}\n", s, index);
    return 0;
}


int 
bind_parameter_index (void *s, char *name)
{
    static i = 1;
    printf ("bind_parameter_index: {%p, '%s'} -> %d\n", s, name, i);

    return i++;
}


/* end of file */