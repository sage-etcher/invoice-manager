
#ifndef MY_SQLITE_WRAPPER_HEADER
#define MY_SQLITE_WRAPPER_HEADER

#include <sqlite3.h>
#include <stddef.h>


typedef struct 
{
    char *name;
    int type;
    size_t bytes;
    union 
    {
        int i;
        float f;
        void *p;
        char *s;
    } m;
} column_t;


void sqlwrap_log_error (sqlite3 *db);
void sqlwrap_log_errorcode (int errcode);

sqlite3 *sqlwrap_open (const char *dbfile, int flags);
sqlite3 *sqlwrap_open_memory (const char *dbfile, int flags);
int      sqlwrap_close (sqlite3 *db);

size_t sqlwrap_prepare_n (sqlite3 *db, const char **stmt_texts, sqlite3_stmt **stmts, size_t n);
void   sqlwrap_finalize_n (sqlite3_stmt **stmts, size_t n);
int    sqlwrap_execute (sqlite3 *db, sqlite3_stmt *stmt, int retry_count, void **result_ptr, void *(*callback_get_item)(sqlite3_stmt *));

column_t    column_get (sqlite3_stmt *stmt, int i);
const char *column_type_string (int type);
int         column_match_type (column_t col, int *types, size_t n);


/* msvc _HATES_ this for some reason? but it does compile! */
/* it looks like, msvc is doing type checks before expanding the _Generic */
#define SQLWRAP_BIND_N(stmt, index, value, n) _Generic((value),               \
       int: sqlite3_bind_int  (stmt, index, value),                           \
    char *: sqlite3_bind_text (stmt, index, value, n, SQLITE_STATIC),         \
    void *: sqlite3_bind_null (stmt, index))


#define SQLWRAP_BIND(stmt, index, value)                                      \
    (SQLWRAP_BIND_N (stmt, index, value, -1))


#define SQLWRAP_BIND_NAME(stmt, name, value)                                  \
    (SQLWRAP_BIND (stmt, (sqlite3_bind_parameter_index (stmt, name)), value))


#define SQLWRAP_BIND_NAME_OR_NULL(stmt, name, value)                          \
    (value ? SQLWRAP_BIND_NAME(stmt, name, value) :                           \
             SQLWRAP_BIND_NAME(stmt, name, NULL)) 


#endif