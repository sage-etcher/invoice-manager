
#include "database.h"

#include <logging-lib/logging.h>
#include <mystring-lib/mystring.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* msvc _HATES_ this for somereason? but it does compile! */
/* it looks like, msvc is doing type checks before expanding the _Generic */
#define SQLITE_BIND_N(stmt, index, value, n) _Generic((value),                \
       int: sqlite3_bind_int  (stmt, index,    (int)value),                   \
    char *: sqlite3_bind_text (stmt, index, (char *)value, n, SQLITE_STATIC), \
    void *: sqlite3_bind_null (stmt, index))


#define SQLITE_BIND(stmt, index, value)                                       \
    (SQLITE_BIND_N (stmt, index, value, -1))


#define SQLITE_BIND_NAME(stmt, name, value)                                   \
    (SQLITE_BIND (stmt, (sqlite3_bind_parameter_index (stmt, name)), value))


#define SQLITE_BIND_NAME_OR_NULL(stmt, name, value)                           \
    (value ? SQLITE_BIND_NAME(stmt, name, value) :                            \
             SQLITE_BIND_NAME(stmt, name, NULL)) 


enum
{
    STMT_CREATE_TABLES,
    STMT_INSERT,
    STMT_UPDATE_BY_FILEPATH,
    STMT_UPDATE_BY_INVOICE_ID,
    STMT_SELECT_BY_CUSTOMER_NAME,
    STMT_SELECT_BY_FILEPATH,
    STMT_SELECT_BY_INVOICE_ID,
    STMT_MAX,
};
const char *S_STMTS_TEXT[STMT_MAX] = {
    [STMT_CREATE_TABLES] = 
        "CREATE TABLE IF NOT EXISTS invoices ("
            "invoice_id INTEGER PRIMARY KEY ASC, "
            "filepath TEXT NOT NULL UNIQUE, "
            "customer_name TEXT NOT NULL, "
            "year INTEGER, "
            "month INTEGER, "
            "day INTEGER, "
            "search_date INTEGER, "
            "error_flag INTEGER NOT NULL"
        ");",

    [STMT_INSERT] =
        "INSERT INTO invoices ("
            "filepath, customer_name, year, month, day, search_date, error_flag"
        ") " 
        "VALUES ("
            ":FILEPATH, "
            ":CUSTOMER, "
            ":YEAR, "
            ":MONTH, "
            ":DAY, "
            ":DATE, "
            ":ERROR"
        ");",

    [STMT_UPDATE_BY_FILEPATH] =
        "UPDATE invoices "
        "SET customer_name = :CUSTOMER, "
            "year"       " = :YEAR, "
            "month"      " = :MONTH, "
            "day"        " = :DAY, "
            "search_date"" = :DATE, "
            "error_flag" " = :ERROR "
        "WHERE filepath = :FILEPATH;",

    [STMT_UPDATE_BY_INVOICE_ID] =
        "UPDATE invoices "
        "SET "
            "filepath"   " = :FILEPATH, "
            "customer_name = :CUSTOMER, "
            "year"       " = :YEAR, "
            "month"      " = :MONTH, "
            "day"        " = :DAY, "
            "search_date"" = :DATE, "
            "error_flag" " = :ERROR "
        "WHERE invoice_id = :INVOICE_ID;",

    [STMT_SELECT_BY_CUSTOMER_NAME] = 
        "SELECT invoice_id, filepath, customer_name, year, month, day, search_date, error_flag "
        "FROM invoices "
        "WHERE customer_name = :CUSTOMER;",
    
    [STMT_SELECT_BY_FILEPATH] = 
        "SELECT invoice_id, filepath, customer_name, year, month, day, search_date, error_flag "
        "FROM invoices "
        "WHERE filepath = :FILEPATH;",

    [STMT_SELECT_BY_INVOICE_ID] = 
        "SELECT invoice_id, filepath, customer_name, year, month, day, search_date, error_flag "
        "FROM invoices "
        "WHERE invoice_id = :INVOICE_ID;",
};
static sqlite3_stmt *s_stmts[STMT_MAX];


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
} db_column_t;


static void log_sqlite_eror (sqlite3 *db);
static void log_sqlite_errorcode (int errcode);

