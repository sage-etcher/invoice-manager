
#include "database.h"

#include <date-lib/date.h>
#include <logging-lib/logging.h>
#include <mystring-lib/mystring.h>
#include <sqlite3.h>
#include "sqlite3-wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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


static int create_tables (sqlite3 *db);

static invoice_t *select_invoice_callback (sqlite3_stmt *stmt);
static void      *select_invoice_wrapper  (sqlite3_stmt *stmt);
static int        select_invoice (sqlite3 *db, sqlite3_stmt *stmt, int retry_count, invoice_t **ret_invoice);


sqlite3 *
db_init (const char *dbfile, int dryrun)
{
    sqlite3 *db = NULL;
    const int NORMAL_FLAGS = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    const int MEMORY_FLAGS = NORMAL_FLAGS | SQLITE_OPEN_MEMORY;

    /* open the database */
    if (dryrun) db = sqlwrap_open_memory (dbfile, MEMORY_FLAGS); 
    else        db = sqlwrap_open        (dbfile, NORMAL_FLAGS);

    if (db == NULL) return NULL;

    /* create the tables (virutal only). table definitions are required to 
     * prepare many statements */
    create_tables (db);

    /* prepare statements */
    /* memset (s_stmts, 0, STMT_MAX * sizeof (sqlite3_stmt *)); */
    if (sqlwrap_prepare_n (db, S_STMTS_TEXT, s_stmts, STMT_MAX) != STMT_MAX)
    {
        sqlwrap_close (db);
        db = NULL;
        return NULL;
    }

    return db;
}


void 
db_quit (sqlite3 *db)
{
    if (db == NULL) return;

    /* finalize all prepared statements */
    sqlwrap_finalize_n (s_stmts, STMT_MAX);

    /* close the database */
    (void)sqlwrap_close (db);
    db = NULL;

    return;
}


static int
create_tables (sqlite3 *db)
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
        sqlwrap_log_error (db);
        log_error ("Failed to create initial database contexts\n");
    }

    return retcode;
}


int 
db_insert (sqlite3 *db, char *filepath, char *customer_name, 
           int year, int month, int day)
{
    int retcode = 1;

    sqlite3_stmt *stmt = s_stmts[STMT_INSERT];

    int date = date_format_int_atoz (year, month, day);
    int error_flag = ((day == 0) || (month == 0) || (year == 0));

#pragma warning( push )
#pragma warning( disable : 4047 4024)
    int ret_filepath = SQLWRAP_BIND_NAME (stmt, ":FILEPATH", filepath);
    int ret_customer = SQLWRAP_BIND_NAME (stmt, ":CUSTOMER", customer_name);
    int ret_error    = SQLWRAP_BIND_NAME (stmt, ":ERROR", error_flag);
    int ret_year     = SQLWRAP_BIND_NAME_OR_NULL (stmt, ":YEAR",  year);
    int ret_month    = SQLWRAP_BIND_NAME_OR_NULL (stmt, ":MONTH", month);
    int ret_day      = SQLWRAP_BIND_NAME_OR_NULL (stmt, ":DAY",   day);
    int ret_date     = SQLWRAP_BIND_NAME_OR_NULL (stmt, ":DATE",  date);
#pragma warning( pop )

    if (SQLITE_OK != (ret_filepath | ret_customer | ret_year | ret_month
                      | ret_day | ret_date | ret_error))
    {
        sqlwrap_log_error (db);
        log_error ("SQLite3: failed to bind value\n");
        goto database_insert_invoice_exit;
    }

    if (sqlwrap_execute (db, stmt, 3, NULL, NULL) != SQLITE_DONE)
    {
        sqlwrap_log_error (db);
        log_error ("SQLite3: execution failed\n");
        goto database_insert_invoice_exit;
    } 

    retcode = 0;
database_insert_invoice_exit:
    (void)sqlite3_reset (stmt);
    return retcode;
}


int 
db_update_by_file (sqlite3 *db, char *filepath, char *customer_name, 
                       int year, int month, int day)
{
    int retcode = 1;

    sqlite3_stmt *stmt = s_stmts[STMT_UPDATE_BY_FILEPATH];

    int date = date_format_int_atoz (year, month, day);
    int error_flag = ((day == 0) || (month == 0) || (year == 0));

#pragma warning( push )
#pragma warning( disable : 4047 4024)
    int ret_filepath = SQLWRAP_BIND_NAME (stmt, ":FILEPATH", filepath);
    int ret_customer = SQLWRAP_BIND_NAME (stmt, ":CUSTOMER", customer_name);
    int ret_error    = SQLWRAP_BIND_NAME (stmt, ":ERROR", error_flag);
    int ret_year     = SQLWRAP_BIND_NAME_OR_NULL (stmt, ":YEAR",  year);
    int ret_month    = SQLWRAP_BIND_NAME_OR_NULL (stmt, ":MONTH", month);
    int ret_day      = SQLWRAP_BIND_NAME_OR_NULL (stmt, ":DAY",   day);
    int ret_date     = SQLWRAP_BIND_NAME_OR_NULL (stmt, ":DATE",  date);
#pragma warning( pop )

    if (SQLITE_OK != (ret_filepath | ret_customer | ret_year | ret_month
                      | ret_day | ret_date | ret_error))
    {
        sqlwrap_log_error (db);
        log_error ("SQLite3: failed to bind value\n");
        goto database_update_invoice_exit;
    }

    if (sqlwrap_execute (db, stmt, 3, NULL, NULL) != SQLITE_DONE)
    {
        sqlwrap_log_error (db);
        log_error ("SQLite3: execution failed\n");
        goto database_update_invoice_exit;
    } 

    retcode = 0;
database_update_invoice_exit:
    (void)sqlite3_reset (stmt);
    return retcode;
}


