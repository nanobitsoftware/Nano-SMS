/*
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

/* A t-buf is something I use from time to time when I am working with
   sql (specifically sqlite) and the software has a lot of stuff it
   wants to ingest into the database. Normally, software wil just feed
   it statements as it comes so that it can store it. However, I have found
   that it will cause a huge slow down with the applications AND sqlite.

   So I wrote this little buffer that will store up a large amount of memory
   and, on time, halt the main program and do a large stuff rather than lot
    of small stuff. This makes the program run faster, and sqlite runs faster
    due to sqlite having to constantly run its high overhead parsing.

    This one might be a bit different than wwhat I normaly do, since I know
    I am going to have some that have large media items or even really
    long text items. So I may have to give each buffer a priority
    number for it to know when and how to injest. Fingers crossed kids.

    */

#pragma once
#include "nano-io.h"
#include "XML.h"
#include "nano-sms.h"
#include <stdio.h>
#include <stdlib.h>
#include "sqlite3/sqlite3.h"
#define LOG_SQL_ERROR(x) ((LOG("SQL_ERROR (__FILE__)(__LINE__): %s", (x))))

#define SET_PRI(x,y) ((x) ? x->priority = y : LOG_SQL_ERROR ("TBUF is NULL, cannot set priority."))
#define GET_PRI(x) ((x) ? x->priority : (LOG_SQL_ERROR ("TBUF is NULL, cannot get priority."), -9999))
#define PRI_COMP(x,y) ((GET_PRI(x) > GET_PRI(Y) ? x :y ))

#define MAX_TBUF 1024 // Max number of t-bufs we can have at once. Will force a flush if exceeded.
#define TBUF_GROW 4096 // How much to grow the t-buf by when needed. Will offer this ability.
#define TBUF_MIN 24 // The minimum amount it must hold before a scheduled run is allowed.

/*  The name of the game with sms reader is that we managed memory quite well, as
    some of the things we ingest can be quite large, even out pacing the amount of
    physical memory on the machine. Which is why we have a streaming chunk option.
    that whole 'ideal' needs to come to the sql injection as well!
*/

typedef struct t_buf TBUF;
typedef struct t_buf_manager TBUF_MAN;

#define DEBUG_MODE_TBUF_FREE_SHIFT TRUE // Set to TRUE to enable debug mode for t-buf free operations.

enum M_TYPE
{
    TYPE_UNKNOWN = 0,
    TYPE_SQL = 1,
    TYPE_SMS_XML = 2,
    TYPE_MEDIA_BLOB = 3,
    TYPE_TEXT = 4,
    TYPE_CALL = 5,
    TYPE_CONTACT = 6,
    TYPE_MMS_XML = 7,
    TYPE_GROUP = 8
};

typedef struct t_buf_manager
{
    TBUF** tbufs; // Array of pointers to TBUF structures.
    size_t count; // Number of registered t-bufs.
    size_t capacity; // Capacity of the bufs array.
} TBUF_MANAGER;

typedef struct t_buf
{
    char* buffer; // The buffer to hold the data.
    size_t size;  // The current size of the buffer.
    size_t used; //amount actually used in the buffer
    int   priority; // The priority of the buffer. Higher priority buffers get flushed first.
    enum M_TYPE type; // The type of data in the buffer.
    BOOL in_use; // Is this buffer in use? Basically to make sure we're freeing these.
} TBUF;

// Function prototypes for t-buf management.
static void init_tbuf_manager();
BOOL free_tbuf_man( void );
BOOL free_tbuf( TBUF* tbuf );
TBUF* create_tbuf( void );
BOOL tbuf_add_data( TBUF* tbuf, const char* data, size_t len );
BOOL tbuf_flush( TBUF* tbuf, sqlite3* db );
size_t tbuf_get_used( TBUF* tbuf );
size_t tbuf_get_size( TBUF* tbuf );