static sqlite3 *open_sqlite_db (const char *dbfile, int flags);
static sqlite3 *open_sqlite_dryrun (const char *dbfile, int flags);
static int close_sqlite_db (sqlite3 *db);
static size_t prepare_sqlite_stmts (sqlite3 *db, const char **stmt_texts, 
                                    sqlite3_stmt **stmts, size_t n);
static void finalize_sqlite_stmts (sqlite3_stmt **stmts, size_t n);
static int create_initial_database_contexts (sqlite3 *db);
static int generate_date (int year, int month, int day);

static int db_execute (sqlite3 *db, sqlite3_stmt *stmt, int retry_count, void **result_ptr, void *(*callback_get_item)(sqlite3_stmt *));

static int exec_select_invoice (sqlite3 *db, sqlite3_stmt *stmt, int retry_count, db_invoice_item_t **ret_invoice);
static db_invoice_item_t *get_invoice_item (sqlite3_stmt *stmt);
static void *get_invoice_item_callback (sqlite3_stmt *stmt);


static void
log_sqlite_error (sqlite3 *db)
{
    int errcode = (!db ? SQLITE_NOMEM : sqlite3_errcode (db));
    log_sqlite_errorcode (errcode);

    return;
}

static void
log_sqlite_errorcode (int errcode)
{
    const char *errmsg = sqlite3_errstr (errcode);

    log_error ("SQLite3 Error: %d: %s\n", errcode, errmsg);

    return;
}


static sqlite3 *
open_sqlite_db (const char *dbfile, int flags)
{
    sqlite3 *db = NULL;

    /* null guard */
    if (dbfile == NULL) return NULL;

    log_verbose ("Opening database at \"%s\"\n", dbfile);

    /* open database file */
    if (sqlite3_open_v2 (dbfile, &db, flags, NULL) != SQLITE_OK)
    {
        log_sqlite_error (db);
        log_error ("Failed to open database\n");

        (void)close_sqlite_db (db);
        return NULL;
    }
    
    log_verbose ("Succeeded openning database\n");

    return db;

}


static int 
close_sqlite_db (sqlite3 *db)
{
    int retcode;

    log_verbose ("Closing database\n");

    retcode = sqlite3_close (db);

    if (retcode == SQLITE_OK)
    {
        log_verbose ("Successfully closed database\n");
    }
    else
    {
        log_sqlite_error (db);
        log_error ("Failed to close database\n");
    }

    /* sqlite return/error code */
    /* retcode == SQLITE_OK when closed succesfully */
    return retcode;
}


static size_t 
prepare_sqlite_stmts (sqlite3 *db, const char **stmt_texts, 
                      sqlite3_stmt **stmts, size_t n)
{
    int retcode = SQLITE_OK;    /* if no statements are proviced, assume OK */
    size_t i = 0;

    char *tail= NULL;
    sqlite3_stmt *stmt = NULL;

    log_verbose ("Preparing statements\n");

    for (i = 0; i < n; i++)
    {
        const char *stmt_text = stmt_texts[i];
        retcode = sqlite3_prepare_v2 (db, stmt_text, -1, &stmt, &tail);
        if (retcode != SQLITE_OK)
        {
            log_sqlite_error (db);
            log_debug ("Failed at statement:\n"
                       "  index: %zu\n"
                       "  statment: '''%s'''\n"
                       "  tail: '''%s'''\n", 
                       i, stmt_text, tail);

            finalize_sqlite_stmts (stmts, i);

            log_error ("Failed to prepare statements\n");
            break;
        }

        s_stmts[i] = stmt;
    }
    
    if (retcode == SQLITE_OK)
    {
        log_verbose ("Succeeded in preparing statements\n");
    }

    return i;
}


static void
finalize_sqlite_stmts (sqlite3_stmt **stmts, size_t n)
{
    int retcode;
    size_t i = 0;

    log_verbose ("Finalizing statements\n");

    for (i = 0; i < n; i++)
    {
        retcode = sqlite3_finalize (stmts[i]);
        stmts[i] = NULL;

        if (retcode != SQLITE_OK)
        {
            log_debug ("unexpected return, %d, while finalizing stmt %zu\n", 
                       retcode, i);
        }
    }

    log_verbose ("Successfully finalized statements\n");

    return;
}


