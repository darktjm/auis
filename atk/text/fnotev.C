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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/fnotev.C,v 3.8 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


ATK_IMPL("fnotev.H")
#include <text.H>
#include <textview.H>
#include <view.H>
#include <im.H>
#include <rect.h>
#include <fontdesc.H>
#include <cursor.H>
#include <message.H>
#include <ctype.h>
#include <environ.H>
#include <fnote.H>
#include <environment.H>
#include <texttroff.H>
#include <fnotev.H>
#define FONTNAME "andy"
#define FONTSIZE 16
#define OFNAME "andy"
#define OFSIZE 12
#define SHIM 3
#define DataObject(A) (A->dataobject)
#define Text(A) ((class text *) DataObject(A))
#define Fnote(A) ((class fnote *) DataObject(A))
#define Graphic(A) (((class view *)(A))->drawable)
#define CANTSWITCHTEXTVIEW 1 /* textview currently doesn't allow itself to switch to a new text */
struct impair {
    class im *host,*fn;
    class fnotev *cur;
    class textview *textview;
    class view *app;
    struct impair *next;
};
static struct impair *list;
static int endnotes = FALSE;

ATKdefineRegistry(fnotev, view, fnotev::InitializeClass);
#ifndef NORCSID
#endif
void initci(class fnotev  *self);
#if 0
fnotev_FindLoc(class fnotev  *self,class text  *parent);
#endif
static struct impair *findwindow(class fnotev  *self,class text  *pc);
static void DoUpdate(class fnotev  *self,boolean  full);
#if 0
ismyenv(class fnotev  *self,class text  *d,long  pos,class environment  *env);
#endif


void initci(class fnotev  *self)
{
    if((self)->GetDrawable() != NULL){
	(self->fd)->CharSummary((self)->GetDrawable(),*(self->displaystr),&(self->ci[0]));
	(self->ofd)->CharSummary((self)->GetDrawable(),*(self->displaystr),&(self->ci[1]));
    }
}

void fnotev::GetOrigin(long  width, long  height, long  *originX, long  *originY)
                    {
    *originX = 0;
    *originY = height - 3;
}
static int clevel = -100;
void fnotev::Print(FILE  *f, char  *process, char  *final, int  toplevel)
                    {
	class text *d;
	long i,doclen,ln,cs,c,addNewLine;
	if(endnotes){
/*
	    fn = Fnote(self);
	    fprintf(f,"\\v'-.3v'\\\\*(Fn\\s-3 %d\\s0\\fP\\v'.3v'",
		    fn->notecount);
*/
	    if((Text(this))->GetLength() > 0)
		fprintf(f, "\\**\n");	/* automatic footnote counter string */
	    return;
	}
	addNewLine = 0;
	d = Text(this);
	doclen = (d)->GetLength();
	if(doclen == 0) return;
	fprintf(f, "\\**\n");	/* automatic footnote counter string */
	fprintf(f,".FS\n"); 
	if(clevel == toplevel){
	for(i = 0,ln = 0,cs = 0; i < doclen;i++) {
	    if ((c = (d)->GetChar( i)) == '\n') {
		if(cs++ == 0 && ln == 0) fputc('\n',f);	/* count line Spacing */
		ln = 0;
		fputc('\n',f);
		continue;
	    }
	    else cs = 0;
	    if (ln++ > 80 && (c == ' ' || c == '\t')) {
		/* Don't let lines get too long */
		addNewLine++;
	    } else if (addNewLine) {
		/* Add the newline before the first */
		/* non-blank character (if still needed) */
		if (ln > 80) {
		    fputc('\n', f);
		    ln = 0;
		    addNewLine = 0;
		}
	    }
	    if ((c == '\\') || (ln == 0 && (c == '\'' || c == '.'))) {
		/* quote special characters */
		fputc('\\', f);
		if (c == '.') {
		    fputc('&', f);
		    ln++;
		}
		ln++;
	    }
	    if(!(isprint(c) || isspace(c))) continue;
	    fputc(c, f);
	}
	if(ln > 0) fputc('\n',f);
	}
	else{
	    clevel = toplevel;
	    texttroff::WriteSomeTroff( this, d, f, toplevel, 0);
	}
	fprintf(f,".FE\n");
	clevel = -100;
    }

