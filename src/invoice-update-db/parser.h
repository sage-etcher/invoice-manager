
#ifndef PARSER_HEADER
#define PARSER_HEADER

typedef struct
{
    char *filepath;
    char *name;
    char year[4];
    char month[2];
    char day[2];
} parsed_t;


parsed_t parse_path (char *filepath);


#endif /* header guard */
/* end of file */