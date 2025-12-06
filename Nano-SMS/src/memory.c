/* This was written for use in all my software. Over the years it has been
   tweaked and changed for my needs. Some of it may not be used at all here
   but I include it as a form of library only. Everything written below
   is from past me.
   */

/*Copyright(c) 2016 Michael Hayes (Bioteq - Nanobit Software)
* All rights reserved.
*
* Redistribution and use in source and binary forms are permitted
* provided that the above copyright notice and this paragraph are
* duplicated in all such forms and that any documentation,
* advertising materials, and other materials related to such
* distribution and use acknowledge that the software was developed
* by (http://biomud.nanobit.net) Nanobit Software.The name of
* Nanobit Software may not be used to endorse or promote products derived
* from this software without specific prior written permission.
* THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <sys/types.h>
#include <time.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <uxtheme.h>
#include <commctrl.h>
#include "stdbool.h"
#include "sqlite3/sqlite3.h"

#include "nano-sms.h"
#include "nano-io.h"

#undef malloc
#undef free
#undef realloc

unsigned long int full_mem = 0;
unsigned long int malloc_calls = 0;
unsigned long int free_calls = 0;
unsigned long long int total_alloc;
const unsigned char uninit = 0x0;
typedef struct mem_heap HEAP;

#ifdef BIT_ALIGN_MALLOC
#define ALIGN BIT_ALIGNMENT_AMOUNT_MALLOC
#endif

// We allow an error to form from the above defines. That is because
// if you changed the bit alignment toggle in the header file then you
// intend to use your own alignment, so you must supply your OWN #define ALIGN value.
// Sorry.

/*
#define ALIGN 8 // Using our own alignment value because we don't want to use built in.
*/

unsigned long int alloced = 0;
unsigned long int unalloced = 0;

#define MALLOC_MAGIC 0xF00F

struct mem_heap
{
    uintptr_t m_add;
    size_t size;
    char  file[ 100 ];
    int    line;
    HEAP *next;
    HEAP *prev;
    void *chunk;
};

HEAP *firstheap;
HEAP *heaplist;
HEAP *freeheap;
HEAP *lastheap;

BOOL ERRORS;
typedef long double MEM_TYPE;

// Thread safety: use a critical section for all heap operations
static CRITICAL_SECTION heap_cs;
static BOOL heap_cs_initialized = FALSE;

// Helper to initialize critical section once
static void init_heap_cs( void )
{
    if ( !heap_cs_initialized )
    {
        InitializeCriticalSection( &heap_cs );
        heap_cs_initialized = TRUE;
    }
}

#define MULTITHREADED 1
// Helper to enter/leave critical section
#ifdef MULTITHREADED
#define HEAP_LOCK()   do { init_heap_cs(); EnterCriticalSection(&heap_cs); } while(0)
#define HEAP_UNLOCK() LeaveCriticalSection(&heap_cs)
#else
#define HEAP_LOCK()
#define HEAP_UNLOCK()
#endif

HEAP *new_heap( void )
{
    static HEAP  h;
    HEAP *ph;
    if ( !IS_IN_DEBUGGING_MODE )
    {
        return NULL;
    }

    HEAP_LOCK( );
    if ( freeheap == NULL )
    {
        ph = ( HEAP * )malloc( sizeof( *ph ) );
    }
    else
    {
        ph = freeheap;
        freeheap = freeheap->next;
    }
    *ph = h;
    total_alloc += ( sizeof( *ph ) );
    HEAP_UNLOCK( );

    return ph;
}

