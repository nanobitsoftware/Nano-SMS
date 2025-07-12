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


// Boiler plate



// Include NWC for our Windowing system

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
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
#include "NWC.h"











int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR argstr,int fun)
{

    MSG msg;




    while (GetMessage(&msg, NULL, 0, 0))
    {
        // Process the message.
        if (msg.message == WM_QUIT)
        {
            // If we receive a quit message, we can break out of the loop.
            break;
        }
        // If we have a message, we can process it.
        if (msg.message == WM_CREATE)
        {
            // Initialize the window here.
          //  initialize_windows();
        }
    }
    {
        // Put if statement here ffor checking if we're still running!

    
            TranslateMessage(&msg); // Main message loops.
            DispatchMessage(&msg);
    
    }

    return msg.wParam;
}










// Function to initialize the SMS_BACKUP structure.
SMS_BACKUP* init_sms_backup()
{
    SMS_BACKUP* backup = (SMS_BACKUP*)malloc(sizeof(SMS_BACKUP));
    if (!backup)
    {
        fprintf(stderr, "Failed to allocate memory for SMS_BACKUP.\n");
        return NULL;
    }
    backup->sms_items = NULL;
    backup->sms_count = 0;
    backup->call_items = NULL;
    backup->call_count = 0;
    backup->media_items = NULL;
    backup->media_count = 0;
    return backup;
}

