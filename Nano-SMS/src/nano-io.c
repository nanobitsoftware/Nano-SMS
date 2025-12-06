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
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <time.h>
#include <stdarg.h>

#include "nano-io.h"
#include "nano-sms.h"

// Globals for this scope.

char stream_scratch[ STREAM_SCRATCH ] = "\0"; // The scratch buffer for reading.
extern void x( char *fmt, ... );

// We keep this struct to this scope so it cannot be used elsewhere.
// This is basically a private data struct.
struct
{
    FILESTREAM **streams; // Array of pointers to FILESTREAM structures.
    size_t count;         // Number of registered streams.
    size_t capacity;      // Capacity of the streams array.
} _fs_registry = { NULL, 0, 0 };

/**
 * @brief Find a file stream by its file path.
 *
 * Searches the internal file stream registry for a stream matching the given path.
 *
 * @param path The file path to search for.
 * @return Pointer to the FILESTREAM if found, NULL otherwise.
 */
FILESTREAM *_fs_find_handle( char *path )
{
    if ( path == NULL || strlen( path ) == 0 )
    {
        return NULL;
    }
    for ( size_t i = 0; i < _fs_registry.count; i++ )
    {
        if ( strcmp( _fs_registry.streams[ i ]->file_path, path ) == 0 )
        {
            return _fs_registry.streams[ i ];
        }
    }
    return NULL; // Not found.
}

BOOL _init_fs( void )
{
    _fs_registry.streams = NULL;
    _fs_registry.count = 0;
    _fs_registry.capacity = 0;
    return TRUE;
}

/**
 * @brief Register a file stream in the internal registry.
 *
 * Adds the given FILESTREAM pointer to the registry, expanding the registry if needed.
 * IMPORTANT: This function does not check for duplicates.
 * IMPORTANT: YOU MUST CHECK THE RETURN VALUE!
 * IMPORTANT: THIS IS AN INTERNAL FUNCTION; NOT FOR PUBLIC USE!
 *
 * @param fs Pointer to the FILESTREAM to register.
 * @return TRUE if registration succeeded, FALSE if memory allocation failed.
 */
BOOL _register_fs_handle( FILESTREAM *fs )
{
    FILESTREAM **new_streams;

    if ( fs == NULL )
    {
        return FALSE;
    }

    if ( _fs_registry.count >= _fs_registry.capacity )
    {
        size_t new_capacity = ( _fs_registry.capacity == 0 ) ? 4 : _fs_registry.capacity * 2;
        if ( _fs_registry.streams )
        {
            new_streams = ( FILESTREAM ** )realloc( _fs_registry.streams, new_capacity * sizeof( FILESTREAM * ) );
        }
        else
        {
            new_streams = ( FILESTREAM ** )malloc( new_capacity * sizeof( FILESTREAM * ) );
        }

        _fs_registry.streams = new_streams;
        _fs_registry.capacity = new_capacity;
    }

    _fs_registry.streams[ ( _fs_registry.count++ ) - 1 ] = fs;
    return TRUE;
}

/**
 * @brief Unregister a file stream from the internal registry.
 *
 * Removes the given FILESTREAM pointer from the registry.
 * IMPORTANT: This function does not free the stream.
 * IMPORTANT: YOU MUST CHECK THE RETURN VALUE!
 *
 * @param fs Pointer to the FILESTREAM to unregister.
 * @return TRUE if unregistration succeeded, FALSE if the stream was not found.
 */
BOOL _unregister_fs_handle( FILESTREAM *fs )
{
    for ( size_t i = 0; i < _fs_registry.count; i++ )
    {
        if ( _fs_registry.streams[ i ] == fs )
        {
            // Found the stream, remove it by shifting the rest down.
            memmove( &_fs_registry.streams[ i ], &_fs_registry.streams[ i + 1 ], ( _fs_registry.count - i - 1 ) * sizeof( FILESTREAM * ) );
            _fs_registry.count--;
            return TRUE;
        }
    }

    return FALSE; // Stream not found.
}

/**
 * @brief Unregister all file streams from the registry.
 *
 * This function is intended as a cleanup in case streams were not properly cleared.
 * Frees the registry array and resets the registry state.
 *
 * @return TRUE if all streams were unregistered, FALSE if some could not be unregistered.
 */