void add_heap( HEAP *hp )
{
    char temp[ 5000 ];
    int b_type = 0;
    float t_total = 0.0;

    temp[ 0 ] = '\0';
    b_type = 0;

    if ( !IS_IN_DEBUGGING_MODE )
    {
        return;
    }
    HEAP_LOCK( );
    printf( "%d\n", REPORT_ALLOCATION );
    if ( REPORT_ALLOCATION )
    {
        t_total = ( float )total_alloc;

        if ( t_total > 1024 )
        {
            b_type = 1; // KB
            t_total = t_total / 1024;
        }
        if ( t_total > 1024 )
        {
            b_type = 2; // MB
            t_total = t_total / 1024;
        }
        if ( t_total > 1024 )
        {
            b_type = 3; // GB
            t_total = t_total / 1024;
        }

        LOG( "--------------------------------------------------------------------------------------------------" );
        LOG( "Allocation:" );
        LOG( "Calling file: %s", hp->file );
        LOG( "Calling line: %d", hp->line );
        LOG( "Calling size: %lu", hp->size );
        LOG( "Total allocation: %5.5f %s", t_total, b_type == 0 ? "bytes" : b_type == 1 ? "kilobytes" : b_type == 2 ? "megabytes" : "gigabytes" );
        LOG( "Calling address (returned): 0X%X", hp->m_add );
        LOG( "--------------------------------------------------------------------------------------------------\r\n" );
    }
    hp->next = NULL;

    if ( lastheap != NULL )
    {
        lastheap->next = hp;
    }

    hp->prev = lastheap;

    lastheap = hp;

    if ( firstheap == NULL )
    {
        firstheap = hp;
    }
    alloced += 1;
    HEAP_UNLOCK( );
    return;
}
/*

void show_heap ( void )
{
    unsigned long int i = 0;
    unsigned long int size;
    unsigned long int s_size;
    char s[100];
    char color[100];
    HEAP *hp;
    HEAP *last;

    char dheap[1000];
    char str[20];
    color[0] = '\0';

    if ( !IS_IN_DEBUGGING_MODE )
    {
        give_term_error ( "Program is not in Debug mode. Heap dump is not permitted.\r\n" );
    }

    str[0] = '\0';

    dheap[0] = '\0';
    HEAP_LOCK ();
    last = lastheap;

    for ( i = 0, hp = firstheap; hp != last; hp = hp->next, i++ )
    {
        if ( hp->chunk == NULL )
        {
            size = 0;
        }
        else
        {
            hp->chunk = ( char * )hp->chunk - sizeof ( MEM_TYPE );
            memset ( s, '\0', 100 );
            memcpy ( s, ( char * )hp->chunk, sizeof ( MEM_TYPE ) );
            size = atoi ( s );
            s_size = size;
        }

        if ( size > 1024 )
        {
            size = size / 1024;
            sprintf ( str, " KB" );
            sprintf ( color, "%s", ANSI_GREEN );
        }
        if ( size > 1024 )
        {
            size = size / 1024;
            sprintf ( str, " MB" );
            sprintf ( color, "%s", ANSI_RED );
        }
        if ( size > 1024 )
        {
            size = size / 1024;
            sprintf ( str, "GB" );
            sprintf ( color, "%s", ANSI_YELLOW );
        }

        if ( i % 20000 == 0 )
        {
            nasty = FALSE;
            update_term ();
            do_peek ();
        }

        sprintf ( dheap, "%s%lu) Address: 0x%lx, File: %s, Line: %d Size(bit-stored): %ld%s", color, i, hp->m_add == 0 ? 0 : hp->m_add, hp->file == NULL ? "Undefined" : hp->file, hp->line, size, str[0] == '\0' ? " B" : str );
        color[0] = '\0';
        realize_lines ( dheap );
        dheap[0] = '\0';
        str[0] = '\0';
    }
    update_term ();
    HEAP_UNLOCK ();
}
*/

void walk_heap( void )
{
    unsigned long int i = 0;
    size_t  count = 0;
    HEAP *hp;

    if ( !IS_IN_DEBUGGING_MODE )
    {
        return;
    }
    HEAP_LOCK( );
    for ( i = 0, hp = firstheap; hp; hp = hp->next, i++ )
    {
        LOG( "Walkheap: %d) m_add: 0x%x, file: %s, line: %d", i, hp->m_add == 0 ? 0 : hp->m_add, hp->file == NULL ? "Undefined" : hp->file, hp->line );
        count += hp->size;
    }
    LOG( "Walkheap: Total size unfreed: %d bytes", ( unsigned long )count );
    LOG( "Walkheap: Allocations called: %d. Deallocations called: %d, total: %d. (This number SHOULD be zero. \n\tIf not, then we got some problems.\n", alloced, unalloced, alloced - unalloced );
    HEAP_UNLOCK( );
}

