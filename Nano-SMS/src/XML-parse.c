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


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <sys/types.h>
#include <time.h>


#include "nano-sms.h"
#include "XML.h"
#include "sms_db.h" 
#include "nano-io.h"
#include "sqlite3/sqlite3.h"



// I know this is a lot of 'global' variables, but this is a simple
// parser, and I want to keep it simple. So deal with it.
// None of these will be used outside of this file.
// They are all static to this file only.
// I could make this a struct, but that would be more work
// than it's worth for this simple parser.

FILESTREAM *xml_stream = NULL; // The file stream to read the XML data from.


static long int xml_line_number = 0; // Keep track of the line number for error reporting.
static long int xml_char_number = 0; // Keep track of the character number for error reporting.
static long int xml_nest_number = 0;
static long int xml_leftparen = 0;
static long int xml_rightparen = 0;
static long int xml_lt_number = 0;
static long int xml_gt_number = 0;
static long int xml_slash_number = 0;
static long int xml_bslash_number = 0;
static long int xml_equal_number = 0;
static long int xml_quote_number = 0;
static long int xml_squote_number = 0;
static long int xml_dquote_number = 0;
static BOOL xml_in_tag = FALSE;
static BOOL xml_in_attr = FALSE;
static BOOL xml_in_value = FALSE;
static BOOL xml_in_comment = FALSE;
static BOOL xml_in_cdata = FALSE;
static BOOL xml_in_declaration = FALSE;
static BOOL xml_in_doctype = FALSE;
static BOOL xml_in_entity = FALSE;
static BOOL xml_in_processing = FALSE;
static char *xml_current_tag = NULL; // Dynamically allocated. Grown; never shrunken.
static int xml_tag_malloc_add = 256 * sizeof( char ); // Allocate in chunks of 256 bytes. For all buffers.
static char *xml_attr = NULL; // Dynamically allocated. Grown; never shrunken.
static char *xml_value = NULL; // Dynamically allocated. Grown; never shrunken.
static char *xml_comment = NULL; // Dynamically allocated. Grown; never shrunken.
static char *xml_cdata = NULL; // Dynamically allocated. Grown; never shrunken.
static char *xml_declaration = NULL; // Dynamically allocated. Grown; never shrunken.
static char *xml_doctype = NULL; // Dynamically allocated. Grown; never shrunken.
static char *xml_entity = NULL; // Dynamically allocated. Grown; never shrunken.
static char *xml_processing = NULL; // Dynamically allocated. Grown; never shrunken.
static char *xml_data = NULL; // The XML data to parse.
static size_t xml_data_len = 0; // The length of the XML data.
static size_t xml_data_pos = 0; // The current position in the XML data.
static size_t xml_tag_len = 0;  // Length of tag buffer.
static size_t xml_attr_len = 0; // Length of attr buffer.
static size_t xml_value_len = 0; // Length of value buffer.
static size_t xml_comment_len = 0; // Length of comment buffer.
static size_t xml_cdata_len = 0; // Length of cdata buffer.
static size_t xml_declaration_len = 0; // Length of declaration buffer.
static size_t xml_doctype_len = 0; // Length of doctype buffer.
static size_t xml_entity_len = 0; // Length of entity buffer.
static size_t xml_processing_len = 0; // Length of processing buffer.

#define MAX_TOKEN 2048 // Max tokens allowed in any given 'line'

struct lex
{
    enum token_type type; // Define in nano-io.h
    char *value; // Pointer to a char of our value.
    char *point; // Pointer to the position in the data where this token was found.
    size_t size; // Size o the 'value'
    int  str_start; // Starting index in the data. Only returned when we're in a string literal.
    int  str_end; // Ending index in the data.

};

struct lex xml_tokens[ MAX_TOKEN ];
static size_t xml_token_count = 0;




/*
  msg_box: 1=inbox, 2=sent, 3=draft, 4=outbox, 5=failed, 6=queued
  ct_cls: content class, 0-255, 0=personal, 1=advertisement, 2=informational, 3=auto
  ct_t: content type, text/plain, application/vnd.wap.multipart.related, etc.
  ct_l: content location, url or filename
  m_type: message type, 128=mms, 129=mms notification, 130=mms delivery report, 131=mms read report, 132=mms send request
  pri: priority, -1=low, 0=normal, 1=high
  rr_st: read report status, 0=not requested, 1=requested, 2=received
  rr: read report, null=not requested or not received, timestamp=received
  st: status, null=none, -1=pending, 128=sending, 129=sending failed, 130=sending complete, 131=downloading, 132=download failed, 133=download complete

  parts:
    seq: sequence number, 0-n
    ct: content type, text/plain, image/jpeg, etc.
    name: name of the part, body for text, filename for attachments
    chset: charset, 0-255, 106=UTF-8, 3=US-ASCII, etc.
    cd: content disposition, null=inline, attachment=attachment
    fn: filename, null or filename
    cid: content id, null or id
    cl: content location, null or url or filename
    ctt_s: content transfer encoding size, null or size
    ctt_t: content transfer encoding type, null or type
    text: text content for text parts, null for non-text parts
    data: base64 encoded data for non-text parts, null for text parts
    sef_type: self defined type, 0=none, 1=audio, 2=video, 3=image, 4=document, 5=other
    decorate_bubble_value: null or value
    sub_id: subscription id, 1-n
    // addr types: 130=from, 137=to, 151=cc
    */
    /* Function to read and parse the XML file
    * @param filename: the name of the XML file to read
    * @param backup: pointer to the SMS_BACKUP structure to populate
    * @return: 1 on success, -1 on failure
    */


