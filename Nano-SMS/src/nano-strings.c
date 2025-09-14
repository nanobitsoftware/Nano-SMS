/* This is an old library of mine wwhere I re-wrote or optimized specific string
   related functions. They may or may not be safe. But it is a left over that I
   bring to programs now and then for ease of use, and easy debugging for memory
   issues.

    This is not a complete library, and is not meant to be. It is simply a collection
    of string related functions that I use in my projects. It is not meant to be a
    complete replacement for the standard string library, but rather a collection of
    functions that I find useful and that I have written myself.

    Everything in this comment written below are from past me. Take it with a grain of salt.

    ***MOTE*** Some things are new, as this is an evolving library. I will try to keep
    *  the documentation up to date as I make changes.

Most code is in an alpha stage. These tags will be updated as the code matures.
These tags will also be added with information regarding the
specific file that theyre in a the time.

 Code description:
 I tend to we-write all my own string commnads over the gnu / glib
 library....for obvious reasons. But I also tend to have some of my own special
 library needs so I Just amalgamate them all in to strings.c I use this in all my projects
 so it's nothing to write home about, but it does make for some fun ways to fi erors in your
 program that relies on glib.

 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <uxtheme.h>
#include <commctrl.h>
#include <stdarg.h>
#include "stdbool.h"
#include "sqlite3/sqlite3.h"


#include "nano-sms.h"


char str_empty[1];
char *string_space;
char *top_string;
#undef strdup

#define MAX_BUF 512
#define MAX_CHAR_COL 74

int str_isnumber ( const char *s );

#define is_number(x) str_isnumber(x)
#define is_number2(x) str_isnumber(x)



double fround ( double f )
{
    f *= 100;
    f += 0.5;
    f = ( float )(( int )f);
    f /= 100;

    return f;
}


// Going to call nano_malloc directly here for not-so-obvious reasons.
// This function is rolled, locally, for our memory manager.

char *str_dup1 ( const char *str, char *file, int line )
{
    char *result;

    if ( !str )
        return NULL;

    result = ( char * )nano_malloc ( (( int )strlen ( str ) + 10) * sizeof ( char * ), file, line );
    //	result  = (char*)malloc(((int) strlen(str) + 10) * sizeof(char*));
    if ( result == ( char * )0 )
    {
        free ( result );

        return ( char * )0;
    }
    strcpy ( result, str );
    return result;
}

/* Personal string search. Only searches for one char, not a whole string. */
BOOL str_search ( const char *str )
{
    const char *point = str;

    BOOL efound = FALSE;
    BOOL mfound = FALSE;
    if ( str == NULL )
        return FALSE;
    for ( ;; point++ )
    {
        if ( *point == '\033' )
            efound = TRUE;
        if ( *point == 'm' )
            mfound = TRUE;
        if ( !*point )
            break;
    }
    if ( efound && !mfound )
        return FALSE;
    else
        return TRUE;
}
int str_ret ( const char *str )
{
    const char *point = str;
    int i;
    for ( i = 0; *point; point++, i++ )
    {
        if ( *point == '\033' )
            return i;
    }
    return 0;
}
void strip_junk ( char *str )
{
    const char *point;
    char fleh[8000] = "";

    char *buf;

    buf = fleh;

    if ( str == NULL )
        return;
    point = str;
    for ( point = str; *point; point++ )
    {
        if ( *point != '\033' )
        {
            *buf = *point;
            *++buf = '\0';
        }
    }
    *++buf = '\0';
    str = buf;
    return;
}

BOOL strprefix1 ( const char *astr, const char *bstr )
{
    if ( astr == NULL )
        return TRUE;
    if ( bstr == NULL )
        return TRUE;

    for ( ; (*astr || *bstr) && *bstr != '\0'; astr++, bstr++ )
    {
        if ( LOWER ( *bstr ) != LOWER ( *astr ) )
            return TRUE;
    }

    return FALSE;
}

char *one_argument ( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace ( *argument ) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *arg_first = LOWER ( *argument );
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while ( isspace ( *argument ) )
        argument++;

    return argument;
}
char *script_strip ( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace ( *argument ) )
        argument++;

    cEnd = '{';
    if ( *argument == '{' || *argument == '}' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd || *argument == '}' )
        {
            argument++;
            break;
        }

        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while ( isspace ( *argument ) )
        argument++;
    while ( *argument == '{' || *argument == '}' )
        argument++;

    return argument;
}

BOOL simple_str_match ( char *input, char *pattern )
{
    char *inputPtr = input;
    char *whereInput = input;
    char *wherePattern = pattern;

    for ( ; inputPtr != '\0'; inputPtr++ )
    {
        whereInput = inputPtr;
        wherePattern = pattern;

        if ( *whereInput == *wherePattern )
        {
            for ( ; wherePattern != '\0'; wherePattern++ )
            {
                whereInput++;

                if ( *whereInput != *wherePattern )
                {
                    break;
                }
            }

            return TRUE;
        }
    }

    return FALSE;
}

