#include "database.h"

#include "logging.h"
#include <sqlite3.h>
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
    STMT_SELECT_BY_CUSTOMER_NAME,
    STMT_SELECT_BY_FILEPATH,
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
            "error INTEGER NOT NULL"
        ");",

    [STMT_INSERT] =
        "INSERT INTO invoices ("
            "filepath, customer_name, year, month, day, search_date, error"
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

    [STMT_SELECT_BY_CUSTOMER_NAME] = 
        "SELECT filepath, customer_name, year, month, day, search_date, error "
        "FROM invoices "
        "WHERE customer_name = :CUSTOMER;",
    
    [STMT_SELECT_BY_FILEPATH] = 
        "SELECT filepath, customer_name, year, month, day, search_date, error "
        "FROM invoices "
        "WHERE filepath = :FILEPATH;",
};
static sqlite3_stmt *s_stmts[STMT_MAX];


static void log_sqlite_eror (sqlite3 *db);
static sqlite3 *open_sqlite_db (const char *dbfile);
static int close_sqlite_db (sqlite3 *db);
static size_t prepare_sqlite_stmts (sqlite3 *db, const char **stmt_texts, 
                                    sqlite3_stmt **stmts, size_t n);
static void finalize_sqlite_stmts (sqlite3_stmt **stmts, size_t n);
static int create_initial_database_contexts (sqlite3 *db);
static int generate_date (int year, int month, int day);


static void
log_sqlite_error (sqlite3 *db)
{
    int errcode = (!db ? SQLITE_NOMEM : sqlite3_errcode (db));
    const char *errmsg = sqlite3_errstr (errcode);

    log_error ("SQLite3 Error: %d: %s\n", errcode, errmsg);

    return;
}


static sqlite3 *
open_sqlite_db (const char *dbfile)
{
    sqlite3 *db = NULL;

    log_verbose ("Opening database at \"%s\"\n", dbfile);

    /* open database file */
    if (sqlite3_open (dbfile, &db) != SQLITE_OK)
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
    int retcode;
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
database_init (const char *dbfile)
{
    sqlite3 *db = NULL;

    /* null guard */
    if (dbfile == NULL) return NULL;

    /* open the database */
    db = open_sqlite_db (dbfile);
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

    if (sqlite3_step (stmt) != SQLITE_DONE)
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




/* end of file */