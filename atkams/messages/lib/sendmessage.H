#ifndef _sendmessage_H_
#define _sendmessage_H_
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


 

#define SM_STATE_NOSTATE -1
#define SM_STATE_READY 0
#define SM_STATE_INPROGRESS 1
#define SM_STATE_SENDING 4
#define SM_STATE_SENT 5
#define SM_STATE_VALIDATING 6
#define SM_STATE_VALIDATED 7
#define SM_STATE_VALIDATEFAILED 8

#define SM_RESET 0
#define SM_SIGN 1
#define SM_HIDE 2
#define SM_CLEAR 3
#define SM_BLIND 4

#include <view.H>
class sendmessage: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
     void Update();
     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void PostKeyState(class keystate *keystate);
     void PostMenus(class menulist *menulist);
     void WantUpdate(class view *v);
     void LinkTree(class view *parent);
     void UnlinkTree();

    
	sendmessage();
    
	virtual ~sendmessage();
    
	static boolean InitializeClass();
	static boolean atk_inited;


    virtual int  Clear()  ;
    virtual int  ReadFromFile(char *SourceFile, Boolean DeleteIt)  ;
    virtual void Reset();
    virtual void  AddHeaderLine(const char *line)  ;
    virtual void  AddToToHeader(const char *line)  ;
    virtual void  ResetSendingDot()  ;
    virtual int  ResetFromParameters(char *ToName, char *Subject, char *CC, char *IncludeFile, int Delete)  ;
    virtual int  Checkpoint()  ;
    virtual int  WriteFile(char *FileName)  ;
    virtual void  SetFoldersView(class folders *mess)  ;
    virtual void CheckButtons();
    virtual int HasChanged()  ;
    virtual void SetCurrentState(int state);
    virtual int AppendBugInfoToBody(Boolean IsMessagesBug)  ;
    virtual int AskEraseUnsentMail()  ;
    virtual void QuoteBody();
    virtual int CheckRecipients()  ;
    virtual class folders * NewFoldersInNewWindow()  ;
    virtual int NeedsCheckpointing()  ;

    class lpair *SendLpair;
    class scroll *HeadScroll, *BodyScroll;
    class textview *HeadTextview, *BodyTextview;
    class text *HeadText, *BodyText;
    struct sbutton_prefs *prefs;
    class sbuttonv *buttons;
    /*
    struct butter *bclear, *bhide, *bblind, *breset, *bsign;
    struct butterview *bcv, *bhv, *bbv, *brv, *bsv; */
    class style *DefaultStyle, *DefaultHeadStyle, *BoldStyle;
    class folders *foldersp;
    class menulist *mymenulist, *myheadmenulist;
    class keystate *keys, *headkeys;
    long HeadModified, BodyModified; /* stored mod #'s for texts */
    int HeadCheckpoint, BodyCheckpoint;  /* Value of above at time of last checkpoint */
    int NeedsTemplate, CurrentState;
    char *CKPFileName, *PSMsg;
    class frame *myframe;
    int HasSigned;
    class lpair **randomlpairs;
};


#endif /* _sendmessage_H_ */