char *commaize ( unsigned long long int x, char buf[] ) // Turn a ULONG_INT into a comma string.
// IE: 300000 = 300,000. Probably not the
// most efficent way to do it, but ah well
{
    static char *to_ret = NULL;
    char temp[1024] = "";
    char *point = temp;
    int i, c, f = 0;
    buf[0] = '\0';

    temp[0] = '\0';
    sprintf ( temp, "%llu", x ); // Yes yes, it's a hack.

    to_ret = temp;

    if ( x <= 999 )
    {
        buf[0] = '\0';
        sprintf ( &buf[0], "%llu", x );

        return &buf[0];
    }
    else if ( x > 999 )
    {
        point += ( int )strlen ( temp );

        if ( !*--point )
            return to_ret;
        i = ( int )( int )strlen ( temp ) - 1;
        c = i / 3; // How many commas. :)
        i += c;
        temp[i + c + 1] = '\0';

        for ( ; *point; point--, i-- )
        {
            if ( i < 0 )
            {
                buf[0] = '\0';
                strcat ( buf, temp );

                return to_ret;
            }
            if ( f == 3 )
            {
                f = 0;
                temp[i] = ',';
                *++point;

                continue;
            }

            temp[i] = *point;
            f++;
        }

        to_ret = temp;
        buf[0] = '\0';
        strcat ( buf, temp );

        return &buf[0];
    }

    return &buf[0];
}

BOOL string_compare ( const char *ostr, const char *tstr )
{
    if ( ostr == NULL || tstr == NULL )
    {
        return TRUE;
    }
    for ( ; *ostr || *tstr; ostr++, tstr++ )
    {
        if ( LOWER ( *ostr ) != LOWER ( *tstr ) )

            return TRUE;
    }
    return FALSE;
}

char *get_date ( void )
{
    time_t t;
    struct tm *t_m;

    static char d[1024];
    t = time ( NULL );

    t_m = localtime ( &t );

    sprintf ( d, "%2.2d/%2.2d/%d %2.2d:%2.2d%s", t_m->tm_mon + 1, t_m->tm_mday, t_m->tm_year + 1900, t_m->tm_hour == 0 ? 12 : (t_m->tm_hour) > 12 ? (t_m->tm_hour) - 12 : (t_m->tm_hour), t_m->tm_min,
              (t_m->tm_hour) > 12 ? "pm" : "am" );

    return d;
}

char *get_date_notime ( void )
{
    time_t t;
    struct tm *t_m;

    static char d[1024];
    t = time ( NULL );

    t_m = localtime ( &t );

    sprintf ( d, "%2.2d/%2.2d/%d", t_m->tm_mon + 1, t_m->tm_mday, t_m->tm_year + 1900 );

    return d;
}
/*
 * Truncate line to MAX_CHAR_COL, wrapping at
 * last space found. */

char *make_to_col ( char *str )
{
    char *tmp_str;
    unsigned long int i;
    int i_space;
    int x;
    char *point;
    char *last_space;

    if ( !str )
        return NULL;

    if ( ( int )strlen ( str ) <= MAX_CHAR_COL )
        return str;

    i = ( int )strlen ( str );

    i = (i + (i / 2));

    if ( i < 1 )
        i = 2;
    if ( i > ( int )strlen ( str ) )
        i = (( int )strlen ( str ) * 2);
    tmp_str = malloc ( sizeof ( char * ) * i );

    last_space = NULL;
    i = 0;
    x = 0;

    for ( point = str; *point; point++ )
    {
        if ( *point == ' ' )
        {
            last_space = point;
            i_space = i;
        }

        if ( *point == '\n' )
            x = 0;

        if ( x >= MAX_CHAR_COL )
        {
            if ( last_space == NULL )
            {
                // break at the line anyways.
                tmp_str[i] = '\n';
                tmp_str[i + 1] = '\0';
                i = i + 1;
                x = 0;
                point--;

                last_space = NULL;
            }
            else
            {
                tmp_str[i_space] = '\n';
                tmp_str[i_space + 1] = '\0';
                i = i_space + 1;
                x = 0;

                point = last_space;
                last_space = NULL;
            }
            continue;
        }
        tmp_str[i] = *point;
        tmp_str[i + 1] = '\0';
        i++;
        x++;
        if ( *point == '\b' || *point == '\033' || *point == '\032' )
        {
            x--;

            continue;
        }
    }

    return tmp_str;
}

