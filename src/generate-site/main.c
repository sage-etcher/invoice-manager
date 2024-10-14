
#include "cli-interface.h"
#include <database-lib/database.h>
#include <logging-lib/logging.h>
#include "settings.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>


int
main (int argc, char **argv)
{
    sqlite3 *db = NULL;
    FILE *output = NULL;

    /* load options passed by commandline */
    settings_load_defaults ();
    (void)cli_parse_arguements (argc, argv);

    /* initialize the logging system */
    logging_init (g_set_logging_mode, NULL);
    /* set the output file stream */
    if (g_set_output_file) 
    {
        output = fopen (g_set_output_file, "a+");
        if (output == NULL)
        {
            log_error ("cannot open output file: '%s'\n", g_set_output_file);
            return 1;
        }

        g_info = output;
    }

    /* log current settings */
    log_debug ("logging mode: %d\n",  g_set_logging_mode);
    log_debug ("database: '%s'\n",    g_set_database);
    log_debug ("format file: %s\n",   g_set_fmt_file);
    log_debug ("section: %s\n",       g_set_secname);
    log_debug ("query: '%s'\n",       g_set_sqlquery);
    log_debug ("output file: '%s'\n", g_set_output_file);

    /* open the database in memory (database dryrun mode) */
    db = database_init (g_set_database, 1);
    if (db == NULL)
    {
        log_error ("Failed to initialze database\n");
        goto main_exit_output;
    }

    


/* main_exit_database: */
    database_quit (db); db = NULL;
main_exit_output:
    (void)fclose (output); output = NULL;
    return 0;
}