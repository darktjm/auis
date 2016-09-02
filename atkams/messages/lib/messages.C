/* ********************************************************************** *\
  *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
  *        For full copyright information see:'andrew/config/COPYRITE'     *
  \* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <andrewos.h>
#include <textview.H>
#include <sys/param.h>
#include <errprntf.h>

#include <message.H>
#include <completion.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <proctable.H>
#include <im.H>
#include <text.H>
#include <frame.H>
#include <environ.H>
#include <search.H>
#include <datacata.H>
#include <cui.h>
#include <fdphack.h>

#include <ams.H>
#include <amsutil.H>

#include <t822view.H>

#define dontDefineRoutinesFor_text822
#include <text822.H>
#undef dontDefineRoutinesFor_text822

#include <captions.H>
/* #include <options.ih> */
#include <messages.H>

#ifndef NOTREESPLEASE
#include <org.H>
#include <foldertreev.H>
#define dontDefineRoutinesFor_tree
#include <tree.H>
#undef dontDefineRoutinesFor_tree
#endif


ATKdefineRegistry(messages, textview, messages::InitializeClass);
void NoOp();
void BSM_RefreshDisplayedFolder(class messages  *self, char  *CheckAll);
void PurgeAllDeletions(class messages  *self);
void MessagesFocusCaptions(class messages  *self);
void MessagesFocusBodies(class messages  *self);
void TextviewCompound(class textview  *tv, char  *cmds);
void MessagesCompound(class messages  *self, char  *cmds);
void MessagesTextviewCommand(class messages  *self, char  *cmds);
void MessagesCaptionsCommand(class messages  *self, char  *cmds);
void MessagesBodiesCommand(class messages  *self, char  *cmds);
void BSM_DummyQuit(class messages  *self);
static void QuitMessages(class messages  *self);
void BSearchFPlease(class messages  *self);
void BSearchRPlease(class messages  *self);
void CSearchFPlease(class messages  *self);
void CSearchRPlease(class messages  *self);
void GSearchFPlease(class messages  *self);
void GSearchRPlease(class messages  *self);
void BSM_CreatePlease(class messages  *self);
void BSM_DeleteFolder(class messages  *self);
void BSM_SetPrinter(class messages  *self);
void NextMarked(class messages  *self);
void PrevMarked(class messages  *self);
void DeleteMarked(class messages  *self);
void UndeleteMarked(class messages  *self);
void PrintMarked(class messages  *self);
void ClassifyMarked(class messages  *self, char  *name);
void CopyMarked(class messages  *self, char  *name);
void AppendMarked(class messages  *self, char  *name);
int GetFolderName(class captions  *self, const char  *prompt , char  *FName);
void AppendMarkedToFile(class messages  *self);
void AppendMarkedToRawFile(class messages  *self);
void BackUp(class messages  *self);
void BSM_DeletePlease(class messages  *self);
void BSM_UndeletePlease(class messages  *self);
void BSM_ClassifyPlease(class messages  *self);
void BSM_CopyPlease(class messages  *self);
void FileByName(class messages  *self, char  *name, boolean  ReallyAppend);
void FileIntoByName(class messages  *self, char  *name);
void FileOntoByName(class messages  *self, char  *name);
void BSM_FileInto(class messages  *self);
void BSM_AppendOnto(class messages  *self);
void BSM_AppendPlease(class messages  *self);
void BSM_AppendDelPlease(class messages  *self);
void BSM_ReplyToSender(class messages  *self);
void BSM_SendForward(class messages  *self);
void BSM_MarkCurrent(class messages  *self);
void PrintVisibleMessage(class messages  *self);
#ifdef RCH_ENV
void PreviewVisibleMessage(class messages  *self);
#endif
void BSM_ReplyToReaders(class messages  *self);
void BSM_ReplyToBoth(class messages  *self);
void BSM_RestoreAsDraft(class messages  *self);
void BSM_ReplyToAll(class messages  *self);
void ThisIsFlorida(class messages  *self);
int AppendMessageToFile(class messages  *self);
int AppendMessageToRawFile(class messages  *self);
int PrepareAppendFileName(class captions  *self, char  *Buf);
void MarkVisibleMessageUnseen(class messages  *self);
void FindRelatedMessages(class messages  *self);
char *GetLastResendName();
void BSM_ReSendPlease(class messages  *self, char  *towhom);
void BSM_RedisplayFormat(class messages  *self);
void BSM_RedisplayNormal(class messages  *self);
void BSM_RedisplayFixedWidth(class messages  *self);
void BSM_RedisplayRot13(class messages  *self);
void BSM_ToggleRaw(class messages  *self, char  *ctype);
void BSM_DifferentContentType(class messages  *self, char  *ctype);
void BSM_ModifiableBody(class messages  *self);
void BSM_SelectBody(class messages  *self);
void PuntCurrent(class messages  *self, int  GoToNext);
void BSM_SendFresh(class messages  *self);
void ReplySendersMarked(class messages  *self);
void ReplyAllMarked(class messages  *self);
void ResendMarked(class messages  *self, char  *towhom);
void ExcerptMarked(class messages  *self);
void ClearMarks(class messages  *self);
void CountMarks(class messages  *self);
void RestoreOldMarks(class messages  *self);
void FindAllCaptions(class messages  *self);
void MarkRange(class messages  *self);
void ExpandFileIntoMenus(class messages  *self);
void ShrinkFileIntoMenus(class messages  *self);
void BSM_MorePlease(class messages  *self);
void BSM_NextPlease(class messages  *self);
void BSM_ScrollBackBody(class messages  *self);
void BSM_ScrollForwardBody(class messages  *self);
int ReadByName(class messages  *self);
int ReadNamedFolder(class messages  *self, char  *ShortName);
void BSM_RenamePlease(class messages  *self);
#ifndef NOTREESPLEASE
void GenNodeName(class tree  *tree, struct tree_node  *node, char  *buf);
void OrgHit(class messages  *self, class foldertreev  *folderTreeView, struct tree_node  *node, enum view_MouseAction  action, long  x , long  y , long  clicks);
int countdots(char  *s);
#endif
void BSM_ShowTreePlease(class messages  *self);
void SubscribeByName(class messages  *self);
void UnSubscribeByName(class messages  *self);
void AlterSubByName(class messages  *self);
void SetSubStatus(char  *nickname, int  substatus);
void DeleteWindow(class messages  *self);
void DirectlyClassify(class messages  *self, int  classnum);
void CheckMenuMasks(class messages  *self);
void BSM_NextDigest(class messages  *self);
void BSM_PrevDigest(class messages  *self);


static class keymap *messages_standardkeymap, *messages_permkeymap;
static class menulist *messages_standardmenulist, *messages_permmenulist;

static int folderTreeExists = 0;

#define MENUMASK_MSGSHOWING 1
#define MENUMASK_MSGWRITABLE 2
#define MENUMASK_MSGDELETED 4
#define MENUMASK_MSGNOTDELETED 8
#define MENUMASK_HASMARKS 16
#define MENUMASK_SHOWMORENEXT 32
#define MENUMASK_SETQUITHERE 64
#define MENUMASK_MARKING 128
#define MENUMASK_FILEINTO 256
#define MENUMASK_THREEREPLIES 512
#define MENUMASK_MARKASUNREAD 1024
#define MENUMASK_FORMATMENUS 2048
#define MENUMASK_APPENDBYNAME 4096
#define MENUMASK_FILEINTOMENU 8192
#define MENUMASK_MARKEDEXTRAS 16384
#define MENUMASK_EXPANDEDMENUS 32768
#define MENUMASK_PUNTMENU 65536
#define MENUMASK_NOTTHREEREPLIES 131072

#define ISSTRING(s) ((long)(s) < 0 || (long)(s)>255)

extern void FSearchFPlease(class messages *);
extern void FSearchRPlease(class messages *);
extern class captions *GetCaptions(class messages  *self);
extern class captions *GetCaptionsNoCreate(class messages  *self);
extern class t822view *GetBodies(class messages  *self);
extern boolean ClearSM(class captions  *self);
extern void sm_SetMessagesOptions(class messages  *self);
extern void BSM_ShowHelp(class messages  *self);
extern void BSM_CheckNewPlease(class messages  *self);
extern void BSM_ReadMailPlease(class messages  *self);
extern void BSM_ShowNewPlease(class messages  *self);
extern void BSM_ShowPersonalPlease(class messages  *self);
extern void BSM_ShowAllPlease(class messages  *self);
extern void BSM_ShowSubscribedPlease(class messages  *self);
extern void MessagesSendmessageCommand(class messages  *self, char  *cmds);
extern void MessagesFoldersCommand(class messages  *self, char  *cmds);
extern void MessagesFocusFolders(class messages  *self);


/* permbindings: the key and menu bindings listed here always appear. */
static struct bind_Description messages_permbindings [] = {
    /* procname, keysequenece, key rock, menu string, menu rock, menu mask, proc, docstring, dynamic autoload */
    {NULL, "\033\t", 0, NULL, 0, 0, NULL, NULL, NULL}, /* No inset insertion */
    {"messages-dummy-quit", "\003", 0, NULL, 0, 0, (proctable_fptr)BSM_DummyQuit, "dummy quit command"},
    {"messages-quit", "\030\003", 0, "Quit~99", 0, 0, (proctable_fptr)QuitMessages, "exit messages"},
    {"messages-current-search", "\023", 0, NULL, 0, 0, (proctable_fptr)GSearchFPlease, "Search forward in current view"},
    {"messages-current-rsearch", "\022", 0, NULL, 0, 0, (proctable_fptr)GSearchRPlease, "Search backward in current view"}, 
    {"messages-send-message", "\030\023", 0, "Send/Post Message~51", 0, 0, (proctable_fptr)BSM_SendFresh, "send a message"},
    {"messages-delete-window", "\030\004", 0, "Delete Window~89", 0, 0, (proctable_fptr)DeleteWindow, "Delete this messages subwindow."},

    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL},
};