void dump_heap( void )
{
    unsigned long int i = 0;
    size_t count = 0;
    unsigned long int total = 0;
    char buf[ 5000 ];

    HEAP *hp;

    if ( !IS_IN_DEBUGGING_MODE )
    {
        return;
    }
    buf[ 0 ] = '\0';
    HEAP_LOCK( );
    for ( i = 0, hp = firstheap; hp; hp = hp->next, i++ )
    {
        count += hp->size;
        total++;
    }

    LOG( "Dumpheap: %d total allocations managed. %zu bytes size total. Total alloc: %lld\r\n", total, count, total_alloc );
    HEAP_UNLOCK( );
    return;
}

int count_heap( void )
{
    unsigned long int i = 0;
    HEAP *hp;

    if ( !IS_IN_DEBUGGING_MODE )
    {
        return 0;
    }
    HEAP_LOCK( );
    for ( i = 0, hp = firstheap; hp; hp = hp->next )
    {
        i++;
    }
    HEAP_UNLOCK( );
    return i;
}

void del_heap( uintptr_t m_add, int line, char *file )
{
    HEAP *h;
    char temp[ 5000 ];
    int b_type = 0;
    float t_total = 0.0;

    BOOL found = FALSE;

    temp[ 0 ] = '\0';
    b_type = 0;

    if ( !IS_IN_DEBUGGING_MODE )
    {
        return;
    }
    HEAP_LOCK( );
    if ( REPORT_DEALLOCATION )
    {
        t_total = ( float )total_alloc;

        if ( t_total > 1024 )
        {
            b_type = 1; // KB
            t_total = t_total / 1024;
        }
        if ( t_total > 1024 )
        {
            b_type = 2; // MB
            t_total = t_total / 1024;
        }
        if ( t_total > 1024 )
        {
            b_type = 3; // GB
            t_total = t_total / 1024;
        }

        LOG( "--------------------------------------------------------------------------------------------------" );
        LOG( "Deallocation:" );
        LOG( "Calling file: %s", file );
        LOG( "Calling line: %d", line );
    }

    for ( h = firstheap; h; h = h->next )
    {
        if ( h->m_add == m_add )
        {
            found = TRUE;
            if ( REPORT_DEALLOCATION )
            {
                LOG( "Size freeing: %zu", h->size );
            }

            if ( h->prev != NULL )
            {
                h->prev->next = h->next;
            }

            if ( h == firstheap )
            {
                firstheap = h->next;
            }

            if ( h->next != NULL )
            {
                h->next->prev = h->prev;
            }

            if ( h == lastheap )
            {
                lastheap = h->prev;
            }
            h->chunk = NULL;
            free( h );
            unalloced += 1;
            break;
        }
    }
    if ( REPORT_DEALLOCATION )
    {
        LOG( "Total allocation: %5.5f %s", t_total, b_type == 0 ? "bytes" : b_type == 1 ? "kilobytes" : b_type == 2 ? "megabytes" : "gigabytes" );
        LOG( "Calling address (returned): 0X%X", m_add );
        LOG( "--------------------------------------------------------------------------------------------------\r\n" );
    }

    if ( found == FALSE )
    {
        LOG( "Del_heap: m_add does not match a heap we manage. m_add; 0x%x\n\tCalling file: %s, calling line: %d\n", m_add, file, line );
    }
    HEAP_UNLOCK( );
    return;
}

