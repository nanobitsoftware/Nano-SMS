#pragma once


/* This file will contain all the raw sql commands so that we cna send them
   to sqlite3 to create our databases at the beginning of the program launch.
   It will create the file inside the main directory if the file does not exist

  */








#define SMS_CREATE_MEDIA_TABLE "CREATE TABLE IF NOT EXISTS \"nano_media\" (" \
"\"msg_refid\"	INTEGER NOT NULL DEFAULT 0 UNIQUE," \
"\"msg_type\"	INTEGER NOT NULL DEFAULT 0," \
"\"msg_size\"	INTEGER NOT NULL DEFAULT 0," \
"\"msg_xsize\"	INTEGER NOT NULL DEFAULT 0," \
"\"msg_ysize\"	INTEGER NOT NULL DEFAULT 0," \
"\"msg_blob\"	BLOB NOT NULL," \
"PRIMARY KEY(\"msg_refid\" AUTOINCREMENT)" \
"); \n"

#define SMS_CREATE_MSG_TABLE "CREATE TABLE IF NOT EXISTS \"nano_msg\" (" \
"\"msg_id\"	INTEGER NOT NULL DEFAULT 0 UNIQUE," \
"\"msg_refid\"	INTEGER NOT NULL DEFAULT 0," \
"\"msg_date\"	INTEGER NOT NULL DEFAULT 0," \
"\"msg_mediacount\"	INTEGER NOT NULL DEFAULT 0," \
"\"msg_to\"	INTEGER NOT NULL DEFAULT 0," \
"\"msg_from\"	TEXT NOT NULL DEFAULT 'someone'," \
"\"msg_msg\"	TEXT NOT NULL DEFAULT 'empty'," \
"\"msg_hasmedia\"	INTEGER NOT NULL DEFAULT 'false'," \
"\"msg_sent\"	INTEGER NOT NULL DEFAULT 0," \
"PRIMARY KEY( \"msg_id\",\"msg_refid\")" \
"); \n"

#define SMS_CREATE_SMS_TABLE "CREATE TABLE IF NOT EXISTS \"nano_sms\" (" \
"\"sms_id\"	INTEGER NOT NULL UNIQUE," \
"\"sms_hasmedia\"	TEXT NOT NULL DEFAULT 'false'," \
"\"sms_count\"	INTEGER NOT NULL," \
"\"sms_lastdate\"	INTEGER NOT NULL DEFAULT 0," \
"\"sms_firstdate\"	INTEGER DEFAULT 0," \
"\"sms_statid\"	INTEGER NOT NULL DEFAULT 0," \
"\"sms_firstmsg\"	INTEGER NOT NULL DEFAULT 0 UNIQUE" \
");\n"

#define SMS_CREATE_TABLE_STATS "CREATE TABLE IF NOT EXISTS \"nano_stats\" ( " \
	"\"refid\"	INTEGER NOT NULL DEFAULT 0 UNIQUE," \
	"\"msgcount\"	INTEGER NOT NULL DEFAULT 0," \
	"\"mediacount\"	INTEGER NOT NULL DEFAULT 0," \
	"\"firstdate\"	INTEGER NOT NULL DEFAULT 0," \
	"\"lastdate\"	INTEGER NOT NULL DEFAULT 0," \
	"\"dbsize\"	INTEGER NOT NULL DEFAULT 0," \
	"\"recv_count\"	INTEGER NOT NULL DEFAULT 0," \
	"\"send_count\"	INTEGER NOT NULL DEFAULT 0," \
	"\"last_sent_id\"	INTEGER NOT NULL DEFAULT 0," \
	"\"date_read\"	INTEGER NOT NULL DEFAULT 0," \
	"\"date_created\"	INTEGER NOT NULL DEFAULT 0," \
	"\"db_created\"	INTEGER NOT NULL DEFAULT 0," \
	"\"db_readtime\"	INTEGER NOT NULL DEFAULT 0," \
	"\"db_errors\"	INTEGER NOT NULL DEFAULT -1," \
	"\"file_name\"	TEXT NOT NULL DEFAULT 'file'," \
	"\"file_location\"	TEXT NOT NULL DEFAULT 'somewhere'," \
	"PRIMARY KEY(\"refid\")" \
");\n"




#define CREATE_TABLES  SMS_CREATE_SMS_TABLE \
SMS_CREATE_MSG_TABLE SMS_CREATE_MEDIA_TABLE SMS_CREATE_TABLE_STATS
