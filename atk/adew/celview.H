ATK_INTER
#ifndef _celview_H_
#define _celview_H_
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


 

#define celview_UnInitialized 0
#define celview_MakeHorz 1
#define celview_MakeVert 2
#define celview_INSERTVIEW 3
#define celview_HasView 4
#define celview_Initialized 5
#define celview_IsSplit 6
#define celview_UpdateView 7
#define celview_FirstUpdate 8
#define celview_NoUpdate 9
#define celview_NewOverlay 10
#define celview_DoFull 11

#include <view.H>
class celview;
typedef class view *(*celview_hitfptr)(class celview *self, enum view_MouseAction, long x, long y, long clicks, long rock);
class celview: public view {
    public:
      virtual ATKregistryEntry *ATKregistry();


      void WantNewSize(class view *requestor);
      void PostMenus(class menulist *menulistp);
      void PostKeyState(class keystate *keystate);
      void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
      void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
      void Update();
      class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
      void ReceiveInputFocus();
      void LoseInputFocus();
      void SetDataObject(class dataobject *ls);
      void ObservedChanged (class observable *changed, long value);
      void GetManyParameters( struct resourceList * resources, class atomlist * name, class atomlist * class_c );		      
      view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
      void LinkTree(class view *parent);
      void InitChildren();
      boolean CanView(const char *TypeName)  ;
      void WantUpdate(class view *requestor);
      void PostCursor(struct rectangle *rec,class cursor *cursor) ;
      ATK  * WantHandler(const char *handlerName)  ;
      void GetOrigin(long width, long height, long *originX, long *originY);
      boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
      boolean RecSrchResume(struct SearchPattern *pat);
      boolean RecSrchReplace(class dataobject *text, long pos, long len);
      void RecSrchExpose(const struct rectangle &logical, struct rectangle &hit);

      virtual void SetResizing(long key);
      virtual class view * makeview(class cel *ls)  ;
      virtual int PromptForInfo(class arbiterview *arb,boolean promptForViewName,boolean changeRefName)  ;
      virtual void PostParameters();
      virtual void Copy();
      virtual void SetHitfunc(celview_hitfptr hitfunc,long hitrock);
      virtual void SetKeymap( class keymap *km);
      virtual void SetMenulist( class menulist *ml);
      virtual void Repost();
      virtual void PushOverlay(class view *view,struct rectangle *rect,long flags);
      virtual class view * PopOverlay(class view *view)  ;

      inline class view * GetTrueChild() { return ((this)->truechild); }
      inline class view * GetApplication() { return ((this)->child); }
      inline class arbiterview * GetRemoteArb() { return ((this)->rarb); }
      inline void SetRemoteArb(class arbiterview *Arb) { ((this)->rarb = (Arb)); }
      inline class cel * GetCel() { return (class cel *) (this)->GetDataObject(); }
      inline boolean GetAppFlag() { return ((this)->AddAppLayer); }
      inline void SetAppFlag(boolean VAL)  { ((this)->AddAppLayer = (VAL)); }


      static boolean InitializeClass();


      virtual ~celview();

      celview();
      static int ResourceListToString(char *str,struct resourceList *rl)  ;
      static class celview * GetCelviewFromView(class view *v)  ;

      int HasFocus;
      class keymap *keymapp;
      class keystate *keystatep;
      class menulist *menulistp;
      int mode;
      class arbiterview *arb,*rarb;
      int level;
      class text *script;
      class view *child,*truechild,*safec,*safetc;
      int promptforparameters;
      class text *pdoc;
      int desw,desh;
      class cursor *widthcursor, *heightcursor;
      int Moving,resizing,WasMoving, WasResizing;
      class menulist *menus;
      int drawing, OldMode,sizepending,NeedsPost,vismode;
      class atom *viewatm,*dataatm,*refatm;
      int application,NeedsRemade,NeedsReinit;
      celview_hitfptr hitfunc;
      long hitrock;
      struct rectangle enclosingRect,childRect;
      struct overlay *olist;
      boolean TopLevel;
      boolean AddAppLayer; /* Add an application layer, regardless of flag in cel */
};


#endif /* _celview_H_ */