static int
create_initial_database_contexts (sqlite3 *db)
{
    const char *stmt_create_tables = S_STMTS_TEXT[STMT_CREATE_TABLES];
    int retcode;

    log_verbose ("Creating initial database contexts\n");

    retcode = sqlite3_exec (db, stmt_create_tables, NULL, NULL, NULL);
    if (retcode == SQLITE_OK)
    {
        log_verbose ("Succeeded in creating initial database contexts\n");
    }
    else
    {
        log_sqlite_error (db);
        log_error ("Failed to create initial database contexts\n");
    }

    return retcode;
}


sqlite3 *
database_init (const char *dbfile, int dryrun)
{
    sqlite3 *db = NULL;
    const int NORMAL_FLAGS = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    const int MEMORY_FLAGS = NORMAL_FLAGS | SQLITE_OPEN_MEMORY;

    /* open the database */
    if (dryrun) db = open_sqlite_dryrun (dbfile, MEMORY_FLAGS); 
    else        db = open_sqlite_db     (dbfile, NORMAL_FLAGS);

    if (db == NULL) return NULL;

    /* create the tables (virutal only). table definitions are required to 
     * prepare many statements */
    create_initial_database_contexts (db);

    /* prepare statements */
    /* memset (s_stmts, 0, STMT_MAX * sizeof (sqlite3_stmt *)); */
    if (prepare_sqlite_stmts (db, S_STMTS_TEXT, s_stmts, STMT_MAX) != STMT_MAX)
    {
        close_sqlite_db (db);
        db = NULL;
        return NULL;
    }

    return db;
}


static sqlite3 *
open_sqlite_dryrun (const char *dbfile, int flags)
{
    sqlite3_backup *backup = NULL;
    sqlite3 *src = NULL;
    sqlite3 *dst = NULL;

    const int READONLY_FLAGS = SQLITE_OPEN_READONLY;

    /* log verbose */
    log_verbose ("Creating a RAM backup of database\n");

    /* open the dest database as in memory readwrite */
    const char *tmp_filename = tmpnam (NULL);
    if (tmp_filename == NULL) return NULL;
    dst = open_sqlite_db (tmp_filename, flags);
    if (dst == NULL) return NULL;

    /* dont try an copy any data over if dbfile is NULL */
    if (dbfile == NULL) 
    {
        log_debug ("no src database exists, using memory only\n");
        return dst;
    }

    /* open source database as readonly */
    src = open_sqlite_db (dbfile, READONLY_FLAGS);
    if (src == NULL) goto open_sqlite_dryrun_exit_failure;

    /* copy from src to dst */
    backup = sqlite3_backup_init (dst, "main", src, "main");
    if (backup == NULL) 
    {
        log_sqlite_error (dst);
        log_error ("failed to initialize the backup\n");
        goto open_sqlite_dryrun_exit_failure;
    }

    if (sqlite3_backup_step (backup, -1) != SQLITE_DONE)
    {
        log_sqlite_error (dst);
        log_error ("failed to step through backup\n");
        goto open_sqlite_dryrun_exit_failure;
    }

    /* exit */
open_sqlite_dryrun_exit:
    (void)sqlite3_backup_finish (backup); backup = NULL;
    (void)close_sqlite_db (src); src = NULL;
    return dst;

open_sqlite_dryrun_exit_failure:
    (void)close_sqlite_db (dst); dst = NULL;
    goto open_sqlite_dryrun_exit;
}


void 
database_quit (sqlite3 *db)
{
    if (db == NULL) return;

    /* finalize all prepared statements */
    finalize_sqlite_stmts (s_stmts, STMT_MAX);

    /* close the database */
    (void)close_sqlite_db (db);
    db = NULL;

    return;
}


static int
generate_date (int year, int month, int day)
{
    return ((day * 1) + (month * 100) + (year * 10000));
}


