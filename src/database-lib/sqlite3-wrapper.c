
#include "sqlite3-wrapper.h"

#include <logging-lib/logging.h>
#include <mystring-lib/mystring.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void
sqlwrap_log_error (sqlite3 *db)
{
    int errcode = (!db ? SQLITE_NOMEM : sqlite3_errcode (db));
    sqlwrap_log_errorcode (errcode);

    return;
}


void
sqlwrap_log_errorcode (int errcode)
{
    const char *errmsg = sqlite3_errstr (errcode);

    log_error ("SQLite3 Error: %d: %s\n", errcode, errmsg);

    return;
}


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


int 
sqlwrap_close (sqlite3 *db)
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
        sqlwrap_log_error (db);
        log_error ("Failed to close database\n");
    }

    /* sqlite return/error code */
    /* retcode == SQLITE_OK when closed succesfully */
    return retcode;
}


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


void
sqlwrap_finalize_n (sqlite3_stmt **stmts, size_t n)
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


int
sqlwrap_execute (sqlite3 *db, sqlite3_stmt *stmt, int retry_count, void **result_ptr, void *(*callback_get_item)(sqlite3_stmt *))
{
    int sqlite_ret;
    void *result = NULL;

    while (((sqlite_ret = sqlite3_step (stmt)) == SQLITE_BUSY) && (retry_count > 0))
    {
        log_warning ("SQLite2: cannot execute statment, database is busy. Retying...\n");
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


column_t
column_get (sqlite3_stmt *stmt, int i)
{
    column_t col;

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


const char *
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


int
column_match_type (column_t col, int *types, size_t n)
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