/* standardbindings: the key bindings listed here only show up if you have the keystrokes option turned on. The menu bindings always appear. */
static struct bind_Description messages_standardbindings [] = {
    /* procname, keysequenece, key rock, menu string, menu rock, menu mask, proc, docstring, dynamic autoload */
    {"messages-subscribe-by-name", NULL, 0, NULL, 0, 0, (proctable_fptr)SubscribeByName, "Subscribe to a folder by name"},
    {"messages-unsubscribe-by-name", NULL, 0, NULL, 0, 0, (proctable_fptr)UnSubscribeByName, "Unsubscribe to a folder by name"},
    {"messages-alter-subscription-by-name", NULL, 0, NULL, 0, 0, (proctable_fptr)AlterSubByName, "Alter a subscription to a folder by name"},
    {"messages-purge-deletions", NULL, 0, "Other~60,Purge Deletions~55", 0, 0, (proctable_fptr)PurgeAllDeletions, "Purge all deleted messages."},
    {"messages-set-options", NULL, 0, "Other~60,Set Options~44", 0, 0, (proctable_fptr)sm_SetMessagesOptions, "Set Messages Options"},
    {"messages-refresh-displayed-folder", NULL, 0, NULL, 0, 0, (proctable_fptr)BSM_RefreshDisplayedFolder, "Refresh the folder currently on display"},
    {"messages-expose-new", NULL, 0, "Message Folders~55,Expose Changed~12", 0, 0, (proctable_fptr)BSM_ShowNewPlease, "Show Changed folders"},
    {"messages-expose-all", NULL, 0, "Message Folders~55,Expose All~14", 0, 0, (proctable_fptr)BSM_ShowAllPlease, "Show all folders"},
    {"messages-expose-subscribed", NULL, 0, "Message Folders~55,Expose Subscribed~15", 0, 0, (proctable_fptr)BSM_ShowSubscribedPlease, "Show subscribed folders"},
    {"messages-expose-personal", NULL, 0, "Message Folders~55,Expose Personal~16", 0, MENUMASK_FILEINTO, (proctable_fptr)BSM_ShowPersonalPlease, "Show personal folders"},
#ifndef NOTREESPLEASE
    {"messages-expose-tree", NULL, 0, "Message Folders~55,Expose Tree~17", 0, 0, (proctable_fptr)BSM_ShowTreePlease, "Show all folders"},
#endif
    {"messages-create-folder", NULL, 0, "Message Folders~55,Create~21", 0, MENUMASK_FILEINTO, (proctable_fptr)BSM_CreatePlease, "Create folder"},
    {"messages-delete-folder", NULL, 0, "Message Folders~55,Delete~22", 0, MENUMASK_FILEINTO, (proctable_fptr)BSM_DeleteFolder, "Delete folder"},
    {"messages-rename-folder", NULL, 0, "Message Folders~55,Rename~23", 0, MENUMASK_FILEINTO, (proctable_fptr)BSM_RenamePlease, "Rename folder"},
    {"messages-read-by-name", "\030\026", 0, "Message Folders~55,Read By Name~51", 0, 0, (proctable_fptr)ReadByName, "Read any named message folder"},
    {"messages-read-mail", "\030\022", 0, "Read Mail~41", 0, 0, (proctable_fptr)BSM_ReadMailPlease, "Read your mail"},
    {"messages-check-new-messages", "\030\001", 0, "Other~60,Check New Messages~20", 0, 0, (proctable_fptr)BSM_CheckNewPlease, "Check for new messages"},
#ifdef RCH_ENV
    {"messages-print-current", "\033\020", 0, "This Message~30,Print...~41", 0, MENUMASK_MSGSHOWING, (proctable_fptr)PrintVisibleMessage, "Print current message"},
    {"messages-preview-current", 0, 0, "This Message~30,Preview...~42", 0, MENUMASK_MSGSHOWING, (proctable_fptr)PreviewVisibleMessage, "Preview current message"},
#else
    {"messages-set-printer", NULL, 0, "Other~60,Set Printer~45", 0, 0, (proctable_fptr)BSM_SetPrinter, "Set printer to use"},
    {"messages-print-current", "\033\020", 0, "This Message~30,Print~41", 0, MENUMASK_MSGSHOWING, (proctable_fptr)PrintVisibleMessage, "Print current message"},
#endif
    {"messages-show-help", "?", 0, NULL, 0, 0, (proctable_fptr)BSM_ShowHelp, "Show basic help text"},
    {"messages-captions-search", NULL, 0, "Search/Spell~1,Captions Forward~31", 0, 0, (proctable_fptr)CSearchFPlease, "Search forward in captions"},
    {"messages-captions-rsearch", NULL, 0, "Search/Spell~1,Captions Backward~32", 0, 0, (proctable_fptr)CSearchRPlease, "Search backward in captions"},
    {"messages-folders-search", NULL, 0,  "Search/Spell~1,Folders Forward~21", 0, 0, (proctable_fptr)FSearchFPlease, "Search forward in folders"},
    {"messages-folders-rsearch", NULL, 0, "Search/Spell~1,Folders Backward~22", 0, 0, (proctable_fptr)FSearchRPlease, "Search backward in folders"},
    {"messages-bodies-search", NULL, 0,  "Search/Spell~1,Body Forward~41", 0, 0, (proctable_fptr)BSearchFPlease, "Search forward in bodies"},
    {"messages-bodies-rsearch", NULL, 0, "Search/Spell~1,Body Backward~42", 0, 0, (proctable_fptr)BSearchRPlease, "Search backward in bodies"},
    {NULL, NULL, 0,  "Search/Spell~1,Forward", 0, 0, NULL, NULL},
    {NULL, NULL, 0,  "Search/Spell~1,Backward", 0, 0, NULL, NULL},

    {NULL, NULL, 0,  "Page,Insert Pagebreak", 0, 0, NULL, NULL},
    {NULL, NULL, 0,  "Page,Next Page", 0, 0, NULL, NULL},
    {NULL, NULL, 0,  "Page,Previous Page", 0, 0, NULL, NULL},
    {NULL, NULL, 0,  "Page,Insert Footnote", 0, 0, NULL, NULL},
    {NULL, NULL, 0,  "Page,Open Footnotes", 0, 0, NULL, NULL},
    {NULL, NULL, 0,  "Page,Close Footnotes", 0, 0, NULL, NULL},
    {NULL, NULL, 0,  "Page,Table of Contents", 0, 0, NULL, NULL},

    {"messages-next-marked", NULL, 0, "Marked Messages~25,Next~2", 0, MENUMASK_HASMARKS, (proctable_fptr)NextMarked, "Show next marked message."},
    {"messages-previous-marked", NULL, 0, "Marked Messages~25,Previous~4", 0, MENUMASK_HASMARKS, (proctable_fptr)PrevMarked, "Show previous marked message."},
    {"messages-delete-marked", NULL, 0, "Marked Messages~25,Delete All~10", 0, MENUMASK_HASMARKS, (proctable_fptr)DeleteMarked, "Delete all marked messages."},
    {"messages-undelete-marked", NULL, 0, "Marked Messages~25,Undelete All~11", 0, MENUMASK_HASMARKS, (proctable_fptr)UndeleteMarked, "Undelete all marked messages."},
    {"messages-print-marked", NULL, 0, "Marked Messages~25,Print All~20", 0, MENUMASK_HASMARKS, (proctable_fptr)PrintMarked, "Print all marked messages."},
    {"messages-clear-marks", NULL, 0, "Marked Messages~25,Clear Marks~70", 0, MENUMASK_HASMARKS, (proctable_fptr)ClearMarks, "Clear out all marks on messages."},
    {"messages-count-marks", NULL, 0, "Marked Messages~25,Count Marks~80", 0, MENUMASK_HASMARKS, (proctable_fptr)CountMarks, "Count marked messages."},


    {"messages-file-all-marked", NULL, 0, "Send/File Marked~26,File All Into~20", 0, MENUMASK_HASMARKS, (proctable_fptr)ClassifyMarked, "File all marked messages into some folder."},
    {"messages-copy-all-marked", NULL, 0, "Send/File Marked~26,Copy All Into~21", 0, MENUMASK_HASMARKS | MENUMASK_MARKEDEXTRAS, (proctable_fptr)CopyMarked, "File all marked messages into some folder."},
    {"messages-append-all-marked", NULL, 0, "Send/File Marked~26,Append To Folder~22", 0, MENUMASK_HASMARKS | MENUMASK_MARKEDEXTRAS, (proctable_fptr)AppendMarked, "Append all marked messages onto some folder."},
    {"messages-append-all-to-file", NULL, 0, "Send/File Marked~26,Append To File~30", 0, MENUMASK_HASMARKS, (proctable_fptr)AppendMarkedToFile, "Append all marked messages onto some file."},
    {"messages-append-all-to-file-raw", NULL, 0, "Send/File Marked~26,Append To Raw File~30", 0, MENUMASK_HASMARKS, (proctable_fptr)AppendMarkedToRawFile, "Append all marked messages onto some file in RAW format."},
    {"messages-reply-to-senders", NULL, 0, "Send/File Marked~26,Reply To Senders~40", 0, MENUMASK_HASMARKS, (proctable_fptr)ReplySendersMarked, "Reply to senders of all marked messages."},
    {"messages-reply-all-to-all", NULL, 0, "Send/File Marked~26,Reply To All~42", 0, MENUMASK_HASMARKS, (proctable_fptr)ReplyAllMarked, "Reply to senders and readers of all marked messages."},
    {"messages-resend-all", NULL, 0, "Send/File Marked~26,Resend All~50", 0, MENUMASK_HASMARKS, (proctable_fptr)ResendMarked, "Resend all marked messages."},
    {"messages-excerpt-all", NULL, 0, "Send/File Marked~26,Excerpt All~60", 0, MENUMASK_HASMARKS, (proctable_fptr)ExcerptMarked, "Excerpt all marked messages into sendmessage window."},

