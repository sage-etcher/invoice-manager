
#include <assert.h>
#include <logging-lib/logging.h>
#include <sqlite3.h>
#include <stdio.h>


int
main (void)
{
    const char *src_db_file = "src.db";
    const char *dst_db_file = "dst.db";

    const int READONLY    = SQLITE_OPEN_READONLY;
    const int READWRITE   = SQLITE_OPEN_READWRITE;
    const int FULL_ACCESS = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    const int MEMORY      = SQLITE_OPEN_READWRITE | SQLITE_OPEN_MEMORY;

    sqlite3 *src = NULL;
    sqlite3 *dst = NULL;

    sqlite3_backup *backup = NULL;

    logging_init (LOG_DEBUG, NULL);

    log_debug ("opening database handles\n");
    (void)sqlite3_open_v2 (src_db_file, &src, READONLY, NULL);
    (void)sqlite3_open_v2 (dst_db_file, &dst, MEMORY, NULL);
    //(void)sqlite3_open_v2 (src_db_file, &src, READONLY, NULL);
    //(void)sqlite3_open_v2 (dst_db_file, &dst, READWRITE, NULL);
    assert (src != NULL);
    assert (dst != NULL);

    log_debug ("initializing backup handle\n");
    backup = sqlite3_backup_init (dst, "main", src, "main");
    assert (backup != NULL);

    log_debug ("stepping through backup\n");
    (void)sqlite3_backup_step (backup, -1);

    log_debug ("finishing backup\n");
    (void)sqlite3_backup_finish (backup);
    backup = NULL;

    log_debug ("closing database handles\n");
    (void)sqlite3_close (dst); dst = NULL;
    (void)sqlite3_close (src); src = NULL;

    log_debug ("exiting\n");

    return 0;
}