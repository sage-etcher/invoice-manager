#include "cli-interface.h"

#include "config.h"
#include <errno.h>
#include <hemlock-argparser-lib/arguement.h>
#include <logging-lib/logging.h>
#include <mystring-lib/mystring.h>
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>


static void help_page (FILE *stream);
static void version_page (FILE *stream);


void
cli_parse_arguements (int argc, char **argv)
{
    const enum 
    {
        DISABLE_CACHE = CONARG_ID_CUSTOM,
        ENABLE_CACHE,
        DRYRUN,
        DATABASE,
        BADFILELOG,
        DEBUG,
        VERBOSE,
        TERSE,
        HELP,
        VERSION,
    };
    const conarg_t ARG_LIST[] = {

        { DATABASE,      "-d", "--database",    CONARG_PARAM_REQUIRED },
        { BADFILELOG,    "-l", "--badfilelog",  CONARG_PARAM_REQUIRED },
        
        { DISABLE_CACHE, NULL, "--disable-cache", CONARG_PARAM_NONE },
        { ENABLE_CACHE,  NULL, "--enable-cache",  CONARG_PARAM_NONE },
        { DRYRUN,        NULL, "--dryrun",        CONARG_PARAM_NONE },

        { DEBUG,         NULL, "--debug",       CONARG_PARAM_NONE },
        { VERBOSE,       "-v", "--verbose",     CONARG_PARAM_NONE },
        { TERSE,         "-t", "--terse",       CONARG_PARAM_NONE },

        { HELP,          "-h", "--help",        CONARG_PARAM_NONE },
        { VERSION,       "-V", "--version",     CONARG_PARAM_NONE },
    };
    const size_t ARG_COUNT = LEN (ARG_LIST);
    
    int id;
    conarg_status_t param_stat;

    CONARG_STEP (argc, argv);
    while (argc > 0)
    {
        param_stat = CONARG_STATUS_NA;
        id = conarg_check (ARG_LIST, ARG_COUNT, argc, argv, &param_stat);

        switch (id)
        {
        case ENABLE_CACHE:
            g_set_ignore_cached = 1;
            break;

        case DISABLE_CACHE:
            g_set_ignore_cached = 0;
            break;

        case BADFILELOG:
            CONARG_STEP (argc, argv);
            g_set_badfilelog = conarg_get_param (argc, argv);
            break;

        case DATABASE:
            CONARG_STEP (argc, argv);
            g_set_database = conarg_get_param (argc, argv);
            break;

        case DRYRUN:
            g_set_dryrun = 1;
            break;

        case DEBUG:
            g_set_logging_mode = LOG_DEBUG; 
            break;

        case VERBOSE:
            g_set_logging_mode = LOG_VERBOSE;
            break;

        case TERSE:
            g_set_logging_mode = LOG_TERSE;
            break;

        case HELP:
            help_page (stdout);
            exit (EXIT_SUCCESS);
        
        case VERSION:
            version_page (stdout);
            exit (EXIT_SUCCESS);

        /* error states */
        case CONARG_ID_UNKNOWN:
        case CONARG_ID_PARAM_ERROR:
        default:
            help_page (stderr);
            exit (EXIT_FAILURE);
        }

        CONARG_STEP (argc, argv);
    }

    return;
}


static void
version_page (FILE *stream)
{
    const char *VERSION_MSG = {
        PROJECT_NAME " (" CMAKE_PROJECT_NAME ") " PROJECT_VERSION "\n"
        "Copyright (c) " COPYRIGHT_YEAR " Sage I. Hendricks\n"
        "License MIT: The MIT license, <https://spdx.org/licenses/MIT.html>\n"
        "This is free software: you are free to change and redistribute it.\n"
        "These is NO WARRANTY, to the extent permitted by law.\n"
    };
    (void)fprintf (stream, "%s", VERSION_MSG);
    (void)fflush (stream);

    return;
}


static void
help_page (FILE *stream)
{
    const char *HELP_MSG = {
        "Usage: " PROJECT_NAME "[OPTIONS...]\n"
        "Analize a list of filenames piped from stdin, uploading each into a database\n"
        "\n"
        "Mandatory arguements to long options are mandatory for short options too\n"
        "  -d, --database FILEPATH     use an alternative database file\n"
        "  -l, --badfilelog FILEPATH   output bad files to an alternate file\n"
        "      --dryrun                dont update the database\n"
        "      --enable-cache          skip files already cached in the database\n"
        "      --disable-cache         update all files, ignoring weather they are\n"
        "                                cached or not (verry slow)\n"
        "  -t, --terse                 show minimal output/information\n"
        "  -v, --verbose               show more details and warnings at runtime\n"
        "      --debug                 show every last drop of information\n"
        "  -h, --help                  show this page =D\n"
        "  -V, --version               show copyright and version information\n"
        "\n"
        "Exit status:\n"
        " 0  if OK,\n"
        " 1  if minor error (such as a bad file),\n"
        " 2  if fatal error (such as a failure to initialize),\n"
        "\n"
        "Source: <https://github.com/sage-etcher/invoice-manager.git>\n"
        "\n"
    };

    (void)fprintf (stream, "%s", HELP_MSG);
    (void)fflush (stream);

    return;
}

/* end of file */