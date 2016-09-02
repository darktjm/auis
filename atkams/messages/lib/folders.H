#ifndef _folders_H_
#define _folders_H_
/***********************************************************
		Copyright IBM Corporation 1991

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

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


 

#define TEMPFAILSTRING " (Temporarily unavailable) \n"

#define BADFAILSTRING " (Error -- Directory unreadable) \n"

#define LIST_NEWONLY 0
#define LIST_SUBSCRIBED 1
#define LIST_MAIL_FOLDERS 2
#define LIST_ALL_FOLDERS 3
#define LIST_AS_REQUESTED 4

struct BEDirCache {
    class environment *env;
    int pos, len, commentstart, commentlen, substatus, SkipMe;
    char *FullName, *ShortName;
};

#include <messages.H>
class folders: public  messages {
public:
	virtual ATKregistryEntry *ATKregistry();


     void PostKeyState(class keystate *keystate);
     void PostMenus(class menulist *ml);
     class view * Hit(enum view_MouseAction action, long x, long y, long nClicks)  ;
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
     class view * GetApplicationLayer()  ;
     void DeleteApplicationLayer(class view *scrollbar);

    
	folders();
    
	virtual ~folders();
    
	static boolean InitializeClass();
	static boolean atk_inited;


    virtual void HandleAsyncPrefetch();
    virtual int AlterSubscriptionStatus(char *dir, int status, char *shortname)  ;
    virtual void AlterFolderNames(char *name, char *nick, boolean doinsert);
    virtual void ActionHit(int substatus, char *fullname, char *nickname);
    virtual void HighlightFolder(char *name, char *comment);
    virtual void SimulateClick(boolean IsLeftClick);
    virtual void FindNextFolder(int *which, int *skipped);
    virtual void Reconfigure(int listcode);
    virtual void UpdateMsgs(int code, const char *thingstoread[], boolean ShowHelp);
    virtual void NextFolder(boolean ShowFirst);
    virtual void SetCaptions(class captions *c);
    virtual void SetVeryNarrow(boolean isnarrow);
    virtual void ExplainDir(char *fullname, char *nickname);
    virtual void SetSkip(char *fullname, boolean DoSkip);
    virtual class sendmessage * ExposeSend()  ;
    virtual void GrowWindow();
    virtual int WriteFormattedBodyFile(char *fname, char *captbuf)  ;
    virtual void FolderOnDisplay(char *shortname, char *longname);
    virtual void ShowHelp();
    virtual class captions * NewCaptionsInNewWindow()  ;
    virtual void SetSendmessage(class sendmessage *sm);
    virtual void ReadMail(boolean CheckMailbox);
    virtual class captions * GetCaptions()  ;

    class environment *HighlightEnv;
    struct BEDirCache *MainDirCache;
    int MainDirCacheSize, MainDirCacheCount;
    int HasSetUp:1;
    int ShowingAsRequested:1;
    class style *Activefolderstyle; /* Bold */
    class style *Normalfolderstyle; /* Indented */
    class style *GlobalCapStyle; /* for the folders doc */
    class style *IconicStyle; /* for icons from messages10 */
    class style *BoldStyle;
    class style *ItalicStyle;
    class style *CenterStyle;
    class style *BigCenterStyle;
    class style *SmallCenterStyle;
    class cursor *mycursor;
    class keystate *mykeys;
    class menulist *mymenulist;
    class captions *mycaps;
    class lpair *puntlp;
    int MailOnlyMode, CurrentConfiguration, VeryNarrow;
    class sendmessage *sm;
    class sbuttonv *buttons;
    class scroll *myscroll;
    class frame *myframe;
    struct sbutton_prefs *prefs;
    int NeedsToPrefetch;
};


#endif /* _folders_H_ */
