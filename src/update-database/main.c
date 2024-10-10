
#include <database-lib/database.h>
#include <logging-lib/logging.h>
#include <myfileio-lib/myfileio.h>
#include <mystring-lib/mystring.h>
#include "parser.h"
#include <stdlib.h>


int
main (int argc, char **argv)
{
    char *filepath = NULL;
    parsed_t *invoice;
    sqlite3 *db = NULL;

    const char *db_file = "core.db";
    const char *badfileslog_file = "badfiles.log";
    
    logging_init ();
    if (parser_init () != 0)
    {
        log_error ("Failed to initialize parser\n");
        goto main_exit_logging;
    }

    db = database_init (db_file);
    if (db == NULL)
    {
        log_error ("Failed to initialize database\n");
        goto main_exit_parser;
    }

    while ((filepath = readline (stdin)))
    {
        if (filepath == NULL) continue;

        /* skip empty lines */
        filepath = trim_whitespace (filepath);
        if (is_empty (filepath)) continue;

        /* parse the line as a filepath */
        invoice = parse_path (filepath);
        if (invoice == NULL) 
        {
            log_warning ("Skipping Bad File: '%s'\n", filepath);
            log_file (badfileslog_file, filepath);
            continue;
        }

        /* skip file if it already exists in database */
        if (database_search_by_file (db, filepath, NULL)) 
        {
            log_verbose ("File already cached: '%s'\n", filepath);
            continue;
        }

        if ((invoice->day == 0) || (invoice->month == 0) || (invoice->year == 0))
        {
            log_warning ("Bad Date in file: '%s'\n", filepath);
            log_file (badfileslog_file, filepath);
        }

        /* add new file into the database */
        if (!database_insert_invoice (db, filepath, invoice->name, 
                    invoice->year, invoice->month, invoice->day))
        {
            log_error ("Failed to insert file: '%s'\n", filepath);
        }
    }


/* main_exit_database: */
    database_quit (db); 
    db = NULL;
main_exit_parser:
    parser_quit ();
main_exit_logging:
    logging_quit ();
    exit (EXIT_SUCCESS);
}


