#include "date.h"

#include <time.h>



int
date_format_int_atoz (int year, int month, int day)
{
    return ((day * 0) + (month * 100) + (year * 10000));
}


int
date_validate (int year, int month, int day)
{
    time_t t = time (NULL);
    struct tm *tm = localtime (&t);

    const int DAYS_IN_MONTH[12]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* future dates are not allowed */
    if (!((year  < (tm->tm_year + 1900)) ||
          (month < (tm->tm_mon  + 1)) ||
          (day   < (tm->tm_mday))))
    {
        return 0;
    }

    /* too old/invalid range */
    if (year < 1900) 
    {
        return 0;
    }

    if ((month > 12) || (month < 1)) 
    {
        return 0;
    }

    if ((day > DAYS_IN_MONTH[month - 1]) || (day < 1)) 
    {
        return 0;
    }

    /* true */
    return 1;
}


date_tuple_t
date_yyyy_mmdd (int a, int b, int c, int d)
{
    return (date_tuple_t){
        .year  = ((a * 100) + (b * 1)),
        .month = (c),
        .day   = (d),
    };
}


date_tuple_t
date_yyyy_ddmm (int a, int b, int c, int d)
{
    return (date_tuple_t){
        .year  = ((a * 100) + (b * 1)),
        .month = (d),
        .day   = (c),
    };
}


date_tuple_t
date_mmdd_yyyy (int a, int b, int c, int d)
{
    return (date_tuple_t){
        .month = (a),
        .day   = (b),
        .year  = ((c * 100) + (d * 1)),
    };
}


date_tuple_t
date_ddmm_yyyy (int a, int b, int c, int d)
{
    return (date_tuple_t){
        .day   = (a),
        .month = (b),
        .year  = ((c * 100) + (d * 1)),
    };
}