BOOL unregister_all_streams( void )
{
    BOOL all_unregistered = TRUE;

    if ( _fs_registry.streams == NULL || _fs_registry.count == 0 )
    {
        return TRUE; // Nothing to unregister.
    }

    for ( size_t i = 0; i < _fs_registry.count; i++ )
    {
        if ( !_unregister_fs_handle( _fs_registry.streams[ i ] ) )
        {
            all_unregistered = FALSE;
        }
    }

    free( _fs_registry.streams );
    _fs_registry.streams = NULL;
    _fs_registry.count = 0;
    _fs_registry.capacity = 0;

    // Do we have some still registered for some reason? Maybe we should take a look
    if ( !all_unregistered )
    {
        LOG( "Warning: Some file streams were not unregistered properly. Possible memory leak." );
    }

    return all_unregistered;
}

/**
 * @brief Allocate and initialize a new FILESTREAM structure.
 *
 * Allocates memory for a new FILESTREAM and its buffer, initializes all fields,
 * and registers the stream in the internal registry.
 *
 * @param line The line number where the function is called (for logging).
 * @param file The file name where the function is called (for logging).
 * @return Pointer to the new FILESTREAM, or NULL on failure.
 */
FILESTREAM *_new_fs_stream( int line, char *file )
{
    FILESTREAM *fs = ( FILESTREAM * )malloc( sizeof( FILESTREAM ) );

    if ( fs == NULL )
    {
        return NULL;
    }

    fs->buffer = ( char * )malloc( BUFFER_INIT );

    if ( fs->buffer == NULL )
    {
        free( fs );
        return NULL;
    }

    fs->size = BUFFER_INIT;
    fs->length = 0;
    fs->pos = 0;
    fs->file_read = 0;
    fs->file_size = 0;
    fs->hr_size = 0.0f;
    fs->size_type = TYPE_BYTE;
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
    memset( fs->file_path, 0, sizeof( fs->file_path ) );
    memset( fs->file_name, 0, sizeof( fs->file_name ) );
    if ( _register_fs_handle( fs ) == FALSE )
    {
        LOG( "Unable to register a new file stream within the system. Bailing: %s / %d", file, line );

        free( fs->buffer );
        free( fs );
        return NULL;
    }
    return fs;
}

/**
 * @brief Free a FILESTREAM structure and its resources.
 *
 * Closes the file if open, frees the buffer and the FILESTREAM itself,
 * and unregisters the stream from the registry.
 *
 * @param fs Pointer to the FILESTREAM to free.
 * @param line The line number where the function is called (for logging).
 * @param file The file name where the function is called (for logging).
 * @return TRUE if the stream was freed, FALSE if the pointer was NULL.
 */
BOOL _free_fs_stream( FILESTREAM *fs, int line, char *file )
{
    if ( fs == NULL )
    {
        return FALSE;
    }

    if ( fs->is_open )
    {
        // If the stream is still open, close it first.
        fclose( fs->file );
        fs->is_open = FALSE;
    }

    if ( !_unregister_fs_handle( fs ) )
    {
        LOG( "Unable to unregister file stream within the system. Possible memory leak: %s / %d", file, line );
        // We continue to free the memory even if unregistering fails.
    }

    if ( fs->buffer )
        free( fs->buffer );

    // Set the static strings to null just in case.

    fs->file_path[ 0 ] = '\0';
    fs->file_name[ 0 ] = '\0';
    fs->size = 0;
    fs->hr_size = 0.0f;
    fs->size_type = TYPE_BYTE;
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

    free( fs );
    return TRUE;
}

char *size_type_to_string( int type )
{
    switch ( type )
    {
        case TYPE_BYTE:
            return "Bytes";
        case TYPE_KILOBYTE:
            return "KB";
        case TYPE_MEGABYTE:
            return "MB";
        case TYPE_GIGABYTE:
            return "GB";
        case TYPE_TERABYTE:
            return "TB";
        default:
            return "Unknown";
    }
}

char *size_type_from_len( size_t len )
{
    if ( len < 1024 )
    {
        return "Bytes";
    }
    else if ( len >= 1024 && len < 1048576 )
    {
        return "KB";
    }
    else if ( len >= 1048576 && len < 1073741824 )
    {
        return "MB";
    }
    else if ( len >= 1073741824 && len < 1099511627776 )
    {
        return "GB";
    }
    else if ( len >= 1099511627776 )
    {
        return "TB";
    }
    else
    {
        return "Unknown";
    }
}

