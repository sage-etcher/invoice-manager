
#include "extra-fileio.h"
#include "logging.h"
#include "parser.h"
//#include "database.h"
#include <stdlib.h>


int
main (int argc, char **argv)
{
    char *filepath = NULL;
    parsed_t invoice;
    //sqlite3 *db = NULL;
    
    logging_init ();
    // db = database_init (db_file);

    while ((filepath = readline (stdin)))
    {
        log_debug ("filepath: %s\n", filepath);

        /* clean up the file path */
        //trim_whitespace (filepath);

        invoice = parse_path (filepath)


        //(void)find_replace_char (invoice.name, '_', ' ');
        //trip_whitespace (invoice.name);

        //if (!validate_date (invoice.year, invoice.month, invoice.day))
        //{
        //    /* handle error */
        //}

        //db_insert_invoice (db, invoice.path, invoice.name, invoice.year, 
        //                   invoice.month, invoice.day);
    }


    //database_quit (db_file);
    logging_quit ();
    exit (EXIT_SUCCESS);
}



