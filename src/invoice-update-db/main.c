
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
    
    time_t t = time (NULL);
    struct tm *tm = localtime (&t);

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
        if (invoice == NULL) continue;

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
            log_error ("Date out of range for file: '%s'\n", filepath);
            year  = 0;
            month = 0;
            day   = 0;
        }
        
        //log_debug ("{ '%s', '%s', '%u', '%u', '%u' },\n", filepath, 
        //        customer_name, year, month, day);


        db_insert_invoice (db, filepath, customer_name, year, month, day);
    }

main_exit_database:
    database_quit (db_file);
main_exit_parser:
    parser_quit ();
main_exit_logging:
    logging_quit ();
    exit (EXIT_SUCCESS);
}



