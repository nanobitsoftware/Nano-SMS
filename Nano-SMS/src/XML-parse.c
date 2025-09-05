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

#include "nano-sms.h"
#include "sqlite3/sqlite3.h"
#include "NWC.h"
#include "sms_db.h"
#include "XML.h"

/*////////////SMS XML
 <smses count="#" backup_set="<smsbackup Guid" backup_date="#" type="<full>">
 sms: <sms protocol="0" address="#" date="#" type="#" subject="<token>?" body='<sms body>' toa="null" sc_toa="null" service_center="<sms center>" read="#" status="-#" locked="#" date_sent="#" sub_id="#" readable_date="<date>" contact_name="<name of sender>" />
 <mms date="#" spam_report="#" predefined_id="-#" ct_t="text/plain" msg_box="#" address="#" sub_cs="<charset>" re_type="0" retr_st="null" re_original_body="null" d_tm="null" exp="null" locked="0" msg_id="0" app_id="0" from_address="null" m_id="<message id>" spam_type="0" retr_txt="null" date_sent="#" read="1" rpt_a="null" ct_cls="#" bin_info="0" pri="null" sub_id="1" re_content_type="null" object_id="null" resp_txt="null" re_content_uri="null" ct_l="null" re_original_key="null" d_rpt="null" reserved="0" using_mode="0" _id="#" rr_st="0" m_type="132" favorite="0" rr="null" sub="" hidden="0" deletable="0" read_status="null" d_rpt_st="0" callback_set="0" re_count_info_custom_reaction="null" seen="1" re_recipient_address="null" device_name="null" cmc_prop="null" resp_st="null" text_only="1" sim_slot="0" st="null" retr_txt_cs="null" creator="com.google.android.apps.messaging" m_size="0" sim_imsi="null" block_filtered_status="null" correlation_tag="null" re_body="null" safe_message="0" tr_id="" m_cls="null" v="null" secret_mode="0" re_file_name="null" re_count_info="null" readable_date="<date>" contact_name="<contact name>">
    <parts>
      <part seq="0" ct="text/plain" name="body|filename" chset="#" cd="null" fn="null" cid="null" cl="null" ctt_s="null" ctt_t="null" text="<mms text>" sef_type="0" decorate_bubble_value="null" sub_id="1" />
    </parts>
    <addrs>
      <addr address="<numbers>" type="130" charset="106" />
      <addr address="<nuymbers>" type="137" charset="106" />
      <addr address="<numbers>" type="151" charset="106" />
    </addrs>

  </mms>
  <mms>
    <parts>
        <part seq="0" ct="image/jpeg" name="<filename>" chset="null" cd="null" fn="null" cid="<id>" cl="<filename>" ctt_s="null" ctt_t="null" text="null" sef_type="0" decorate_bubble_value="null" sub_id="1" data="<datablob>" />
    </parts>
</mms>
 */

/* explain mms xml above
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
SMS_BACKUP * XML_new (void)
{
    SMS_BACKUP *backup = (SMS_BACKUP *)malloc ( sizeof ( SMS_BACKUP ) );
    if ( !backup )
    {
        fprintf ( stderr, "Failed to allocate memory for SMS_BACKUP structure\n" );
        return NULL;
    }
    memset ( backup, 0, sizeof ( SMS_BACKUP ) );
    return backup;
}

int XML_free (SMS_BACKUP *backup)
{
    if (backup)
    {
        if (backup->sms_items)
        {
            free(backup->sms_items);
            backup->sms_items = NULL;
        }
        if (backup->call_items)
        {
            free(backup->call_items);
            backup->call_items = NULL;
        }
        if (backup->media_items)
        {
            free(backup->media_items);
            backup->media_items = NULL;
        }
        free(backup);
    }
    return 1;
}

int XML_Snarf ( const char *filename, SMS_BACKUP *backup )
{
    FILE *file = fopen ( filename, "r" );
    if ( !file )
    {
        fprintf ( stderr, "Failed to open file: %s\n", filename );
        return -1;
    }
    fclose ( file );
    return 1;
}
/* Function to free the memory allocated for the SMS_BACKUP structure
 * @param backup: pointer to the SMS_BACKUP structure to free
 */
