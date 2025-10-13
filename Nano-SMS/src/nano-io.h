/* Nano-SMS: My interpretation and creation of an app to read SMS BACKUP AND RESTORE files.
   All other apps I have used, have been web based, and cannot process large files well, if at
   at all (All my tests have resulted in crashes. Even though I have 96gB of memory avail)

   This program is obviously windows based. I plan, if it works, is to make a linux/mac
   version that is command line based, that will spit out the items you're looking for into
   an html file, splitting to keep size issues from cropping up. THat is for another day,
   and may even be put into this program as well as cross platform.


    This program is not meant to be a full replacement for the SMS Backup and Restore app.
    It is meant to be a tool to read the files, and extract the data you want, and then
    save it to a file, or copy it to the clipboard, or whatever you want to do with it.

    I do not maintain. nor do I claim to maintain, the SMS Backup and Restore app.
    This program is not affiliated with the SMS Backup and Restore app in any way.
    It is simply a tool to read the files created by the SMS Backup and Restore app.

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
#pragma once
#include <stdio.h>
#include <stdlib.h>
#define BOOL int




#define BUFFER_GROW (4096 * 1024) // Let's grow the stream buffer by 4mb at a time.
#define BUFFER_INIT (1024 * 1024) // Start with a 1mb buffer.
// returns from the streaming functions
#define STREAM_EOF (-1) // End of file indicator.
#define STREAM_ERROR (-2) // Error indicator.
#define STREAM_UNKNOWN (-3) // Unknown error indicator.
#define MEMORY_ERROR (-4) // Memory error indicator.

// we're going to have a 'scratch' buffer so we're not using too much
// stack space. Trying to be cautious of memory use, but also
// using as much as we can get away with to stream properly.
#define STREAM_SCRATCH (8192) // 8kB scratch buffer for reading.



typedef struct filestream FILESTREAM;

#define HAS_ERROR (x) ((x) ? (x->last_error == STREAM_EOF ||          \
                              x->last_error == STREAM_ERROR ||      \
                              x->last_error == STREAM_UNKNOWN) : FALSE)

/* These macros are just going to make streaming code a little easier on the eyes*/
#define STREAM_OPENED(x) ((x) ? (x->is_open == TRUE) : FALSE)
#define STREAM_IS_EOF(x) ((x) ? (x->eof == TRUE) : FALSE)
#define STREAM_CAN_READ(x) ((x) ? (x->is_open == TRUE && x->eof == FALSE && x->last_error == 0) : FALSE)
#define STREAM_CAN_WRITE(x) ((x) ? (x->is_open == TRUE && x->eof == FALSE && x->last_error == 0) : FALSE)
#define STREAM_CAN_SEEK(x) ((x) ? (x->is_open == TRUE && x->last_error == 0) : FALSE)
#define STREAM_POS(x) ((x) ? (x->pos > 0 ? x->pos : 0) : -1)
#define STREAM_BUF_SIZE(x) ((x) ? (x->size > 0 ? x->size : 0) : -1)
#define STREAM_BUF_LEN(x) ((x) ? (x->length > 0 ? x->length : 0) : -1)
#define STREAM_BYTES_READ(x) ((x) ? (x->file_read > 0 ? x->file_read : 0) : -1)
#define STREAM_FILE_SIZE(x) ((x) ? (x->file_size > 0 ? x->file_size : 0) : -1)
#define STREAM_STATE(x) ((x) ? (x->state) : STREAM_ERROR)
#define STREAM_MODE(x) ((x) ? (x->mode) : STREAM_ERROR)
#define STREAM_LAST_ERROR(x) ((x) ? x->last_error : 0)
#define STREAM_LAST_TOKEN(x) ((x) ? x->last_token : 0)
#define STREAM_CUR_TOKEN(x) ((x) ? (x->cur_token) : 0)
#define STREAM_SEEK_POS(x) ((x) ? (x->seek_pos > 0 ? x->seek_pos : 0) : -1)

// These are for error purposes and logging. They're just file wrappers.
#define new_fs_stream() _new_fs_stream(__LINE__, __FILE__)
#define free_fs_stream(x) _free_fs_stream(x, __LINE__, __FILE__)




