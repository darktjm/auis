ATK_INTER
#ifndef _lsetview_H_
#define _lsetview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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


 

#define lsetview_UnInitialized 0
#define lsetview_MakeHorz 1
#define lsetview_MakeVert 2
#define lsetview_INSERTVIEW 3
#define lsetview_HasView 4
#define lsetview_Initialized 5
#define lsetview_IsSplit 6
#define lsetview_UpdateView 7
#define lsetview_FirstUpdate 8
#define lsetview_NoUpdate 9

#include <lpair.H>
class lsetview: public lpair {
public:
	virtual ATKregistryEntry *ATKregistry();


     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
     void Update();
     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void ReceiveInputFocus();
     void LoseInputFocus();
     void SetDataObject(class dataobject *ls);
     void ObservedChanged (class observable *changed, long value);
     void LinkTree(class view *parent);
     void WantNewSize(class view *requestor);
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
     void InitChildren();
     boolean CanView(const char *TypeName)  ;

    virtual void ReadFile(FILE *thisFile,const char *iname);
    virtual boolean Unsplit(class lsetview *who)  ;

    
	static boolean InitializeClass();

    static class lsetview * Create(int level,class lset *ls,class view *parent)  ;
    
	lsetview();    
    
	virtual ~lsetview();

	int HasFocus;
	class keystate *keystatep;
	class menulist *menulistp;
	class cursor *cursorp;
	int mode;
	int level;
    class view *child,*app;
    int promptforparameters;
    class text *pdoc;
    int revision;
};


#endif /* _lsetview_H_ */