    {"messages-restore-marks", NULL, 0, "Search/Spell~1,Restore Old Marks~63", 0, MENUMASK_MARKING, (proctable_fptr)RestoreOldMarks, "Restore the most recent set of marks."},
    {"messages-find-all", NULL, 0, "Search/Spell~1,Mark By Keyword~51", 0, MENUMASK_MARKING, (proctable_fptr)FindAllCaptions, "Mark all captions containing a given string."},
    {"messages-find-range", NULL, 0, "Search/Spell~1,Mark By Date~52", 0, MENUMASK_MARKING, (proctable_fptr)MarkRange, "Mark all messages in a given date range."},
    {"messages-shrink-file-into-menus", NULL, 0, "File Into...~40,Shrink Menu~99", 0, MENUMASK_MSGSHOWING | MENUMASK_FILEINTOMENU, (proctable_fptr)ShrinkFileIntoMenus, "Shrink the list on the 'file into' menu."},
    {"messages-expand-file-into-menus", NULL, 0, "File Into...~40,Expand Menu~99", 0, MENUMASK_MSGSHOWING | MENUMASK_FILEINTOMENU, (proctable_fptr)ExpandFileIntoMenus, "Expand the list on the 'file into' menu."},
    {"messages-show-more", " ", 0, "Show More~1", 0, MENUMASK_SHOWMORENEXT, (proctable_fptr)BSM_MorePlease, "Show more or next"},
    {"messages-show-next", "\n", 0, "Show Next~2", 0, MENUMASK_SHOWMORENEXT, (proctable_fptr)BSM_NextPlease, "Show more or next"},
    {"messages-show-next", "n", 0, NULL, 0, 0, (proctable_fptr)BSM_NextPlease, "Show next"},
    {"messages-scroll-back-body", "b", 0, NULL, 0, 0, (proctable_fptr)BSM_ScrollBackBody, "Scroll body backward"},
    {"messages-scroll-forward-body", "v", 0, NULL, 0, 0, (proctable_fptr)BSM_ScrollForwardBody, "Scroll body forward"},
    {"messages-show-previous", "p", 0, NULL, 0, 0, (proctable_fptr)BackUp, "Show previous"},
    {"messages-delete-current", "d", 0, "Delete~21", 0, MENUMASK_MSGSHOWING | MENUMASK_MSGNOTDELETED, (proctable_fptr)BSM_DeletePlease, "Delete current message"},
    {"messages-undelete-current", "u", 0, "Undelete~21", 0, MENUMASK_MSGSHOWING | MENUMASK_MSGDELETED, (proctable_fptr)BSM_UndeletePlease, "Delete current message"},
    {"messages-classify-this", "c", 0, NULL, 0, 0, (proctable_fptr)BSM_ClassifyPlease, "Classify current message"},
    {"messages-file-and-delete", NULL, 0, "File Into...~40,By Name~77", 0, MENUMASK_MSGSHOWING | MENUMASK_FILEINTOMENU, (proctable_fptr)BSM_FileInto, "File this into some folder & delete it if possible."},
    {"messages-classify-by-name", NULL, 0, NULL, 0, 0, (proctable_fptr)FileIntoByName, "File this into some named folder."},
    {"messages-append-by-name", NULL, 0, NULL, 0, 0, (proctable_fptr)FileOntoByName, "Append this onto some named folder."},
    {"messages-append-and-delete", NULL, 0, "File Into...~40,Append By Name~88", 0, MENUMASK_APPENDBYNAME | MENUMASK_MSGSHOWING | MENUMASK_FILEINTOMENU, (proctable_fptr)BSM_AppendOnto, "Append this to some folder & delete it if possible."},
    {"messages-copy-to-folder", "C", 0, NULL, 0, 0, (proctable_fptr)BSM_CopyPlease, "Copy current message into folder"},
    {"messages-append-this", "A", 0, NULL, 0, 0, (proctable_fptr)BSM_AppendPlease, "Append current message to a folder"},
    {"messages-append-delete", "a", 0, NULL, 0, 0, (proctable_fptr)BSM_AppendDelPlease, "Append current message to a folder and delete it"},
    {"messages-reply-to-sender", "r", 0, "Reply to Sender~31", 0, MENUMASK_MSGSHOWING, (proctable_fptr)BSM_ReplyToSender, "Reply to sender"},
    {"messages-forward-message", "f", 0, "This Message~30,Forward To~11", 0, MENUMASK_MSGSHOWING, (proctable_fptr)BSM_SendForward, "Forward current message"},
    {"messages-mark-current", "M", 0, NULL, 0, 0, (proctable_fptr)BSM_MarkCurrent, "Mark current message"},
    {"messages-reply-to-readers", NULL, 0, "Reply to Readers~32", 0, MENUMASK_MSGSHOWING | MENUMASK_THREEREPLIES, (proctable_fptr)BSM_ReplyToReaders, "Reply to readers"},
    {"messages-reply-to-both", NULL, 0, "Reply to Both~33", 0, MENUMASK_MSGSHOWING | MENUMASK_THREEREPLIES, (proctable_fptr)BSM_ReplyToBoth, "Reply to both"},
    {"messages-reply-to-all", "R", 0, "Reply to All~33", 0, MENUMASK_MSGSHOWING | MENUMASK_NOTTHREEREPLIES, (proctable_fptr)BSM_ReplyToAll, "Reply to all"},
    {"messages-restore-as-draft", NULL, 0, "This Message~30,Restore Draft~15", 0, MENUMASK_MSGSHOWING | MENUMASK_MSGWRITABLE | MENUMASK_FILEINTO, (proctable_fptr)BSM_RestoreAsDraft, "Restore as draft"},
    {"messages-set-quit-here", NULL, 0, "Other~60,Set Quit Here~88", 0, MENUMASK_SETQUITHERE, (proctable_fptr)ThisIsFlorida, "Set the current message to be the first one seen next time."},
    {"messages-append-to-file", NULL, 0, "This Message~30,Append to File~21", 0, MENUMASK_MSGSHOWING, (proctable_fptr)AppendMessageToFile, "Append to file"},
    {"messages-append-to-raw-file", NULL, 0, "This Message~30,Append to Raw File~21", 0, MENUMASK_MSGSHOWING, (proctable_fptr) AppendMessageToRawFile, "Append to file in raw format"},
    {"messages-mark-as-unseen", NULL, 0, "This Message~30,Mark as Unread~80", 0, MENUMASK_MSGSHOWING | MENUMASK_MSGWRITABLE | MENUMASK_MARKASUNREAD, (proctable_fptr)MarkVisibleMessageUnseen, "Mark this message as unread."},
    {"messages-find-related", NULL, 0, "Search/Spell~1,Mark Related Messages~53", 0, MENUMASK_MARKING | MENUMASK_MSGSHOWING, (proctable_fptr)FindRelatedMessages, "Find all captions related to the message on display"},
    {"messages-resend", NULL, 0, "This Message~30,Resend To~12", 0, MENUMASK_MSGSHOWING, (proctable_fptr)BSM_ReSendPlease, "ReSend current message"},
    {"messages-redisplay-formatted", NULL, 0, NULL, 0, 0, (proctable_fptr)BSM_RedisplayFormat, "Show interpreting formatting"},
    {"messages-redisplay-normal", NULL, 0, NULL, 0, 0, (proctable_fptr)BSM_RedisplayNormal, "Show formatting normally"},
    {"messages-redisplay-fixed", NULL, 0, "This Message~30,Fixed Width~32", 0, MENUMASK_MSGSHOWING | MENUMASK_FORMATMENUS, (proctable_fptr)BSM_RedisplayFixedWidth, "Show in fixed-width font"},
    {"messages-redisplay-rot13", NULL, 0, "This Message~30,Descramble~31", 0, MENUMASK_MSGSHOWING | MENUMASK_FORMATMENUS, (proctable_fptr)BSM_RedisplayRot13, "Run rot-13 descrambling algorithm"},
    {"messages-select-body", NULL, 0, NULL, 0, 0, (proctable_fptr)BSM_SelectBody, "Select the body of the message"},
    {"messages-next-digest", "\033n", 0, NULL, 0, 0, (proctable_fptr)BSM_NextDigest, "Find the next encapsulated message in the message body"},
    {"messages-previous-digest", "\033p", 0, NULL, 0, 0, (proctable_fptr)BSM_PrevDigest, "Find the previous encapsulated message in the message body"},
    {"messages-modifiable-body", NULL, 0, NULL, 0, 0, (proctable_fptr)BSM_ModifiableBody, "Make the body of the message not be read-only"},
    {"messages-different-content-type", NULL, 0, NULL, 0, 0, (proctable_fptr)BSM_DifferentContentType, "Display the body of a message using a different content-type header"},
    {"messages-show-as-plain-text", NULL, 0, NULL, 0, MENUMASK_MSGSHOWING, (proctable_fptr)BSM_ToggleRaw, "Toggle display of the body of a message between normal and plain text"},
    {"messages-toggle-raw-text", NULL, 0, "This Message~30,Toggle Raw Body~33", 0, MENUMASK_MSGSHOWING, (proctable_fptr)BSM_ToggleRaw, "Toggle display of the body of a message between normal and plain text"},
    {"messages-punt", ">", 1, "Other~60,Punt~94", 1, MENUMASK_PUNTMENU, (proctable_fptr)PuntCurrent, "Punt current folder and go to the next one"},
    {"messages-punt-and-stay", "~", 0, NULL, 0, 0, (proctable_fptr)PuntCurrent, "Punt current folder but don't go on to the next one"},
    {"textview-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)TextviewCompound, "Execute a compound textview operation"},
    {"messages-compound-operation", NULL, 0, NULL, 0, 0, (proctable_fptr)MessagesCompound, "Execute a compound Messages operation"},
    {"messages-textview-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)MessagesTextviewCommand, "Execute a compound 'textview' operation on the folders"},
    {"messages-sendmessage-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)MessagesSendmessageCommand, "Execute a compound 'sendmessage' operation."},
    {"messages-folders-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)MessagesFoldersCommand, "Execute a compound 'captions' operation."},
    {"messages-captions-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)MessagesCaptionsCommand, "Execute a compound 'captions' operation."},
    {"messages-bodies-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)MessagesBodiesCommand, "Execute a compound 'captions' operation."},
    {"messages-focus-on-folders", NULL, 0, NULL, 0, 0, (proctable_fptr)MessagesFocusFolders, "Make the current folders the input focus."},
    {"messages-focus-on-captions", NULL, 0, NULL, 0, 0, (proctable_fptr)MessagesFocusCaptions, "Make the current captions the input focus."},
    {"messages-focus-on-bodies", NULL, 0, NULL, 0, 0, (proctable_fptr)MessagesFocusBodies, "Make the current bodies area the input focus."},


    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL},
};


static long NoOp(class ATK *dummy, long rock) {
    message::DisplayString(NULL, 10, "This command does nothing.");
    return 0;
}

proctable_fptr messtextv_ReverseSearchCmd	= NoOp,
messtextv_ScrollScreenBackCmd	= NoOp,
messtextv_ScrollScreenForwardCmd   = NoOp,
messtextv_ForwardSearchCmd		= NoOp;

boolean messages::InitializeClass() 
{
    struct proctable_Entry *tempProc;

    ATK::LoadClass("textview");
    if ((tempProc = proctable::Lookup("textview-search")) != NULL) {
	messtextv_ForwardSearchCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-reverse-search")) != NULL) {
	messtextv_ReverseSearchCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-prev-screen")) != NULL) {
	messtextv_ScrollScreenBackCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-next-screen")) != NULL) {
	messtextv_ScrollScreenForwardCmd = proctable::GetFunction(tempProc);
    }

    messages_standardmenulist = new menulist;
    messages_standardkeymap = (class keymap *)ATK::NewObject("keymap");
    bind::BindList(messages_standardbindings, messages_standardkeymap, messages_standardmenulist, &messages_ATKregistry_ );

    messages_permmenulist = new menulist;
    messages_permkeymap = (class keymap *)ATK::NewObject("keymap");
    bind::BindList(messages_permbindings, messages_permkeymap, messages_permmenulist, &messages_ATKregistry_ );
    return(TRUE);
}

#define BIGMENUCARD 25  /* Most menu items to construct on a single file into card */

void messages::ResetFileIntoMenus()
{
    static struct proctable_Entry *directlyclassifyproc = NULL;
    int i, max, smallmax, biggermax;
    char MenuString[25+MAXPATHLEN];

    ams::InitializeClassList();
    if (this->fileintomenulist) {
	(this->fileintomenulist)->ClearML();
    } else {
	this->fileintomenulist = menulist::Create(this);
    }

    if(this->expandedmenuslist) (this->expandedmenuslist)->ClearML();
    else this->expandedmenuslist=menulist::Create(this);

    if (ams::GetLastMenuClass() < 0) return;
    max = ams::GetClassListCount();
    smallmax = environ::GetProfileInt("messages.maxclassmenu", 8);
    biggermax = environ::GetProfileInt("messages.maxtotalclassmenu", BIGMENUCARD);
    if(biggermax>BIGMENUCARD) biggermax=BIGMENUCARD;

    if (smallmax > max) smallmax = max;
    if (!directlyclassifyproc) {
	directlyclassifyproc = proctable::DefineProc("messages-directly-classify", (long (*)(class ATK*, long))DirectlyClassify, &messages_ATKregistry_ , NULL, "");
    }
    for(i=max - 1; i>=0; --i) {
	if (i < biggermax) {
	    sprintf(MenuString, "File Into...~40,%s", ams::GetClassListEntry(i));
	} else {
	    sprintf(MenuString, "File Into (%d)~%d,%s", i/biggermax, 40+i/biggermax, ams::GetClassListEntry(i));
	}
	if(i<smallmax)  (this->fileintomenulist)->AddToML( MenuString, directlyclassifyproc,  i, MENUMASK_MSGSHOWING | MENUMASK_FILEINTOMENU);
	else (this->expandedmenuslist)->AddToML( MenuString, directlyclassifyproc,  i, MENUMASK_MSGSHOWING | MENUMASK_FILEINTOMENU);
    }
}

void BSM_RefreshDisplayedFolder(class messages  *self, char  *CheckAll)
{
    class captions *c = GetCaptions(self);
    if (c->FullName) {
	char nickname[1+MAXPATHLEN], fullname[1+MAXPATHLEN];

	strcpy(nickname, c->ShortName);
	strcpy(fullname, c->FullName);
	if (ISSTRING(CheckAll)) {
	    (ams::GetAMS())->CUI_CheckMailboxes( fullname);
	    (c)->ClearAndUpdate( TRUE, TRUE);
	}
	(c)->InsertUpdatesInDocument( nickname, fullname, FALSE);
    } else {
	message::DisplayString(NULL, 10, "There is no folder on display.");
    }
}

void PurgeAllDeletions(class messages  *self)
{
    ams::WaitCursor(TRUE);
    (ams::GetAMS())->CUI_PurgeMarkedDirectories( FALSE, FALSE);
    BSM_RefreshDisplayedFolder(self, FALSE); 
    ams::WaitCursor(FALSE);
}

void MessagesFocusCaptions(class messages  *self)
{
    class captions *c = GetCaptions(self);
    (c)->WantInputFocus( c);
}

void MessagesFocusBodies(class messages  *self)
{
    class t822view *tv = GetBodies(self);
    (tv)->WantInputFocus( tv);
}

void TextviewCompound(class textview  *tv, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( tv, "textview", cmds);
}

void MessagesCompound(class messages  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( self, "messages", cmds);
}

void MessagesTextviewCommand(class messages  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( self, "textview", cmds);
}

void MessagesCaptionsCommand(class messages  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( GetCaptions(self), "captions", cmds);
}

void MessagesBodiesCommand(class messages  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( GetBodies(self), "t822view", cmds);
}

void messages::SetWhatIAm(int  w)
{
    this->WhatIAm = w;
}

void BSM_DummyQuit(class messages  *self)
{
    message::DisplayString(NULL, 10, "Use ^X^C to quit.");
}

static void QuitMessages(class messages  *self) 
{
    ams::CommitState(TRUE, FALSE, TRUE, TRUE);
}

void BSearchFPlease(class messages  *self)
{
    messtextv_ForwardSearchCmd(GetBodies(self),0);
    (self)->WantInputFocus( self);
}

void BSearchRPlease(class messages  *self)
{
    messtextv_ReverseSearchCmd( GetBodies(self), 0);
    (self)->WantInputFocus( self);
}

void CSearchFPlease(class messages  *self)
{
    messtextv_ForwardSearchCmd( GetCaptions(self), 0);
    (self)->WantInputFocus( self);
}

void CSearchRPlease(class messages  *self)
{
    (GetCaptions(self))->CapReverseSearch(); /* Handles virtual scroll bar */
}

void GSearchFPlease(class messages  *self)
{
    switch(self->WhatIAm) {
	case WHATIAM_FOLDERS:
	    FSearchFPlease(self);
	    break;
	case WHATIAM_CAPTIONS:
	    CSearchFPlease(self);
	    break;
	case WHATIAM_BODIES:
	    BSearchFPlease(self);
	    break;
    }
}

void GSearchRPlease(class messages  *self)
{
    switch(self->WhatIAm) {
	case WHATIAM_FOLDERS:
	    FSearchRPlease(self);
	    break;
	case WHATIAM_CAPTIONS:
	    CSearchRPlease(self);
	    break;
	case WHATIAM_BODIES:
	    BSearchRPlease(self);
	    break;
    }
}

void BSM_CreatePlease(class messages  *self)
{
    char *s, *s2, ShortName[1+MAXPATHLEN], NewFullName[1+MAXPATHLEN], *TempPtr, ErrorText[100 + MAXPATHLEN];
    if (message::AskForString(NULL, 50, "Create folder: ", "", ShortName, sizeof(ShortName)) < 0) return;

    s = rindex(ShortName, '.');
    s2 = rindex(ShortName, '/');
    if (s2 > s) s = s2;
    if (s) *s++ = '\0';
    if (s == NULL) {
	sprintf(NewFullName, "%s/%s", ams::GetMailPath(), ShortName);
    } else {
	long code;
	code = (ams::GetAMS())->CUI_DisambiguateDir( ShortName, &TempPtr);
	if (code) {
	    code = (ams::GetAMS())->MS_DisambiguateFile( ShortName, NewFullName, AMS_DISAMB_EXISTS);
	    if (code) {
		(ams::GetAMS())->CUI_ReportAmbig( ShortName, "folder");
		return;
	    } else {
		sprintf(ErrorText, "Warning -- %s is not a message directory; I assume it is a root", ShortName);
		message::DisplayString(NULL, 10, ErrorText);
	    }
	} else {
	    strcpy(NewFullName, TempPtr);
	}
	strcat(NewFullName, "/");
	strcat(NewFullName, s);
    }
    ams::WaitCursor(TRUE);
    message::DisplayString(NULL, 10, "Creating folder; please wait...");
    im::ForceUpdate();
    (ams::GetAMS())->CUI_CreateNewMessageDirectory( NewFullName, NewFullName); /* reports own errors */
    ams::WaitCursor(FALSE);
}

void BSM_DeleteFolder(class messages  *self)
{
    char ShortName[1+MAXPATHLEN], *FullName;

    if (message::AskForString(NULL, 50, "Delete folder: ", "", ShortName, sizeof(ShortName)) < 0) return;
    if ((ams::GetAMS())->CUI_DisambiguateDir( ShortName, &FullName)) {
	(ams::GetAMS())->CUI_ReportAmbig( ShortName, "folder");
	return;
    }
    ams::WaitCursor(TRUE);
    message::DisplayString(NULL, 10, "Removing folder; please wait...");
    im::ForceUpdate();
    (ams::GetAMS())->CUI_RemoveDirectory( FullName); /* reports own errors */
    ams::WaitCursor(FALSE);
}

void BSM_SetPrinter(class messages  *self)
{
    char Pnamebuf[100], Prompt[110];
    static char LastPrinterName[100] = "";

    strcpy(Prompt, "Printer to use");
    if (LastPrinterName[0]) {
	strcat(Prompt, " [");
	strcat(Prompt, LastPrinterName);
	strcat(Prompt, "]");
    }
    strcat(Prompt, ": ");
    if (message::AskForString(NULL, 50, Prompt, "", Pnamebuf, sizeof(Pnamebuf)) < 0) return;
    if (Pnamebuf[0]) {
	if (!(ams::GetAMS())->CUI_SetPrinter( Pnamebuf)) {
	    strcpy(LastPrinterName, Pnamebuf);
	}
    }
}

void NextMarked(class messages  *self)
{
    (GetCaptions(self))->ShowMore( FALSE, FALSE, TRUE);
}

void PrevMarked(class messages  *self)
{
    (GetCaptions(self))->BackUpCheckingMarks( TRUE);
}

void DeleteMarked(class messages  *self)
{
    (GetCaptions(self))->ActOnMarkedMessages( MARKACTION_DELETE, NULL);
}

void UndeleteMarked(class messages  *self)
{
    (GetCaptions(self))->ActOnMarkedMessages( MARKACTION_UNDELETE, NULL);
}

void PrintMarked(class messages  *self)
{
    (GetCaptions(self))->ActOnMarkedMessages( MARKACTION_PRINT, NULL);
}

void ClassifyMarked(class messages  *self, char  *name)
{
    char FName[10+MAXPATHLEN];
    class captions *c = GetCaptions(self);

    if (!ISSTRING(name)) {
	GetFolderName(c, "File", FName);
	if (FName[0] == '\0') return;
	name = FName;
    }
    (c)->ActOnMarkedMessages( MARKACTION_CLASSIFYBYNAME, name);
}

void CopyMarked(class messages  *self, char  *name)
{
    char FName[10+MAXPATHLEN];
    class captions *c = GetCaptions(self);

    if (!ISSTRING(name)) {
	GetFolderName(c, "Copy", FName);
	if (FName[0] == '\0') return;
	name = FName;
    }
    (c)->ActOnMarkedMessages( MARKACTION_COPYBYNAME, name);
}

void AppendMarked(class messages  *self, char  *name)
{
    char FName[10+MAXPATHLEN];
    class captions *c = GetCaptions(self);

    if (!ISSTRING(name)) {
	GetFolderName(c, "Append", FName);
	if (FName[0] == '\0') return;
	name = FName;
    }
    (c)->ActOnMarkedMessages( MARKACTION_APPENDBYNAME, name);
}

int GetFolderName(class captions  *self, const char  *prompt , char  *FName)
{
    char ErrorText[500];
    const char *lc = (self)->GetLastClassification();

    FName[0] = '\0';
    sprintf(ErrorText, "%s all %d marked messages into what folder [%s] : ",
	     prompt, self->MarkCount, lc);
    if (ams::GetFolderName(ErrorText, FName, MAXPATHLEN, "", FALSE)) return(-1);
    if (FName[0] == '\0') {
	strcpy(FName, lc);
    }
    return(0);
}

void AppendMarkedToFile(class messages  *self)
{
    char Buf[1+MAXPATHLEN];
    class captions *c = GetCaptions(self);

    if (PrepareAppendFileName(c, Buf)) return;
    (c)->ActOnMarkedMessages( MARKACTION_APPENDTOFILE, Buf);
}    

void AppendMarkedToRawFile(class messages  *self)
{
    char Buf[1+MAXPATHLEN];
    class captions *c = GetCaptions(self);

    if (PrepareAppendFileName(c, Buf)) return;
    (c)->ActOnMarkedMessages( MARKACTION_APPENDTOFILERAW, Buf);
}    

void BackUp(class messages  *self)
{
    (GetCaptions(self))->BackUpCheckingMarks( FALSE);
}

void BSM_DeletePlease(class messages  *self)
{
    (GetCaptions(self))->DeleteVisibleMessage( TRUE);
}

void BSM_UndeletePlease(class messages  *self)
{
    (GetCaptions(self))->DeleteVisibleMessage( FALSE);
}

void BSM_ClassifyPlease(class messages  *self)
{
    (GetCaptions(self))->CloneMessage( MS_CLONE_COPYDEL);
}

void BSM_CopyPlease(class messages  *self)
{
    (GetCaptions(self))->CloneMessage( MS_CLONE_COPY);
}

void FileByName(class messages  *self, char  *name, boolean  ReallyAppend)
{
    int OpCode;
    class captions *c = GetCaptions(self);

    if (c->VisibleCUID < 1) {
	message::DisplayString(NULL, 10, "There is no message on display.");
	return;
    }
    ams::WaitCursor(TRUE);
    OpCode = AMS_GET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_MAYMODIFY) ? MS_CLONE_COPYDEL : MS_CLONE_COPY;
    if (ReallyAppend) {
	OpCode = AMS_GET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_MAYMODIFY) ? MS_CLONE_APPENDDEL : MS_CLONE_APPEND;
    }
    if (!(ams::GetAMS())->CUI_CloneMessage( c->VisibleCUID, name, OpCode)) {
	/* cuilib reports errors, here we deal with success */
	if (OpCode == MS_CLONE_APPENDDEL || OpCode == MS_CLONE_COPYDEL) {
	    (c->CaptText)->SetEnvironmentStyle( c->HighlightEnv, c->ActiveDeletedStyle);
	    AMS_SET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_DELETED);
	    (c)->WantUpdate( c);
	    (c)->AlterDeletedIcon( c->HighlightStartPos, TRUE);
	    (c)->PostMenus( NULL);
	}
    }
    ams::WaitCursor(FALSE);
}

void FileIntoByName(class messages  *self, char  *name)
{
  if (!ISSTRING(name)) name = 0;
    FileByName(self, name, FALSE);
}

void FileOntoByName(class messages  *self, char  *name)
{
  if (!ISSTRING(name)) name = 0;
    FileByName(self, name, TRUE);
}

void BSM_FileInto(class messages  *self)
{
    class captions *c = GetCaptions(self);

    (c)->CloneMessage( (AMS_GET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_MAYMODIFY)) ? MS_CLONE_COPYDEL : MS_CLONE_COPY);
}

void BSM_AppendOnto(class messages  *self)
{
    class captions *c = GetCaptions(self);

    (c)->CloneMessage( (AMS_GET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_MAYMODIFY)) ? MS_CLONE_APPENDDEL : MS_CLONE_APPEND);
}

