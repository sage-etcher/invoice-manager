
#ifndef DATABASE_HEADER
#define DATABASE_HEADER

#include "logging.h"
#include <sqlite3.h>


sqlite3 *database_init (const char *dbfile);
void database_quit (sqlite3 *db);

int database_insert_invoice (sqlite3 *db, char *filepath, char *customer_name, 
                             int year, int month, int day);


#endif