char *make_to_page ( char *str )
{
    char *tmp_str;
    unsigned long int i;
    int i_space;
    int x;
    char *point;
    char *last_space;

    if ( !str )
        return NULL;

    if ( ( int )strlen ( str ) <= 90 )
        return str;

    i = ( int )strlen ( str );
    i = (i + (i / 2));

    if ( i < 1 )
        i = 2;
    if ( i > ( int )strlen ( str ) )
        i = (( int )strlen ( str ) * 2);
    tmp_str = malloc ( sizeof ( char * ) * i );

    last_space = NULL;
    i = 0;
    x = 0;

    for ( point = str; *point; point++ )
    {
        if ( *point == ' ' )
        {
            last_space = point;
            i_space = i;
        }

        if ( *point == '\n' )
            x = 0;

        if ( x >= 100 )
        {
            if ( last_space == NULL )
            {
                // break at the line anyways.
                tmp_str[i] = '\n';
                tmp_str[i + 1] = '\0';
                i = i + 1;
                x = 0;
                point--;

                last_space = NULL;
            }
            else
            {
                tmp_str[i_space] = '\n';
                tmp_str[i_space + 1] = '\0';
                i = i_space + 1;
                x = 0;

                point = last_space;
                last_space = NULL;
            }
            continue;
        }
        tmp_str[i] = *point;
        tmp_str[i + 1] = '\0';
        i++;
        x++;
        if ( *point == '\b' || *point == '\033' || *point == '\032' )
        {
            x--;

            continue;
        }
    }

    return tmp_str;
}

int count_lines ( char *str )
{
    char *point;
    int len;

    if ( !str )
        return 0;

    len = 0;

    for ( point = str; *point; point++ )
    {
        if ( *point == '\n' )
            len++;
    }
    return len == 0 ? 1 : (len + 1);
}

BOOL is_date ( char *range )
{
    char str[1024];
    char delim[] = "/";
    char *token;
    BOOL correct_date;

    correct_date = TRUE;

    if ( !range )
        return FALSE;

    sprintf ( str, "%s", range );
    // Lots of sanity checking. Mine or the code? Who knows.

    token = strtok ( str, delim );

    if ( (token) && !is_number ( token ) )
        correct_date = FALSE;

    if ( !token )
        correct_date = FALSE;

    if ( (token) && ( int )strlen ( token ) > 2 || ( int )strlen ( token ) < 2 )
        correct_date = FALSE;

    if ( atoi ( token ) > 12 )
        correct_date = FALSE;

    token = strtok ( NULL, delim );
    if ( !token )
        correct_date = FALSE;
    if ( !(token) && is_number2 ( token ) )
        correct_date = FALSE;

    if ( (token) && ( int )strlen ( token ) > 2 || ( int )strlen ( token ) < 2 )
        correct_date = FALSE;

    if ( atoi ( token ) > 31 )
        correct_date = FALSE;

    token = strtok ( NULL, delim );
    if ( !token )
        correct_date = FALSE;
    if ( (token) && !is_number2 ( token ) )
        correct_date = FALSE;

    if ( (token) && ( int )strlen ( token ) > 4 || ( int )strlen ( token ) < 4 )
        correct_date = FALSE;
    if ( atoi ( token ) > 2500 || atoi ( token ) < 1950 )
        correct_date = FALSE;

    return correct_date;
}






void write_buffer ( const char *str )
{
    FILE *fp;

    if ( str == NULL )
        return;

    if ( (fp = fopen ( DEBUG_FILE, "a" )) == NULL )
    {
        GiveError ( "Unable to open debug file. Please restart with debugging support off.", FALSE );
        return;
    }

    fprintf ( fp, "%s\n", str );
    fclose ( fp );
}



int read_string ( char buf[], FILE *fp )
{

    int c;
    int len = 0;
    int f;
    if ( fp == NULL )
        return -1;
    len = 0;
    f = 0;
    while ( !feof ( fp ) )
    {
        f++;




        c = getc ( fp );



        if ( c == '\0' )
            return len;
        if ( c == '\n' )
            return len;

        if ( c == '\r' )
            return len;
        buf[len++] = c;
        //              
        //		if (f >= 100)
        //return len;        



    }
    return EOF;
}
// Physically trims a string. Left, right, or both, for the following three functions.
// AI written.
int str_lefttrim ( char *str, int len )
{
    int i, j;
    if ( len <= 0 || str == NULL )
        return 0;
    for ( i = 0; i < len && isspace ( str[i] ); i++ );
    if ( i == 0 )
        return len;
    for ( j = 0; j < len - i; j++ )
        str[j] = str[j + i];
    str[j] = '\0';
    return len - i;
}
int str_righttrim ( char *str, int len )
{
    int i;
    if ( len <= 0 || str == NULL )
        return 0;
    for ( i = len - 1; i >= 0 && isspace ( str[i] ); i-- );
    str[i + 1] = '\0';
    return i + 1;
}
int str_trim ( char *str, int len )
{
    len = str_righttrim ( str, len );
    len = str_lefttrim ( str, len );
    return len;
}
// End AI code