HEAP *find_heap( uintptr_t m_add )
{
    HEAP *h;
    if ( !IS_IN_DEBUGGING_MODE )
    {
        return NULL;
    }
    HEAP_LOCK( );
    for ( h = firstheap; h; h = h->next )
    {
        if ( h->m_add == m_add )
        {
            HEAP_UNLOCK( );
            return h;
        }
    }
    HEAP_UNLOCK( );
    return NULL;
}

void *nano_malloc( size_t chunk, const char *file, int line )
{
    void *mem;
    size_t aligned_size = chunk;

    while ( ( aligned_size + sizeof( size_t ) ) % ALIGN != 0 )
    {
        aligned_size++;
    }

    mem = malloc( aligned_size + sizeof( size_t ) );
    if ( !mem )
    {
        LOG( "Memory failed to allocate! File: %s, line: %d, size: %zu", file, line, aligned_size );
        GiveError( "Memory allocation failed", TRUE );
        exit( 1 );
        return NULL;
    }

    *( ( size_t * )mem ) = aligned_size | MALLOC_MAGIC;

    HEAP_LOCK( );
    total_alloc += aligned_size + sizeof( size_t );
    if ( IS_IN_DEBUGGING_MODE )
    {
        HEAP *h = new_heap( );
        h->m_add = ( uintptr_t )mem;
        h->size = aligned_size;
        h->chunk = ( char * )mem + sizeof( size_t );
        add_heap( h );
    }
    HEAP_UNLOCK( );

    return ( char * )mem + sizeof( size_t );
}

void nano_free( void *seg, const char *file, int line )
{
    char madd[ 100 ];
    uintptr_t m_add;
    char *tail;

    static char t_t;

    if ( !seg || seg == NULL )
    {
        LOG_MALLOC_ERROR( "Free passed an invalid segment. Bailing." );
        return;
    }

    if ( IS_IN_DEBUGGING_MODE == 1 )
    {
        sprintf( madd, "%p", ( ( char * )seg - sizeof( MEM_TYPE ) ) );
        m_add = strtoull( madd, &tail, 16 );
        del_heap( ( unsigned long )m_add, line, ( char * )file );
        madd[ 0 ] = '\0';
    }

    if ( seg != NULL )
    {
        char *header = ( char * )seg - sizeof( size_t );
        size_t stored = *( ( size_t * )header );
        if ( ( stored & MALLOC_MAGIC ) != MALLOC_MAGIC )
        {
            LOG_MALLOC_ERROR( "Memory error. Bailing. ID:%zu -- With Magic:%zu", stored, ( stored & MALLOC_MAGIC ) );
            return;
        }

        HEAP_LOCK( );
        if ( IS_IN_DEBUGGING_MODE == 1 )
        {
            del_heap( ( uintptr_t )header, line, ( char * )file );
        }
        total_alloc -= ( ( stored & ~MALLOC_MAGIC ) + sizeof( size_t ) );
        free( header );
        seg = NULL;
        HEAP_UNLOCK( );
    }
    else
    {
        LOG_MALLOC_ERROR( "Memory error: %p. (%s/%d)", seg, file, line );
    }
    return;
}

