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
        DATABASE = CONARG_ID_CUSTOM,
        FORMAT,
        SECTION,
        QUERY,
        OUTPUT,
        DEBUG,
        VERBOSE,
        TERSE,
        HELP,
        VERSION,
    };
    const conarg_t ARG_LIST[] = {

        { DATABASE, "-d", "--database", CONARG_PARAM_REQUIRED },
        { FORMAT,   "-f", "--format",   CONARG_PARAM_REQUIRED },
        { SECTION,  "-s", "--section",  CONARG_PARAM_REQUIRED },
        { QUERY,    "-q", "--query",    CONARG_PARAM_REQUIRED },
        { OUTPUT,   NULL, "--output",   CONARG_PARAM_REQUIRED },

        { DEBUG,    NULL, "--debug",    CONARG_PARAM_NONE },
        { VERBOSE,  "-v", "--verbose",  CONARG_PARAM_NONE },
        { TERSE,    "-t", "--terse",    CONARG_PARAM_NONE },

        { HELP,     "-h", "--help",     CONARG_PARAM_NONE },
        { VERSION,  "-V", "--version",  CONARG_PARAM_NONE },
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
        case DATABASE:
            CONARG_STEP (argc, argv);
            g_set_database = conarg_get_param (argc, argv);
            break;

        case FORMAT:
            CONARG_STEP (argc, argv);
            g_set_fmt_file = conarg_get_param (argc, argv);
            break;

        case SECTION:
            CONARG_STEP (argc, argv);
            g_set_secname = conarg_get_param (argc, argv);
            break;

        case QUERY:
            CONARG_STEP (argc, argv);
            g_set_sqlquery = conarg_get_param (argc, argv);
            break;

        case OUTPUT:
            CONARG_STEP (argc, argv);
            g_set_output_file = conarg_get_param (argc, argv);
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
        "write \n"
        "\n"
        "Mandatory arguements to long options are mandatory for short options too\n"
        "  -d, --database FILEPATH     use an alternative database file\n"
        "  -f, --format FILEPATH       format specification file\n"
        "  -n, --section NAME          result section's name\n"
        "  -q, --query SQLQUERY        result items search query\n"
        "      --output FILEPATH       write outputs to file instead of stdout\n"
        "  -t, --terse                 show minimal output/information\n"
        "  -v, --verbose               show more details and warnings at runtime\n"
        "      --debug                 show every last drop of information\n"
        "  -h, --help                  show this page =D\n"
        "  -V, --version               show copyright and version information\n"
        "\n"
        "Exit status:\n"
        " 0  if OK,\n"
        " 1  if error.\n"
        "\n"
        "Source: <https://github.com/sage-etcher/invoice-manager.git>\n"
        "\n"
    };

    (void)fprintf (stream, "%s", HELP_MSG);
    (void)fflush (stream);

    return;
}

/* end of file */