void *fnotev::GetPSPrintInterface(char *printtype)
{
    static struct textview_insetdata dat;

    if (!strcmp(printtype, "text")) {
	dat.type = textview_Footnote;
	dat.u.Footnote = this;
	return (void *)(&dat);
    }

    return NULL;
}



class view *fnotev::Hit(enum view_MouseAction  action,long  mousex ,long  mousey ,long  numberOfClicks) 
{

    if(action == view_LeftUp || action == view_RightUp){
/*	fnotev_WantInputFocus(self,self); */
#if 0
	if(this->fnotetype == -1)
	    (this)->pushchild();
	else{
	    char *ib[2048],*ob[2048];
	    int len;
	    len = (Text(this))->GetLength();
	    if(len > 2047) len = 2047;
	    (Text(this))->CopySubString(0,len,ib,FALSE);
	    if(numberOfClicks == 1 || len == 0 || (Text(this))->GetReadOnly())
		message::DisplayString(NULL,this->fnotetype,ib);
	    else{
		if(message::AskForString(NULL,this->fnotetype,"new footnote? ",ib,ob,2047) >= 0) 
		    (Text(this))->ReplaceCharacters(0,len,ob,strlen(ob));
	    }
	}
#else 
	class fnote *fn = Fnote(this);
	if((fn)->IsOpen())
	    (fn)->Close(this->parenttext);
	else 
	    (fn)->Open(this->parenttext);
	(this->parentview)->SetDotPosition((fn)->GetLoc() + 1);
	(this->parentview)->SetDotLength((fn)->GetLocLength());
	(fn)->NotifyObservers(0);

#endif
    }	
    return (class view *) this;
}
#if 0
fnotev_FindLoc(class fnotev  *self,class text  *parent)
{
    class text *d,*parent;
    strcut textv *pv;
    
    d = Text(self);
    
    
}
#endif
view_DSattributes fnotev::DesiredSize(long  width , long  height, enum view_DSpass  pass, long  *desiredwidth , long  *desiredheight)
{
/*    *desiredwidth = 17;
    *desiredheight = 14; */
    if(this->ci[0].width == 0) initci(this);

    if(this->ci[0].width != 0)	*desiredwidth = this->ci[0].width + SHIM /*+ SHIM */;
    else *desiredwidth = 13;
    if(this->ci[0].height != 0) *desiredheight = this->ci[0].height  + SHIM + 1/* + SHIM */;
    else *desiredheight = 11;
    return (view_DSattributes)(view_HeightFlexible | view_WidthFlexible);
}
static struct impair *findwindow(class fnotev  *self,class text  *pc)
{
    class im *m;
    struct impair *cim;
#ifdef CANTSWITCHTEXTVIEW
    class view *oldview,*oldapp;
#endif
    m = (self)->GetIM();
    cim = self->imp;
    if(cim == NULL){
	for(cim = list; cim != NULL; cim = cim->next){
	    if(cim->host == m) break;
	}
	if(cim == NULL) {
	    cim = (struct impair *) malloc(sizeof( struct impair));
	/* these structures are shared and never freed, but few are ever created */
	    if(cim == NULL) return NULL;
	    cim->next = list;
	    list = cim;
	    cim->host = m;
	    self->imp = cim;
	    cim->textview = NULL;
	    cim->app = NULL;
	    cim->fn = NULL; 
	}
    }
    if(cim->fn == NULL){
	if((cim->fn = im::Create(NULL)) == NULL) return NULL;
	(cim->fn)->SetTitle("Footnotes");
    }
    else {	
	if(cim->cur == self) return cim;
	if(cim->cur){
	    (cim->fn)->RemoveObserver(cim->cur);
	}
    }
#ifdef CANTSWITCHTEXTVIEW
    oldview = (class view *) cim->textview;
    oldapp = cim->app;
    cim->textview = NULL;
#endif
    if(cim->textview == NULL){
	if((cim->textview = new textview) == NULL) return NULL;
	cim->app = (cim->textview)->GetApplicationLayer();
	(cim->fn)->SetView( cim->app);
    }
    cim->cur = self;
    (cim->fn)->AddObserver(self);
    (cim->textview)->SetDataObject(pc);
    (cim->textview)->WantInputFocus(cim->textview);
#ifdef CANTSWITCHTEXTVIEW
    if(oldview){
	if(oldview != oldapp)
	    (oldview)->DeleteApplicationLayer(oldapp);
	(oldview)->Destroy();
    }
#else 
    (pc)->NotifyObservers(0);
    (cim->textview)->SetDotPosition(0);
    (cim->textview)->SetDotLength(0);
    cim->textview->force = TRUE; /* bogus, shouldn't be necessary */
    (cim->textview)->FrameDot(0);
    (cim->textview)->WantUpdate(cim->textview);
#endif
    return NULL;
}
void fnotev::pushchild()
{
    class text *pc = Text(this);
    if(pc ){
	findwindow(this,pc); 
    }
}
void fnotev::popchild()
{
}

