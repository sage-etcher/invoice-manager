/**
 * @file
 * @brief  A wrapper to extend the SQLite3 library.
 * @author 
 * @version
 * @copyright MIT
 * @sa <a href="https://">Wikipedia:Copyrights - Wikipedia</a>
 */


#include "sqlite3-wrapper.h"

#include <logging-lib/logging.h>
#include <mystring-lib/mystring.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/** 
 * Log an SQLite error via an open database.
 * 
 * Get the errorcode from the provided database handle then use 
 * sqlwrap_log_errorcode() to log the error.
 * 
 * If a NULL pointer is provided, assume an implicit SQLITE_NOMEM errorcode. 
 * 
 * @param db an open sqlite database or NULL.  
 * 
 * @sa sqlwrap_log_errocode()
 */
void
sqlwrap_log_error (sqlite3 *db)
{
    int errcode = (!db ? SQLITE_NOMEM : sqlite3_errcode (db));
    sqlwrap_log_errorcode (errcode);

    return;
}


/**
 * Log an SQLite error via error code.
 * 
 * Get the error message from sqlite3_errstr(), then log the errorcode and 
 * message using log_error().
 * 
 * @param errcode a valid sqlite error code.
 
 * @sa log_error()
 */
void
sqlwrap_log_errorcode (int errcode)
{
    const char *errmsg = sqlite3_errstr (errcode);

    log_error ("SQLite3 Error: %d: %s\n", errcode, errmsg);

    return;
}


/**
 * Open a sqlite3 database.
 * 
 * A wrapper around sqlite3_open_v2() for opening a database with logging.
 * When passed a NULL pointer, it is a is a harmless no-op.
 * 
 * @param dbfile a filename to use for the database.
 * @param flags a OR'd list of SQLITE_OPEN_* flags to use.
 * 
 * @return the opened database on a success or NULL on a failure.
 */
sqlite3 *
sqlwrap_open (const char *dbfile, int flags)
{
    sqlite3 *db = NULL;

    /* null guard */
    if (dbfile == NULL) return NULL;

    log_verbose ("Opening database at \"%s\"\n", dbfile);

    /* open database file */
    if (sqlite3_open_v2 (dbfile, &db, flags, NULL) != SQLITE_OK)
    {
        sqlwrap_log_error (db);
        log_error ("Failed to open database\n");

        (void)sqlwrap_close (db);
        return NULL;
    }
    
    log_verbose ("Succeeded openning database\n");

    return db;

}


/**
 * Open a sqlite3 database as a temporary backup.
 *
 * Create a backup of the database file using flags as the backup's open 
 * permissions. Intended for use with in-memory databases.
 * 
 * @param dbfile a database filename.
 * @param flags an OR'd list of SQLITE_OPEN_* flags to use for the backup.
 *
 * @return the opened backup on a success or NULL on a failure.
 * 
 * @note this feels, klunky, like it should be refactored into a more consice 
 *       function.
 */
sqlite3 *
sqlwrap_open_memory (const char *dbfile, int flags)
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
    dst = sqlwrap_open (tmp_filename, flags);
    if (dst == NULL) return NULL;

    /* dont try an copy any data over if dbfile is NULL */
    if (dbfile == NULL) 
    {
        log_debug ("no src database exists, using memory only\n");
        return dst;
    }

    /* open source database as readonly */
    src = sqlwrap_open (dbfile, READONLY_FLAGS);
    if (src == NULL) goto open_sqlite_dryrun_exit_failure;

    /* copy from src to dst */
    backup = sqlite3_backup_init (dst, "main", src, "main");
    if (backup == NULL) 
    {
        sqlwrap_log_error (dst);
        log_error ("failed to initialize the backup\n");
        goto open_sqlite_dryrun_exit_failure;
    }

    if (sqlite3_backup_step (backup, -1) != SQLITE_DONE)
    {
        sqlwrap_log_error (dst);
        log_error ("failed to step through backup\n");
        goto open_sqlite_dryrun_exit_failure;
    }

    /* exit */
