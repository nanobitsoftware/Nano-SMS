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






#define BUFFER_GROW (4096 * 1024) // Let's grow the stream buffer by 4mb at a time.
#define BUFFER_INIT (1024 * 1024) // Start with a 1mb buffer.
// returns from the streaming functions
#define STREAM_EOF (-1) // End of file indicator.
#define STREAM_ERROR (-2) // Error indicator.
#define STREAM_UNKNOWN (-3) // Unknown error indicator.
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


// Data structures below are for our own special file streaming for
// our xml (or anything else) parsing. We are diferent than other SMS 
// readers,because all of them break on big files. Ours will read them.

struct filestream
{
    
    char* buffer;        // The buffer to hold the file data.
    char file_path[1024];// The path to the file. 256 is stock, but what if large is enabled?
    char file_name[256]; // The name of the file.
    size_t size;         // The size of the buffer.
    size_t length;       // The length of the data in the buffer.
    size_t pos;          // The current position in the buffer.
    size_t file_read;    // The number of bytes read from the file.
    size_t file_size;    // The size of the file.
    size_t seek_pos;     // The position that has been seeked to.
    time_t opened;       // The time the file was opened.
    time_t modified;     // The time the file was last modified.
    time_t accessed;     // The time the file was last accessed.
    FILE *file;          // The file pointer.
    int last_token;      // The last token read from the file.
    int cur_token;       // The current token being processed.
    int last_error;      // Error flag.
    enum {
        STATE_READY,
        STATE_ANALYZE,
        STATE_OPEN,
        STATE_READING,
        STATE_WRITING,
        STATE_CLOSING,
        STATE_ERRORS,
        STATE_SEEKING
    } state;             // The current state of the stream.
    enum 
    {

        MODE_READ,
        MODE_WRITE,
        MODE_APPEND,
        MODE_READWRITE,
        MODE_BINARY,
        MODE_ERRORS,
        MODE_NONE

    } mode;              // The mode the file was opened in.
    BOOL eof;            // End of file flag.
    BOOL is_open;        // File open flag.
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