static void DoUpdate(class fnotev  *self,boolean  full)
{
    struct rectangle enclosingRect;
/*     long xsize,ysize; 
    struct point pt[5]; */
/*
    enclosingRect.top = 0; enclosingRect.left = 0;
    enclosingRect.width  = fnotev_GetVisualWidth(self) -2 ;
    enclosingRect.height = fnotev_GetVisualHeight(self) -2 ;
*/
    if(self->ci[0].width == 0) initci(self);
    (self)->GetVisualBounds(&enclosingRect);
    enclosingRect.width--; enclosingRect.height--; 
    enclosingRect.width--; enclosingRect.height--; 
    (self)->SetTransferMode(graphic_WHITE);
    (self)->EraseRect(&(enclosingRect));
    (self)->SetTransferMode(graphic_COPY);
  /*  fnotev_FillRect(self,&(enclosingRect), fnotev_GrayPattern(self,4,16) );*/
    (self)->PostCursor(&(enclosingRect),self->cursor) ;

#if 0 
    enclosingRect.left = enclosingRect.width / 3;
    enclosingRect.top =enclosingRect.height / 3;
    enclosingRect.width  =  enclosingRect.width / 2 ;
    enclosingRect.height = enclosingRect.height / 2 ;
    ysize = enclosingRect.height - enclosingRect.top;
    xsize = enclosingRect.width - enclosingRect.left;
    (self)->DrawRect(&(enclosingRect));
    pt[0].x = enclosingRect.left - 1;
    pt[0].y = enclosingRect.height + enclosingRect.top - ysize;
    pt[1].x = pt[0].x - xsize;
    pt[1].y = pt[0].y ;
    pt[2].x = pt[1].x ;
    pt[2].y = pt[0].y  + ysize + ysize;
    pt[3].x = pt[0].x + xsize;
    pt[3].y = pt[2].y;
    pt[4].x = pt[3].x;
    pt[4].y = enclosingRect.top + enclosingRect.height + 1;
    (self)->DrawPath(pt,5);
#else
    enclosingRect.width++;
    enclosingRect.height++; 
/* fprintf(stderr,"wid = %d, height = %d\n",enclosingRect.width,enclosingRect.height);fflush(stdout);*/
    if(Fnote(self)->open == -333)
	(Fnote(self))->Open(self->parenttext);
    if((Fnote(self))->IsOpen()){
	(self)->SetFont(self->ofd);
	(self)->MoveTo(/* self->ci[1].xOriginOffset + */SHIM,/*self->ci[1].yOriginOffset +*/SHIM);
    }
    else{
	(self)->SetFont(self->fd);
	(self)->MoveTo(/*self->ci[0].xOriginOffset + */1,/*self->ci[0].yOriginOffset + */1);
    }
    (self)->DrawString(self->displaystr,(view_ATTOP | view_ATLEFT)); 
    (self)->DrawRect(&(enclosingRect)); 
    /*	fnotev_MoveTo(self,enclosingRect.width / 2,enclosingRect.height / 2);
     fnotev_DrawString(self,"*",(view_BETWEENLEFTANDRIGHT | view_BETWEENTOPANDBOTTOM)); */


#endif
}
void fnotev::LinkTree(class view  *parent)
{
    (this)->view::LinkTree(parent);
    while(!ATK::IsTypeByName((parent)->GetTypeName(),"textview")){
	if((parent = parent->parent) == NULL ) return;
    }
    this->parenttext = (class text *) (parent)->GetDataObject();
    this->parentview = (class textview *) parent;
}

