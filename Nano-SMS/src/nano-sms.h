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


    // Boiler plate; makes include files easier to manage, and keeps the code cleaner.


//#include <corecrt.h>
//#include <stdio.h>

#define WIN32_LEAN_AND_MEAN 



/* MSVC, hell, c in general, has poor support for boolean types.
   So we will define our own boolean type, and use it throughout the code.
   This will make it easier to port to other compilers, and make the code
   more readable.
 */

// DEFINES BEGIN

#define TRUE 1
#define FALSE 0
#define BOOL int
#define IS_IN_DEBUGGING_MODE FALSE // Set to TRUE to enable debugging mode. This will enable more verbose logging and error checking.
#define REPORT_ALLOCATION 0 // Set to TRUE to enable allocation reporting. This will report all allocations and deallocations.
#define REPORT_DEALLOCATION 0 // Set to TRUE to enable deallocation reporting. This will report all deallocations and memory leaks.


#define LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)        ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))

#define DEBUG_FILE "debug.log"




// DEFINES END




 // GLOBALS BEGIN

/* Note: I don't really like using globals in programs. However, it IS
   Accepted as a beast we must endure; kind of like GOTO. That is why
   I will try my best to keep them at a very small amount.
      */
HWND mainwindow; // The main window handle, used for all windows.
HINSTANCE g_hInst; // Global instance handle for the application.

char ERROR_STRING[1024]; // Global error string for the application. Used to store error messages.

#define  APP_NAME  "Nano-SMS" 
#define  APP_VERSION  "0.1.0" 
#define  APP_AUTHOR  "Bi0teQ @ NanobitSoftware."
#define  APP_LICENSE  "GPLv3" 






// GLOBALS END


// FUNCTION MAPPING BEGIN
#define strdup(x) str_dup1(x, __FILE__,__LINE__)
//#define strcpy(x,y) strcpy1(x, y)
#define free(x)   nano_free  (x, __FILE__, __LINE__)
#define malloc(x) nano_malloc(x, __FILE__, __LINE__)
#define realloc(x, y) nano_realloc (x,y, __FILE__, __LINE__)
#define str_dup(x)  str_dup1(x, __FILE__, __LINE__)
#define strprefix(x,y) strprefix1(x, y)
//deefine sprintf(x, ...) _sprintf(x)
//#define vsprintf(x) _vsprintf(x)
//#define strcat(x,y) _strcat(x, y)
//#define strtok(x,y) _strtok(x, y)
//#define localtime(x) _localtime(x)


// FUNCTION MAPPING END

// STRUCT PROTOYPES BEGIN

typedef struct smsbackup SMS_BACKUP;
typedef struct smsitem SMS_ITEM;
typedef struct callitem CALL_ITEM;
typedef struct mediaitem MEDIA_ITEM;

// STRUCT PROTOYPES END

// ENUMS BEGIN

// Datatypes for sms entities.
enum sms_data_type
{
    SMS_CALL, // A ca;;
    SMS_MSG,// A text message.
    SMS_MEDIA  // A media item, such as an image, video, or audio file.

} SMS_DATA_TYPE;

enum sms_media_type
{
    SMS_MEDIA_IMAGE, // Image media item.
    SMS_MEDIA_VIDEO, // Video media item.
    SMS_MEDIA_AUDIO, // Audio media item.
    SMS_MEDIA_OTHER // Other media item.
} SMS_MEDIA_TYPE;
// Datatypes for call entities.
enum call_data_type
{
    CALL_OUTGOING, // Outgoing call item.
    CALL_INCOMING, // Incoming call item.
    CALL_MISSED, // Missed call item.
    CALL_OTHER // Other call item.
} CALL_DATA_TYPE;
// Datatypes for msg entities.
enum msg_data_type
{
    MSG_SENT, // Sent message item.
    MSG_RECEIVED, // Received message item.
    MSG_DRAFT, // Draft message item.
    MSG_DELETED, // Deleted message item.
    MSG_FAILED, // Failed message item.

    MSG_OTHER // Other message item.
} MSG_DATA_TYPE;

