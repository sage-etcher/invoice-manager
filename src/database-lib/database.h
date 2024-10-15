
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
} invoice_t;


sqlite3 *db_init (const char *dbfile, int dryrun);
void     db_quit (sqlite3 *db);

int db_insert (sqlite3 *db, char *filepath, char *customer_name, int year, int month, int day);

int db_update_by_file (sqlite3 *db, char *filepath, char *customer_name, int year, int month, int day);

int db_search_by_file (sqlite3 *db, char *filepath, invoice_t **ret_invoice);
int db_search_by_id (sqlite3 *db, int id, invoice_t **ret_invoice);

#endif