int str_upper ( char *str )
{
    if ( str == NULL )
        return 0;
    if ( str[0] >= 'a' && str[0] <= 'z' )
        str[0] = str[0] - ('a' - 'A');
    return 1;
}
int str_lower ( char *str )
{
    if ( str == NULL )
        return 0;
    if ( str[0] >= 'A' && str[0] <= 'Z' )
        str[0] = str[0] + ('a' - 'A');
    return 1;
}
int str_nocase_cmp ( const char *s1, const char *s2 )
{
    int c1, c2;
    if ( s1 == NULL || s2 == NULL )
        return -1;
    do
    {
        c1 = *s1++;
        c2 = *s2++;
        if ( c1 >= 'a' && c1 <= 'z' )
            c1 -= ('a' - 'A');
        if ( c2 >= 'a' && c2 <= 'z' )
            c2 -= ('a' - 'A');
        if ( c1 == '\0' )
            return c1 - c2;
    }
    while ( c1 == c2 );
    return c1 - c2;
}
int str_nocase_ncmp ( const char *s1, const char *s2, size_t n )
{
    int c1, c2;
    if ( s1 == NULL || s2 == NULL || n == 0 )
        return -1;
    do
    {
        c1 = *s1++;
        c2 = *s2++;
        if ( c1 >= 'a' && c1 <= 'z' )
            c1 -= ('a' - 'A');
        if ( c2 >= 'a' && c2 <= 'z' )
            c2 -= ('a' - 'A');
        if ( c1 == '\0' || --n == 0 )
            return c1 - c2;
    }
    while ( c1 == c2 );
    return c1 - c2;
}
char *str_nocase_str ( const char *haystack, const char *needle )
{
    size_t needle_len;
    if ( haystack == NULL || needle == NULL )
        return NULL;
    needle_len = strlen ( needle );
    if ( needle_len == 0 )
        return ( char * )haystack;
    while ( *haystack )
    {
        if ( str_nocase_ncmp ( haystack, needle, needle_len ) == 0 )
            return ( char * )haystack;
        haystack++;
    }
    return NULL;
}
char *str_nocase_chr ( const char *s, int c )
{
    if ( s == NULL )
        return NULL;
    if ( c >= 'a' && c <= 'z' )
        c -= ('a' - 'A');
    while ( *s )
    {
        int sc = *s;
        if ( sc >= 'a' && sc <= 'z' )
            sc -= ('a' - 'A');
        if ( sc == c )
            return ( char * )s;
        s++;
    }
    if ( c == '\0' )
        return ( char * )s;
    return NULL;
}
size_t str_nocase_spn ( const char *s, const char *accept )
{
    size_t count = 0;
    if ( s == NULL || accept == NULL )
        return 0;
    while ( *s )
    {
        if ( str_nocase_chr ( accept, *s ) == NULL )
            return count;
        count++;
        s++;
    }
    return count;
}
size_t str_nocase_cspn ( const char *s, const char *reject )
{
    size_t count = 0;
    if ( s == NULL || reject == NULL )
        return 0;
    while ( *s )
    {
        if ( str_nocase_chr ( reject, *s ) != NULL )
            return count;
        count++;
        s++;
    }
    return count;
}
char *str_nocase_dup ( const char *s )
{
    char *dup;
    size_t len;
    if ( s == NULL )
        return NULL;
    len = strlen ( s );
    dup = ( char * )malloc ( len + 1 );
    if ( dup == NULL )
        return NULL;
    memcpy ( dup, s, len + 1 );
    return dup;
}


char *str_rev ( char *s )
{
    char *start, *end, tmp;
    if ( s == NULL )
        return NULL;
    start = s;
    end = s + strlen ( s ) - 1;
    while ( start < end )
    {
        tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
    return s;
}

int str_isnumber ( const char *s )
{
    if ( s == NULL || *s == '\0' )
        return 0;
    if ( *s == '-' || *s == '+' )
        s++;
    if ( *s == '\0' )
        return 0;
    while ( *s )
    {
        if ( *s < '0' || *s > '9' )
            return 0;
        s++;
    }
    return 1;
}

int is_number_decimal ( const char *s )
{
    int seen_decimal = 0;
    if ( s == NULL || *s == '\0' )
        return 0;
    if ( *s == '-' || *s == '+' )
        s++;
    if ( *s == '\0' )
        return 0;
    while ( *s )
    {
        if ( *s < '0' || *s > '9' || *s == '.' ) // Added check for decimal. But also make sure only one.
        {
            if ( *s == '.' && !seen_decimal )
            {
                seen_decimal = 1;
            }
            else
            {
                return 0;
            }
        }
        return 0;
        s++;
    }
    return 1;
}