int 
database_insert_invoice (sqlite3 *db, char *filepath, char *customer_name, 
                         int year, int month, int day)
{
    int retcode = 1;

    sqlite3_stmt *stmt = s_stmts[STMT_INSERT];

    int date = generate_date (year, month, day);
    int error_flag = ((day == 0) || (month == 0) || (year == 0));

    int ret_filepath = SQLITE_BIND_NAME (stmt, ":FILEPATH", filepath);
    int ret_customer = SQLITE_BIND_NAME (stmt, ":CUSTOMER", customer_name);
    int ret_error    = SQLITE_BIND_NAME (stmt, ":ERROR", error_flag);
    int ret_year     = SQLITE_BIND_NAME_OR_NULL (stmt, ":YEAR",  year);
    int ret_month    = SQLITE_BIND_NAME_OR_NULL (stmt, ":MONTH", month);
    int ret_day      = SQLITE_BIND_NAME_OR_NULL (stmt, ":DAY",   day);
    int ret_date     = SQLITE_BIND_NAME_OR_NULL (stmt, ":DATE",  date);

    if (SQLITE_OK != (ret_filepath | ret_customer | ret_year | ret_month
                      | ret_day | ret_date | ret_error))
    {
        log_sqlite_error (db);
        log_error ("SQLite3: failed to bind value\n");
        goto database_insert_invoice_exit;
    }

    if (db_execute (db, stmt, 3, NULL, NULL) != SQLITE_DONE)
    {
        log_sqlite_error (db);
        log_error ("SQLite3: execution failed\n");
        goto database_insert_invoice_exit;
    } 

    retcode = 0;
database_insert_invoice_exit:
    (void)sqlite3_reset (stmt);
    return retcode;
}


int 
database_update_invoice (sqlite3 *db, char *filepath, char *customer_name, 
                         int year, int month, int day)
{
    int retcode = 1;

    sqlite3_stmt *stmt = s_stmts[STMT_UPDATE_BY_FILEPATH];

    int date = generate_date (year, month, day);
    int error_flag = ((day == 0) || (month == 0) || (year == 0));

    int ret_filepath = SQLITE_BIND_NAME (stmt, ":FILEPATH", filepath);
    int ret_customer = SQLITE_BIND_NAME (stmt, ":CUSTOMER", customer_name);
    int ret_error    = SQLITE_BIND_NAME (stmt, ":ERROR", error_flag);
    int ret_year     = SQLITE_BIND_NAME_OR_NULL (stmt, ":YEAR",  year);
    int ret_month    = SQLITE_BIND_NAME_OR_NULL (stmt, ":MONTH", month);
    int ret_day      = SQLITE_BIND_NAME_OR_NULL (stmt, ":DAY",   day);
    int ret_date     = SQLITE_BIND_NAME_OR_NULL (stmt, ":DATE",  date);

    if (SQLITE_OK != (ret_filepath | ret_customer | ret_year | ret_month
                      | ret_day | ret_date | ret_error))
    {
        log_sqlite_error (db);
        log_error ("SQLite3: failed to bind value\n");
        goto database_update_invoice_exit;
    }

    if (db_execute (db, stmt, 3, NULL, NULL) != SQLITE_DONE)
    {
        log_sqlite_error (db);
        log_error ("SQLite3: execution failed\n");
        goto database_update_invoice_exit;
    } 

    retcode = 0;
database_update_invoice_exit:
    (void)sqlite3_reset (stmt);
    return retcode;
}


static db_column_t
get_column (sqlite3_stmt *stmt, int i)
{
    db_column_t col;

    col.name = (char *)sqlite3_column_name (stmt, i);
    col.type = sqlite3_column_type (stmt, i);

    switch (col.type)
    {
    case SQLITE_INTEGER:
        col.m.i = sqlite3_column_int (stmt, i);
        break;
    case SQLITE_FLOAT:
        log_error ("UNIMPLEMENTED SQLITE_FLOAT");
        abort ();
        break;
    case SQLITE_BLOB:
        col.bytes = (size_t)sqlite3_column_bytes (stmt, i);
        col.m.p = (void *)sqlite3_column_blob (stmt, i);
        break;
    case SQLITE_TEXT:
        col.bytes = (size_t)sqlite3_column_bytes (stmt, i);
        col.m.s = (char *)sqlite3_column_text (stmt, i);
        break;
    case SQLITE_NULL:
        break;
    default:
        log_error ("SQLite3: Unknown column type!\n");
        break;
    }

    return col;
}


static const char *
column_type_string (int type)
{
    switch (type)
    {
    case SQLITE_INTEGER:
        return "SQLITE_INTEGER";
    case SQLITE_FLOAT:
        return "SQLITE_FLOAT";
    case SQLITE_BLOB:
        return "SQLITE_BLOB";
    case SQLITE_TEXT:
        return "SQLITE_TEXT";
    case SQLITE_NULL:
        return "SQLITE_NULL";
    }
    
    return "UNKNOWN";
}


