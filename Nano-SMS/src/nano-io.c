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
#include <time.h>
#include <io.h>

#include "nano-io.h"

// Globals for this scope.

char stream_scratch [ STREAM_SCRATCH ] = "\0"; // The scratch buffer for reading.
extern void LOG ( char *fmt, ... );

// We keep this struct to this scope so it cannot be used elsewhere.
// This is basically a private data struct.
struct
{
    FILESTREAM **streams; // Array of pointers to FILESTREAM structures.
    size_t count;         // Number of registered streams.
    size_t capacity;      // Capacity of the streams array.
} _fs_registry = { NULL, 0, 0 };



/* Function to find a file stream by its path */ 

FILESTREAM *_fs_find_handle ( char *path )
{
    if ( path == NULL || strlen ( path ) == 0 )
    {
        return NULL;
    }
    for ( size_t i = 0; i < _fs_registry.count; i++ )
    {
        if ( strcmp ( _fs_registry.streams[i]->file_path, path ) == 0 )
        {
            return _fs_registry.streams[i];
        }
    }
    return NULL; // Not found.
}


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

    if ( fs->buffer)
        free ( fs->buffer );
    
    // Set the static strings to null just in case.
    
    fs->file_path[ 0 ] = '\0';
    fs->file_name[ 0 ] = '\0';
    fs->size = 0;
    fs->length = 0;
    fs->pos = 0;
    fs->file_read = 0;
    fs->file_size = 0;
    fs->seek_pos = 0;
    fs->opened = 0;
    fs->modified = 0;
    fs->accessed = 0;
    fs->last_token = 0;
    fs->cur_token = 0;
    fs->last_error = 0;
    fs->state = STATE_READY;
    fs->mode = MODE_NONE;
    fs->eof = FALSE;
    fs->is_open = FALSE;


    
    free ( fs );
    return TRUE;
}

/* Function to open a file stream */

FILESTREAM * fs_open ( FILESTREAM *fs, const char *path, const char *mode )
{
    FILESTREAM *fsnew = NULL;






    if ( fs == NULL )
    {
        // If sent a NULL, let's make one and register it for the user.
        fsnew = _new_fs_stream ( __LINE__, __FILE__ );
        
        if ( fsnew == NULL )
        {
            return NULL;
        }
        
        fs = fsnew;
               

    }
    if (fs == NULL || path == NULL || mode == NULL )
    {
        if ( fsnew != NULL )
        {
            _free_fs_stream ( fsnew, __LINE__, __FILE__ ); // If we made it, let's free it before returning

        }
        return NULL;
    }

    if ( fs->is_open )
    {
        // If the stream is already open, close it first.
        fclose ( fs->file );
        fs->is_open = FALSE;
    }

    fs->file = fopen ( path, mode );

    if ( fs->file == NULL )
    {
        if (fsnew != NULL)
        {
            _free_fs_stream ( fsnew, __LINE__, __FILE__ ); // If we made it, let's free it before returning
        }
        else
        {
            fs->last_error = STREAM_ERROR;
        }
        LOG ( "Unable to open %s within mode %s, returning NULL.", path, mode );
        
        
        return NULL;
    }


    // Get the file size
    _fseeki64 ( fs->file, 0, SEEK_END );
    fs->file_size = _ftelli64 ( fs->file );
    _fseeki64     ( fs->file, 0, SEEK_SET );
    


    // Update the file path and name
    strncpy ( fs->file_path, path, sizeof ( fs->file_path ) - 1 );
  
    const char *slash = strrchr ( path, '\\' );
    if ( slash )
    {
        strncpy ( fs->file_name, slash + 1, sizeof ( fs->file_name ) - 1 );
    }
    else
    {
        strncpy ( fs->file_name, path, sizeof ( fs->file_name ) - 1 );
    }

    // Update the opened time
    fs->opened = time ( NULL );

    // Set the mode
    if ( strcmp ( mode, "r" ) == 0 )
    {
        fs->mode = MODE_READ;
    }
    else if ( strcmp ( mode, "w" ) == 0 )
    {
        fs->mode = MODE_WRITE;
    }
    else if ( strcmp ( mode, "a" ) == 0 )
    {
        fs->mode = MODE_APPEND;
    }
    else if ( strcmp ( mode, "r+" ) == 0 )
    {
        fs->mode = MODE_READWRITE;
    }
    else if ( strcmp ( mode, "rb" ) == 0 || strcmp ( mode, "wb" ) == 0 || strcmp ( mode, "ab" ) == 0 || strcmp ( mode, "r+b" ) == 0 )
    {
        fs->mode = MODE_BINARY;
    }
    else
    {
        fs->mode = MODE_NONE;
    }

    fs->accessed = fs->opened;
    fs->modified = fs->opened;
    fs->is_open = TRUE;
    fs->eof = FALSE;
    fs->state = STATE_OPEN;
    fs->last_error = 0;
    fs->pos = 0;
    fs->length = 0;
    fs->file_read = 0;
    fs->seek_pos = 0;
    return fs;
}