/* Return a nummber for human readability from a double.*/
double double_to_human( size_t hr_size )
{
    if ( hr_size < 1024 )
    {
        return ( double )hr_size;
    }
    else if ( hr_size >= 1024 && hr_size < 1048576 )
    {
        return ( double )hr_size / 1024;
    }
    else if ( hr_size >= 1048576 && hr_size < 1073741824 )
    {
        return ( double )hr_size / 1024 / 1024;
    }
    else if ( hr_size >= 1073741824 && hr_size < 1099511627776 )
    {
        return ( double )hr_size / 1024 / 1024 / 1024;
    }
    else if ( hr_size >= 1099511627776 )
    {
        return ( double )hr_size / 1024 / 1024 / 1024 / 1024;
    }
    else
    {
        return 0.00f;
    }
}

/**
 * @brief Open a file stream.
 *
 * Opens a file with the specified path and mode, initializes the FILESTREAM structure,
 * and updates all relevant fields. If the provided FILESTREAM pointer is NULL, a new one is allocated.
 *
 * @param fs Pointer to an existing FILESTREAM, or NULL to allocate a new one.
 * @param path The file path to open.
 * @param mode The mode string for fopen (e.g., "r", "w", "rb").
 * @return Pointer to the opened FILESTREAM, or NULL on failure.
 */