open_sqlite_dryrun_exit:
    (void)sqlite3_backup_finish (backup); backup = NULL;
    (void)sqlwrap_close (src); src = NULL;
    return dst;

open_sqlite_dryrun_exit_failure:
    (void)sqlwrap_close (dst); dst = NULL;
    goto open_sqlite_dryrun_exit;
}


/** 
 * Close a sqlite3 database. 
 * 
 * A wrapper around sqlite3_close() for closing a database with logging.
 * 
 * @param db an open database handle.
 * @return a dirrect pass-through from sqlite3_close().
*/
int 
sqlwrap_close (sqlite3 *db)
{
    int retcode;

    /** @bug logs even on NULL */

    log_verbose ("Closing database\n");

    retcode = sqlite3_close (db);

    if (retcode == SQLITE_OK)
    {
        log_verbose ("Successfully closed database\n");
    }
    else
    {
        sqlwrap_log_error (db);
        log_error ("Failed to close database\n");
    }

    /* sqlite return/error code */
    /* retcode == SQLITE_OK when closed succesfully */
    return retcode;
}


/** Prepare an array of SQL statments. 
 * 
 * Iterate through the stmt_texts array, prepare each statement, write the 
 * returned statement into the stmts array, and log each step of the way.
 * 
 * If an error occurs, all prepared statements are destroyed and set to NULL.
 * 
 * @param db an open database.
 * @param stmt_texts an array of raw SQL statement strings.
 * @param stmts the result array of prepared statements.
 * @param n the array lengths.
 * 
 * @return the number of iterations accomplished. Is equal to n on a success.
 * 
 * @sa sqlwrap_finalize_n()
 */
size_t 
sqlwrap_prepare_n (sqlite3 *db, const char **stmt_texts, 
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
            sqlwrap_log_error (db);
            log_debug ("Failed at statement:\n"
                       "  index: %zu\n"
                       "  statment: '''%s'''\n"
                       "  tail: '''%s'''\n", 
                       i, stmt_text, tail);

            sqlwrap_finalize_n (stmts, i);

            log_error ("Failed to prepare statements\n");
            break;
        }

        stmts[i] = stmt;
    }
    
    if (retcode == SQLITE_OK)
    {
        log_verbose ("Succeeded in preparing statements\n");
    }

    return i;
}


/**
 * Finalize an array of SQL statements.
 * 
 * Iterate through the stmts array, finalize each element, set the element 
 * to NULL, and log each step of the way.
 * 
 * @param stmts an array of prepared SQL statements.
 * @param n the array length.
 */
void
sqlwrap_finalize_n (sqlite3_stmt **stmts, size_t n)
{
    int retcode;
    size_t i = 0;

    log_verbose ("Finalizing statements\n");

    for (i = 0; i < n; i++)
    {
        /** @bug memory leak if sqlite3_finalize fails. */
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


/** Execute (step through) a prepared SQL Statement.
 * 
 * Execute a prepared SQL Statement, logging as it does. If SQLITE_ROW is 
 * returned execute the provided callback and save the results in result_ptr.
 * If the statement fails, retry up until retry_count times.
 * 
 * while result_ptr is NULL the callback's output is discarded.
 * while callback_get_item is NULL all statement results are discarded. 
 * 
 * Behavior is undefined while db or stmt is NULL or unprepared. 
 * 
 * Logging relies on logging-lib being initiallized. if uninitialized no 
 * information/warnings/errors will be logged.
 *
 * @param db the sqlite3 database handle associated with the statement.
 * @param stmt the prepared statement to evaluate.
 * @param retry_count if execution fails, retry n times.
 * @param result_ptr a result pointer for the callback functions output.
 * @param callback_get_item if the statement returns data, proccess the data
 *                          within the provided callback funtion.
 * 
 * @return the result from sqlite3_step.
 * 
 * @see sqlwrap_open()
 * @see sqlwrap_open_memory()
 * @see sqlwrap_prepare_n()
 * @see logging_init()
 */
int
sqlwrap_execute (sqlite3 *db, sqlite3_stmt *stmt, int retry_count, 
                 void **result_ptr, void *(*callback_get_item)(sqlite3_stmt *))
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
        log_error ("SQLite2: failed to execute statement, database is busy. Aborting!\n");
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
        sqlwrap_log_error (db);
        log_debug ("SQLite2: retcode %d\n", sqlite_ret);
        log_error ("SQLite2: statement failed to execute\n");
        break;
    }

    if (result_ptr) *result_ptr = result;
    return sqlite_ret;
}