void  init_tokens( void )
{
    int i;
    for ( i = 0; i < MAX_TOKEN; i++ )
    {
        xml_tokens[ i ].type = TOKEN_NONE;
    }
}


// Read chunk of the fs into a buffer. Used for if we will use too much memory.
// Mean to be called on a loop until EOF.
/**
 * @brief Read a chunk of the file stream into a buffer
 * @param fs: pointer to the FILESTREAM structure
 * @param buffer: pointer to the buffer to fill
 * @param buffer_size: size of the buffer
 * @param chunk_size: size of the chunk to read
 * @return: the number of bytes read into the buffer
 */
size_t chunk_xml_buffer( FILESTREAM *fs, char *buffer, size_t buffer_size, size_t chunk_size )
{
    if ( !fs || buffer_size == 0 || chunk_size == 0 )
    {
        fprintf( stderr, "Invalid parameters provided to chunk_xml_buffer\n" );
        return 0;
    }


    size_t total_bytes_read = 0;
    size_t bytes_to_read = buffer_size;
    if ( buffer == NULL )
    {
        buffer = ( char * )malloc( buffer_size + 1024 );
    }
    else
    {
        buffer = ( char * )realloc( buffer, buffer_size + 1024 );
    }
    if ( !buffer )
    {
        fprintf( stderr, "Failed to allocate memory for XML buffer\n" );
        return 0;
    }

    memset( buffer, 0, buffer_size + 1024 );

    while ( total_bytes_read < buffer_size )
    {
        size_t bytes_read = fs_read( fs, chunk_size );
        if ( bytes_read == 0 )
        {
            break; // End of file reached
        }
        memcpy( buffer + total_bytes_read, fs->buffer + fs->pos - bytes_read, bytes_read );
        total_bytes_read += bytes_read;
        bytes_to_read -= bytes_read;
    }
    return total_bytes_read;
}

/** @brief Fill the XML buffer with data from the file stream
 * @param fs: pointer to the FILESTREAM structure
 * @param buffer: pointer to the buffer to fill
 * @return: the number of bytes read into the buffer
 */
size_t fill_xml_buffer( FILESTREAM *fs, char *buffer )
{

    // Figure out size of file, make a buffer to match
    size_t file_size = fs->file_size;
    if ( buffer == NULL )
    {
        buffer = ( char * )malloc( file_size + 1024 );
    }
    else
    {
        buffer = ( char * )realloc( buffer, file_size + 1024 );
    }
    if ( !buffer )
    {
        fprintf( stderr, "Failed to allocate memory for XML buffer\n" );
        return 0;
    }
    memset( buffer, 0, file_size + 1024 );
    size_t total_bytes_read = 0;
    while ( total_bytes_read < file_size )
    {
        // TODO: Add ability to send a signal to the updater so we're not 'stuck' on GUI.
        size_t bytes_read = fs_read( fs, file_size - total_bytes_read );
        if ( bytes_read == 0 )
        {
            break; // End of file reached
        }
        memcpy( buffer + total_bytes_read, fs->buffer + fs->pos - bytes_read, bytes_read );
        total_bytes_read += bytes_read;
    }
    return total_bytes_read;

}
/**
 * @brief Determine the XML read method based on the file stream and system memory
 * @param fs: pointer to the FILESTREAM structure
 * @return: the determined read method (STREAM_AS_FILE, STREAM_AS_CHUNKS, or STREAM_AS_ERROR)
 */