static int
verify_column_type (db_column_t col, int *types, size_t n)
{
    if ((n == 0) || (types == NULL)) return 1;

    for (size_t i = 0; i < n; i++)
    {
        if (col.type == types[i]) return 1;
    }

    const char *real_string = column_type_string (col.type);

    char **expected_types = malloc (n * sizeof (char *));
    if (expected_types == NULL) goto verify_column_type_exit;
    for (size_t i = 0; i < n; i++)
    {
        expected_types[i] = (char *)column_type_string (types[i]);
    }
    char *expected_string = string_join (expected_types, n, " or ");
    if (expected_string == NULL) goto verify_column_type_exit_string;

    log_error ("SQLite3: Bad '%s' type, expected %s but got %s!\n", 
               col.name, expected_string, real_string);

/* verify_column_type_exit_types: */
    free (expected_types);  expected_types  = NULL;
verify_column_type_exit_string: 
    free (expected_string); expected_string = NULL;
verify_column_type_exit:
    return 0;
}


static db_invoice_item_t *
get_invoice_item (sqlite3_stmt *stmt)
{
    static db_column_t column;
    static db_invoice_item_t s;
        
    int found_invoice_id = 0;
    int found_customer_name = 0;
    int found_filepath = 0;
    int found_date = 0;
    int found_year = 0;
    int found_month = 0;
    int found_day = 0;
    int found_error_flag = 0;

    int column_count = sqlite3_column_count (stmt);

    int INTEGER[]        = { SQLITE_INTEGER, SQLITE_NULL };
    int INTEGER_STRICT[] = { SQLITE_INTEGER };
    int TEXT_STRICT[]    = { SQLITE_TEXT };

    for (int i = 0; i < column_count; i++)
    {
        column = get_column (stmt, i);

        if ((found_invoice_id) || (strcmp (column.name, "invoice_id") == 0))
        {
            found_invoice_id = 1;
            if (verify_column_type (column, INTEGER_STRICT, LEN(INTEGER_STRICT))) 
            {
                return NULL;
            }

            s.invoice_id = column.m.i;
        }
        else if ((found_filepath) || (strcmp (column.name, "filepath") == 0))
        {
            found_filepath = 1;
            if (verify_column_type (column, TEXT_STRICT, LEN(TEXT_STRICT))) 
            {
                return NULL;
            }
            
            (void)strncpy_s (s.filepath, MY_MAX_PATH, column.m.s, column.bytes);
        }
        else if ((found_customer_name) || (strcmp (column.name, "customer_name") == 0))
        {
            found_customer_name = 1;
            if (verify_column_type (column, TEXT_STRICT, LEN(TEXT_STRICT))) 
            {
                return NULL;
            }

            (void)strncpy_s (s.customer_name, MY_MAX_PATH, column.m.s, column.bytes);
        }
        else if ((found_date) || (strcmp (column.name, "date") == 0))
        {
            found_date = 1;
            if (verify_column_type (column, INTEGER, LEN(INTEGER))) 
            {
                return NULL;
            }

            s.date = (column.type == SQLITE_NULL ? 0 : column.m.i);
        }
        else if ((found_year) || (strcmp (column.name, "year") == 0))
        {
            found_year = 1;
            if (verify_column_type (column, INTEGER, LEN(INTEGER))) 
            {
                return NULL;
            }

            s.year = (column.type == SQLITE_NULL ? 0 : column.m.i);
        }
        else if ((found_month) || (strcmp (column.name, "month") == 0))
        {
            found_month = 1;
            if (verify_column_type (column, INTEGER, LEN(INTEGER))) 
            {
                return NULL;
            }

            s.month = (column.type == SQLITE_NULL ? 0 : column.m.i);
        }
        else if ((found_day) || (strcmp (column.name, "day") == 0))
        {
            found_day = 1;
            if (verify_column_type (column, INTEGER, LEN(INTEGER))) 
            {
                return NULL;
            }

            s.day = (column.type == SQLITE_NULL ? 0 : column.m.i);
        }
        else if ((found_error_flag) || (strcmp (column.name, "error_flag") == 0))
        {
            found_error_flag = 1;
            if (verify_column_type (column, INTEGER_STRICT, LEN(INTEGER_STRICT))) 
            {
                return NULL;
            }

            s.error_flag = column.m.i;    
        }
        else
        {
            log_error ("SQLite3: Unknown column name '%s'\n", column.name);
        }
    }

    return &s;
}


