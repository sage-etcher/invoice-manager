
#include "cli-interface.h"
#include <database-lib/database.h>
#include <logging-lib/logging.h>
#include <myfileio-lib/myfileio.h>
#include <mystring-lib/mystring.h>
#include "parser.h"
#include "settings.h"
#include <stdlib.h>


static int bad_date (int year, int month, int day);
static int update_database_with_file (sqlite3 *db, char *filepath, char *name, 
                                      int year, int month, int day);


int
main (int argc, char **argv)
{
    char *filepath = NULL;
    parsed_t *invoice;
    sqlite3 *db = NULL;

    settings_load_defaults ();
    (void)cli_parse_arguements (argc, argv);

    logging_init (g_set_logging_mode, g_set_badfilelog);

    if (parser_init () != 0)
    {
        log_error ("Failed to initialize parser\n");
        goto main_exit_logging;
    }

    db = database_init (g_set_database);
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

        /* if there is an issue with the parse */
        if ((invoice == NULL) ||
            (bad_date (invoice->year, invoice->month, invoice->day)))
        {
            log_warning ("Skipping Bad File: '%s'\n", filepath);
            log_file (filepath);
            continue;
        }

        /* update the database */
        (void)update_database_with_file (db, filepath, invoice->name, 
                invoice->year, invoice->month, invoice->day);
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


static int
bad_date (int year, int month, int day)
{
    return ((year  == 0) || 
            (month == 0) || 
            (day   == 0));
}


static int
update_database_with_file (sqlite3 *db, char *filepath, char *name, int year, 
                           int month, int day)
{
    int file_cached = database_search_by_file (db, filepath, NULL);
    int retcode;

    if ((file_cached) && (g_set_ignore_cached))
    {
        log_warning ("File already cached: '%s'\n", filepath);
        return 0;
    }

    if (file_cached) 
    {
        retcode = database_update_invoice (db, filepath, name, year, month, 
                                           day);
    }
    else
    {
        retcode = database_insert_invoice (db, filepath, name, year, month, 
                                           day);
    }

    if (retcode)
    {
        log_verbose ("Failed to update the database: '%s'\n", filepath);
        return 1;
    }

    return 0;
}