void BSM_AppendPlease(class messages  *self)
{
    (GetCaptions(self))->CloneMessage( MS_CLONE_APPEND);
}

void BSM_AppendDelPlease(class messages  *self)
{
    (GetCaptions(self))->CloneMessage( MS_CLONE_APPENDDEL);
}

void BSM_ReplyToSender(class messages  *self)
{
    (GetCaptions(self))->SendMessage( AMS_REPLY_SENDER);
}

void BSM_SendForward(class messages  *self)
{
    (GetCaptions(self))->SendMessage( AMS_REPLY_FORWARD_FMT);
}

void BSM_MarkCurrent(class messages  *self)
{
    (GetCaptions(self))->MarkCurrent();
}

void PrintVisibleMessage(class messages  *self)
{
#ifdef RCH_ENV
    captions *c = GetCaptions(self);
    ATK::LoadClass("rchprint");	// Hack...call it via a proc to avoid a direct reference
    struct proctable_Entry *pe = proctable::Lookup("rchprint-doprint");
    if (pe) {
	void (*doprint)(view *, view *, char *, int) = (void (*)(view *, view *, char *, int))(pe->proc);
	// Warning:  as a kludge, doprint knows about "Mail-Message" as a title.
	doprint(c, c->BodView, "Mail-Message", 1);
    } else {
	message::DisplayString(NULL, 10, "Cannot load rchprint dialog.");
    }
#else
    (GetCaptions(self))->PrintVisibleMessage();
#endif
}

