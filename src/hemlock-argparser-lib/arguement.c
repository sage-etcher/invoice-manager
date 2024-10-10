/* HEMLOCK - a system independent package manager. */
/* <https://github.com/SoftFauna/HEMLOCK.git> */
/* Copyright (c) 2024 The SoftFauna Team */

#include "arguement.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static bool strcmp_nullsafe (char *a, char *b);


bool
conarg_is_flag (char *arg)
{
    if (NULL == arg) 
    {
        errno = EINVAL;
        return false;
    }

    if ('-' == arg[0]) return true;

    return false;
}


static bool
strcmp_nullsafe (char *a, char *b)
{
    if ((NULL == a) || (NULL == b))
    {
        errno = EINVAL;
        return false;
    }
    
    if (0 == strcmp (a, b))
    {
        return true;
    }

    return false;
}


int
conarg_check (const conarg_t *defs, size_t n, int argc, char **argv, 
              conarg_status_t *status_out)
{
    char *arguement = NULL;
    char *parameter = NULL;
    int id = CONARG_ID_UNKNOWN;
    conarg_status_t status = CONARG_STATUS_NA;

    /* validate parameters */
    if ((NULL == defs) || (0 >= n) || (0 >= argc) || 
        (NULL == argv) || (NULL == *argv))
    {
        errno = EINVAL;
        id = CONARG_ID_ERROR;
        goto conarg_check_exit;
    }

    arguement = *argv;

    /* loop over all definitions */
    for (size_t i = 0; i < n; i++)
    {
        /* if neither short or long strings match, goto next */
        if (!strcmp_nullsafe (arguement, defs[i].short_name) &&
            !strcmp_nullsafe (arguement, defs[i].long_name))
        {
            continue;
        }

        /* if listed, check that the next value can be a parameter */
        if ((CONARG_PARAM_OPTIONAL == defs[i].takes_param) ||
            (CONARG_PARAM_REQUIRED == defs[i].takes_param))
        {
            parameter = conarg_get_param (argc - 1, argv + 1);
            if (NULL == parameter)
            {   /* not a valid parameter */
                status = CONARG_STATUS_NO_PARAM;
            }
            else if (conarg_is_flag (parameter))
            {   /* next value exsists, but is a flag */
                status = CONARG_STATUS_INVALID_PARAM;
            }
            else
            {   /* valid parameter */
                status = CONARG_STATUS_VALID_PARAM;
            }
            
            /* if param is required and the status is any such invalid case 
             * return id as invalid */
            if ((CONARG_PARAM_REQUIRED == defs[i].takes_param) &&
                (CONARG_STATUS_VALID_PARAM != status))
            {
                id = CONARG_ID_PARAM_ERROR;
                goto conarg_check_exit;
            }
        }
       
        /* other wise, return the valid ID */
        id = defs[i].id;
        goto conarg_check_exit;
    }


conarg_check_exit:
    if (NULL != status_out) *status_out = status;

    return id;
}


char *
conarg_get_param (int argc, char **argv)
{
    if ((NULL == argv) || (argc < 1) || (NULL == *argv))
    {
        errno = EINVAL;
        return NULL;
    }

    return *argv;
}


int 
conarg_get_sequence (sequence_t *defs, size_t n, int argc, char **argv)
{
    char *temp = NULL;
    size_t i = 0;

    /* dont deref null */
    if (NULL == defs) goto sequence_exit;

    /* for each item try to get it */
    for (; i < n; i++, defs++)
    {
        /* don deref null */
        if (NULL == *defs) goto sequence_exit;

        /* grab and validate the arugement */
        temp = conarg_get_param (argc, argv);
        if ((NULL == temp) || (conarg_is_flag (temp))) goto sequence_exit;

        /* save the arg into output */
        **defs = temp;
    
        /* update iterators to next value */
        CONARG_STEP (argc, argv);
    }

sequence_exit:
    /* size_t to int errno guard */
    if (INT_MAX < i)
    {
        errno = ERANGE;
    }

    return (int)i;
}

/* end of file */