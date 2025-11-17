/*
Copyright( c ) 2025 Mike H.
This program is free software : you can redistribute it and /or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
( at your option ) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.If not, see < http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include <math.h>
#include <sys/types.h>
#include <time.h>


#include "nano-io.h"
#include "nano-sms.h"
#include "nano-sql.h"
#include "sms_db.h" 
#include "sqlite3/sqlite3.h"


/// <summary>
/// Globals for this scope.
/// </summary>
extern void LOG( char *fmt, ... );

TBUF_MAN *buf_manager = NULL; // The global t-buf manager.
BOOL tbuf_init = FALSE; // Have the t-bufs been initialized?



// Internal function to initialize the t-buf manager.
static void init_tbuf_manager( )
{
    if ( tbuf_init )
        return;
    buf_manager = ( TBUF_MAN * )malloc( sizeof( TBUF_MAN ) );
    if ( !buf_manager )
    {
        LOG_SQL_ERROR( "Failed to allocate memory for t-buf manager." );
        return;
    }
    buf_manager->tbufs = ( TBUF ** )malloc( sizeof( TBUF * ) * MAX_TBUF ); // Allocate array for t-buf pointers.
    for ( int i = 0; i < MAX_TBUF; i++ ) // Probably not needed. But a condom is a condom.
        buf_manager->tbufs[ i ] = NULL; // Initialize all pointers to NULL.

    buf_manager->count = 0;
    tbuf_init = TRUE;
}

 /*     Create a new t-buf and add it to the manager.
        Returns pointer to the new t-buf, or NULL on failure.
        WARNING: Remember to free the t-buf when done!
        WARNING: THE BUFFER ALLOCATED IS JUST A STARTING POINT.
        WARNING: YOU MUST MANAGE THE BUFFER YOURSELF!
 */
TBUF *create_tbuf( void )
{
    init_tbuf_manager( );
    if ( buf_manager->count >= MAX_TBUF )
    {
        LOG_SQL_ERROR( "Maximum number of t-bufs reached." );
        return NULL;
    }
    TBUF *new_tbuf = ( TBUF * )malloc( sizeof( TBUF ) );
    if ( !new_tbuf )
    {
        LOG_SQL_ERROR( "Failed to allocate memory for new t-buf." );
        return NULL;
    }

    new_tbuf->buffer = ( char * )malloc( TBUF_GROW ); // Give it some safe memory. Just don't forget!
    if ( !new_tbuf->buffer )
    {
        LOG_SQL_ERROR( "Failed to allocate memory for t-buf buffer." );
        free( new_tbuf );
        return NULL;
    }
    new_tbuf->size = TBUF_GROW;
    new_tbuf->in_use = 0;
    buf_manager->tbufs[ buf_manager->count++ ] = new_tbuf; // Add to manager.
    return new_tbuf;
}

/* Free a t-buf and remove it from the manager.
   Returns TRUE on success, FALSE on failure.
*/
BOOL free_tbuf( TBUF *tbuf )
{
    if ( !tbuf_init || !buf_manager || !tbuf )
    {
        LOG_SQL_ERROR( "T-buf manager not initialized or invalid t-buf." );
        return FALSE;
    }
    for ( size_t i = 0; i < buf_manager->count; i++ )
    {
        if ( buf_manager->tbufs[ i ] == NULL )
            continue; // Safety check. Since we allow null spots.
        /* On allowing null spots: our list isn't -THAT- big. 4096
           entries at most. Allowing something to bet set as null
           is very performant as opposed to shifting down by one.
           imagine if the shift down by one has to occur on the last
           element -every- time. That is a worst cast scenario. So
           for now we have a debug flag (change it in nano-sql.h)
           set to disable or enable the shifting of the list.
        */

        if ( buf_manager->tbufs[ i ] == tbuf )
        {
            if ( tbuf->buffer ) // Insurance
                free( tbuf->buffer ); // Free the buffer.
            if ( tbuf ) // Insurance
                free( tbuf );         // Free the t-buf structure.
            // Shift remaining t-bufs down in the array.
            if ( DEBUG_MODE_TBUF_FREE_SHIFT )
            {
                LOG( "Freeing t-buf at index %zu. Shifting remaining t-bufs down.", i );

                for ( size_t j = i; j < buf_manager->count - 1; j++ )
                {
                    buf_manager->tbufs[ j ] = buf_manager->tbufs[ j + 1 ];
                }
                buf_manager->tbufs[ buf_manager->count - 1 ] = NULL; // Clear last pointer.
            }
            else
            {

                buf_manager->tbufs[ i ] = NULL; // Clear last pointer.
            }
            buf_manager->count--;
            return TRUE;
        }
    }
    LOG_SQL_ERROR( "T-buf not found in manager." );
    return FALSE;
}

BOOL free_tbuf_man( void )
{
    if ( !tbuf_init || !buf_manager )
    {
        LOG_SQL_ERROR( "T-buf manager not initialized." );
        return FALSE;
    }
    for ( size_t i = 0; i < buf_manager->count; i++ )
    {
        if ( buf_manager->tbufs[ i ] )
        {
            free_tbuf( buf_manager->tbufs[ i ] );
        }
    }
    if ( buf_manager->tbufs ) // Insurance
        free( buf_manager->tbufs );
    if ( buf_manager ) // Insurance
        free( buf_manager );
    buf_manager = NULL;
    tbuf_init = FALSE;
    return TRUE;
}

size_t tbuf_get_size( TBUF *tbuf )
{
    if ( !tbuf )
    {
        LOG_SQL_ERROR( "Invalid t-buf provided to tbuf_get_size." );
        return 0;
    }
    return tbuf->size;
}


size_t tbuf_get_used( TBUF *tbuf )
{
    if ( !tbuf )
    {
        LOG_SQL_ERROR( "Invalid t-buf provided to tbuf_get_used." );
        return 0;
    }
    return tbuf->used;
}


/* Since we memcpy the data to our own buffer, IT IS UP TO THE
    CALLEE to clear their buffer as well.
*/
BOOL tbuf_add_data( TBUF *tbuf, const char *data, size_t len )
{
    if ( !tbuf || !data || len == 0 )
    {
        LOG_SQL_ERROR( "Invalid arguments provided to tbuf_add_data." );
        return FALSE;
    }
    // Check if we need to expand the buffer.
    if ( tbuf->used + len >= tbuf->size )
    {
        size_t new_size = tbuf->size + TBUF_GROW;
        char *new_buffer = ( char * )realloc( tbuf->buffer, new_size );
        if ( !new_buffer )
        {
            LOG_SQL_ERROR( "Failed to expand t-buf buffer." );
            return FALSE;
        }
        tbuf->buffer = new_buffer;
        tbuf->size = new_size;
    }
    // Copy the data into the buffer.
    memcpy( tbuf->buffer + tbuf->used, data, len );
    tbuf->used += len;
    return TRUE;
}