#ifdef RCH_ENV
void PreviewVisibleMessage(class messages  *self)
{
    captions *c = GetCaptions(self);

    ATK::LoadClass("rchprint");	// Hack...call it via a proc to avoid a direct reference
    struct proctable_Entry *pe = proctable::Lookup("rchprint-doprint");
    if (pe) {
	void (*doprint)(view *, view *, char *, int) = (void (*)(view *, view *, char *, int))(pe->proc);
	// Warning:  as a kludge, doprint knows about "Mail-Message" as a title.
	doprint(c, c->BodView, "Mail-Message", 2);
    } else {
	message::DisplayString(NULL, 10, "Cannot load rchprint dialog.");
    }
}
#endif

void BSM_ReplyToReaders(class messages  *self)
{
    (GetCaptions(self))->SendMessage( AMS_REPLY_WIDE);
}

void BSM_ReplyToBoth(class messages  *self)
{
    (GetCaptions(self))->SendMessage( AMS_REPLY_WIDER);
}

void BSM_RestoreAsDraft(class messages  *self)
{
    (GetCaptions(self))->SendMessage( AMS_REPLY_REDRAFT);
}

void BSM_ReplyToAll(class messages  *self)
{
    class captions *c = GetCaptions(self);
    if (AMS_GET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_MAYMODIFY)) {
	(c)->SendMessage( AMS_REPLY_WIDER);
    } else {
	(c)->SendMessage( AMS_REPLY_WIDE);
    }
}

void ThisIsFlorida(class messages  *self)
{
    (GetCaptions(self))->ThisIsFlorida();
}

int AppendMessageToFile(class messages  *self)
{
    char Buf[1+MAXPATHLEN];
    class captions *c = GetCaptions(self);

    if (c->VisibleCUID < 0) {
	message::DisplayString(NULL, 25, "There is no message on display.");
	return(-1);
    }
    if (PrepareAppendFileName(c, Buf)) return(-1);
    return (self)->AppendOneMessageToFile( c->VisibleCUID, Buf, 0);
}

int AppendMessageToRawFile(class messages  *self)
{
    char Buf[1+MAXPATHLEN];
    class captions *c = GetCaptions(self);

    if (c->VisibleCUID < 0) {
	message::DisplayString(NULL, 25, "There is no message on display.");
	return(-1);
    }
    if (PrepareAppendFileName(c, Buf)) return(-1);
    return (self)->AppendOneMessageToFile( c->VisibleCUID, Buf, 1);
}

static char LastAppendFileName[1+MAXPATHLEN] = "~/SavedMessages";

int PrepareAppendFileName(class captions  *self, char  *Buf)
{
    if (LastAppendFileName[0]) {
	strcpy(Buf, LastAppendFileName);
    } else {
	(ams::GetAMS())->TildeResolve( "~", Buf);
    }
    if (completion::GetFilename(NULL, "Append message body to file: ", Buf, Buf, MAXPATHLEN, FALSE, FALSE) == -1 ) {
	return (-1);
    }	
    return(0);
}