// Function to free the SMS_BACKUP structure.
void free_sms_backup(SMS_BACKUP* backup)
{
    if (!backup) return;
    for (int i = 0; i < backup->sms_count; i++)
    {
        free(backup->sms_items[i].id);
        free(backup->sms_items[i].address);
        free(backup->sms_items[i].body);
    }
    free(backup->sms_items);
    for (int i = 0; i < backup->call_count; i++)
    {
        free(backup->call_items[i].id);
        free(backup->call_items[i].address);
    }
    free(backup->call_items);
    for (int i = 0; i < backup->media_count; i++)
    {
        free(backup->media_items[i].id);
        free(backup->media_items[i].address);
        free(backup->media_items[i].file_path);
    }
    free(backup->media_items);
    free(backup);
}
// Function to add an SMS item to the backup.
void add_sms_item(SMS_BACKUP* backup, const char* id, const char* address, const char* body, time_t date, int type)
{
    if (!backup) return;
    backup->sms_items = (SMS_ITEM*)realloc(backup->sms_items, sizeof(SMS_ITEM) * (backup->sms_count + 1));
    if (!backup->sms_items)
    {
        fprintf(stderr, "Failed to allocate memory for SMS items.\n");
        return;
    }
    SMS_ITEM* item = &backup->sms_items[backup->sms_count];
    item->id = strdup(id);
    item->address = strdup(address);
    item->body = strdup(body);
    item->date = date;
    item->type = type;
    backup->sms_count++;
}
// Function to add a call item to the backup.
void add_call_item(SMS_BACKUP* backup, const char* id, const char* address, time_t date, int type)
{
    if (!backup) return;
    backup->call_items = (CALL_ITEM*)realloc(backup->call_items, sizeof(CALL_ITEM) * (backup->call_count + 1));
    if (!backup->call_items)
    {
        fprintf(stderr, "Failed to allocate memory for call items.\n");
        return;
    }
    CALL_ITEM* item = &backup->call_items[backup->call_count];
    item->id = strdup(id);
    item->address = strdup(address);
    item->date = date;
    item->type = type;
    backup->call_count++;
}
// Function to add a media item to the backup.
void add_media_item(SMS_BACKUP* backup, const char* id, const char* address, const char* file_path, time_t date)
{
    if (!backup) return;
    backup->media_items = (MEDIA_ITEM*)realloc(backup->media_items, sizeof(MEDIA_ITEM) * (backup->media_count + 1));
    if (!backup->media_items)
    {
        fprintf(stderr, "Failed to allocate memory for media items.\n");
        return;
    }
    MEDIA_ITEM* item = &backup->media_items[backup->media_count];
    item->id = strdup(id);
    item->address = strdup(address);
    item->file_path = strdup(file_path);
    item->date = date;
    backup->media_count++;
}
// Function to print the SMS backup contents.
void print_sms_backup(const SMS_BACKUP* backup)
{
    if (!backup) return;
    printf("SMS Items:\n");
    for (int i = 0; i < backup->sms_count; i++)
    {
        const SMS_ITEM* item = &backup->sms_items[i];
        printf("ID: %s, Address: %s, Body: %s, Date: %ld, Type: %d\n", item->id, item->address, item->body, item->date, item->type);
    }
    printf("Call Items:\n");
    for (int i = 0; i < backup->call_count; i++)
    {
        const CALL_ITEM* item = &backup->call_items[i];
        printf("ID: %s, Address: %s, Date: %ld, Type: %d\n", item->id, item->address, item->date, item->type);
    }
    printf("Media Items:\n");
    for (int i = 0; i < backup->media_count; i++)
    {
        const MEDIA_ITEM* item = &backup->media_items[i];
        printf("ID: %s, Address: %s, File Path: %s, Date: %ld\n", item->id, item->address, item->file_path, item->date);
    }
}
// Function to save the SMS backup to a file (not implemented yet).
void save_sms_backup_to_file(const SMS_BACKUP* backup, const char* filename)
{
    // This function will be implemented later to save the backup to a file.
    // For now, we will just print a message.
    printf("Saving SMS backup to file: %s (not implemented yet)\n", filename);
}
// Function to load the SMS backup from a file (not implemented yet).
void load_sms_backup_from_file(SMS_BACKUP* backup, const char* filename)
{
    // This function will be implemented later to load the backup from a file.
    // For now, we will just print a message.
    printf("Loading SMS backup from file: %s (not implemented yet)\n", filename);
}
// Function to clear the SMS backup.
void clear_sms_backup(SMS_BACKUP* backup)
{
    if (!backup) return;
    free_sms_backup(backup);
    backup = init_sms_backup();
    if (!backup)
    {
        fprintf(stderr, "Failed to reinitialize SMS_BACKUP after clearing.\n");
    }
}
// Function to get the count of SMS items.
int get_sms_count(const SMS_BACKUP* backup)
{
    if (!backup) return 0;
    return backup->sms_count;
}
// Function to get the count of call items.
int get_call_count(const SMS_BACKUP* backup)
{
    if (!backup) return 0;
    return backup->call_count;
}
// Function to get the count of media items.
int get_media_count(const SMS_BACKUP* backup)
{
    if (!backup) return 0;
    return backup->media_count;
}
// Function to get an SMS item by index.
SMS_ITEM* get_sms_item(const SMS_BACKUP* backup, int index)
{
    if (!backup || index < 0 || index >= backup->sms_count) return NULL;
    return &backup->sms_items[index];
}
// Function to get a call item by index.
CALL_ITEM* get_call_item(const SMS_BACKUP* backup, int index)
{
    if (!backup || index < 0 || index >= backup->call_count) return NULL;
    return &backup->call_items[index];
}
// Function to get a media item by index.
MEDIA_ITEM* get_media_item(const SMS_BACKUP* backup, int index)
{
    if (!backup || index < 0 || index >= backup->media_count) return NULL;
    return &backup->media_items[index];
}
// Function to find an SMS item by ID.
SMS_ITEM* find_sms_item_by_id(const SMS_BACKUP* backup, const char* id)
{
    if (!backup || !id) return NULL;
    for (int i = 0; i < backup->sms_count; i++)
    {
        if (strcmp(backup->sms_items[i].id, id) == 0)
        {
            return &backup->sms_items[i];
        }
    }
    return NULL;
}
// Function to find a call item by ID.
CALL_ITEM* find_call_item_by_id(const SMS_BACKUP* backup, const char* id)
{
    if (!backup || !id) return NULL;
    for (int i = 0; i < backup->call_count; i++)
    {
        if (strcmp(backup->call_items[i].id, id) == 0)
        {
            return &backup->call_items[i];
        }
    }
    return NULL;
}
// Function to find a media item by ID.
MEDIA_ITEM* find_media_item_by_id(const SMS_BACKUP* backup, const char* id)
{
    if (!backup || !id) return NULL;
    for (int i = 0; i < backup->media_count; i++)
    {
        if (strcmp(backup->media_items[i].id, id) == 0)
        {
            return &backup->media_items[i];
        }
    }
    return NULL;
}
// Function to find SMS items by address.
SMS_ITEM* find_sms_items_by_address(const SMS_BACKUP* backup, const char* address, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    SMS_ITEM* results = (SMS_ITEM*)malloc(sizeof(SMS_ITEM) * backup->sms_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for SMS search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->sms_count; i++)
    {
        if (strcmp(backup->sms_items[i].address, address) == 0)
        {
            results[*count] = backup->sms_items[i];
            (*count)++;
        }
    }
    return results;
}