enum token_type
{
    TOKEN_NONE,
    TOKEN_OPEN_LT, /* Less-than < */
    TOKEN_CLOSE_GT, /* Greater-than > */
    TOKEN_OPEN_BK, /* Brackets {}*/
    TOKEN_CLOSE_BK, /* Brackets {}*/
    TOKEN_OPEN_PR, /* Parenthesis ()*/
    TOKEN_CLOSE_PR, /* Parenthesis ()*/
    TOKEN_OPEN_SQBK, /* Square brackets []*/
    TOKEN_CLOSE_SQBK, /* Square brackets []*/
    TOKEN_QUEST, /* Question mark ? */
    TOKEN_EXCLAMATION, /* Exclamation ! */
    TOKEN_COLON, /* Colon : */
    TOKEN_SEMI, /* Semicolon ; */
    TOKEN_COMMA, /* Comma , */
    TOKEN_DOT, /* Dot . */
    TOKEN_PIPE, /* Pipe | */
    TOKEN_AMPERSAND, /* Ampersand & */
    TOKEN_HASH, /* Hash # */
    TOKEN_DOLLAR, /* Dollar sign $ */
    TOKEN_PERCENT, /* Percent % */
    TOKEN_CARET, /* Caret ^ */
    TOKEN_TILDE, /* Tilde ~ */
    TOKEN_BACK_TICK, /* Backtick ` */
    TOKEN_SQUOTE, /* Single quote ' */
    TOKEN_QUOTE, /* Double quote " */
    TOKEN_BACKSLASH, /* Backslash \ */
    TOKEN_EQUALS, /* Equals = */
    TOKEN_DASH, /* Dash - */
    TOKEN_PLUS, /* Plus + */
    TOKEN_FSLASH, /* Forward slash / */
    TOKEN_STRING, /* String, value, variable, etc; not literal */
    TOKEN_NUMBER, /* Number */
    TOKEN_LITERAL, /* String Literal: aka: 'string' or "string" */
    TOKEN_NULL /* Null */
};



// Data structures below are for our own special file streaming for
// our xml (or anything else) parsing. We are diferent than other SMS 
// readers,because all of them break on big files. Ours will read them
// and potentially stream them from disk, if ram is an issue. My personal
// file is 18GB in size.

struct filestream
{
    char* buffer;        // The buffer to hold the file data. Allocated dynamically, used for streaming file contents.
    char file_path[_MAX_PATH]; // The path to the file. Stores the full path for reference and operations.
    char file_name[_MAX_FNAME]; // The name of the file. Used for display/logging and file management.
    size_t size;         // The size of the buffer. Indicates the total allocated memory for 'buffer'.
    size_t length;       // The length of the data in the buffer. Actual data size currently held.
    size_t pos;          // The current position in the buffer. Used for reading/writing operations.
    size_t file_read;    // The number of bytes read from the file. Useful for progress tracking.
    size_t file_size;    // The size of the file. Used for validation and progress indication.
    double hr_size;      // The human readable size of the file. Used for display purposes.
    size_t seek_pos;     // The position that has been seeked to. Used for random access within the file.
    time_t opened;       // The time the file was opened. For logging and auditing purposes.
    time_t modified;     // The time the file was last modified. Useful for file change detection.
    time_t accessed;     // The time the file was last accessed. For tracking usage and access patterns.
    FILE *file;          // The file pointer. Standard C FILE* used for file I/O operations.
    int last_token;      // The last token read from the file. Used in parsing routines.
    int cur_token;       // The current token being processed. For stateful parsing.
    int last_error;      // Error flag. Stores the last error code encountered.
    size_t total_lines;     // Internal number just for ease of parsing.
    enum {
        STATE_READY,     // Stream is ready for operations.
        STATE_ANALYZE,   // Stream is being analyzed (e.g., format detection).
        STATE_OPEN,      // Stream is open.
        STATE_READING,   // Stream is currently reading.
        STATE_WRITING,   // Stream is currently writing.
        STATE_CLOSING,   // Stream is closing.
        STATE_ERRORS,    // Stream is in error state.
        STATE_SEEKING    // Stream is seeking to a position.
    } state;             // The current state of the stream. Used for managing stream lifecycle.
    enum 
    {
        MODE_READ,       // File opened for reading.
        MODE_WRITE,      // File opened for writing.
        MODE_APPEND,     // File opened for appending.
        MODE_READWRITE,  // File opened for both reading and writing.
        MODE_BINARY,     // File opened in binary mode.
        MODE_ERRORS,     // Error mode.
        MODE_NONE        // No mode set.
    } mode;              // The mode the file was opened in. Used for access control.
    enum 
    {
        TYPE_BYTE,      // Size in bytes.
        TYPE_KILOBYTE,  // Size in kilobytes.
        TYPE_MEGABYTE,  // Size in megabytes.
        TYPE_GIGABYTE,   // Size in gigabytes.
        TYPE_TERABYTE // Size in terabytes.
    } size_type;
    BOOL eof;           // End of file flag. TRUE if end of file reached.
    BOOL is_open;       // File open flag. TRUE if file is currently open.
 };

// Public facing functions
BOOL unregister_all_streams( void );
FILESTREAM* _fs_find_handle( char* path );
BOOL _register_fs_handle( FILESTREAM* fs );
BOOL _unregister_fs_handle( FILESTREAM* fs );
FILESTREAM* _new_fs_stream( int line, char* file );
BOOL _free_fs_stream( FILESTREAM* fs, int line, char* file );
void logfs( FILESTREAM* fs );
FILESTREAM* fs_open( FILESTREAM* fs, const char* path, const char* mode );
BOOL fs_close( FILESTREAM* fs, BOOL cleanup );
void LOG( char* fmt, ... );
char* size_type_to_string( int type );
char* size_type_from_len( size_t len );
double double_to_human( size_t hr_size );
size_t fs_read( FILESTREAM* fs, size_t size );
BOOL _init_fs( void );