FILESTREAM *fs_open( FILESTREAM *fs, const char *path, const char *mode )
{
    FILESTREAM *fsnew = NULL;

    if ( fs == NULL )
    {
        // If sent a NULL, let's make one and register it for the user.
        fsnew = _new_fs_stream( __LINE__, __FILE__ );

        if ( fsnew == NULL )
        {
            return NULL;
        }

        fs = fsnew;
    }
    if ( fs == NULL || path == NULL || mode == NULL )
    {
        if ( fsnew != NULL )
        {
            _free_fs_stream( fsnew, __LINE__, __FILE__ ); // If we made it, let's free it before returning
        }
        return NULL;
    }

    if ( fs->is_open )
    {
        // If the stream is already open, close it first.
        fclose( fs->file );
        fs->is_open = FALSE;
    }

    fs->file = fopen( path, mode );

    if ( fs->file == NULL )
    {
        if ( fsnew != NULL )
        {
            _free_fs_stream( fsnew, __LINE__, __FILE__ ); // If we made it, let's free it before returning
        }
        else
        {
            fs->last_error = STREAM_ERROR;
            fs->mode = MODE_ERROR;
            fs->state = STATE_ERRORS;
        }
        LOG( "Unable to open %s within mode %s, returning NULL.", path, mode );
        return NULL;
    }

    // Get the file size
    // !!!! I think we're using a windows api for this. Should change it for portability.
    _fseeki64( fs->file, 0, SEEK_END );
    fs->file_size = _ftelli64( fs->file );
    _fseeki64( fs->file, 0, SEEK_SET );

    /* Determine the size type and human-readable size
     * numbers are pre-calculated at code time. Check
     * my math if you must.
     */
    if ( fs->file_size < 1024 )
    {
        fs->size_type = TYPE_BYTE;
    }
    else if ( fs->file_size >= 1024 && fs->file_size < 1048576 )
    {
        fs->size_type = TYPE_KILOBYTE;
        fs->hr_size = ( double )fs->file_size / 1024;
    }
    else if ( fs->file_size >= 1048576 && fs->file_size < 1073741824 )
    {
        fs->size_type = TYPE_MEGABYTE;
        fs->hr_size = ( double )fs->file_size / 1024 / 1024;
    }
    else if ( fs->file_size >= 1073741824 && fs->file_size < 1099511627776 )
    {
        fs->size_type = TYPE_GIGABYTE;
        fs->hr_size = ( double )fs->file_size / 1024 / 1024 / 1024;
    }
    else if ( fs->file_size >= 1099511627776 )
    {
        fs->size_type = TYPE_TERABYTE;
        fs->hr_size = ( double )fs->file_size / 1024 / 1024 / 1024 / 1024;
    }
    else
    {
        fs->size_type = TYPE_BYTE;
    }

    // Update the file path and name
    strncpy( fs->file_path, path, sizeof( fs->file_path ) - 1 );

    const char *slash = strrchr( path, '\\' );
    if ( slash )
    {
        strncpy( fs->file_name, slash + 1, sizeof( fs->file_name ) - 1 );
    }
    else
    {
        strncpy( fs->file_name, path, sizeof( fs->file_name ) - 1 );
    }

    // Update the opened time
    fs->opened = time( NULL );

    // Set the mode
    if ( strcmp( mode, "r" ) == 0 )
    {
        fs->mode = MODE_READ;
    }
    else if ( strcmp( mode, "w" ) == 0 )
    {
        fs->mode = MODE_WRITE;
    }
    else if ( strcmp( mode, "a" ) == 0 )
    {
        fs->mode = MODE_APPEND;
    }
    else if ( strcmp( mode, "r+" ) == 0 )
    {
        fs->mode = MODE_READWRITE;
    }
    else if ( strcmp( mode, "b" ) == 0 )
    {
        fs->mode = MODE_BINARY;
    }
    else if ( strcmp( mode, "rw" ) == 0 )
    {
        fs->mode = MODE_READWRITE;
    }
    else if ( strcmp( mode, "rb" ) == 0 )
    {
        fs->mode = MODE_READBINARY;
    }
    else if ( strcmp( mode, "wb" ) == 0 )
    {
        fs->mode = MODE_WRITEBINARY;
    }
    else if ( strcmp( mode, "ab" ) == 0 )
    {
        fs->mode = MODE_APPENDBINARY;
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
    fs->state = STATE_OPEN;

    fs->size_type = TYPE_BYTE;
    return fs;
}

/* fs_close: close the filestream and clean up al ittle
*/

BOOL fs_close( FILESTREAM *fs, BOOL cleanup )
{
    if ( fs == NULL )
    {
        return FALSE;
    }

    if ( !fs->is_open && !cleanup )
    {
        return TRUE; // Already closed.
    }
    else
    {
        if ( !fs->is_open && cleanup )
        {
            // If we're cleaning up, we should free the stream.
            return _free_fs_stream( fs, __LINE__, __FILE__ );
        }
    }

    if ( fclose( fs->file ) != 0 )
    {
        if ( cleanup )
        {
            _free_fs_stream( fs, __LINE__, __FILE__ ); // If we can't close it, we should still free it if cleanup is requested.
            LOG( "Unable to free file stream!" );
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
        return _free_fs_stream( fs, __LINE__, __FILE__ );
    }

    fs->is_open = FALSE;
    fs->eof = TRUE;
    fs->state = STATE_CLOSING;
    fs->last_error = 0;
    return TRUE;
}

/* logfs will report the structure to the logfile for debug purposes.*/
void logfs( FILESTREAM *fs )
{
    static char buf[ 2048 * 2 ];

    if ( !fs )
        return; // Need a valid struct.
    if ( fs->is_open != TRUE )
        return;
    if ( !fs->file )
        return;
    buf[ 0 ] = '\0';
    LOG( "-------------------------------------------------------------" );
    LOG( "FileStream Structure:" );
    LOG( "-------------------------------------------------------------" );
    sprintf( buf,

             "File Path     : %25s\n"
             "File Name     : %25s\n"
             "Buf Allocated : %23.2f%s\n"
             "Buffer Length : %25zu\n"
             "Pos           : %25zu\n"
             "Bytes Read    : %25zu\n"
             "File Size     : %19zu bytes\n"
             "Human Size    : %22.2f %s\n"
             "Seek Position : %25zu\n"
             "Opened        : %25s\n"
             "Modified      : %25s\n"
             "Accessed      : %25zu\n"
             "File pointer  : %25p\n"
             "Eof           : %25s\n",
             fs->file_path,
             fs->file_name,
             double_to_human( fs->buffer ? fs->size : 0 ),
             size_type_from_len( fs->size ),
             fs->length,
             fs->pos,
             fs->file_read,
             fs->file_size,
             fs->hr_size,
             size_type_to_string( fs->size_type ),
             fs->seek_pos,
             fs->is_open ? "TRUE" : "FALSE",
             fs->modified ? "TRUE" : "FALSE",
             fs->accessed,
             fs->file,
             fs->eof ? "TRUE" : "FALSE"
    );
    LOG( buf );
    LOG( "-------------------------------------------------------------\n\n" );
}

enum token_type token_from_char( char c )
{
    switch ( c )
    {
        case '<':
            return TOKEN_OPEN_LT;
        case '>':
            return TOKEN_CLOSE_GT;
        case '{':
            return TOKEN_OPEN_BK;
        case '}':
            return TOKEN_CLOSE_BK;
        case '(':
            return TOKEN_OPEN_PR;
        case ')':
            return TOKEN_CLOSE_PR;
        case '[':
            return TOKEN_OPEN_SQBK;
        case ']':
            return TOKEN_CLOSE_SQBK;
        case '?':
            return TOKEN_QUEST;
        case '!':
            return TOKEN_EXCLAMATION;
        case ':':
            return TOKEN_COLON;
        case ';':
            return TOKEN_SEMI;
        case ',':
            return TOKEN_COMMA;
        case '.':
            return TOKEN_DOT;
        case '|':
            return TOKEN_PIPE;
        case '&':
            return TOKEN_AMPERSAND;
        case '#':
            return TOKEN_HASH;
        case '$':
            return TOKEN_DOLLAR;
        case '%':
            return TOKEN_PERCENT;
        case '^':
            return TOKEN_CARET;
        case '~':
            return TOKEN_TILDE;
        case '`':
            return TOKEN_BACK_TICK;
        case '\'':
            return TOKEN_SQUOTE;
        case '\"':
            return TOKEN_QUOTE;
        case '\\':
            return TOKEN_BACKSLASH;
        case '=':
            return TOKEN_EQUALS;
        case '-':
            return TOKEN_DASH;
        case '+':
            return TOKEN_PLUS;
        case '/':
            return TOKEN_FSLASH;

        default:
            if ( ( c >= '0' && c <= '9' ) || c == '-' )
                return TOKEN_NUMBER; // Numbers can be negative.
            if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || c == '_' )
                return TOKEN_STRING; // Strings can start with _ as well.
            return TOKEN_NONE; // Unknown token.
    }
}

/* fs_read: read from the filestream into the buffer.
 * Returns number of bytes read, or negative error code on failure.
 * This function will read up to size bytes from the file into the buffer.
 * It will update the pos, length, and file_read fields of the FILESTREAM.
 * It will also set the eof field if the end of the file is reached.
 * The buffer is null-terminated after reading.
 * The buffer is resized if necessary to hold the data.
 * The function will return the number of bytes read, or a negative error code on failure.
 * If the buffer is NULL, it will be allocated with a default size.
 * If size is 0, it will read until the end of the file.
 */
long long int fs_read( FILESTREAM *fs, size_t size )
{
    size_t i;
    time_t start, stop, total;

    start = time( NULL );

    if ( fs == NULL || !fs->is_open || fs->file == NULL )
    {
        return STREAM_ERROR; // Changed from -1 to STREAM_ERROR constant. Invalid stream.
    }
    if ( fs->mode != MODE_READ && fs->mode != MODE_READWRITE && fs->mode != MODE_BINARY )
    {
        fs->last_error = STREAM_ERROR;
        return STREAM_ERROR; // Changed from -1 to STREAM_ERROR constant. Stream not opened in read mode.
    }
    if ( size == 0 )
    {
        size = fs->file_size; // Read the whole file if size is 0.
    }
    if ( fs->buffer == NULL )
    {
        fs->buffer = ( char * )malloc( BUFFER_INIT );
        if ( fs->buffer == NULL )
        {
            GiveError( "ERROR!!_", FALSE );
            fs->last_error = MEMORY_ERROR;
            return MEMORY_ERROR; // Changed from -1 to MEMORY_ERROR constant. Memory allocation failed.
        }
        fs->size = BUFFER_INIT;
    }
    // Resize the buffer if necessary
    if ( size >= fs->size )
    {
        char *new_buffer = ( char * )realloc( fs->buffer, size + 1 ); // +1 for null terminator
        if ( new_buffer == NULL )
        {
            GiveError( "ERROR", FALSE );

            fs->last_error = MEMORY_ERROR;
            return MEMORY_ERROR; // Changed from -1 to MEMORY_ERROR constant. Memory allocation failed.
        }
        fs->buffer = new_buffer;
        fs->size = size + 1;
    }
    size_t bytes_to_read = size;
    if ( fs->pos + bytes_to_read > fs->file_size )
    {
        bytes_to_read = fs->file_size - fs->pos; // Adjust to not read past EOF.
    }
    size_t bytes_read = fread( fs->buffer + fs->pos, 1, bytes_to_read, fs->file );

    fs->pos = fs->pos + bytes_read;

    if ( bytes_read < bytes_to_read )
    {
        if ( ferror( fs->file ) )
        {
            logfs( fs );
            LOG( "ERRORNO: %d", errno );
            GiveError( "ERROR_", FALSE );
            fs->last_error = STREAM_ERROR;
            return STREAM_ERROR; // Changed from -1 to STREAM_ERROR constant. Read error.
        }
        if ( feof( fs->file ) )
        {
            fs->eof = TRUE; // End of file reached.
        }
    }
    stop = time( NULL );
    total = stop - start;

    //LOG( "Time taken to read: %zu seconds\n", total );
    start = time( NULL );

    fs->total_lines = 0;
    for ( i = 0; i < fs->pos; i++ )
    {
        if ( fs->buffer[ i ] == '\n' || fs->buffer[ i ] == '\r' )
        {
            fs->total_lines++;
        }
    }

    stop = time( NULL );
    total = stop - start;
    //LOG( "Time taken to count lines: %zu seconds\n", total );

    fs->length += bytes_read;
    fs->file_read += bytes_read;
    fs->buffer[ 250 ] = '\0'; // Null-terminate the buffer.
    return ( long long  int )bytes_read; // Return as int to preserve negative error codes when called.
}

size_t get_total_system_memory( void )
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof( statex );
    GlobalMemoryStatusEx( &statex );
    return ( size_t )statex.ullTotalPhys;
}

