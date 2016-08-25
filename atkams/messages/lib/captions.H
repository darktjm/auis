#ifndef _captions_H_
#define _captions_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


#include <ams/cui.h>
#include <ams/ams.h>
/* Definitions of the icons as characters */

#define ICON_MAIL 'c'
#define ICON_READMAIL 'f'
#define ICON_DUP 'e'

#define ICON_MARK 'j'
#define ICON_FOLDER 'r'

#define ICON_SEE 'g' /* Not currently used */
#define ICON_HELP 'h'
#define ICON_DELMAIL 'o'
#define ICON_POST 't'
#define ICON_DELPOST 'i'

#define ICON_READPOST 'n'
#define ICON_READDELMAIL 's'
#define ICON_READDELPOST 'b'

#define ICON_SUB_NONE 'B'
#define ICON_SUB_NORM 'C'
#define ICON_SUB_PRINT 'D'
#define ICON_SUB_ASK 'E'
#define ICON_SUB_ALL 'F'

/* Definitions of the icons as strings */

#define SICON_MAIL "c"
#define SICON_READMAIL "f"
#define SICON_DUP "e"

#define SICON_MARK "j"
#define SICON_FOLDER "r"

#define SICON_SEE "g" /* Not currently used */
#define SICON_HELP "h"
#define SICON_DELMAIL "o"
#define SICON_POST "t"
#define SICON_DELPOST "i"

#define SICON_READPOST "n"
#define SICON_READDELMAIL "s"
#define SICON_READDELPOST "b"

#define SICON_SUB_NONE "B"
#define SICON_SUB_NORM "C"
#define SICON_SUB_PRINT "D"
#define SICON_SUB_ASK "E"
#define SICON_SUB_ALL "F"

/* Constants used for the "ActOnMarked" procedures */

#define MARKACTION_DELETE 1
#define MARKACTION_UNDELETE 2
#define MARKACTION_CLASSIFYBYNAME 3
#define MARKACTION_PRINT 4
#define MARKACTION_APPENDBYNAME 5
#define MARKACTION_RESEND 6
#define MARKACTION_RESTORE 7
#define MARKACTION_EXCERPT 8
#define MARKACTION_REPLYSENDERS 9
#define MARKACTION_REPLYALL 10
#define MARKACTION_COPYBYNAME 11
#define MARKACTION_APPENDTOFILE 12
#define MARKACTION_APPENDTOFILERAW 13

struct CaptionCache {
    int offset, cuid;
    class environment *env, *iconenv;
    char Date[AMS_DATESIZE];
    char Attributes[AMS_ATTRIBUTESIZE];
    long Chain;
    int IsMarked:1;
    int MayModify:1;
    int IsDup: 1;
};

#include <messages.H>
#include <t822view.H>

class captions: public  messages {
public:
	virtual ATKregistryEntry *ATKregistry();


     void PostKeyState(class keystate *keystate);
     void PostMenus(class menulist *ml);
     class view * Hit(enum view_MouseAction action, long x, long y, long nClicks)  ;
     const void * GetInterface(const char *interfaceName)  ;
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);

    
	captions();
    
	virtual ~captions();
    
	static boolean InitializeClass();
	static boolean atk_inited;

	class view *GetApplicationLayer();
    virtual void ResetVisibleCaption();
    virtual void ClearAndUpdate(int ConsiderPurging, int SaveState);
    virtual void MakeCachedUpdates();
    virtual void AlterPrimaryFolderName(char *addname, char *delname);
    virtual void AlterDeletedIcon(int position, boolean delete_c);
    virtual void CapReverseSearch();
    virtual int DeleteVisibleMessage(boolean delete_c)  ;
    virtual int DisplayNewBody(int thisCUID, int linestart, int linelen, class environment *env)  ;
    virtual int FindCUIDByDocLocation(int *position,  int *len, class environment **envptr, int *whichcaption)  ;
    virtual int GetBodyFromCUID(int cuid, int Mode, char *ContentTypeOverride)  ;
    virtual int GuaranteeFetchedRange(int min, int max)  ;
    virtual int InsertUpdatesInDocument(char *shortname, char *dname, boolean ShowFirst)  ;
    virtual void MarkVisibleMessageStateofSeeing(boolean hasseen);
    virtual void ReportMarkedMessageCount();
    virtual int ShowMore(boolean mayscroll, boolean maygoon, boolean insistonmark)  ;
    virtual void ToggleMark(struct CaptionCache *hc, int linestart);
    virtual void SearchAll();
    virtual void FileCurrent(char *FullName, char *NickName);
    virtual void FileMarked(char *FullName, char *NickName);
    virtual void ThisIsFlorida();
    virtual void FindRelatedMessages();
    virtual void SimulateClick(boolean IsLeftClick);
    virtual void PuntCurrent(boolean GoToNext);
    virtual void MarkRangeOfMessages();
    virtual void SendMessage(int code);
    virtual void SetFolders(class folders *f);
    virtual void SetBodies(class t822view *bv);
    virtual void SetLabel(char *label);
    virtual void ShowHelp();
    virtual class t822view * NewBodiesInNewWindow()  ;
    virtual class folders * NewFoldersInNewWindow()  ;
    virtual void BackUpCheckingMarks(boolean checkmarks);
    virtual void ActOnMarkedMessages(int markaction, char *string);
    virtual char * GetLastClassification()  ;
    virtual void ClearMarks();
    virtual void AlterFileIntoMenus(boolean shrink);
    virtual void CloneMessage(int code);
    virtual void MarkCurrent();
    virtual void PrintVisibleMessage();
    virtual void Redisplay(int Mode, char *contenttype);
    virtual class folders * GetFolders()  ;
    virtual class t822view * GetBodView()  ;

    inline class text * GetBodDoc() { return (class text *) (((this)->GetBodView())->GetDataObject()); }

    class t822view *BodView;
    int VisibleCUID;
    char VisibleSnapshot[AMS_SNAPSHOTSIZE];
    int HighlightStartPos;
    int HighlightLen;
    int StartOfRealBody;
    class environment *HighlightEnv;
    class environment *SouthPoint;
    int CurrentFormatting;
    int FolderSize, FetchedFromStart, FetchedFromEnd;
    int MarkCount;
    class text *CaptText; 
    char *CommentText;
    int IsFullMail:1;
    class style *ActiveCaptionStyle; /* Bold */
    class style *NormalCaptionStyle; /* Indented */
    class style *HighlightStyle; /* Italicize */
    class style *GlobalCapStyle; /* for the captions doc */
    class style *DeletedStyle; /* for deleted messages */
    class style *ActiveDeletedStyle; /* active/deleted combination */
    class style *IconicStyle; /* for icons from messages10 */
    class style *UnderlinedIconicStyle; /* ditto, but underlined */
    class style *MailStyle; /* Bigger, better? */
    class style *FixedStyle; /* For fixed width display */
    const struct scrollfns *textscrollinterface;
    class cursor *mycursor;
    class keystate *privkeys;
    class menulist *privmenus;
    int OldMarkCount;
    int *OldMarks;
    struct CaptionCache *capcache;
    int captioncachecount, captioncachesize;
    char *FullName, *ShortName;
    int firstcuid, substatus;
    boolean MenusExpanded;
    class folders *myfold;
    class frame *myframe;
    int downdot; /* for click-drag message selections */
    boolean rawText;
};

#endif /* _captions_H_ */
