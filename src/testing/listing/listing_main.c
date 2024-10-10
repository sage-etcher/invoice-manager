
#include <errno.h>
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define strdup _strdup

typedef struct
{
    char *parrent;
    char *name;
    char *updatedate;
} myfile_t;

static void free_listing (myfile_t *listing, size_t n);
static void read_listing (char *dir, myfile_t **listing_out, size_t *n_out);
static char *append_generic (char *dir);

/* invoice-core */

int
main (int argc, char *argv[])
{
    myfile_t *files = NULL;
    size_t file_count = 0;

#ifdef _DEBUG
    const char *temp[] = {
        "filename",
        "//PESERVER/Groupdata/ScannedFiles/ScannedMaterial(NEWSERVER2009)",
    };
    argv = temp;
    argc = 2;
#endif

    logging_init ();
    if ((argc < 2) || (argv == NULL))
    {
        log_error ("no directories passed\n");
        exit (EXIT_FAILURE); 
    }

    char *directory_iter = argv[1];
    errno = 0;
    read_listing (directory_iter, &files, &file_count);
    if (files == NULL)
    {
        log_error ("%s\n", strerror (errno));
        exit (EXIT_FAILURE);
    }

    for (size_t i = 0; i < file_count; i++)
    {
        log_info ("%s/%s\n", files[i].parrent, files[i].name);        
    }


    free_listing (files, file_count); 
    files = NULL; file_count = 0;

    fflush (stdout);
    fflush (stderr);

    exit (EXIT_SUCCESS);
}


static void
free_listing (myfile_t *listing, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (listing[i].name)       { free (listing[i].name); listing[i].name = NULL; }
        if (listing[i].parrent)    { free (listing[i].parrent); listing[i].parrent = NULL; }
        if (listing[i].updatedate) { free (listing[i].updatedate); listing[i].updatedate = NULL; }
    }
    free (listing); listing = NULL;
}


static char *
append_generic (char *dir)
{
    char *result = NULL;
    const char *SEARCH_APPENTION = "/*";

    if (dir == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (SIZE_MAX - sizeof (SEARCH_APPENTION) < strlen (dir))
    {
        errno = ERANGE;
        return NULL;
    }
    result = (char *)malloc (strlen (dir) + sizeof (SEARCH_APPENTION));
    if (result == NULL)
    {
        return NULL;
    }
    (void)strcpy (result, dir);
    (void)strcat (result , SEARCH_APPENTION);

    return result;
}


static void
read_listing (char *dir, myfile_t **result_out, size_t *n_out)
{
    /* error code */
    errno_t errcode = errno;

    /* overhead */
    void *tmp = NULL;
    char *search_query = NULL;
    WIN32_FIND_DATA data;
    HANDLE hFind;

    /* result array */
    myfile_t *listing = NULL;
    size_t alloc = 0, count = 0;
    const size_t DEFAULT_ALLOC = 2;


    /* null header guard */
    if ((dir == NULL) || (result_out == NULL) || (n_out == NULL))
    {
        errcode = EINVAL;
        goto read_listing_exit;
    }

    /* convert the dir into a search query */
    search_query = append_generic (dir);
    if (search_query == NULL)
    {
        errcode = errno;
        goto read_listing_exit;
    }

    /* initialize contents and associated vars */
    if (SIZE_MAX / sizeof (myfile_t) < DEFAULT_ALLOC)
    {
        errcode = ERANGE;
        goto read_listing_free_search;
    }
    listing = (myfile_t *)malloc (DEFAULT_ALLOC * sizeof(myfile_t));
    if (listing == NULL) 
    {
        errcode = errno;
        goto read_listing_free_search;
    }
    alloc = DEFAULT_ALLOC;

    /* loop throught directory */
    hFind = FindFirstFile(search_query, &data);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                (data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)    ||
                (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
                log_debug ("skipping '%s' found in '%s'\n", 
                        data.cFileName, search_query);
                continue;
            }

            log_debug ("found: '%s'\n", data.cFileName);

            /* extend contents if needed */
            while (count >= alloc)
            {
                if (SIZE_MAX / 2 / sizeof (myfile_t) < alloc )
                {
                    errcode = ERANGE;
                    goto read_listing_free_result;
                }
                tmp = realloc (listing, alloc * 2 * sizeof (myfile_t));
                if (tmp == NULL)
                {
                    errcode = errno;
                    goto read_listing_free_result;
                }
                listing = (myfile_t *)tmp;
                alloc *= 2;
            }

            /* save the file into our contents array */ 
            listing[count].name = strdup (data.cFileName);
            listing[count].parrent = strdup (dir);
            listing[count].updatedate = NULL;

            if (SIZE_MAX - 1 < count)
            {
                errcode = ERANGE;
                goto read_listing_free_result;
            }
            count++;
        }
        while (FindNextFile (hFind, &data));

        goto read_listing_close_find;

    read_listing_free_result:
        free_listing (listing, count); 
        listing = NULL; count = 0; alloc = 0;

    read_listing_close_find:
        FindClose (hFind);
    }

read_listing_free_search:
    free (search_query); search_query = NULL;
read_listing_exit:
    errno = errcode;
    if (result_out) *result_out = listing;
    if (n_out)      *n_out = count;
    return;

}

