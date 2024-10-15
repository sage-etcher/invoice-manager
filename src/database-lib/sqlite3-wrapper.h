
#ifndef MY_SQLITE_WRAPPER_HEADER
#define MY_SQLITE_WRAPPER_HEADER

#include <sqlite3.h>
#include <stddef.h>


/** An enum of column_t types.
 * 
 * Is mapped 1 to 1 to sqlite3's column types SQLITE_*
 * 
 * @see column_t
 * @see column_get()
 * @see column_type_string()
 * @see column_match_type()
 */
typedef enum
{
    COLUMN_NULL    = SQLITE_NULL,       /**< Equivalent to SQLITE_NULL */
    COLUMN_INTEGER = SQLITE_INTEGER,    /**< Equivalent to SQLITE_INTEGER */
    COLUMN_FLOAT   = SQLITE_FLOAT,      /**< Equivalent to SQLITE_FLOAT */
    COLUMN_BLOB    = SQLITE_BLOB,       /**< Equivalent to SQLITE_BLOB */
    COLUMN_TEXT    = SQLITE3_TEXT,      /**< Equivalent to SQLITE3_TEXT */
} column_type_t;


/** An object for reading sqlite3_stmt results.
 * 
 * Contains a type signature, column name, result size (where applicable), 
 * and column data.
 *
 * @see column_type_t 
 * @see column_get()
 * @see column_type_string()
 * @see column_match_type()
 * @see sqlwrap_execute()
 */
typedef struct 
{
    column_type_t type; /**< the type of data stored. */
    char *name;         /**< the columns name. */
    size_t bytes;       /**< (string and blob only) size of data. */

    /** A union of posible column data types.
     * 
     * Behavior is defiend by the objects type feild.
     * If type is COLUMN_NULL none of the union values are guaranteed.
     */
    union 
    {
        int i;          /**< valid while type is COLUMN_INTEGER */
        float f;        /**< valid while type is COLUMN_FLAOT */
        void *p;        /**< valid while type is COLUMN_BLOB */
        char *s;        /**< valid while type is COLUMN_TEXT */
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
const char *column_type_string (column_type_t type);
int         column_match_type (column_t col, column_type_t *types, size_t n);


/** bind template by index with size. 
 * 
 * A generic macro for sqlite3_bind_*() api.
 * 
 * Depending on the type of value, expands to bind various types of data 
 * to template by index.
 * 
 * safe macro. each parameter is only expanded once.
 * 
 * @note for some reason, MSVC HATES this generic. throws 4047 and 4024 
 * warnings everytime it is called. :/
 *  
 * @param stmt a valid prepared sqlite3_stmt.
 * @param index the index of the SQL Statement template (1 based).
 * @param value a value to bind.
 * @param n (required for text and blob) size of data, -1 for full string.
 * 
 * @return (int)SQLITE_OK on a success.
 * 
 * @see SQLWRAP_BIND()
 * @see SQLWRAP_BIND_NAME()
 * @see SQLWRAP_BIND_NAME_OR_NULL()
 * @see sqlwrap_prepare_n()
 */
#define SQLWRAP_BIND_N(stmt, index, value, n) _Generic((value),               \
       int: sqlite3_bind_int  (stmt, index, value),                           \
    char *: sqlite3_bind_text (stmt, index, value, n, SQLITE_STATIC),         \
    void *: sqlite3_bind_null (stmt, index))


/** bind template by index.
 *
 * expands to SQLWRAP_BIND_N setting n to -1 for use in cases of null 
 * terminated strings and most other datatypes.
 * 
 * safe macro. each parameter is only expanded once.
 * 
 * @param stmt a valid prepared sqlite3_stmt.
 * @param index the index of the SQL Statement template (1 based).
 * @param value a value to bind.
 * 
 * @return (int)SQLITE_OK on a success.
 * 
 * @see SQLWRAP_BIND_N()
 * @see sqlwrap_prepare_n()
 */
#define SQLWRAP_BIND(stmt, index, value)                                      \
    (SQLWRAP_BIND_N (stmt, index, value, -1))


/** bind template by name.
 *
 * bind value to a template referenced by name. 
 * 
 * unsafe macro. stmt is expanded twice.
 * 
 * @param stmt a valid prepared sqlite3_stmt.
 * @param name a SQL Statment Template name, to insert value at.
 * @param value a value to bind.
 * 
 * @return (int)SQLITE_OK on a success.
 * 
 * @see SQLWRAP_BIND()
 * @see SQLWRAP_BIND_N()
 * @see SQLWRAP_BIND_NAME_OR_NULL()
 * @see sqlwrap_prepare_n()
 */
#define SQLWRAP_BIND_NAME(stmt, name, value)                                  \
    (SQLWRAP_BIND (stmt, (sqlite3_bind_parameter_index (stmt, name)), value))


/** bind template by name, value may be NULL.
 *
 * bind value to a template referenced by name, where value may implicitly 
 * be NULL if value expands to a falsy value.
 * 
 * unsafe macro. stmt and value are both expanded twice.
 * 
 * @param stmt a valid prepared sqlite3_stmt.
 * @param name a SQL Statment Template name, to insert value at.
 * @param value a value to bind, falsey for NULL.
 * 
 * @return (int)SQLITE_OK on a success.
 * 
 * @see SQLWRAP_BIND()
 * @see SQLWRAP_BIND_N()
 * @see SQLWRAP_BIND_NAME()
 * @see sqlwrap_prepare_n()
 */
#define SQLWRAP_BIND_NAME_OR_NULL(stmt, name, value)                          \
    (value ? SQLWRAP_BIND_NAME(stmt, name, value) :                           \
             SQLWRAP_BIND_NAME(stmt, name, NULL)) 


#endif