/* fs_close: close the filestream and clean up al ittle 
*/

BOOL fs_close ( FILESTREAM *fs, BOOL cleanup )
{
    if ( fs == NULL )
    {
        return FALSE;
    }
    
    if ( !fs->is_open && !cleanup)
    {
        return TRUE; // Already closed.
    }
    else
    {
        if ( !fs->is_open && cleanup )
        {
            // If we're cleaning up, we should free the stream.
            return _free_fs_stream ( fs, __LINE__, __FILE__ );
        }
    }
    
    if ( fclose ( fs->file ) != 0 )
    {
        if (cleanup)
        {
            _free_fs_stream ( fs, __LINE__, __FILE__ ); // If we can't close it, we should still free it if cleanup is requested.
            LOG ( "Unable to free file stream!" );
            return FALSE;
        }
        else
        {
            fs->last_error = STREAM_ERROR;
            return FALSE; // Error closing file.
        }
    }
    
    
    if ( cleanup )
    {
        return _free_fs_stream ( fs, __LINE__, __FILE__ );
    }

    fs->is_open = FALSE;
    fs->eof = TRUE;
    fs->state = STATE_CLOSING;
    fs->last_error = 0;
    return TRUE;
}

/* logfs will report the structure to the logfile for debug purposes.*/
void logfs ( FILESTREAM *fs )
{
    
    static char buf[ 2048 * 2 ];


    if ( !fs )
        return; // Need a valid struct.
    if ( fs->is_open != TRUE )
        return;
    if ( !fs->file )
        return;
    buf[ 0 ] = '\0';
    LOG ( "-------------------------------------------------------------" );
    LOG ( "FileStream Structure:" );
    LOG ( "-------------------------------------------------------------" );
    sprintf ( buf, 
              
              "File Path:    %s\n"
                     "File Name:    %s\n"
            
                     "Buffer Length:%zu\n"
                     "Pos:          %zu\n"
                     "Bytes Read:   %zu\n"
                     "File Size:    %zu mb\n"
                     "Seek Position %zu\n"
                     "Opened:       %s\n"
                     "Modified:     %s\n"
                     "Accessed:     %zu\n"
                     "File pointer: %p\n"
                     "Eof:          %s\n",
              fs->file_path,
              fs->file_name,
              
              fs->length,
              fs->pos,
              fs->file_read,
              fs->file_size / 1024 / 1024  ,
              fs->seek_pos,
              fs->is_open ? "TRUE" : "FALSE",
              fs->modified ? "TRUE" : "FALSE",
              fs->accessed,
              fs->file,
              fs->eof ? "TRUE" : "FALSE"
    );
    LOG ( buf );
    LOG ( "-------------------------------------------------------------\n\n" );

}