int messages::AppendOneMessageToFile(int  cuid, char  *Buf, int  DoRaw)
{
    char ErrorText[256], TmpFile[1+MAXPATHLEN], Splat[5000];
    FILE *fp, *rfp;
    int ShouldDelete, amtread, magicnum;
    class datacata *datacat;
    boolean dres;

    ams::WaitCursor(TRUE);
    fp = fopen(Buf, "a");
    if (fp == NULL) {
	sprintf(ErrorText, "Sorry; can't open file '%s' (error %d).", (ams::GetAMS())->ap_Shorten( Buf), errno);
	message::DisplayString(NULL, 50, ErrorText);
	ams::WaitCursor(FALSE);
	im::ForceUpdate();
	return(-1);
    }
    if ((ams::GetAMS())->CUI_ReallyGetBodyToLocalFile( cuid, TmpFile, &ShouldDelete, !(ams::GetAMS())->CUI_SnapIsRunning())) {
	fclose(fp);
	ams::WaitCursor(FALSE);
	return(-1); /* error message already reported */
    }
    rfp = fopen(TmpFile, "r");
    if (!rfp) {
	fclose(fp);
	sprintf(ErrorText, "Cannot open local file %s for reading", (ams::GetAMS())->ap_Shorten( TmpFile));
	message::DisplayString(NULL, 50, ErrorText);
	ams::WaitCursor(FALSE);
	return(-1);
    }
    magicnum = ftell(fp) + 2623;
    if (!DoRaw) fprintf(fp, "\\begindata{text822, %d}\n", magicnum);
    while ((amtread = fread(Splat, sizeof(char), sizeof(Splat), rfp)) > 0) {
	if ((ams::GetAMS())->fwriteallchars( Splat, amtread, fp) != amtread) {
	    sprintf(ErrorText, "Error writing file %s--sorry.", (ams::GetAMS())->ap_Shorten( Buf));
	    message::DisplayString(NULL, 50, ErrorText);
	    fclose(rfp);
	    fclose(fp);
	    ams::WaitCursor(FALSE);
	    return(-1);
	}
    }
    if (ferror(rfp)) {
	sprintf(ErrorText, "Error reading file %s--sorry.", (ams::GetAMS())->ap_Shorten( TmpFile));
	message::DisplayString(NULL, 50, ErrorText);
	fclose(rfp);
	fclose(fp);
	ams::WaitCursor(FALSE);
	return(-1);
    }
    fclose(rfp);
    if (ShouldDelete) {
	unlink(TmpFile);
    }
    if (!DoRaw) fprintf(fp, "\\enddata{text822, %d}\n", magicnum);
    if (vfclose(fp)) {
	sprintf(ErrorText, "Sorry; can't close file '%s' (error %d).", (ams::GetAMS())->ap_Shorten( Buf), errno);
	message::DisplayString(NULL, 50, ErrorText);
	ams::WaitCursor(FALSE);
	return(-1);
    }

    /* invoke datacat to clean up result */
    datacat = new datacata;
    if (!datacat) {
	sprintf(ErrorText, "Unable to invoke datacat; '%s' may be corrupt.", (ams::GetAMS())->ap_Shorten( Buf));
	message::DisplayString(NULL, 10, ErrorText);
	ams::WaitCursor(FALSE);
	return(-1);
    }

    (datacat)->BeginRun();
    (datacat)->SetCleanMode( TRUE);
    (datacat)->SetVerboseLevel( datacata_TrackQuiet);
    dres = (datacat)->AddFile( Buf);
    if (dres) {
	(datacat)->SetOutputFile( Buf);
	(datacat)->FinishRun();
	delete datacat;
    }
    else {
	(datacat)->SetOutputFile( "/dev/null");
	(datacat)->FinishRun();
	delete datacat;
	sprintf(ErrorText, "Datacat failed; '%s' may be corrupt.", (ams::GetAMS())->ap_Shorten( Buf));
	message::DisplayString(NULL, 10, ErrorText);
	ams::WaitCursor(FALSE);
	return(-1);
    }

    sprintf(ErrorText, "Wrote file '%s'", (ams::GetAMS())->ap_Shorten( Buf));
    message::DisplayString(NULL, 10, ErrorText);
    strcpy(LastAppendFileName, Buf);
    
    ams::WaitCursor(FALSE);
    return(0);
}

void MarkVisibleMessageUnseen(class messages  *self)
{
    class captions *ci = GetCaptions(self);

    if (AMS_GET_ATTRIBUTE(ci->VisibleSnapshot, AMS_ATT_MAYMODIFY) && ! AMS_GET_ATTRIBUTE(ci->VisibleSnapshot, AMS_ATT_UNSEEN) && (ams::GetAMS())->CUI_MarkAsUnseen( ci->VisibleCUID)) {
	return; /* error was reported */
    }
    AMS_SET_ATTRIBUTE(ci->VisibleSnapshot, AMS_ATT_UNSEEN);
    (ci)->MarkVisibleMessageStateofSeeing( FALSE);
}

void FindRelatedMessages(class messages  *self)
{
    (GetCaptions(self))->FindRelatedMessages();
}

static char LastResendName[1+MAXPATHLEN] = "";

char *GetLastResendName() {return(LastResendName);}

void BSM_ReSendPlease(class messages  *self, char  *towhom)
{
    char ShortName[1+MAXPATHLEN], Prompt[100+MAXPATHLEN];

    if (LastResendName[0]) {
	sprintf(Prompt, "Resend this message to [%s]: ", LastResendName);
    } else {
	strcpy(Prompt, "Resend this message to: ");
    }
    if (ISSTRING(towhom)) {
	strcpy(ShortName, towhom);
    } else {
	if (message::AskForString(NULL, 50, Prompt, "", ShortName, sizeof(ShortName)) < 0) return;
	if (ShortName[0] == '\0') strcpy(ShortName, LastResendName);
    }
    if (ShortName[0] == '\0') {
	message::DisplayString(NULL, 10, "Not resending the message -- no recipient specified.");
	return;
    }
    message::DisplayString(NULL, 10, "Resending message; please wait");
    ams::WaitCursor(TRUE);
    if (!(ams::GetAMS())->CUI_ResendMessage( GetCaptions(self)->VisibleCUID, ShortName)) {
	/* That routine reports its own errors & success, here we just remember the latest successful resend... */
	strcpy(LastResendName, ShortName);
    }
    ams::WaitCursor(FALSE);
}

void BSM_RedisplayFormat(class messages  *self)
{
    class captions *c = GetCaptions(self);

    (c)->Redisplay( c->CurrentFormatting ^ MODE822_FORMAT, NULL);
}

void BSM_RedisplayNormal(class messages  *self)
{
    (GetCaptions(self))->Redisplay( MODE822_NORMAL, NULL);
}

void BSM_RedisplayFixedWidth(class messages  *self)
{
    class captions *c = GetCaptions(self);

    (c)->Redisplay( c->CurrentFormatting ^ MODE822_FIXEDWIDTH, NULL);
}


void BSM_RedisplayRot13(class messages  *self)
{
    class captions *c = GetCaptions(self);

    (c)->Redisplay( c->CurrentFormatting ^ MODE822_ROT13, NULL);
}

void BSM_ToggleRaw(class messages  *self, char  *ctype)
{
    captions* caps = GetCaptions (self);

    if (!caps->rawText)
	caps->Redisplay (caps->CurrentFormatting |
			    MODE822_FORMAT, "text/plain");
    else
	caps->Redisplay (caps->CurrentFormatting |
			    MODE822_FORMAT, NULL);
}

void BSM_DifferentContentType(class messages  *self, char  *ctype)
{
    char buf[1000];
    class captions *c = GetCaptions(self);

    if (!ISSTRING(ctype)) {
	if (message::AskForString(NULL, 50, "Enter new content-type field: " , NULL, buf, sizeof(buf)) < 0) {
	    return;
	}
	ctype = buf;
    }
    (c)->Redisplay( c->CurrentFormatting | MODE822_FORMAT, ctype);
}

void BSM_ModifiableBody(class messages  *self) 
{
    class text *text = (class text *) (GetBodies(self))->GetDataObject();
    (text)->SetReadOnly( FALSE);
    (text)->NotifyObservers( 0);	/* Update menus and post key state */
}

void BSM_SelectBody(class messages  *self) 
{
    class t822view *tv = GetBodies(self);
    int bodstart = GetCaptions(self)->StartOfRealBody;

    (tv)->SetDotPosition( bodstart);
    (tv)->SetDotLength( ((class text *) (tv)->GetDataObject())->GetLength() - bodstart);
}

void PuntCurrent(class messages  *self, int  GoToNext)
{
    (GetCaptions(self))->PuntCurrent( GoToNext);
}

void BSM_SendFresh(class messages  *self)
{
    (GetCaptions(self))->SendMessage( AMS_REPLY_FRESH);
}

void ReplySendersMarked(class messages  *self)
{
    class captions *c = GetCaptions(self);
    if (ClearSM(c)) return;
    (c)->ActOnMarkedMessages( MARKACTION_REPLYSENDERS, NULL);
}

void ReplyAllMarked(class messages  *self)
{
    class captions *c = GetCaptions(self);
    if (ClearSM(c)) return;
    (c)->ActOnMarkedMessages( MARKACTION_REPLYALL, NULL);
}

void ResendMarked(class messages  *self, char  *towhom)
{
    char *s, Prompt[100+MAXPATHLEN], ShortResendAddress[1+MAXPATHLEN], *ResendAddress = NULL;

    s = GetLastResendName();
    if (*s) {
	sprintf(Prompt, "Resend these messages to [%s]: ", s);
    } else {
	strcpy(Prompt, "Resend these messages to: ");
    }
    if (ISSTRING(towhom)) {
	strcpy(ShortResendAddress, towhom);
    } else {
	if (message::AskForString(NULL, 50, Prompt, "", ShortResendAddress, sizeof(ShortResendAddress)) < 0) return;
	if (ShortResendAddress[0] == '\0') strcpy(ShortResendAddress, s);
    }
    if (ShortResendAddress[0] == '\0') {
	message::DisplayString(NULL, 10, "Not resending the messages -- no recipient specified.");
	return;
    }
    ams::WaitCursor(TRUE);
    if ((ams::GetAMS())->CUI_RewriteHeaderLine( ShortResendAddress, &ResendAddress)) {
	/* error reported */
	ams::WaitCursor(FALSE);
	return;
    }
    strcpy(s, ShortResendAddress); /* validated well, anyway */
    (GetCaptions(self))->ActOnMarkedMessages( MARKACTION_RESEND, ResendAddress);
    if (ResendAddress) free(ResendAddress);
    ams::WaitCursor(FALSE);
}

void ExcerptMarked(class messages  *self)
{
    (GetCaptions(self))->ActOnMarkedMessages( MARKACTION_EXCERPT, NULL);
}

void ClearMarks(class messages  *self)
{
    (GetCaptions(self))->ClearMarks();
}

void CountMarks(class messages  *self)
{
    char ErrorText[100];
    int ct = GetCaptions(self)->MarkCount;

    if (ct == 1) {
	strcpy(ErrorText, "There is one marked message.");
    } else {
	sprintf(ErrorText, "There are %s marked messages.", amsutil::cvEng(ct, 0, 1000));
    }
    message::DisplayString(NULL, 10, ErrorText);
}

