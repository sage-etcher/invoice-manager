
#ifndef EXTRA_STRING_HEADER
#define EXTRA_STRING_HEADER


char *find_replace_char (char *src, char find, char replace);

char *trim_preceeding (char *src);
char *trim_trailing (char *src);
char *trim_whitespace (char *src);

int is_empty (char *src);

#endif /* header guard */
/* end of file */