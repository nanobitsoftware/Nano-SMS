

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

/**
 * @file nano-io.h
 * @brief Lightweight streaming helper types and prototypes used by Nano-SMS.
 *
 * This header defines a simple FILESTREAM abstraction that supports buffered
 * streaming of very large files, minimizing RAM usage by growing buffers in
 * large increments and optionally streaming data from disk.
 *
 * The header also defines token types used by higher-level parsers and a set
 * of public function prototypes for creating, opening, reading and closing
 * streams.
 */

/** Grow buffer increment (4 MiB). Used when expanding the dynamic buffer. */
#define BUFFER_GROW (4096 * 1024) // Let's grow the stream buffer by 4mb at a time.

/** Initial buffer allocation size (1 MiB). */
#define BUFFER_INIT (1024 * 1024) // Start with a 1mb buffer.

/**
 * Return codes used by streaming functions.
 * These are negative sentinel values returned by operations to indicate
 * special conditions.
 */
#define STREAM_EOF (-1)     /**< End of file indicator. */
#define STREAM_ERROR (-2)   /**< Generic error indicator. */
#define STREAM_UNKNOWN (-3) /**< Unknown error indicator. */
#define MEMORY_ERROR (-4)   /**< Memory allocation failure indicator. */

/**
 * Scratch buffer used for temporary reads. Size measured in bytes.
 * Keeps stack usage modest by using a defined scratch buffer size.
 */
#define STREAM_SCRATCH (8192) // 8kB scratch buffer for reading.

typedef struct filestream FILESTREAM; /**< Forward typedef for the file stream struct. */

/**
 * Convenience macro to test if a stream has an error state.
 * Evaluates to a boolean-like expression (TRUE/FALSE).
 *
 * @param x Pointer to FILESTREAM.
 */
#define HAS_ERROR (x) ((x) ? (x->last_error == STREAM_EOF ||          \
                              x->last_error == STREAM_ERROR ||      \
                              x->last_error == STREAM_UNKNOWN) : FALSE)

/* These macros are just going to make streaming code a little easier on the eyes*/

/**
 * Check whether stream is opened.
 * Returns TRUE if stream pointer is non-null and is_open==TRUE.
 */
#define STREAM_OPENED(x) ((x) ? (x->is_open == TRUE) : FALSE)

/**
 * Check whether end-of-file has been reached on the stream.
 */
#define STREAM_IS_EOF(x) ((x) ? (x->eof == TRUE) : FALSE)

/**
 * Returns TRUE if the stream is open, not at EOF, and no last error is set.
 * Useful to test whether read operations are safe to perform.
 */
#define STREAM_CAN_READ(x) ((x) ? (x->is_open == TRUE && x->eof == FALSE && x->last_error == 0) : FALSE)

/**
 * Returns TRUE if the stream is open, not at EOF, and no last error is set.
 * Useful to test whether write operations are safe to perform.
 */
#define STREAM_CAN_WRITE(x) ((x) ? (x->is_open == TRUE && x->eof == FALSE && x->last_error == 0) : FALSE)

/**
 * Returns TRUE if the stream supports seeking (open and no last error).
 */
#define STREAM_CAN_SEEK(x) ((x) ? (x->is_open == TRUE && x->last_error == 0) : FALSE)

/** Current stream position (non-negative). */
#define STREAM_POS(x) ((x) ? (x->pos > 0 ? x->pos : 0) : -1)

/** Allocated buffer size for the stream. */
#define STREAM_BUF_SIZE(x) ((x) ? (x->size > 0 ? x->size : 0) : -1)

/** Current length of valid data in the stream buffer. */
#define STREAM_BUF_LEN(x) ((x) ? (x->length > 0 ? x->length : 0) : -1)

/** Total bytes read from the underlying file (progress metric). */
#define STREAM_BYTES_READ(x) ((x) ? (x->file_read > 0 ? x->file_read : 0) : -1)

/** Size of the underlying file, when known. */
#define STREAM_FILE_SIZE(x) ((x) ? (x->file_size > 0 ? x->file_size : 0) : -1)

/** Current state enum value of the stream. */
#define STREAM_STATE(x) ((x) ? (x->state) : STREAM_ERROR)

