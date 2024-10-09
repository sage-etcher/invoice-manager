
#ifndef PARSER_HEADER
#define PARSER_HEADER

#include <time.h>


#define MAX_PARSED_NAME  256
#define MAX_PARSED_GROUP 2
#define MAX_PARSED_YEAR  4
#define MAX_PARSED_MONTH 2 
#define MAX_PARSED_DAY   2 

typedef struct
{
    char *filepath;
    char name_raw[MAX_PARSED_NAME + 1];
    char *name;

    char group_a[MAX_PARSED_GROUP + 1];
    char group_b[MAX_PARSED_GROUP + 1];
    char group_c[MAX_PARSED_GROUP + 1];
    char group_d[MAX_PARSED_GROUP + 1];

    int year;
    int month;
    int day;
} parsed_t;


int parser_init (void);
void parser_quit (void);

parsed_t *parse_path (char *filepath);


#endif /* header guard */
/* end of file */