int determine_xml_read_method( FILESTREAM *fs )
{

    size_t system_memory;
    size_t free_system_memory;
    size_t free_percent;
    size_t used_system_memory;
    size_t filesize_delta;
    size_t free_percent_bytes;

    if ( !fs )
    {
        fprintf( stderr, "Invalid file stream provided to determine_xml_read_method\n" );
        return STREAM_AS_ERROR;
    }

    system_memory = get_total_system_memory( );
    free_system_memory = get_free_system_memory( );
    used_system_memory = get_used_system_memory( );
    free_percent = ( free_system_memory * 100 ) / system_memory;
    free_percent_bytes = ( free_system_memory * ALLOWED_RAM_USAGE ) / 100;
    filesize_delta = free_percent_bytes - fs->file_size;

    if ( LOG_PARSING_MEMORY == TRUE )
    {
        LOG( "System Memory: %14zu bytes\n"
             "Used Memory  : %14zu bytes\n"
             "Free Memory  : %14zu bytes\n"
             "File Size    : %14zu bytes\n"
             "Free Percent : %14zu %%\n",
             system_memory,
             used_system_memory,
             free_system_memory,
             fs->file_size,
             free_percent );
        LOG( "%-3zu%% Free bytes: %14zu bytes\n"
             "Filesize delta : %14zu bytes\n"
             "%zu%% of free bytes used\n",
             ALLOWED_RAM_USAGE,
             free_percent_bytes,
             filesize_delta,
             ( 100 ) - ( filesize_delta * 100 ) / free_percent_bytes
        );
    }



    // How big is our file?
    if ( fs->file_size > free_system_memory || free_percent < ALLOWED_RAM_USAGE ||
         fs->file_size >( free_system_memory * 100 ) / ALLOWED_RAM_USAGE )
    {
        // If free memory is below the allowed threshold, use chunked reading.
        return STREAM_AS_CHUNKS;
    }

    // For now, let's just read the whole file into memory.

    return STREAM_AS_FILE;
}
/**
 * @brief Begin reading the SMS data from the XML file
 * @param fs: pointer to the FILESTREAM structure
 */
void begin_read_sms_new( FILESTREAM *fs )
{
    int method;

    if ( !fs )
    {
        fprintf( stderr, "Invalid file stream provided to begin_read_sms_new\n" );
        return;
    }

    xml_stream = fs;

    // Read the entire file into memory for parsing.
    xml_data_len = fs->file_size;

    // Let's determine if we need to read it all as one file, or as in chunks.
    method = determine_xml_read_method( fs );
    fs->accessed = time( NULL );

    xml_data = ( char * )malloc( xml_data_len + 1 );
    if ( !xml_data )
    {
        fprintf( stderr, "Failed to allocate memory for XML data\n" );
        return;

    }

    memset( xml_data, 0, xml_data_len + 1 );
    size_t bytes_read = fs_read( fs, xml_data_len );
    if ( bytes_read != xml_data_len )
    {
        fprintf( stderr, "Failed to read entire XML file into memory\n" );
        free( xml_data );
        xml_data = NULL;
        return;
    }
    xml_data_pos = 0; // Start at the beginning of the data.
}


/* Function to create a new SMS_BACKUP structure
 * @return: pointer to the new SMS_BACKUP structure, or NULL on failure
 */
SMS_BACKUP *XML_new( void )
{
    SMS_BACKUP *backup = ( SMS_BACKUP * )malloc( sizeof( SMS_BACKUP ) );
    if ( !backup )
    {
        fprintf( stderr, "Failed to allocate memory for SMS_BACKUP structure\n" );
        return NULL;
    }
    memset( backup, 0, sizeof( SMS_BACKUP ) );
    return backup;
}

/* Function to grow the XML buffer
 * @param buffer: pointer to the buffer to grow
 * @param current_len: pointer to the current length of the buffer
 * @return: pointer to the grown buffer, or NULL on failure
 */
char *xml_grow_buffer( char **buffer, size_t *current_len )
{
    if ( !buffer || !current_len ) return NULL;
    size_t new_len = *current_len + xml_tag_malloc_add;
    char *new_buffer = ( char * )realloc( *buffer, new_len );
    if ( !new_buffer )
    {
        fprintf( stderr, "Failed to allocate memory for XML buffer\n" );
        return NULL;
    }
    *buffer = new_buffer;
    memset( *buffer + *current_len, 0, xml_tag_malloc_add ); // Zero out the new memory.
    *current_len = new_len;
    return *buffer;
}

/* Function to free the memory allocated for the SMS_BACKUP structure
 * @param backup: pointer to the SMS_BACKUP structure to free
 */
int XML_free( SMS_BACKUP *backup )
{
    if ( backup )
    {
        if ( backup->sms_items )
        {
            free( backup->sms_items );
            backup->sms_items = NULL;
        }
        if ( backup->call_items )
        {
            free( backup->call_items );
            backup->call_items = NULL;
        }
        if ( backup->media_items )
        {
            free( backup->media_items );
            backup->media_items = NULL;
        }
        free( backup );
    }
    return 1;
}

int XML_Snarf( const char *filename, SMS_BACKUP *backup )
{
    FILE *file = fopen( filename, "r" );
    if ( !file )
    {
        fprintf( stderr, "Failed to open file: %s\n", filename );
        return -1;
    }
    fclose( file );
    return 1;
}
/* Function to free the memory allocated for the SMS_BACKUP structure
 * @param backup: pointer to the SMS_BACKUP structure to free
 */