enum msg_type
{
    MSG_TYPE_TEXT, // Text message.
    MSG_TYPE_IMAGE, // Image message.
    MSG_TYPE_VIDEO, // Video message.
    MSG_TYPE_AUDIO, // Audio message.
    MSG_TYPE_OTHER // Other message type.
} MSG_TYPE;

enum msg_state
{
    MSG_STATE_SENT, // Message has been sent.
    MSG_STATE_RECEIVED, // Message has been received.
    MSG_STATE_FAILED, // Message has failed to send.
    MSG_STATE_DRAFT, // Message is a draft.
    MSG_STATE_READ, // Message has been read. Comes after 'received' as to not interfer as read > received.
    MSG_STATE_DELETED // Message has been deleted.

} MSG_STATE;



// ENUMS END
// 
// 
// DATA STRUCTURES BEGIN

/* SMS Backup and Restore uses a form of XML for its storage of the sms, call and media
   backups. I really don't want to implement an XML parser, nor do I want to use a
   prebuilt parsing library because of certain features I wish to implement.
   This might make it harder in the long run, keeping with updates from them, I think
   it should work just fine. Maybe in time I will implement a barebones XML parser
   just to make updating it easier.

   Because of that, I will rely on a few data structures that will interlink and form
   sort of a tree design, but is not a "tree" in the programming sense.*/

struct smsitem
{
    char* id; // The ID of the SMS item.
    char* from; // The sender of the SMS item.
    char* address; // The address of the SMS item.
    char* body; // The body of the SMS item.
    time_t date; // The date of the SMS item.
    time_t date_sent; // The date the SMS item was sent.
    time_t date_received; // The date the SMS item was received.
    int read; // Whether the SMS item has been read (1) or not (0).
    unsigned int thread_id; // The thread ID of the SMS item.
    enum MSG_TYPE type;     // The type of the SMS item (text, image, video, audio, other).
    enum MSG_STATE state;    // The state of the SMS item (sent, received, draft, deleted, failed).
    enum MSG_DATA_TYPE data_type; // The data type of the SMS item (message, media).
    MEDIA_ITEM** media; // Pointer to the media item, if any.
    int media_count; // Count of media items.

};

struct callitem
{
    char* id; // The ID of the call item.
    char* address; // The address of the call item.
    time_t date; // The date of the call item.
    enum CALL_DATA_TYPE type; // The type of the call item (outgoing, incoming, missed).
};



struct mediaitem
{
    char* id; // The ID of the media item.
    char* address; // The address of the media item.
    char* file_path; // The file path of the media item.
    time_t date; // The date of the media item.
};


struct smsbackup
{ // Pointers of pointers. Everywhere! Oh my.
    SMS_ITEM* sms_items; // Array of SMS items.
    int sms_count; // Count of SMS items.
    CALL_ITEM* call_items; // Array of call items.
    int call_count; // Count of call items.
    MEDIA_ITEM* media_items; // Array of media items.
    int media_count; // Count of media items.
};



// DATA STRUCTURES END

// PROTOTYPES BEGIN
void     LOG( char*, ... );
void write_buffer( const char* );
int read_string( char buf[], FILE* fp );


/* Prototypes from nano-sms.c*/
//BEGIN */
//EMD

/* Prototypes from string.c*/
//bool str_cmp( const char* astr, const char* bstr );
BOOL str_search( const char* str );
BOOL string_compare( const char* ostr, const char* tstr );
BOOL strprefix1( const char* astr, const char* bstr );
char* str_dup1( const char* str, char* file, int line );

//BEGIN */



//EMD


/* Prototypes from memory.c*
//BEGIN */

void GiveError( char* wrong, BOOL KillProcess );
void nano_free( void* seg, const char* file, int line );
void* nano_malloc( size_t, const char*, int );
void* nano_realloc( void* seg, size_t sz, const char* file, int line );
char* str_dup1( const char* str, char* file, int line );


//END


// PROTOTYPES END

// list of prototypes from memory.c
void GiveError( char* wrong, BOOL KillProcess );




SMS_BACKUP* XML_new( void );