// Function to find call items by address.
CALL_ITEM* find_call_items_by_address(const SMS_BACKUP* backup, const char* address, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    CALL_ITEM* results = (CALL_ITEM*)malloc(sizeof(CALL_ITEM) * backup->call_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for call search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->call_count; i++)
    {
        if (strcmp(backup->call_items[i].address, address) == 0)
        {
            results[*count] = backup->call_items[i];
            (*count)++;
        }
    }
    return results;
}

// Function to find media items by address.
MEDIA_ITEM* find_media_items_by_address(const SMS_BACKUP* backup, const char* address, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    MEDIA_ITEM* results = (MEDIA_ITEM*)malloc(sizeof(MEDIA_ITEM) * backup->media_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for media search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->media_count; i++)
    {
        if (strcmp(backup->media_items[i].address, address) == 0)
        {
            results[*count] = backup->media_items[i];
            (*count)++;
        }
    }
    return results;
}

// Function to find SMS items by date range.
SMS_ITEM* find_sms_items_by_date_range(const SMS_BACKUP* backup, time_t start_date, time_t end_date, int* count)
{
    if (!backup || !count) return NULL;
    *count = 0;
    SMS_ITEM* results = (SMS_ITEM*)malloc(sizeof(SMS_ITEM) * backup->sms_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for SMS date range search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->sms_count; i++)
    {
        if (backup->sms_items[i].date >= start_date && backup->sms_items[i].date <= end_date)
        {
            results[*count] = backup->sms_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find call items by date range.
CALL_ITEM* find_call_items_by_date_range(const SMS_BACKUP* backup, time_t start_date, time_t end_date, int* count)
{
    if (!backup || !count) return NULL;
    *count = 0;
    CALL_ITEM* results = (CALL_ITEM*)malloc(sizeof(CALL_ITEM) * backup->call_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for call date range search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->call_count; i++)
    {
        if (backup->call_items[i].date >= start_date && backup->call_items[i].date <= end_date)
        {
            results[*count] = backup->call_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find media items by date range.
MEDIA_ITEM* find_media_items_by_date_range(const SMS_BACKUP* backup, time_t start_date, time_t end_date, int* count)
{
    if (!backup || !count) return NULL;
    *count = 0;
    MEDIA_ITEM* results = (MEDIA_ITEM*)malloc(sizeof(MEDIA_ITEM) * backup->media_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for media date range search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->media_count; i++)
    {
        if (backup->media_items[i].date >= start_date && backup->media_items[i].date <= end_date)
        {
            results[*count] = backup->media_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find SMS items by type.
SMS_ITEM* find_sms_items_by_type(const SMS_BACKUP* backup, int type, int* count)
{
    if (!backup || !count) return NULL;
    *count = 0;
    SMS_ITEM* results = (SMS_ITEM*)malloc(sizeof(SMS_ITEM) * backup->sms_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for SMS type search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->sms_count; i++)
    {
        if (backup->sms_items[i].type == type)
        {
            results[*count] = backup->sms_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find call items by type.
CALL_ITEM* find_call_items_by_type(const SMS_BACKUP* backup, int type, int* count)
{
    if (!backup || !count) return NULL;
    *count = 0;
    CALL_ITEM* results = (CALL_ITEM*)malloc(sizeof(CALL_ITEM) * backup->call_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for call type search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->call_count; i++)
    {
        if (backup->call_items[i].type == type)
        {
            results[*count] = backup->call_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find media items by type (not applicable, as media items do not have a type).
MEDIA_ITEM* find_media_items_by_type(const SMS_BACKUP* backup, int type, int* count)
{
    // Media items do not have a type field, so we return NULL.
    if (!backup || !count) return NULL;
    *count = 0;
    return NULL; // No media items by type.
}
// Function to find SMS items by address and date range.
SMS_ITEM* find_sms_items_by_address_and_date_range(const SMS_BACKUP* backup, const char* address, time_t start_date, time_t end_date, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    SMS_ITEM* results = (SMS_ITEM*)malloc(sizeof(SMS_ITEM) * backup->sms_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for SMS address and date range search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->sms_count; i++)
    {
        if (strcmp(backup->sms_items[i].address, address) == 0 &&
            backup->sms_items[i].date >= start_date && backup->sms_items[i].date <= end_date)
        {
            results[*count] = backup->sms_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find call items by address and date range.
CALL_ITEM* find_call_items_by_address_and_date_range(const SMS_BACKUP* backup, const char* address, time_t start_date, time_t end_date, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    CALL_ITEM* results = (CALL_ITEM*)malloc(sizeof(CALL_ITEM) * backup->call_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for call address and date range search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->call_count; i++)
    {
        if (strcmp(backup->call_items[i].address, address) == 0 &&
            backup->call_items[i].date >= start_date && backup->call_items[i].date <= end_date)
        {
            results[*count] = backup->call_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find media items by address and date range.
MEDIA_ITEM* find_media_items_by_address_and_date_range(const SMS_BACKUP* backup, const char* address, time_t start_date, time_t end_date, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    MEDIA_ITEM* results = (MEDIA_ITEM*)malloc(sizeof(MEDIA_ITEM) * backup->media_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for media address and date range search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->media_count; i++)
    {
        if (strcmp(backup->media_items[i].address, address) == 0 &&
            backup->media_items[i].date >= start_date && backup->media_items[i].date <= end_date)
        {
            results[*count] = backup->media_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find SMS items by address and type.
SMS_ITEM* find_sms_items_by_address_and_type(const SMS_BACKUP* backup, const char* address, int type, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    SMS_ITEM* results = (SMS_ITEM*)malloc(sizeof(SMS_ITEM) * backup->sms_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for SMS address and type search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->sms_count; i++)
    {
        if (strcmp(backup->sms_items[i].address, address) == 0 && backup->sms_items[i].type == type)
        {
            results[*count] = backup->sms_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find call items by address and type.
CALL_ITEM* find_call_items_by_address_and_type(const SMS_BACKUP* backup, const char* address, int type, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    CALL_ITEM* results = (CALL_ITEM*)malloc(sizeof(CALL_ITEM) * backup->call_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for call address and type search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->call_count; i++)
    {
        if (strcmp(backup->call_items[i].address, address) == 0 && backup->call_items[i].type == type)
        {
            results[*count] = backup->call_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find media items by address and type (not applicable, as media items do not have a type).
MEDIA_ITEM* find_media_items_by_address_and_type(const SMS_BACKUP* backup, const char* address, int type, int* count)
{
    // Media items do not have a type field, so we return NULL.
    if (!backup || !address || !count) return NULL;
    *count = 0;
    return NULL; // No media items by address and type.
}
// Function to find SMS items by date range and type.
SMS_ITEM* find_sms_items_by_date_range_and_type(const SMS_BACKUP* backup, time_t start_date, time_t end_date, int type, int* count)
{
    if (!backup || !count) return NULL;
    *count = 0;
    SMS_ITEM* results = (SMS_ITEM*)malloc(sizeof(SMS_ITEM) * backup->sms_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for SMS date range and type search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->sms_count; i++)
    {
        if (backup->sms_items[i].date >= start_date && backup->sms_items[i].date <= end_date && backup->sms_items[i].type == type)
        {
            results[*count] = backup->sms_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find call items by date range and type.
CALL_ITEM* find_call_items_by_date_range_and_type(const SMS_BACKUP* backup, time_t start_date, time_t end_date, int type, int* count)
{
    if (!backup || !count) return NULL;
    *count = 0;
    CALL_ITEM* results = (CALL_ITEM*)malloc(sizeof(CALL_ITEM) * backup->call_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for call date range and type search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->call_count; i++)
    {
        if (backup->call_items[i].date >= start_date && backup->call_items[i].date <= end_date && backup->call_items[i].type == type)
        {
            results[*count] = backup->call_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find media items by date range and type (not applicable, as media items do not have a type).
MEDIA_ITEM* find_media_items_by_date_range_and_type(const SMS_BACKUP* backup, time_t start_date, time_t end_date, int type, int* count)
{
    // Media items do not have a type field, so we return NULL.
    if (!backup || !count) return NULL;
    *count = 0;
    return NULL; // No media items by date range and type.
}
// Function to find SMS items by address, date range, and type.
SMS_ITEM* find_sms_items_by_address_date_range_and_type(const SMS_BACKUP* backup, const char* address, time_t start_date, time_t end_date, int type, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    SMS_ITEM* results = (SMS_ITEM*)malloc(sizeof(SMS_ITEM) * backup->sms_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for SMS address, date range, and type search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->sms_count; i++)
    {
        if (strcmp(backup->sms_items[i].address, address) == 0 &&
            backup->sms_items[i].date >= start_date && backup->sms_items[i].date <= end_date &&
            backup->sms_items[i].type == type)
        {
            results[*count] = backup->sms_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find call items by address, date range, and type.
CALL_ITEM* find_call_items_by_address_date_range_and_type(const SMS_BACKUP* backup, const char* address, time_t start_date, time_t end_date, int type, int* count)
{
    if (!backup || !address || !count) return NULL;
    *count = 0;
    CALL_ITEM* results = (CALL_ITEM*)malloc(sizeof(CALL_ITEM) * backup->call_count);
    if (!results)
    {
        fprintf(stderr, "Failed to allocate memory for call address, date range, and type search results.\n");
        return NULL;
    }
    for (int i = 0; i < backup->call_count; i++)
    {
        if (strcmp(backup->call_items[i].address, address) == 0 &&
            backup->call_items[i].date >= start_date && backup->call_items[i].date <= end_date &&
            backup->call_items[i].type == type)
        {
            results[*count] = backup->call_items[i];
            (*count)++;
        }
    }
    return results;
}
// Function to find media items by address, date range, and type (not applicable, as media items do not have a type).
MEDIA_ITEM* find_media_items_by_address_date_range_and_type(const SMS_BACKUP* backup, const char* address, time_t start_date, time_t end_date, int type, int* count)
{
    // Media items do not have a type field, so we return NULL.
    if (!backup || !address || !count) return NULL;
    *count = 0;
    return NULL; // No media items by address, date range, and type.
}




void GiveError(char* wrong, BOOL KillProcess)
{
    if (wrong == NULL)
    {
        MessageBox(GetFocus(), "An unknown error has occured, please restart BioMud and try again.", "BioMud Error", MB_ICONSTOP | MB_OK);
        if (KillProcess)
        {
            exit(0);
        }
        else
        {
            return;
        }
    }

    MessageBox(GetFocus(), wrong, "BioMUD Error", MB_ICONSTOP | MB_OK);

    if (KillProcess)
    {
        exit(0);
    }
    else
    {
        return;
    }
}
