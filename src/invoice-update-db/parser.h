
#ifndef PARSER_HEADER
#define PARSER_HEADER

#include <time.h>


#define MAX_PARSED_NAME  256
#define MAX_PARSED_YEAR  4
#define MAX_PARSED_MONTH 2 
#define MAX_PARSED_DAY   2 
typedef struct
{
    char *filepath;
    char name[MAX_PARSED_NAME + 1];
    char year[MAX_PARSED_YEAR + 1];
    char month[MAX_PARSED_MONTH + 1];
    char day[MAX_PARSED_DAY + 1];
} parsed_t;


int parser_init (void);
void parser_quit (void);

parsed_t *parse_path (char *filepath);
int validate_date (struct tm *tm, int year, int month, int day);


#endif /* header guard */
/* end of file */