/* Nanobit Software's io.c - IO library for personal projects.

    This program is open source, and you are free to use it, modify it, and distribute it
    as you see fit, as long as you keep the original copyright and license information intact.
    This program is released under the GNU General Public License v3.0.
    You can find the license information in the LICENSE file in the root of this project.
    This program is provided "as is", without any warranty of any kind, express or implied.
    In no event shall the authors or copyright holders be liable for any claim, damages, or other
    liability, whether in an action of contract, tort, or otherwise, arising from,
    out of, or in connection with the software or the use or other dealings in the software.
    This program is a work in progress, and is not yet complete. It is meant to be a
    starting point for a more complete program that will be able to read and process
    SMS Backup and Restore files, and extract the data you want from them.
    Please feel free to contribute to this project, and help make it better.
    If you have any questions, comments, or suggestions, please feel free to contact me at
    mike@nanobit.net or visit the project page at https://www.nanobit.net/nano-sms.

    Copyright (c) 2025 Mike H.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <math.h>

#include "io.h"


// We keep this struct to this scope so it cannot be used elsewhere.
// This is basically a private data struct.
struct
{
    FILESTREAM **streams; // Array of pointers to FILESTREAM structures.
    size_t count;         // Number of registered streams.
    size_t capacity;      // Capacity of the streams array.
} _fs_registry = { NULL, 0, 0 };

/* Function to register a file stream 
*  IMPORTANT - This function does not check for duplicates.
* IMPORTANT - YOU MUST CHECK THE RETURN VALUE!
*/
BOOL _register_fs_handle ( FILESTREAM *fs )
{
    if ( _fs_registry.count >= _fs_registry.capacity )
    {
        size_t new_capacity = ( _fs_registry.capacity == 0 ) ? 4 : _fs_registry.capacity * 2;
        FILESTREAM **new_streams = ( FILESTREAM ** ) realloc ( _fs_registry.streams, new_capacity * sizeof ( FILESTREAM * ) );
        
        if ( new_streams == NULL )
        {
            // This is a major problem.
            return FALSE; // Memory allocation failed.
        }

        _fs_registry.streams = new_streams;
        _fs_registry.capacity = new_capacity;
    }

    _fs_registry.streams[_fs_registry.count++] = fs;
    return TRUE;

}
/* Function to unregister a file stream 
* IMPORTANT - This function does not free the stream.
* IMPORTANT - YOU MUST CHECK THE RETURN VALUE!
*/
BOOL _unregister_fs_handle ( FILESTREAM *fs )
{
    for ( size_t i = 0; i < _fs_registry.count; i++ )
    {
        if ( _fs_registry.streams[i] == fs )
        {
            // Found the stream, remove it by shifting the rest down.
            memmove ( &_fs_registry.streams[i], &_fs_registry.streams[i + 1], ( _fs_registry.count - i - 1 ) * sizeof ( FILESTREAM * ) );
            _fs_registry.count--;
            return TRUE;
        }
    }
    
    return FALSE; // Stream not found.
}

/* This function is more of a cleanup for in case someone didn't
    clear their streams properly. It is a public function.
    */
BOOL unregister_all_streams ( void )
{
    BOOL all_unregistered = TRUE;

    if (_fs_registry.streams == NULL || _fs_registry.count == 0 )
    {
        return TRUE; // Nothing to unregister.
    }

    for ( size_t i = 0; i < _fs_registry.count; i++ )
    {
        if ( !_unregister_fs_handle ( _fs_registry.streams[i] ) )
        {
            all_unregistered = FALSE;
        }
    }
    
    free ( _fs_registry.streams );
    _fs_registry.streams = NULL;
    _fs_registry.count = 0;
    _fs_registry.capacity = 0;
    
    // Do we have some still registered for some reason? Maybe we should take a look
    if (!all_unregistered)
    {
        LOG ( "Warning: Some file streams were not unregistered properly. Possible memory leak." );
    }

    return all_unregistered;
}


FILESTREAM *_new_fs_stream ( int line, char * file)
{
    FILESTREAM *fs = ( FILESTREAM * ) malloc ( sizeof ( FILESTREAM ) );
    
    if ( fs == NULL )
    {
        return NULL;
    }
    
    fs->buffer = ( char * ) malloc ( BUFFER_INIT );
    
    if ( fs->buffer == NULL )
    {
        free ( fs );
        return NULL;
    }
    
    fs->size = BUFFER_INIT;
    fs->length = 0;
    fs->pos = 0;
    fs->file_read = 0;
    fs->file_size = 0;
    fs->seek_pos = 0;
    fs->opened = 0;
    fs->modified = 0;
    fs->accessed = 0;
    fs->file = NULL;
    fs->last_token = 0;
    fs->cur_token = 0;
    fs->last_error = 0;
    fs->state = STATE_READY;
    fs->mode = MODE_NONE;
    fs->eof = FALSE;
    fs->is_open = FALSE;
    memset ( fs->file_path, 0, sizeof ( fs->file_path ) );
    memset ( fs->file_name, 0, sizeof ( fs->file_name ) );
    if ( _register_fs_handle ( fs ) == FALSE )
    {
        LOG ( "Unable to register a new file stream within the system. Bailing: %s / %d", file, line );

        free ( fs->buffer );
        free ( fs );
        return NULL;
     }
    return fs;
}

/* Function to free a file stream */
BOOL _free_fs_stream(FILESTREAM *fs, int line, char *file)
{
    if ( fs == NULL )
    {
        return FALSE;
    }
    
    if ( fs->is_open )
    {
        // If the stream is still open, close it first.
        fclose ( fs->file );
        fs->is_open = FALSE;
    }
    
    free ( fs->buffer );
    
    if ( !_unregister_fs_handle ( fs ) )
    {
        LOG ( "Unable to unregister file stream within the system. Possible memory leak: %s / %d", file, line );
        // We continue to free the memory even if unregistering fails.
    }
    
    free ( fs );
    return TRUE;
}