/* return true if an entry is found.
 * return false otherwise.
 * 
 * optionally, if ret_invoice is NOT NULL, the found entry is returned returned
 * through the pointer, if not entry is found, a NULL is returned. 
 * 
 * said entry is static. its contents are guaranteed only until the next call 
 * to select_invoice_callback() (or any function that calls such) */
int 
db_search_by_file (sqlite3 *db, char *filepath, invoice_t **ret_invoice)
{
    int retcode = 0;
    invoice_t *result = NULL;

    sqlite3_stmt *stmt = s_stmts[STMT_SELECT_BY_FILEPATH];

#pragma warning( push )
#pragma warning( disable : 4047 4024)
   if (SQLITE_OK != SQLWRAP_BIND_NAME (stmt, ":FILEPATH", filepath))
   {
        sqlwrap_log_error (db);
        log_error ("SQLite3: failed to bind value\n");
        goto database_search_by_file_exit;
    }
#pragma warning( pop)

    int sqlite_ret = select_invoice (db, stmt, 3, &result);
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
 * to select_invoice_callback() (or any function that calls such) */
int 
db_search_by_id (sqlite3 *db, int invoice_id, invoice_t **ret_invoice)
{
    int retcode = 0;
    invoice_t *result = NULL;
    sqlite3_stmt *stmt = s_stmts[STMT_SELECT_BY_INVOICE_ID];

#pragma warning( push )
#pragma warning( disable : 4047 4024)
   if (SQLITE_OK != SQLWRAP_BIND_NAME (stmt, ":INVOICE_ID", invoice_id))
   {
        sqlwrap_log_error (db);
        log_error ("SQLite3: failed to bind value\n");
        goto database_search_by_id_exit;
    }
#pragma warning( pop)

    int sqlite_ret = select_invoice (db, stmt, 3, &result);
    retcode = (sqlite_ret == SQLITE_ROW);

database_search_by_id_exit:
    (void)sqlite3_reset (stmt);

    if (ret_invoice) *ret_invoice = result;
    return retcode;
}


static invoice_t *
select_invoice_callback (sqlite3_stmt *stmt)
{
    static column_t column;
    static invoice_t s;
        
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
        column = column_get (stmt, i);

        if ((found_invoice_id) || (strcmp (column.name, "invoice_id") == 0))
        {
            found_invoice_id = 1;
            if (column_match_type (column, INTEGER_STRICT, LEN(INTEGER_STRICT))) 
            {
                return NULL;
            }

            s.invoice_id = column.m.i;
        }
        else if ((found_filepath) || (strcmp (column.name, "filepath") == 0))
        {
            found_filepath = 1;
            if (column_match_type (column, TEXT_STRICT, LEN(TEXT_STRICT))) 
            {
                return NULL;
            }
            
            (void)strncpy_s (s.filepath, MY_MAX_PATH, column.m.s, column.bytes);
        }
        else if ((found_customer_name) || (strcmp (column.name, "customer_name") == 0))
        {
            found_customer_name = 1;
            if (column_match_type (column, TEXT_STRICT, LEN(TEXT_STRICT))) 
            {
                return NULL;
            }

            (void)strncpy_s (s.customer_name, MY_MAX_PATH, column.m.s, column.bytes);
        }
        else if ((found_date) || (strcmp (column.name, "date") == 0))
        {
            found_date = 1;
            if (column_match_type (column, INTEGER, LEN(INTEGER))) 
            {
                return NULL;
            }

            s.date = (column.type == SQLITE_NULL ? 0 : column.m.i);
        }
        else if ((found_year) || (strcmp (column.name, "year") == 0))
        {
            found_year = 1;
            if (column_match_type (column, INTEGER, LEN(INTEGER))) 
            {
                return NULL;
            }

            s.year = (column.type == SQLITE_NULL ? 0 : column.m.i);
        }
        else if ((found_month) || (strcmp (column.name, "month") == 0))
        {
            found_month = 1;
            if (column_match_type (column, INTEGER, LEN(INTEGER))) 
            {
                return NULL;
            }

            s.month = (column.type == SQLITE_NULL ? 0 : column.m.i);
        }
        else if ((found_day) || (strcmp (column.name, "day") == 0))
        {
            found_day = 1;
            if (column_match_type (column, INTEGER, LEN(INTEGER))) 
            {
                return NULL;
            }

            s.day = (column.type == SQLITE_NULL ? 0 : column.m.i);
        }
        else if ((found_error_flag) || (strcmp (column.name, "error_flag") == 0))
        {
            found_error_flag = 1;
            if (column_match_type (column, INTEGER_STRICT, LEN(INTEGER_STRICT))) 
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
select_invoice_wrapper (sqlite3_stmt *stmt)
{
    return (void *)select_invoice_callback (stmt);
}


static int
select_invoice (sqlite3 *db, sqlite3_stmt *stmt, int retry_count, 
                invoice_t **ret_invoice)
{
    return sqlwrap_execute (db, stmt, retry_count, ret_invoice, select_invoice_wrapper);
}


/* end of file */