static void *
get_invoice_item_callback (sqlite3_stmt *stmt)
{
    return (void *)get_invoice_item (stmt);
}


static int
exec_select_invoice (sqlite3 *db, sqlite3_stmt *stmt, int retry_count, db_invoice_item_t **ret_invoice)
{
    return db_execute (db, stmt, retry_count, ret_invoice, get_invoice_item_callback);
}


static int
db_execute (sqlite3 *db, sqlite3_stmt *stmt, int retry_count, void **result_ptr, void *(*callback_get_item)(sqlite3_stmt *))
{
    int sqlite_ret;
    void *result = NULL;

    while (((sqlite_ret = sqlite3_step (stmt)) == SQLITE_BUSY) && (retry_count > 0))
    {
        log_warning ("SQLite3: cannot execute statment, database is busy. Retying...\n");
        retry_count--;
    }

    switch (sqlite_ret) 
    {
    case SQLITE_BUSY:
        log_error ("SQLite3: failed to execute statement, database is busy. Aborting!\n");
        break;
    case SQLITE_DONE:
        break;
    case SQLITE_ROW:
        if (callback_get_item) result = callback_get_item (stmt);
        break;
    case SQLITE_ERROR:
    case SQLITE_CORRUPT:
    case SQLITE_MISUSE:
    default:
        log_sqlite_error (db);
        log_debug ("SQLite3: retcode %d\n", sqlite_ret);
        log_error ("SQLite3: statement failed to execute\n");
        break;
    }

    if (result_ptr) *result_ptr = result;
    return sqlite_ret;
}


/* return true if an entry is found.
 * return false otherwise.
 * 
 * optionally, if ret_invoice is NOT NULL, the found entry is returned returned
 * through the pointer, if not entry is found, a NULL is returned. 
 * 
 * said entry is static. its contents are guaranteed only until the next call 
 * to get_invoice_item() (or any function that calls such) */
int 
database_search_by_file (sqlite3 *db, char *filepath, db_invoice_item_t **ret_invoice)
{
    int retcode = 0;
    db_invoice_item_t *result = NULL;

    sqlite3_stmt *stmt = s_stmts[STMT_SELECT_BY_FILEPATH];

   if (SQLITE_OK != SQLITE_BIND_NAME (stmt, ":FILEPATH", filepath))
   {
        log_sqlite_error (db);
        log_error ("SQLite3: failed to bind value\n");
        goto database_search_by_file_exit;
    }

    int sqlite_ret = exec_select_invoice (db, stmt, 3, &result);
    retcode = (sqlite_ret == SQLITE_ROW);

database_search_by_file_exit:
    (void)sqlite3_reset (stmt);

    if (ret_invoice) *ret_invoice = result;
    return retcode;
}


/* return true if an entry is found.
 * return false otherwise.
 * 
 * optionally, if ret_invoice is NOT NULL, the found entry is returned returned
 * through the pointer, if not entry is found, a NULL is returned. 
 * 
 * said entry is static. its contents are guaranteed only until the next call 
 * to get_invoice_item() (or any function that calls such) */
int 
database_search_by_id (sqlite3 *db, int invoice_id, db_invoice_item_t **ret_invoice)
{
    int retcode = 0;
    db_invoice_item_t *result = NULL;
    sqlite3_stmt *stmt = s_stmts[STMT_SELECT_BY_INVOICE_ID];

   if (SQLITE_OK != SQLITE_BIND_NAME (stmt, ":INVOICE_ID", invoice_id))
   {
        log_sqlite_error (db);
        log_error ("SQLite3: failed to bind value\n");
        goto database_search_by_id_exit;
    }

    int sqlite_ret = exec_select_invoice (db, stmt, 3, &result);
    retcode = (sqlite_ret == SQLITE_ROW);

database_search_by_id_exit:
    (void)sqlite3_reset (stmt);

    if (ret_invoice) *ret_invoice = result;
    return retcode;
}



/* end of file */