/** Current mode enum value of the stream. */
#define STREAM_MODE(x) ((x) ? (x->mode) : STREAM_ERROR)

/** Last error code recorded on the stream. */
#define STREAM_LAST_ERROR(x) ((x) ? x->last_error : 0)

/** Last token parsed from the stream. */
#define STREAM_LAST_TOKEN(x) ((x) ? x->last_token : 0)

/** Current token being processed. */
#define STREAM_CUR_TOKEN(x) ((x) ? (x->cur_token) : 0)

/** Last explicit seek position requested on the stream. */
#define STREAM_SEEK_POS(x) ((x) ? (x->seek_pos > 0 ? x->seek_pos : 0) : -1)

/** Stream mode used to iterate through a stream as chunks (vs full file). */
#define STREAM_AS_CHUNKS 1
/** Stream mode used to treat the whole file as loaded. */
#define STREAM_AS_FILE 2
/** Stream mode used to indicate an error while determining mode. */
#define STREAM_AS_ERROR 0

/**
 * Allocation wrappers that capture the source location for diagnostics.
 * new_fs_stream() expands to _new_fs_stream(__LINE__, __FILE__).
 */
#define new_fs_stream() _new_fs_stream(__LINE__, __FILE__)
#define free_fs_stream(x) _free_fs_stream(x, __LINE__, __FILE__)

/**
 * Allowed RAM usage percentage used by streaming heuristics.
 * Value is an integer representing percent of total system memory.
 */
static size_t ALLOWED_RAM_USAGE = 40; // This is in percentage of total system memory

/**
 * Token types used by the file parsing/tokenizing utilities.
 * Each enumerant represents a syntactic token encountered while scanning.
 */
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

/**
 * filestream - in-memory buffer and metadata used for streaming large files.
 *
 * Fields:
 *  - buffer: dynamically allocated buffer containing the data currently loaded.
 *  - file_path, file_name: stored path and filename (null-terminated char arrays).
 *  - size: allocated capacity of 'buffer' in bytes.
 *  - length: number of valid data bytes currently inside 'buffer'.
 *  - pos: current cursor offset inside 'buffer' where next read/write will occur.
 *  - file_read: total bytes read from the backing file so far (monotonic).
 *  - file_size: size of the backing file (if known).
 *  - hr_size: human-readable size representation (double), used for UI display.
 *  - seek_pos: last explicitly requested seek position (for bookkeeping).
 *  - opened/modified/accessed: timestamp values for file metadata.
 *  - file: underlying C FILE* handle when the stream is backed by a real file.
 *  - last_token/cur_token: token indices used by parsers operating on the stream.
 *  - last_error: last error code encountered (0 if none).
 *  - total_lines: helper counter used by parsers to track line numbers.
 *  - state: internal state machine for the stream lifecycle.
 *  - mode: logical open mode of the stream (read/write/append/etc).
 *  - size_type: human unit for displaying sizes (bytes/kb/mb/...).
 *  - eof/is_open: boolean flags for EOF and open state.
 */
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
    FILE* file;          // The file pointer. Standard C FILE* used for file I/O operations.
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
        MODE_NONE,       // No mode set.
        MODE_READBINARY,  // File opened for reading in binary mode.
        MODE_WRITEBINARY, // File opened for writing in binary mode.
        MODE_APPENDBINARY, // File opened for appending in binary mode.
        MODE_RWBINARY      // File opened for reading and writing in binary mode.
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

/* Compatibility macro definitions mirroring the internal mode enum.
   These values are used in higher-level code and map to internal mode
   semantics defined in the filestream struct above. */
#define MODE_READ 0
#define MODE_WRITE 1
#define MODE_APPENT 2
#define MODE_READWRITE 3
#define MODE_BINARY 4
#define MODE_ERROR 5
#define MODE_NONE 6
#define MODE_READBINARY 7
#define MODE_WRITEBINARY 8
#define MODE_APPENDBINARY 9
#define MODE_RWBINARY 10

/* Public facing functions
   Each function below is provided by the implementation (.c/.cpp) and
   operates on FILESTREAM instances. Brief documentation is provided for
   usage and expected behavior. */

