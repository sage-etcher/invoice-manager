
#include <assert.h>
#include "cli-interface.h"
#include <database-lib/database.h>
#include <logging-lib/logging.h>
#include <myfileio-lib/myfileio.h>
#include <mystring-lib/mystring.h>
#include "parser.h"
#include "settings.h"
#include <stdlib.h>

enum
{
    EXIT_OK,
    EXIT_ERROR,
    EXIT_FATAL,
};

static int main_init (sqlite3 **pdb);
static void main_quit (sqlite3 *db);
static int update_database (sqlite3 *db, FILE *input);

static int bad_date (int year, int month, int day);
static int update_database_with_file (sqlite3 *db, char *filepath, char *name, 
                                      int year, int month, int day);

int
main (int argc, char **argv)
{
    int exitcode = EXIT_OK;
    sqlite3 *db = NULL;

    /* initialize all modules */
    if (main_init (&db) == EXIT_FATAL) goto main_exit;

    /* load commandline options */
    (void)cli_parse_arguements (argc, argv);
    log_debug ("logging mode: %d\n",  g_set_logging_mode);
    log_debug ("cache: %s\n",         (g_set_ignore_cached ? "enabled" : "disabled"));
    log_debug ("dryrun: %s\n",        (g_set_dryrun ? "true" : "false"));
    log_debug ("database: '%s'\n",    g_set_database);
    log_debug ("badfilelog: '%s'\n",  g_set_badfilelog);

    /* iterate through each file updating the database */
    int tmp = update_database (db, stdin);
    if (tmp != EXIT_OK) exitcode = tmp;

main_exit:
    main_quit (db); 
    db = NULL;

    exit (exitcode);
}


static int  
main_init (sqlite3 **pdb)
{
    sqlite3 *db = NULL;

    assert (pdb != NULL);

    /* init default settings module */ 
    settings_load_defaults ();

    /* initialize our logging system */
    logging_init (g_set_logging_mode, g_set_badfilelog);

    /* and the local parser */ 
    if (parser_init () != 0)
    {
        log_error ("Failed to initialize parser\n");
        return EXIT_FATAL;
    }

    /* we also would like database access */
    db = db_init (g_set_database, g_set_dryrun);
    if (db == NULL)
    {
        log_error ("Failed to initialize database\n");
        return EXIT_FATAL;
    }

    /* and to return the database we get */
    if (pdb) *pdb = db;
    return EXIT_OK;
}


static void
main_quit (sqlite3 *db)
{
    db_quit (db); db = NULL;
    parser_quit ();
    logging_quit ();

    return;
}


static int
update_database (sqlite3 *db, FILE *input)
{
    int exitcode = EXIT_OK;
    char *filepath = NULL;
    parsed_t *invoice;

    while ((filepath = readline (input)))
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
            log_error ("Skipping Bad File: '%s'\n", filepath);
            log_file (filepath);
            exitcode = EXIT_ERROR;
            continue;
        }

        /* update the database */
        (void)update_database_with_file (db, filepath, invoice->name, 
                invoice->year, invoice->month, invoice->day);
    }

    return exitcode;
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
    int file_cached = db_search_by_file (db, filepath, NULL);
    int retcode;

    if ((file_cached) && (g_set_ignore_cached))
    {
        log_warning ("File already cached: '%s'\n", filepath);
        return 0;
    }

    if (file_cached) 
    {
        log_debug ("updating cached invoice: '%s'\n", filepath);
        retcode = db_update_by_file (db, filepath, name, year, month, 
                                           day);
    }
    else
    {
        log_debug ("inserting new invoice: '%s'\n", filepath);
        retcode = db_insert (db, filepath, name, year, month, 
                                           day);
    }

    if (retcode)
    {
        log_verbose ("Failed to update the database: '%s'\n", filepath);
        return 1;
    }

    return 0;
}