/** The column_t constructor.
 * 
 * Construct a column_t based on a prepared statement and index. prior to 
 * calling stmt should be stepped, confirmed to be returning data, and 
 * confirmed that the index is within range of said data.
 * 
 * The returned column_t is only valid until the next step of stmt.
 * 
 * Logging relies on logging-lib being initiallized. if uninitialized no 
 * information/warnings/errors will be logged.
 *
 * @param stmt an executing sqlite3_stmt.
 * @param i a column index.
 * 
 * @return a column_t for the given statement and index.
 * 
 * @see column_t
 * @see column_type_t
 * @see column_type()
 * @see column_match_type()
 * @see sqlwrap_execute()
 * @see logging_init()
 */
column_t
column_get (sqlite3_stmt *stmt, int i)
{
    column_t col;

    col.name = (char *)sqlite3_column_name (stmt, i);
    col.type = (column_type_t)sqlite3_column_type (stmt, i);

    switch (col.type)
    {
    case COLUMN_INTEGER:
        col.m.i = sqlite3_column_int (stmt, i);
        break;
    case COLUMN_FLOAT:
        log_error ("UNIMPLEMENTED SQLITE_FLOAT");
        abort ();
        break;
    case COLUMN_BLOB:
        col.bytes = (size_t)sqlite3_column_bytes (stmt, i);
        col.m.p = (void *)sqlite3_column_blob (stmt, i);
        break;
    case COLUMN_TEXT:
        col.bytes = (size_t)sqlite3_column_bytes (stmt, i);
        col.m.s = (char *)sqlite3_column_text (stmt, i);
        break;
    case COLUMN_NULL:
        break;
    default:
        log_error ("SQLite3: Unknown column type!\n");
        break;
    }

    return col;
}


/** Convert column type to string.
 * 
 * Convert a column type to string and return it as a constant string. primary 
 * intended for error logging and debugging. if the provided type does not 
 * match any known typings, return "UNKNOWN".
 * 
 * The returned string is readonly and does not require freeing.
 * 
 * @param type a column_type_t type
 * 
 * @return a constant string representation of type
 * 
 * @see column_t
 * @see column_type_t
 * @see column_get()
 * @see column_match_type()
 */
const char *
column_type_string (int type)
{
    switch (type)
    {
    case COLUMN_INTEGER:
        return "SQLITE_INTEGER";
    case COLUMN_FLOAT:
        return "SQLITE_FLOAT";
    case COLUMN_BLOB:
        return "SQLITE_BLOB";
    case COLUMN_TEXT:
        return "SQLITE_TEXT";
    case COLUMN_NULL:
        return "SQLITE_NULL";
    }
    
    return "UNKNOWN";
}


/** Validate the typing of a column_t.
 *
 * Validate that a column_t object's type matches with an array of valid 
 * typings. If no match is found throw a runtime error message listing both 
 * the expected typings and the provided type.
 * 
 * if types is a NULL pointer or if n is 0, return a success.
 * 
 * Logging relies on logging-lib being initiallized. if uninitialized no 
 * information/warnings/errors will be logged.
 * 
 * @param col a columnt_t to verify
 * @param types an array of valid column_type_t types
 * @param n the array length
 * 
 * @return on a success returns 1, on a failure returns 0.
 * 
 * @see column_t
 * @see column_type_t
 * @see column_get()
 * @see column_type_string()
 * @see sqlwrap_execute()
 * @see logging_init()
 */
int
column_match_type (column_t col, column_type_t *types, size_t n)
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


/* end of file */