size_t get_used_system_memory( void )
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof( statex );
    GlobalMemoryStatusEx( &statex );
    return ( size_t )( statex.ullTotalPhys - statex.ullAvailPhys );
}

size_t get_free_system_memory( void )
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof( statex );
    GlobalMemoryStatusEx( &statex );
    return ( size_t )statex.ullAvailPhys;
}

TOKEN *new_token( void )
{
    TOKEN *token = ( TOKEN * )malloc( sizeof( TOKEN ) );
    if ( token == NULL )
    {
        return NULL; // Memory allocation failed.
    }
    token->type = TOKEN_NONE;
    token->value = NULL;
    token->len = 0;
    token->last_token = NULL;
    token->next_token = NULL;
    return token;
}

BOOL free_token( TOKEN *token )
{
    if ( token == NULL )
    {
        return FALSE;
    }
    if ( token->value )
    {
        free( token->value );
    }
    if ( token->last_token )
        token->last_token = NULL;
    if ( token->next_token )
        token->next_token = NULL;
    token->point = 0;
    token->type = TOKEN_NONE;
    token->size = 0;
    token->len = 0;
    free( token );
    return TRUE;
}

/* All of these token functions (ie. pop push reroll,etc)
   take a FILESTREAM pointer as input and return a size_t value.
   it uses the fs->buffer as its input, returning a bytesread as
   a return call. Unless specified otherwise.
   */
// WARNING: This is not very performant. Use with caution.
// We've likely found an incomplete token, so we need to go back to the last valid one.
// typically from a chunk read leaving a 'string' unfinished. So we'll pop back and
// hope the parser reads more into the buffer and tries again.
size_t reroll_to_last_valid_token( FILESTREAM *fs )
{
    if ( fs == NULL )
    {
        return REROLL_BADFS;
    }
    if ( fs->token_count == 0 )
    {
        return 0; // No tokens to reroll.
    }
    fs->cur_token = fs->last_token;
    return 1;
}

size_t push_token( FILESTREAM *fs, enum token_type token )
{
    if ( fs == NULL )
    {
        return TOKENPUSH_BADFS;
    }
    if ( fs->token_count >= MAX_TOKEN )
    {
        return TOKENPUSH_HIGHTOKEN;
    }
    fs->tokens[ fs->token_count ].type = token;
    fs->token_count++;
    return 1;
}

size_t pop_token( FILESTREAM *fs )
{
    if ( fs == NULL )
    {
        return POP_BADFS;
    }
    if ( fs->token_count == 0 )
    {
        return 0; // No tokens to pop.
    }
    fs->token_count--;
    return 1;
}