#ifndef DATE_HEADER
#define DATE_HEADER

typedef struct 
{
    int year;
    int month;
    int day;
} date_tuple_t;


int date_format_int_atoz (int year, int month, int day);
int date_validate (int year, int month, int day);

date_tuple_t date_yyyy_mmdd (int a, int b, int c, int d);
date_tuple_t date_yyyy_ddmm (int a, int b, int c, int d);
date_tuple_t date_ddmm_yyyy (int a, int b, int c, int d);
date_tuple_t date_mmdd_yyyy (int a, int b, int c, int d);



#endif /* header guard */