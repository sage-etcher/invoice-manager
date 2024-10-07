#include "database.h"

#include "logging.h"
#include <sqlite3.h>


sqlite3 *
database_init (const char *dbfile)
{
    /* open database file */

    /* compile statements */

    /* create tables if not exist */
}


void 
database_quit (sqlite3 *db)
{
    if (db == NULL) return;

    /* destroy statements */

    /* close database */

}


int 
database_insert_invoice (sqlite3 *db, char *filepath, char *customer_name, 
                         int year, int month, int day)
{

}


/* end of file */