
#include "extra-fileio.h"
#include "extra-string.h"
#include "logging.h"
#include "parser.h"
#include "database.h"
#include <stdlib.h>
#include <time.h>


int
main (int argc, char **argv)
{
    char *filepath = NULL;
    parsed_t *invoice;
    sqlite3 *db = NULL;

    /* const char *db_file = "/var/db/invoice-manager/core.db"; */
    const char *db_file = "core.db";
    const char *badfileslog_file = "badfiles.log";
    
    time_t t = time (NULL);
    struct tm *tm = localtime (&t);

    logging_init ();
    if (parser_init () != 0)
    {
        log_error ("Failed to initialize parser\n");
        goto main_exit_logging;
    }
    //g_debug = NULL;
    //g_verbose = NULL;

    db = database_init (db_file);
    if (db == NULL)
    {
        log_error ("Failed to initialize database\n");
        goto main_exit_parser;
    }

    g_debug = NULL;

    char *customer_name = NULL;
    int year  = 0;
    int month = 0;
    int day   = 0;

    while ((filepath = readline (stdin)))
    {
        /* skip empty lines */
        filepath = trim_whitespace (filepath);
        if (is_empty (filepath)) continue;

        /* parse the line as a filepath */
        invoice = parse_path (filepath);
        if (invoice == NULL) 
        {
            log_warning ("Bad File: %s\n", filepath);
            log_file (badfileslog_file, filepath);

            continue;
        }

        /* get customer name */
        (void)find_replace_char (invoice->name, '_', ' ');
        customer_name = invoice->name;
        customer_name = trim_whitespace (customer_name);

        /* get invoice date */
        year  = atoi (invoice->year);
        month = atoi (invoice->month);
        day   = atoi (invoice->day);

        /* verify that the date is sane */
        if (!validate_date (tm, year, month, day))
        {
            log_warning ("Bad File: %s\n", filepath);
            log_file (badfileslog_file, filepath);
            
            year  = 0;
            month = 0;
            day   = 0;
        }
        
        (void)database_insert_invoice (db, filepath, customer_name, year, 
                                       month, day);

    }

main_exit_database:
    database_quit (db); 
    db = NULL;
main_exit_parser:
    parser_quit ();
main_exit_logging:
    logging_quit ();
    exit (EXIT_SUCCESS);
}