#if 0
ismyenv(class fnotev  *self,class text  *d,long  pos,class environment  *env)
{
    if(env->type == environment_View && env->data.viewref->dataObject == DataObject(self)){
	self->pos = pos;
	return TRUE;
    }
}
#endif
void fnotev::ObservedChanged(class observable  *changed, long  value)
{
    if(value == observable_OBJECTDESTROYED){
	if(this->imp && changed == (class observable *)this->imp->fn){
#if 0
	    /* can't do this code */
	    struct impair *cim,*lim;
	    lim = NULL;
	    for(cim = list; cim != NULL; cim = cim->next){
		if(cim->fn == this->imp->fn) break;
		lim = cim;
	    }
	    if(cim){
		if(cim == list){
		    list = cim->next;
		}
		else if(lim){
		    lim->next = cim->next;
		}
		free(cim);
	    }
#endif
	    this->imp->fn = NULL;
#if 0
	    if(this->imp->textview){
		(this->imp->textview)->RemoveObserver(this->imp);
		if(this->imp->app && this->imp->app != (class view *)this->imp->textview) 
		    (this->imp->textview)->DeleteApplicationLayer( this->imp->app);
		(this->imp->textview)->Destroy();
		this->imp->textview = NULL;
	    }
#endif
	}
    }
    (this)->WantUpdate(this);
}
void fnotev::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
    DoUpdate(this,TRUE);
}
void fnotev::Update()
{
    DoUpdate(this,FALSE);
}

fnotev::fnotev()
{
	ATKinit;

    this->imp = NULL;
    this->fd = fontdesc::Create(FONTNAME,0,FONTSIZE);
    this->ofd = fontdesc::Create(OFNAME,0,OFSIZE);
    this->cursor = cursor::Create(this);
    (this->cursor)->SetStandard(Cursor_RightPointer);
    this->fnotetype = environ::GetProfileInt("messagefootnote",-1);
    this->parenttext = NULL;
    this->displaystr = "*";
    this->ci = ( (struct fontdesc_charInfo *) calloc(2,sizeof(struct fontdesc_charInfo)));
    this->ci[0].width = 0;
    THROWONFAILURE( TRUE);
}

fnotev::~fnotev()
{
	ATKinit;

    free(this->ci);
}

boolean fnotev::InitializeClass()
{
    list = NULL;
    endnotes = FALSE;
    return TRUE;
}

void fnotev::SetEndnote(boolean  doendnotes)
{
    ATKinit;

    endnotes = doendnotes;
}

void fnotev::ReceiveInputFocus()
{
    (this)->view::ReceiveInputFocus();
    if(this->parentview){	
	/* this should only happen when the view is created */
	(this->parentview)->SetDotPosition((this->parentview)->GetDotPosition() + 1);
	(this->parentview)->WantInputFocus(this->parentview);
    }
}

/* we handle recursive-search the dumb way -- open the footnote and let the parent text take care of it */
boolean fnotev::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    class fnote *d = (class fnote *)this->GetDataObject();
    if (!d->IsOpen())
	d->Open(this->GetParentText());

    return FALSE;
}

boolean fnotev::RecSrchResume(struct SearchPattern *pat)
{
    class fnote *d = (class fnote *)this->GetDataObject();
    if (!d->IsOpen())
	d->Open(this->GetParentText());

    return FALSE;
}