void RestoreOldMarks(class messages  *self)
{
    class captions *c = GetCaptions(self);

    (c)->GuaranteeFetchedRange( 0, c->FolderSize);
    (c)->ActOnMarkedMessages( MARKACTION_RESTORE, NULL);
}

void FindAllCaptions(class messages  *self)
{
    (GetCaptions(self))->SearchAll();
}

void MarkRange(class messages  *self)
{
    (GetCaptions(self))->MarkRangeOfMessages();
}

void ExpandFileIntoMenus(class messages  *self)
{
    (GetCaptions(self))->AlterFileIntoMenus( FALSE);
}

void ShrinkFileIntoMenus(class messages  *self)
{
    (GetCaptions(self))->AlterFileIntoMenus( TRUE);
}

void BSM_MorePlease(class messages  *self)
{
    (GetCaptions(self))->ShowMore( TRUE, TRUE, FALSE);
}

void BSM_NextPlease(class messages  *self)
{
    (GetCaptions(self))->ShowMore( FALSE, TRUE, FALSE);
}

void BSM_ScrollBackBody(class messages  *self)
{
    messtextv_ScrollScreenBackCmd(GetBodies(self), 0);
}

void BSM_ScrollForwardBody(class messages  *self)
{
    messtextv_ScrollScreenForwardCmd(GetBodies(self), 0);
}

int ReadByName(class messages  *self) 
{
    char ShortName[256];

    if (ams::GetFolderName("Which folder do you want to read? ", ShortName, sizeof(ShortName)-1, "", TRUE)) return(-1);
    return(ReadNamedFolder(self, ShortName));
}

int ReadNamedFolder(class messages  *self, char  *ShortName)
{
    char *FullName;
    int code;

    if ((ams::GetAMS())->CUI_DisambiguateDir( ShortName, &FullName)) {
	if ((ams::GetAMS())->AMS_ErrNo() == EACCES) {
	    char ErrorText[1000];

	    sprintf(ErrorText, "%s is private; you do not have read-access.", ShortName);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, TRUE, (ams::GetAMS())->MSErrCode());
	} else {
	    (ams::GetAMS())->CUI_ReportAmbig( ShortName, "folder");
	}
	return(-1);
    }
    code = (GetCaptions(self))->InsertUpdatesInDocument( ShortName, FullName, FALSE);
    if (!code) message::DisplayString(NULL, 10, "Done.");
    return(code);
}

void BSM_RenamePlease(class messages  *self)
{
    char ShortName[1+MAXPATHLEN], *FullName, NewName[1+MAXPATHLEN];

    if (message::AskForString(NULL, 50, "Rename old folder: ", "", ShortName, sizeof(ShortName)) < 0) return;
    if ((ams::GetAMS())->CUI_DisambiguateDir( ShortName, &FullName)) {
	(ams::GetAMS())->CUI_ReportAmbig( ShortName, "folder");
	return;
    }
    if (message::AskForString(NULL, 50, "New name: ", "", NewName, sizeof(NewName)) < 0) return;	
    ams::WaitCursor(TRUE);
    message::DisplayString(NULL, 10, "Renaming folder; please wait...");
    im::ForceUpdate();
    (ams::GetAMS())->CUI_RenameDir( FullName, NewName); /* routine reported errors */
    ams::WaitCursor(FALSE);
}

#ifndef NOTREESPLEASE
void GenNodeName(class tree  *tree, struct tree_node  *node, char  *buf)
{
    char *s;

    if (!node) return; /* Bottom out recursion */
    s =((node) ? node->name : NULL);
    if (!strncmp(s, "ROOT:", 5)) return; /* Another bottoming out */
    GenNodeName(tree, ((node) ? node->parent : NULL), buf);
    if (buf[0]) strcat(buf, ".");
    strcat(buf, s);
}

void OrgHit(class messages  *self, class foldertreev  *folderTreeView, struct tree_node  *node, enum view_MouseAction  action, long  x , long  y , long  clicks)
{
    char Buf[1+MAXPATHLEN];
    class org *org = NULL;

    if (action == view_RightDown) {
	org = (class org*)(folderTreeView)->GetDataObject();
	Buf[0] = '\0';
	GenNodeName(org->tree_data_object, node, Buf);
	ReadNamedFolder(self, Buf);
    }
}

int countdots(char  *s)
{
    int dots = 0;

    while (*s) {
	if (*s++ == '.') ++dots;
    }
    return(dots);
}
#endif

void
messages::ObservedChanged( class observable  *changed, long		       change )
{
    (this)->textview::ObservedChanged(changed,change);
    switch(change) {
	case 1:
	    if(this->folderTree && ((class foldertreev*)changed == this->folderTree)) {
		class im *im = (this->folderFrame)->GetIM();
		(this->folderFrame)->SetView(NULL);
		(im)->SetView(NULL);
		(this->folderFrame)->Destroy();
		(im)->Destroy();
		this->folderTree = NULL;
		this->folderFrame = NULL;
		folderTreeExists--;
	    }
    }
}

void BSM_ShowTreePlease(class messages  *self)
{
#ifndef NOTREESPLEASE
    class org *o;
    class foldertreev *folderTreeView;
    FILE *fp, *mfp;
    char PathElt[1+MAXPATHLEN], MapFile[1+MAXPATHLEN], ErrorText[100+MAXPATHLEN], RemoteMapFile[1+MAXPATHLEN], LineBuf[2000], OrgFileName[1+MAXPATHLEN], RootName[1+MAXPATHLEN], *s, LastName[1+MAXPATHLEN];
    int path, len1, len2;
    long errcode = 0;

    ams::WaitCursor(TRUE);
    if(self->folderTree || (folderTreeExists > 0)) {
	message::DisplayString(self, 10, "A folder tree is already exposed.");
	ams::WaitCursor(FALSE);
	return;
    }
    (ams::GetAMS())->CUI_GenTmpFileName( OrgFileName);
    if ((fp = fopen(OrgFileName, "w")) == NULL) {
	message::DisplayString(self, 10, "Could not open output file.");
	ams::WaitCursor(FALSE);
	return;
    }
    fputs("Folder Tree\n{\n", fp);
    o = new org;
    self->folderTree = folderTreeView = new foldertreev;
    if (!o || !folderTreeView) {
	message::DisplayString(self, 10, "Could not create tree objects!");
	fclose(fp);
	ams::WaitCursor(FALSE);
	return;
    }
    (folderTreeView)->SetDataObject( o);
    (folderTreeView)->SetHitHandler( (org_hitfptr)OrgHit, self);
    PathElt[0] = '\0';
    for (path=0; ; ++path) {
	if ((ams::GetAMS())->MS_GetSearchPathEntry( path, PathElt, MAXPATHLEN)) {
	    break;
	}
	if ((ams::GetAMS())->MS_NameSubscriptionMapFile( PathElt, MapFile)) {
	    sprintf(ErrorText, "MS can not generate subscription map file for %s", PathElt);
	    if ((ams::GetAMS())->AMS_ErrNo() == ENOENT) {
		(ams::GetAMS())->ReportSuccess( ErrorText);
		continue; /* user may not have his own message dir, for example */
	    }
	    (ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, TRUE, errcode);
	    continue;
	}
	if (!(ams::GetAMS())->CUI_OnSameHost()) {
	    strcpy(RemoteMapFile, MapFile);
	    (ams::GetAMS())->CUI_GenTmpFileName( MapFile);
	    if ((errcode = (ams::GetAMS())->CUI_GetFileFromVice( MapFile, RemoteMapFile)) != 0) {
		sprintf(ErrorText, "Cannot copy map file %s from server file %s", MapFile, (ams::GetAMS())->ap_Shorten( RemoteMapFile));
		(ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, TRUE, errcode);
		continue;
	    }
	    (ams::GetAMS())->MS_UnlinkFile( RemoteMapFile);
	}
	if ((mfp = fopen(MapFile, "r")) == NULL) {
	    sprintf(ErrorText, "Cannot open map file %s (for %s - error %d)", MapFile, PathElt, errno);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, FALSE, 0);
	    unlink(MapFile);
	    continue;
	}
	strcpy(RootName, PathElt);
	s = rindex(RootName, '/');
	if (s) *s = '\0';
	s = rindex(RootName, '/');
	if (s) {
	    *s++ = '\0';
	} else {
	    s = RootName;
	}
	fputs("ROOT: ", fp);
	fputs(s, fp);
	fputs("\n{\n", fp);
	LastName[0] = '\0';
	while(fgets(LineBuf, sizeof(LineBuf), mfp)) {
	    s = index(LineBuf, ':');
	    if (s) *s = '\0';
	    len1 = strlen(LineBuf);
	    len2 = strlen(LastName);
	    if ((len2 < len1)
		&& LineBuf[len2] == '.'
		&& !strncmp(LineBuf, LastName, len2)) {
		fputs("{\n", fp);
	    } else {
		len1 = countdots(LastName) - countdots(LineBuf);
		while (len1 > 0) {
		    fputs("}\n", fp);
		    --len1;
		}
		/* This next line is only necessary if there are oddities in the map file, but it has happened before... */
		while (len1 < 0) {
		    fputs("{\n", fp);
		    ++len1;
		}
	    }
	    s = rindex(LineBuf, '.');
	    if (s) {
		++s;
	    } else {
		s = LineBuf;
	    }
	    fputs(s, fp);
	    fputs("\n", fp);
	    strcpy(LastName, LineBuf);
	}
	fclose(mfp);
	unlink(MapFile); 
	len1 = countdots(LastName);
	while (len1 > 0) {
	    fputs("}\n", fp);
	    --len1;
	}
	fputs("}\n", fp);
    }
    fputs("}\n", fp);
    fclose(fp);
    fp = fopen(OrgFileName, "r");
    if (!fp) {
	message::DisplayString(self, 10, "Could not read map file.");
	ams::WaitCursor(FALSE);
	return;
    }
    (o)->Read( fp, 0);
    fclose(fp);
    unlink(OrgFileName); 
    self->folderFrame = ams::InstallInNewWindow((folderTreeView)->GetApplicationLayer(), "messages-tree", "Folder Tree", environ::GetProfileInt("foldertree.width", 600), environ::GetProfileInt("foldertree.height", 400), folderTreeView);
    (self->folderTree)->AddObserver(self);
    folderTreeExists++;
    ams::WaitCursor(FALSE);