void *nano_realloc( void *seg, size_t sz, const char *file, int line )
{
    size_t stored = 0;
    size_t size = 0;
    BOOL hasmagic = FALSE;

    if ( !seg ) // THIS SHOULD ALWAYS BE FIRST BEFORE DOING ANY MATH ON IT.
    {
        LOG_MALLOC_ERROR( "nano_realloc: NULL segment passed. File: %s, line: %d", file, line );
        return NULL;
    }

    char *r = ( char * )seg - sizeof( size_t );
    size_t magic = *( ( size_t * )r );

    if ( ( magic & MALLOC_MAGIC ) != MALLOC_MAGIC )
    {
        LOG_MALLOC_ERROR( "Memory error. Bailing. ID:%zu -- With Magic:%zu", magic, ( magic & MALLOC_MAGIC ) );
        hasmagic = TRUE;
        return NULL;
    }

    if ( !seg )
    {
        LOG_MALLOC_ERROR( "nano_realloc: NULL segment passed. File: %s, line: %d", file, line );
        return NULL;
    }
    while ( ( sz + sizeof( size_t ) ) % ALIGN != 0 )
    {
        sz++;
    }
    //char *header = ( char * )seg - sizeof( size_t );
    //realloc( header, sz + sizeof( size_t ) + 16 );
    seg = realloc( r, sz + sizeof( size_t ) + 16 );

    *( ( size_t * )seg ) = sz | MALLOC_MAGIC;
    total_alloc += sz + sizeof( size_t );

    return ( char * )seg + sizeof( size_t );
}
    /*/
    char *header = ( char * )seg - sizeof( size_t );
    stored = *( ( size_t * )header );
    if ( ( stored & MALLOC_MAGIC ) != MALLOC_MAGIC )
    {
        LOG_MALLOC_ERROR( "Realloc error. Bailing. ID:%zu -- With Magic:%zu", stored, ( stored & MALLOC_MAGIC ) );
        return NULL;
    }

    size = ( stored & ~MALLOC_MAGIC ) + sizeof( size_t );

    sz += size;

    while ( ( sz + sizeof( size_t ) ) % ALIGN != 0 )
    {
        sz++;
    }

    mem = nano_malloc( sz + sizeof( size_t ), __FILE__, __LINE__ );

    if ( !mem )
    {
        LOG_MALLOC_ERROR( "nano_realloc: Memory allocation failed! File: %s, line: %d, size: %zu", file, line, sz );
        GiveError( "Memory allocation failed", TRUE );
        exit( 1 );
        return NULL;
    }

    total_alloc += sz + sizeof( size_t );

   // memcpy( ( char * )mem + sizeof( size_t ), seg, stored );
    realloc( mem, sz );

    nano_free( mem, __FILE__, __LINE__ );

    return ( char * )seg + sizeof( size_t );
}
*/
   /* char *header = ( char * )seg - sizeof( size_t );
    size_t stored = *( ( size_t * )header );
    if ( ( stored & MALLOC_MAGIC ) != MALLOC_MAGIC )
    {
        LOG_MALLOC_ERROR( "nano_realloc: Memory error. Invalid magic. File: %s, line: %d", file, line );
        return NULL;
    }

    HEAP_LOCK( );
    total_alloc -= ( ( stored & ~MALLOC_MAGIC ) + sizeof( size_t ) );
    void *new_mem = realloc( header, aligned_size + sizeof( size_t ) );
    if ( !new_mem )
    {
        LOG_MALLOC_ERROR( "nano_realloc: Memory reallocation failed! File: %s, line: %d, size: %zu", file, line, aligned_size );
        GiveError( "Memory reallocation failed", TRUE );
        exit( 1 );
        HEAP_UNLOCK( );
        return NULL;
    }

    *( ( size_t * )new_mem ) = aligned_size | MALLOC_MAGIC;
    total_alloc += aligned_size + sizeof( size_t );

    if ( IS_IN_DEBUGGING_MODE )
    {
        HEAP *h;
        h = find_heap( ( uintptr_t )header );
        if ( h == NULL )
        {
            LOG( "nano_realloc: Heap tracking error. Original block not found. File: %s, line: %d", file, line );
        }
        else
        {
            h->m_add = ( uintptr_t )new_mem;
            h->size = aligned_size;
            h->chunk = ( char * )new_mem + sizeof( size_t );
            if ( REPORT_ALLOCATION )
            {
                LOG( "nano_realloc: Reallocation tracked. Old address: 0x%X, New address: 0x%X, size: %zu. File: %s, line: %d", ( uintptr_t )header, ( uintptr_t )new_mem, aligned_size, file, line );
            }
        }
    }
    HEAP_UNLOCK( );

    return ( char * )new_mem + sizeof( size_t );

    */

void return_usage( void )
{
    return;
}

unsigned long int get_memory_usage( )
{
    HEAP_LOCK( );
    unsigned long int usage = ( unsigned long int )total_alloc;
    HEAP_UNLOCK( );
    return usage;
}