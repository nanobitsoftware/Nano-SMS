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



#include <time.h>


// !! WARNING !! !! WARNING !! !! WARNING !! !! WARNING !! !! WARNING !! !! WARNING !! 
/* This "XML" stuff isn't going to be any type of standards compliant
    whatsoever. It will mimic XML parsing, and will probably work as
    a parser with some minor modifications; however, this code is meant
    to only work with the XML code outputted by SMSBACKUPRESTART.*/

    // THIS IS NOT A STANDARDS COMPLIANT XML PARSER ON PURPOSE!

// !! WARNING !! !! WARNING !! !! WARNING !! !! WARNING !! !! WARNING !! !! WARNING !! 

    // It is very important for the "name" and "value" properties of these
    // sheets that we get the exact string length before malloc. Some of the
    // properties with SMS backup and restart are gigantic. And need to be 
    // Treated as such before throwing memeory off to them. Remember,
    // Nanobit keeps things small. So we'll do our own precious way of making
    // a 16gb xml file turn into just a few hundred mb of memory :)


    // Safeguards
#define MAX_PARENT 1024 * 4
#define MAX_CHILD MAX_PARENT * 4
#define MAX_STGRING_LENGTH  1024 * 1024 // 1mb max string length. This is a safeguard, not a hard limit.
#define MSL MAX_STRING_LENGTH // just an alias.

typedef struct xml_node_parent XML_NODE_PARENT;
typedef struct xml_node_child XML_NODE_CHILD;
typedef struct xml_node_media XML_NODE_MEDIA;
    // Structs for XML parsing and storage

struct xml_node_parent {
    char* name; // Name of the node.
    char* value; // Value of the node.
    XML_NODE_PARENT* next; // Next node in the list.
    XML_NODE_CHILD* child; // Child nodes. .. Also works as "First child"
    int type; // Type of node, 0 = element, 1 = text, 2 = comment, etc.
    int line; // Line number in the XML file.
    int column; // Column number in the XML file.
    int open_brackets;
    int closed_brackets; // For sanity checking down the line
    int child_count; // Number of child nodes.

};

struct xml_node_child {
    char* name; // Name of the child node.
    char* value; // Value of the child node.
    XML_NODE_CHILD* next; // Next child node in the list.
    XML_NODE_CHILD* prev_child; // Previous child we went to.
    XML_NODE_PARENT* parent; // Parent node.
    XML_NODE_MEDIA* media; //DO we have any media attached?
    int type; // Type of child node, 0 = element, 1 = text, 2 = comment, etc.
    int line; // Line number in the XML file.
    int column; // Column number in the XML file.
    int open_brackets;
    int closed_brackets; // For sanity checking down the line
};

struct xml_node_media {
    char* id; // ID of the media item.
    char* address; // Address of the media item.
    char* file_path; // File path of the media item.
    time_t date; // Date of the media item.
    int type; // Type of media item, 0 = image, 1 = video, etc.
    XML_NODE_MEDIA* next; // Next media item in the list.
    XML_NODE_PARENT* parent;
};