/**
 * Unregister and free all registered streams.
 *
 * @return TRUE on success, FALSE on failure.
 */
BOOL unregister_all_streams( void );

/**
 * Find a registered FILESTREAM by path.
 *
 * @param path Null-terminated path to search for.
 * @return Pointer to FILESTREAM if found, otherwise NULL.
 */
FILESTREAM* _fs_find_handle( char* path );

/**
 * Register a FILESTREAM handle in the global registry.
 *
 * @param fs Pointer to FILESTREAM to register.
 * @return TRUE on success, FALSE otherwise.
 */
BOOL _register_fs_handle( FILESTREAM* fs );

/**
 * Unregister a previously registered FILESTREAM handle.
 *
 * @param fs Pointer to FILESTREAM to unregister.
 * @return TRUE on success, FALSE otherwise.
 */
BOOL _unregister_fs_handle( FILESTREAM* fs );

/**
 * Allocate and initialize a new FILESTREAM and record the allocation site.
 *
 * @param line Source line where allocation is requested (for diagnostics).
 * @param file Source filename where allocation is requested (for diagnostics).
 * @return Pointer to a newly allocated FILESTREAM, or NULL on failure.
 */
FILESTREAM* _new_fs_stream( int line, char* file );

/**
 * Free a FILESTREAM and optionally log the freeing site.
 *
 * @param fs Pointer to FILESTREAM to free.
 * @param line Source line where free is requested.
 * @param file Source filename where free is requested.
 * @return TRUE on success, FALSE on failure.
 */
BOOL _free_fs_stream( FILESTREAM* fs, int line, char* file );

/**
 * Write debug/log information for a FILESTREAM to configured output.
 *
 * @param fs Pointer to stream to log.
 */
void logfs( FILESTREAM* fs );

/**
 * Open a stream for the given path and mode. If 'fs' is non-null it will
 * be used as the target object, otherwise a new stream will be allocated.
 *
 * @param fs Optional preallocated FILESTREAM pointer to initialize.
 * @param path Path to the file to open.
 * @param mode fopen-style mode string (e.g., "rb").
 * @return Pointer to an opened FILESTREAM or NULL on error.
 */
FILESTREAM* fs_open( FILESTREAM* fs, const char* path, const char* mode );

/**
 * Close a FILESTREAM and optionally perform additional cleanup.
 *
 * @param fs Pointer to FILESTREAM to close.
 * @param cleanup If TRUE perform full cleanup and free resources.
 * @return TRUE on success, FALSE on failure.
 */
BOOL fs_close( FILESTREAM* fs, BOOL cleanup );

/**
 * Simple variadic logging helper used throughout the project.
 *
 * @param fmt printf-style format string followed by arguments.
 */
void LOG( char* fmt, ... );

/**
 * Convert size_type enum value to a human-friendly string.
 *
 * @param type The size type enum value.
 * @return Pointer to a static string describing the type (not owned).
 */
char* size_type_to_string( int type );

/**
 * Choose an appropriate size_type from a given length in bytes.
 *
 * @param len Length in bytes.
 * @return Pointer to a static string representation.
 */
char* size_type_from_len( size_t len );

/**
 * Convert a raw size value into a human-readable floating value (e.g., MB).
 *
 * @param hr_size Size in bytes.
 * @return Human readable double (units depend on caller's use).
 */
double double_to_human( size_t hr_size );

/**
 * Read up to `size` bytes from the stream into the stream's buffer.
 * The implementation is responsible for expanding the buffer when necessary.
 *
 * @param fs Pointer to FILESTREAM to read from.
 * @param size Number of bytes to attempt to read.
 * @return Number of bytes actually read, or STREAM_EOF/STREAM_ERROR on failure.
 */
size_t fs_read( FILESTREAM* fs, size_t size );

/**
 * Initialize stream subsystem resources. Should be called once at startup.
 *
 * @return TRUE on success, FALSE on failure.
 */
BOOL _init_fs( void );

/**
 * Query system memory utilities used by streaming heuristics.
 * These functions return sizes in bytes.
 */
size_t get_total_system_memory( void );
size_t get_used_system_memory( void );
size_t get_free_system_memory( void );