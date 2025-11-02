/* This was written for use in all my software. Over the years it has been
   tweaked and changed for my needs. Some of it may not be used at all here
   but I include it as a form of library only. Everything written below
   is from past me.
   */

/*Copyright(c) 2016 Michael Hayes (Bioteq - Nanobit Software)
* All rights reserved.
*
* Redistribution and use in source and binary forms are permitted
* provided that the above copyright notice and this paragraph are
* duplicated in all such forms and that any documentation,
* advertising materials, and other materials related to such
* distribution and use acknowledge that the software was developed
* by (http://biomud.nanobit.net) Nanobit Software.The name of
* Nanobit Software may not be used to endorse or promote products derived
* from this software without specific prior written permission.
* THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#pragma once

// All, hopefully, at least, language or texts showing in this app.
// will be declared here and 'got' by a getter function which will return
// the value in whatever language its supposed to. It will be written in
// english, obviously, bull the getting will check for setl anguage and
// if someone has transcribed some, they'll receive it. No? Then english.
//typedef struct langtable LANGTABLE

struct langtable {
    const char* name; // The name of the text we need.
    char* text; // What will be returned if asked.
    const size_t lang; // Depending on this number is what will decide if we get a different language.
};

// Language defines
#define LANG_ENG 0
#define LANG_SPAN 1
#define LANG_FREN 2
#define LANG_GERM 3
#define LANG_ITAL 4
#define LANG_CHIN 5
#define LANG_JAPN 6

// Define al your texts here. Giving it its own id.
// Example: ID returns to '1' so the getting returns the text of the id and return is.

struct langtable LANGS[] =
{
    { "FILE_NOT_FOUND", "File not found.", LANG_ENG },
    { "FILE_NOT_FOUND", "Archivo no encontrado.", LANG_SPAN },
    { "FILE_NOT_FOUND", "Fichier non trouvé.", LANG_FREN },
    { "FILE_NOT_FOUND", "Datei nicht gefunden.", LANG_GERM },
    { "FILE_NOT_FOUND", "File non trovato.", LANG_ITAL },
    { "FILE_NOT_FOUND", "文件未找到。", LANG_CHIN },
    { "FILE_NOT_FOUND", "ファイルが見つかりません。", LANG_JAPN },
    { "ERROR_OPENING_FILE", "Error opening file.", LANG_ENG },
    { "ERROR_OPENING_FILE", "Error al abrir el archivo.", LANG_SPAN },
    { "ERROR_OPENING_FILE", "Erreur lors de l'ouverture du fichier.", LANG_FREN },
    { "ERROR_OPENING_FILE", "Fehler beim Öffnen der Datei.", LANG_GERM },
    { "ERROR_OPENING_FILE", "Errore durante l'apertura del file.", LANG_ITAL },
    { "ERROR_OPENING_FILE", "打开文件时出错。", LANG_CHIN },
    { "ERROR_OPENING_FILE", "ファイルを開く際にエラーが発生しました。", LANG_JAPN },
    { "IMPORT_SUCCESS", "Import completed successfully.", LANG_ENG },
    { "IMPORT_SUCCESS", "Importación completada con éxito.", LANG_SPAN },
    { "IMPORT_SUCCESS", "Importation terminée avec succès.", LANG_FREN },
    { "IMPORT_SUCCESS", "Import erfolgreich abgeschlossen.", LANG_GERM },
                                    { "IMPORT_SUCCESS", "Importazione completata con successo.", LANG_ITAL },

    { "IMPORT_SUCCESS", "导入成功完成。", LANG_CHIN },
    { "IMPORT_SUCCESS", "インポートが正常に完了しました。", LANG_JAPN },
    { "WELCOME", "Welcome to Nano-SMS. Be sure to read all the information presented as you go.", LANG_ENG },
    { "WELCOME", "Bienvenido a Nano-SMS. Asegúrese de leer toda la información presentada a medida que avanza.", LANG_SPAN },
    { "WELCOME", "Bienvenue dans Nano-SMS. Assurez-vous de lire toutes les informations présentées au fur et à mesure.", LANG_FREN },
    { "WELCOME", "Willkommen bei Nano-SMS. Lesen Sie unbedingt alle Informationen, die Ihnen im Verlauf angezeigt werden.", LANG_GERM },
    { "WELCOME", "Benvenuto in Nano-SMS. Assicurati di leggere tutte le informazioni presentate man mano che procedi.", LANG_ITAL },
    { "WELCOME", "欢迎使用Nano-SMS。请确保在进行过程中阅读所有呈现的信息。", LANG_CHIN },
    { "WELCOME", "Nano-SMSへようこそ。進むにつれて表示されるすべての情報を必ずお読みください。", LANG_JAPN },
    {"SAVE_TO_DB", "¿Desea guardar esto en la base de datos?", LANG_SPAN },
    {"SAVE_TO_DB", "Voulez-vous enregistrer cela dans la base de données?", LANG_FREN },
    {"SAVE_TO_DB", "Möchten Sie dies in der Datenbank speichern?", LANG_GERM },
    {"SAVE_TO_DB", "Vuoi salvare questo nel database?", LANG_ITAL },
    {"SAVE_TO_DB", "您想将此保存到数据库吗？", LANG_CHIN },
    {"SAVE_TO_DB", "データベースに保存しますか？", LANG_JAPN },
    { "SAVE_ON_EXIT", "Do you wish to save changes before exiting?", LANG_ENG },
    { "SAVE_ON_EXIT", "¿Desea guardar los cambios antes de salir?", LANG_SPAN },
    { "SAVE_ON_EXIT", "Voulez-vous enregistrer les modifications avant de quitter?", LANG_FREN },
    { "SAVE_ON_EXIT", "Möchten Sie die Änderungen vor dem Beenden speichern?", LANG_GERM },
    { "SAVE_ON_EXIT", "Vuoi salvare le modifiche prima di uscire?", LANG_ITAL },
    { "SAVE_ON_EXIT", "您是否希望在退出前保存更改？", LANG_CHIN },
    { "SAVE_ON_EXIT", "終了する前に変更を保存しますか？", LANG_JAPN },
    {"WISH_EXIT", "¿Desea salir?", LANG_SPAN },
    {"WISH_EXIT", "Voulez-vous sortir?", LANG_FREN },
    {"WISH_EXIT", "Möchten Sie die Anwendung beenden?", LANG_GERM },
    {"WISH_EXIT", "Vuoi uscire?", LANG_ITAL },
    {"WISH_EXIT", "您想退出吗？", LANG_CHIN },
    {"WISH_EXIT", "終了しますか？", LANG_JAPN },
    {"WARNING_ONE", "Warning: This process could take many minutes, depending on the speed of your computer."\
        "The file it is importing is quite large and could exhsuast all the physical memory in your machine."\
        "However, this program will not allow it to do that, and will create it into a database to lessen"\
        "the memory load. Please be patient and allow it to finish. This will only happen once!", LANG_ENG},
    {"THANKS", "Thank you for using Nano-SMS. I highly encourage you to backup and store your messages"\
    "As often as possible and to keep them in a save space. Are you ready to quit? Yes / NO.", LANG_ENG},
    { "THANKS", "Gracias por usar Nano-SMS. Le recomiendo encarecidamente que haga una copia de seguridad y almacene sus mensajes"\
    "tan a menudo como sea posible y para mantenerlos en un espacio seguro. ¿Estás listo para salir? Sí / NO.", LANG_SPAN
},
    { "THANKS", "Merci d'avoir utilisé Nano-SMS. Je vous encourage vivement à sauvegarder et à stocker vos messages"\
    "aussi souvent que possible et à les conserver dans un espace sûr. Êtes-vous prêt à quitter? Oui / NON.", LANG_FREN
    },
    { "THANKS", "Vielen Dank für die Nutzung von Nano-SMS. Ich empfehle Ihnen dringend, Ihre Nachrichten zu sichern und zu speichern"\
    "so oft wie möglich und sie an einem sicheren Ort aufzubewahren. Sind Sie bereit zu beenden? Ja / NEIN.", LANG_GERM
    },
    { "THANKS", "Grazie per aver utilizzato Nano-SMS. Ti consiglio vivamente di eseguire il backup e memorizzare i tuoi messaggi"\
    "il più spesso possibile e di conservarli in uno spazio sicuro. Sei pronto per uscire? Sì / NO.", LANG_ITAL
    },
    { "THANKS", "感谢您使用Nano-SMS。我强烈建议您尽可能频繁地备份和存储您的消息"\
    "并将它们保存在安全的空间中。您准备好退出了吗？是/否。", LANG_CHIN
    },
    { "THANKS", "Nano-SMSをご利用いただきありがとうございます。メッセージのバックアップと保存を強くお勧めします"\
    "できるだけ頻繁に、安全なスペースに保管してください。終了する準備はできましたか？はい/いいえ。", LANG_JAPN}
};