#endif
}


void SubscribeByName(class messages  *self)
{
    char buf[1+MAXPATHLEN];

    if (ams::GetFolderName("Subscribe to what folder: " , buf, sizeof(buf), NULL, TRUE) == 0) {
	SetSubStatus(buf, AMS_ALWAYSSUBSCRIBED);
    }
}

void UnSubscribeByName(class messages  *self)
{
    char buf[1+MAXPATHLEN];

    if (ams::GetFolderName("Unsubscribe to what folder: " , buf, sizeof(buf), NULL, TRUE) == 0) {
	SetSubStatus(buf, AMS_UNSUBSCRIBED);
    }
}

void AlterSubByName(class messages  *self)
{
    char buf[1+MAXPATHLEN];

    if (ams::GetFolderName("Alter subscription to what folder: " , buf, sizeof(buf), NULL, TRUE) == 0) {
	SetSubStatus(buf, amsutil::ChooseNewStatus(buf, AMS_ALWAYSSUBSCRIBED, TRUE));
    }
}

void SetSubStatus(char  *nickname, int  substatus)
{
    long errcode;
    char *FullName, ErrorText[150+MAXPATHLEN];

    ams::WaitCursor(TRUE);
    if ((ams::GetAMS())->CUI_DisambiguateDir( nickname, &FullName)) {
	(ams::GetAMS())->CUI_ReportAmbig( nickname, "folder");
	return;
    }
    if ((errcode = (ams::GetAMS())->MS_SetSubscriptionEntry( FullName, nickname, substatus)) != 0) {
	ams::WaitCursor(FALSE);
	if ((ams::GetAMS())->AMS_ErrNo() == EACCES) {
	    sprintf(ErrorText, "'%s' is private; you don't have read-access or are unauthenticated.", nickname);
	} else if ((ams::GetAMS())->vdown( (ams::GetAMS())->AMS_ErrNo())) {
	    sprintf(ErrorText, "%s: temporarily unavailable (net/server problem)", nickname);
	} else if ((ams::GetAMS())->AMS_ErrNo() == ENOENT) {
	    sprintf(ErrorText, "Sorry; %s no longer exists, so you cannot subscribe to it.", nickname);
	} else {
	    sprintf(ErrorText, "Cannot set subscription entry to %s", nickname);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, TRUE, errcode);
	    return;
	}
	message::DisplayString(NULL, 75, ErrorText);
	return;
    }
    ams::SubscriptionChangeHook(FullName, nickname, substatus, NULL);
    ams::WaitCursor(FALSE);
}
static const char lastWindowWarning[] =
"This is the last window.";
static const char * const lastWindowChoices[] = {
    "Continue Running",
    "Quit Messages",
    NULL
};

#define lastWindow_CANCEL 0
#define lastWindow_QUIT   1

void DeleteWindow(class messages  *self)
{
    if (ams::CountAMSViews() > 1) {
	if (self->WhatIAm == WHATIAM_CAPTIONS) {
	    ((class captions *)self)->ClearAndUpdate( FALSE, TRUE);
	}
	ams::CommitState(FALSE, FALSE, FALSE, FALSE);
	if (self->GetIM()) (self->GetIM())->Destroy();
	(self)->Destroy();
    }
    else {
	long answer;
	if (message::MultipleChoiceQuestion(NULL, 0,
					    lastWindowWarning, lastWindow_CANCEL,
					    &answer, lastWindowChoices, NULL)
	    == -1)
	    return;
	switch(answer){
	    case lastWindow_CANCEL:
		return;

	    case lastWindow_QUIT :
		QuitMessages(self);
	}
    }
}

void DirectlyClassify(class messages  *self, int  classnum)
{
    int cuid, OpCode;
    char WhichClass[1+MAXPATHLEN], *s, *t;
    class captions *c = GetCaptions(self);

    cuid = c->VisibleCUID;    
    OpCode = AMS_GET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_MAYMODIFY) ? MS_CLONE_COPYDEL : MS_CLONE_COPY;
    if (cuid < 1) {
	message::DisplayString(NULL, 10, "There is no message on display.");
	return /* 0 */;
    }
    ams::WaitCursor(TRUE);
    strcpy(WhichClass, ams::GetClassListEntry(classnum));
    s = index(WhichClass, '~');
    if (s) *s = '\0';
    t = WhichClass;
    if (*t == '*') {
	++t;
	OpCode = AMS_GET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_MAYMODIFY) ? MS_CLONE_APPENDDEL : MS_CLONE_APPEND;
    }
    if (!(ams::GetAMS())->CUI_CloneMessage( cuid, t, OpCode)) {
	/* cuilib reports errors, here we deal with success */
	if (OpCode == MS_CLONE_APPENDDEL || OpCode == MS_CLONE_COPYDEL) {
	    (c->CaptText)->SetEnvironmentStyle( c->HighlightEnv, c->ActiveDeletedStyle);
	    AMS_SET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_DELETED);
	    (c)->WantUpdate( c);
	    (c)->AlterDeletedIcon( c->HighlightStartPos, TRUE);
	    (c)->PostMenus( NULL);
	    (self)->PostMenus( NULL);
	}
    }
    ams::WaitCursor(FALSE);
}

messages::messages()
{
    ATKinit;

    this->fileintomenulist = NULL;
    this->mykeys = keystate::Create(this, messages_standardkeymap);
    this->mymenulist = (messages_standardmenulist)->DuplicateML( this);
    this->mypermkeys = keystate::Create(this, messages_permkeymap);
    this->mypermmenulist = (messages_permmenulist)->DuplicateML( this);
    this->expandedmenuslist = NULL;
    this->WhatIAm = WHATIAM_UNDEFINED;
    this->folderTree = NULL;
    this->folderFrame = NULL;
    THROWONFAILURE((TRUE));
}

messages::~messages()
{
    ATKinit;

    (this->mymenulist)->ClearChain();
    (this->mypermmenulist)->ClearChain();
    delete this->mymenulist;
    delete this->mypermmenulist;
    delete this->mykeys;
    delete this->mypermkeys;
    if (this->fileintomenulist) delete this->fileintomenulist;
}

void CheckMenuMasks(class messages  *self)
{
    long mymask;
    class captions *c; 

    mymask = ((amsutil::GetOptBit(EXP_SHOWMORENEXT) ? MENUMASK_SHOWMORENEXT : 0)
	      | (amsutil::GetOptBit(EXP_SETQUITHERE) ? MENUMASK_SETQUITHERE  : 0)
	      | (amsutil::GetOptBit(EXP_MARKING) ? MENUMASK_MARKING  : 0)
	      | (amsutil::GetOptBit(EXP_FILEINTO) ? MENUMASK_FILEINTO : 0)
	      | (amsutil::GetOptBit(EXP_THREEREPLIES) ? MENUMASK_THREEREPLIES : MENUMASK_NOTTHREEREPLIES)
	      | (amsutil::GetOptBit(EXP_MARKASUNREAD) ? MENUMASK_MARKASUNREAD : 0)
	      | (amsutil::GetOptBit(EXP_FORMATMENUS) ? MENUMASK_FORMATMENUS : 0)
	      | (amsutil::GetOptBit(EXP_APPENDBYNAME) ? MENUMASK_APPENDBYNAME : 0)
	      | (amsutil::GetOptBit(EXP_FILEINTOMENU) ? MENUMASK_FILEINTOMENU : 0)
	      | (amsutil::GetOptBit(EXP_MARKEDEXTRAS) ? MENUMASK_MARKEDEXTRAS : 0)
	      | (amsutil::GetOptBit(EXP_PUNTMENU) ? MENUMASK_PUNTMENU : 0));
    c = GetCaptionsNoCreate(self);
    if (c) {
	if (c->VisibleCUID > 0) {
	    mymask |= MENUMASK_MSGSHOWING;
	    if (AMS_GET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_MAYMODIFY)) {
		mymask |= MENUMASK_MSGWRITABLE;
	    }
	    if (AMS_GET_ATTRIBUTE(c->VisibleSnapshot, AMS_ATT_DELETED)) {
		mymask |= MENUMASK_MSGDELETED;
	    } else {
		mymask |= MENUMASK_MSGNOTDELETED;
	    }
	}
	if (c->MarkCount > 0) mymask |= MENUMASK_HASMARKS;
	if (!self->fileintomenulist) (self)->ResetFileIntoMenus();
	if(self->fileintomenulist) (self->fileintomenulist)->ClearChain();
	if(self->expandedmenuslist) {
	    if (c->MenusExpanded) {
		(self->expandedmenuslist)->ClearChain();
		(self->fileintomenulist)->ChainBeforeML( self->expandedmenuslist, (long)self->expandedmenuslist);
		(self->expandedmenuslist)->SetMask( mymask);
	    }
	}
	(self->fileintomenulist)->SetMask( mymask);
    }
    (self->mymenulist)->SetMask( mymask);
}

void BSM_NextDigest(class messages  *self)
{
    class t822view *tv = (class t822view *) GetBodies(self);
    class text *t = (class text *) (tv)->GetDataObject();
    struct SearchPattern *Pattern = NULL;
    const char *tp = search::CompilePattern("\012--", &Pattern);
    int tmploc;

    if (tp) {
	message::DisplayString(NULL, 10, tp);
    } else {
	tmploc = search::MatchPattern(t, (tv)->GetDotPosition(), Pattern);
	if (tmploc >= 0) {
	    (tv)->SetDotPosition( ++tmploc);
	    (tv)->SetTopPosition( tmploc);
	}
    }
}

void BSM_PrevDigest(class messages  *self)
{
    class t822view *tv = (class t822view *) GetBodies(self);
    class text *t = (class text *) (tv)->GetDataObject();
    struct SearchPattern *Pattern = NULL;
    const char *tp = search::CompilePattern("\012--", &Pattern);
    int tmploc;

    if (tp) {
	message::DisplayString(NULL, 10, tp);
    } else {
	tmploc = search::MatchPatternReverse(t, (tv)->GetDotPosition() - 2, Pattern);
	if (tmploc >= 0) {
	    (tv)->SetDotPosition( ++tmploc);
	    (tv)->SetTopPosition( tmploc);
	}
    }
}
