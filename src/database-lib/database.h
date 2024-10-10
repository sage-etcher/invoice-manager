
#ifndef INVOICE_DATABASE_HEADER
#define INVOICE_DATABASE_HEADER

#include <sqlite3.h>


#define MY_MAX_PATH 255
typedef struct
{
    int invoice_id;
    char filepath[MY_MAX_PATH + 1];
    char customer_name[MY_MAX_PATH + 1];
    int date;
    int year;
    int month;
    int day;
    int error_flag;
} db_invoice_item_t;


sqlite3 *database_init (const char *dbfile);
void database_quit (sqlite3 *db);

int database_insert_invoice (sqlite3 *db, char *filepath, char *customer_name, 
                             int year, int month, int day);

int database_search_by_file (sqlite3 *db, char *filepath, db_invoice_item_t **ret_invoice);

#endif