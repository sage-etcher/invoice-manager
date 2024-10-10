/* HEMLOCK - a system independent package manager. */
/* <https://github.com/SoftFauna/HEMLOCK.git> */
/* Copyright (c) 2024 The SoftFauna Team */

#ifndef HEMLOCK_ARGUEMENT_HEADER
#define HEMLOCK_ARGUEMENT_HEADER
#ifdef __cplusplus  /* C++ compatibility */
extern "C" {
#endif
/* code start */

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
    CONARG_STATUS_ERROR,
    CONARG_STATUS_NA,
    CONARG_STATUS_NO_PARAM,
    CONARG_STATUS_INVALID_PARAM,
    CONARG_STATUS_VALID_PARAM,
} conarg_status_t;

typedef enum
{
    CONARG_ID_ERROR,
    CONARG_ID_UNKNOWN,
    CONARG_ID_PARAM_ERROR,
    CONARG_ID_CUSTOM,
} conarg_id_t;

typedef enum
{
    CONARG_PARAM_NONE,
    CONARG_PARAM_OPTIONAL,
    CONARG_PARAM_REQUIRED,
} conarg_param_t;

typedef struct
{
    int id;
    char *short_name;
    char *long_name;
    conarg_param_t takes_param;
} conarg_t;

typedef enum
{
    SEQ_REQUIRED,
    SEQ_OPTIONAL,
} seq_required_t;

typedef char ** sequence_t;

#define CONARG_STEP_N_UNSAFE(argc, argv, n) { \
    (argv)+=(n); \
    (argc)-=(n); \
}


#define CONARG_STEP(argc, argv) { \
    (argv)++; \
    (argc)--; \
}


int conarg_check (const conarg_t *defs, size_t n, int argc, char **argv, 
                  conarg_status_t *status_out);
char *conarg_get_param (int argc, char **argv);
bool conarg_is_flag (char *arg);
int conarg_get_sequence (sequence_t *defs, size_t n, int argc, char **argv);


/* code end */
#ifdef __cplusplus  /* C++ compatibility */
}
#endif
#endif /* header guard */
/* end of file */