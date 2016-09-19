/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*  Modified 3/15/90  cch@mtgzx.att.com
 *  Made it so that any change in dot position scrolls other window.
   Modified so that scrolls to main window put selection on top line.
 */

#include <andrewos.h>
ATK_IMPL("contentv.H")
#include <ctype.h>
#include <bind.H>
#include <view.H>
#include <menulist.H>
#include <keymap.H>
#include <content.H>
#include <message.H>
#include <mark.H>
#include <buffer.H>
#include <im.H>
#include <frame.H>
#include <environ.H>
#include <print.H>
#include <completion.H>
#include <textview.H>
#include <text.H>
#include <style.H>
#include <stylesheet.H>
#include <fontdesc.H>
#include <proctable.H>
#include <tindex.H>

#include "contentv.H"
static class menulist *contentvMenus;
static class keymap *contentvKeyMap;
static int ScrollTop = FALSE;
#define Data(self) ((class content *)(((class view *) self)->dataobject))
#define Text(v)	(class text *) ((v)->dataobject)
#define Srctext(self) (Data(self)->srctext)

ATKdefineRegistry(contentv, textview, contentv::InitializeClass);
static void reinit(class contentv  *self,long  value);
static void enumerate(class contentv  *self,long  value);
static int LocateInView(class view  *v1,class view  *v2,class view  *v3,long  dat);
static void locate(class contentv  *self,long  value);
static void denumerate(class contentv  *self,long  value);
static int check(class frame  *fr, struct contentv_cntr  *rock);
static class frame *getframe(class view  *vw);
static void destroy(class contentv  *self,long  value);
static void contentv_MakeContents(class textview  *self);
static boolean findframe(class frame  *fr,class buffer  *buf);
static void contentv_doprint(class contentv  *self,const char  *type);
static void contentv_PreviewCmd(class contentv  *self);
static void contentv_PrintCmd(class contentv  *self);


contentv::contentv()
{
	ATKinit;

    this->menus = (contentvMenus)->DuplicateML( this);
    this->srcview = NULL;
    THROWONFAILURE( TRUE);
}
contentv::~contentv()
{
	ATKinit;

     if(this->menus){
	delete this->menus;
	this->menus = NULL;
    }
}
static void reinit(class contentv  *self,long  value)
{
    class content *ct;
    long  pos = (self)->GetDotPosition();

    ct = Data(self);
    (ct)->reinit();
    (self)->SetDotPosition(pos);
    (self)->FrameDot(pos);
}
static void enumerate(class contentv  *self,long  value)
{
    class content *ct;
    long len,tlen;
    ct = Data(self);
    if((len = (self)->GetDotLength()) > 0){
	char buf[128],iname[128];
	long stlen;
	long pos = (self)->GetDotPosition();
	tlen = (ct)->GetLength();
	len--;
	while((tlen > pos + len ) && ((ct)->GetChar(pos + len) != '\n')) len++ ;
	len++;
	if((stlen = (ct)->StringToInts(pos,NULL)) > 0 && stlen < 127)
	    (ct)->CopySubString(pos,stlen - 1,buf,FALSE);
	else {
	    buf[0] = '0';
	    buf[1] = '\0';
	}
	stlen = message::AskForString(self,60,"Starting number ",buf, iname, sizeof(iname));
	if (stlen < 0){
	    message::DisplayString(self, 0, "Punt!");
	    return;
	}
	if(strlen(iname) == 0)(ct)->Enumerate(pos,len,NULL);
	else {
	    strcat(iname,"\t");
	    (ct)->Enumerate(pos,len,iname);
	}
    }
    else {
	(ct)->Enumerate(-1,0,NULL);
    }
}
static int LocateInView(class view  *v1,class view  *v2,class view  *v3,long  dat)
{
    class mark *m = (class mark *) dat;
    if(ATK::IsTypeByName((v2)->GetTypeName(),"textview")){
	class textview *tv = (class textview *)v2;
	(tv)->SetDotPosition((m)->GetPos());
	(tv)->SetDotLength((m)->GetLength());
	if (ScrollTop)
	    (tv)->SetTopPosition( (m)->GetPos());
	else (tv)->FrameDot((m)->GetPos());
    }
    return 0; /* go through all views */
}
static void locate(class contentv  *self,long  value)
{
    class content *ct;
    class mark *loc;
    class buffer *buf;
    ct = Data(self);
    loc = (ct)->locate((self)->GetDotPosition());
    if(loc == NULL) return;

    if(self->srcview)
	LocateInView(NULL,(class view *)self->srcview,NULL,(long) loc);
    else {
	buf = buffer::FindBufferByData((class dataobject *)ct->srctext);
	if(buf)
	    (buf)->EnumerateViews((buffer_evfptr)LocateInView,(long) loc);
    }
}
static void denumerate(class contentv  *self,long  value)
{
    class content *ct;
    long len,pos,tlen;
    ct = Data(self);
    if((len = (self)->GetDotLength()) > 0){
	pos = (self)->GetDotPosition();
	tlen = (ct)->GetLength();
	len--;
	while((tlen > pos + len ) && ((ct)->GetChar(pos + len) != '\n')) len++ ;
	len++;
	(ct)->Denumerate(pos,len);
    }
    else (ct)->Denumerate(-1,0);
}
struct contentv_cntr {
class buffer *buf;
int tc,bc;
};
static int check(class frame  *fr, struct contentv_cntr  *rock)
        {
     rock->tc++;
     if((fr)->GetBuffer() == rock->buf) rock->bc++;
     return FALSE;
}
static class frame *getframe(class view  *vw)
{
    while (vw->parent != NULL){
	vw = vw->parent;
	if(ATK::IsTypeByName((vw)->GetTypeName(),"frame")){
	    return (class frame *) vw;
	}
    }
    return NULL;
}
static void destroy(class contentv  *self,long  value)
{
    class buffer *buffer;
    class frame *fr;
    struct proctable_Entry *pr;
    proctable_fptr proc;
    struct contentv_cntr cc;
    class content *content = (class content*) (self)->GetDataObject();
    if((pr = proctable::Lookup("frame-delete-window")) != NULL && 
	proctable::Defined(pr) &&
	(buffer = buffer::FindBufferByData(Data(self))) != NULL &&
	(fr = getframe((class view *) self)) != NULL){
	proc = proctable::GetFunction(pr) ;
	cc.tc = 0;
	cc.bc = 0;
	cc.buf = buffer;
	frame::Enumerate((frame_effptr)check, (long)&cc);
	(fr)->SetBuffer(NULL,0);
	(*proc)(fr,0);
	if(cc.tc > 1 &&  cc.bc == 1) {
	    (content)->RemoveObserver( buffer);
	    (buffer)->Destroy();
	    (content)->Destroy();
	}
    }
}
static void contentv_MakeContents(class textview  *self)
    {
    contentv::MakeWindow(Text(self));
}
void contentv::GetClickPosition(long  position, long  numberOfClicks, enum view_MouseAction  action, long  startLeft, long  startRight, long  *leftPos, long  *rightPos)
                                    {
	(this)->textview::GetClickPosition( position, numberOfClicks, action, startLeft, startRight, leftPos, rightPos);
	if(numberOfClicks == 1 && (action == view_LeftUp || action == view_RightUp))
	    locate(this,0);
    }
static boolean findframe(class frame  *fr,class buffer  *buf)
{
    if((fr)->GetBuffer() == buf) return TRUE;
    return FALSE;
}
static void contentv_doprint(class contentv  *self,const char  *type)
{
    class buffer *bu;
    class frame *fr;
    class text *txt;
    class content *ct;
    struct proctable_Entry *pr;
    proctable_fptr proc;
    const char *saveenv = NULL;

    ct = Data(self);
    txt = ct->srctext;
    if(txt == NULL || (bu = buffer::FindBufferByData((class dataobject *) txt))== NULL) {
	message::DisplayString(self,0,"Can't find buffer for source");
	return;
    }
    if((fr = frame::Enumerate((frame_effptr)findframe,(long) bu)) == NULL){
	message::DisplayString(self,0,"Can't find view for source");
	return;
    }
    if((pr = proctable::Lookup(type)) != NULL && proctable::Defined(pr) ){
	saveenv = environ::Get("PrintContents");
	proc = proctable::GetFunction(pr) ;
	environ::Put("PrintContents","yes");
	(*proc)(fr,0);
	if (saveenv)
	    environ::Put("PrintContents", saveenv);
	else
	    environ::Delete("PrintContents");
    }
    else {
	char errstring[50];
	sprintf (errstring, "Can't find proctable entry '%s'.", type);
	message::DisplayString(self, 0, errstring);
	return;
    }
}
static void contentv_PreviewCmd(class contentv  *self)
    {
    contentv_doprint(self,"frame-preview");	
}
static void contentv_PrintCmd(class contentv  *self)
    {
    contentv_doprint(self,"frame-print");
}

static struct bind_Description contentvBindings[]={
    {"contentv-reinit",NULL,0,"Contents~1,Update Contents~20", 0,0,(proctable_fptr)reinit,"reinitialize the headings"},
    {"contentv-enumerate",NULL,0,"Contents~1,Enumerate~1",0,0,(proctable_fptr)enumerate,"Enumerate the heading"},
    {"contentv-denumerate",NULL,0,"Contents~1,Denumerate~2",0,0,(proctable_fptr)denumerate,"Denumerate the heading"},
    {"contentv-destroy",NULL,0,"Quit~99",0,0,(proctable_fptr)destroy,"destroy the table of content"}, 
    {"contentv-delete-window",NULL,0,"Delete Window~89",0,0,(proctable_fptr)destroy,"destroy the table of content"},
    {"contentv-preview",		    NULL,0,	    "File~10,Preview~21",0,0, (proctable_fptr)contentv_PreviewCmd, "Previews document."},
    {"contentv-print",		    NULL,0,	    "File~10,Print~22",0,0, (proctable_fptr)contentv_PrintCmd, "Prints document."},
    NULL
};

void contentv::PostMenus(class menulist  *menulist)
{
    (this->menus)->ClearChain();
    (this->menus)->ChainBeforeML( menulist, (long)menulist);
    (this)->textview::PostMenus( this->menus);
}
boolean contentv::InitializeClass()
    {
    struct ATKregistryEntry  *textviewtype = ATK::LoadClass("textview");
    contentvMenus = new menulist;
    contentvKeyMap =  new keymap;
    bind::BindList(contentvBindings, contentvKeyMap , contentvMenus, &contentv_ATKregistry_ );
    proctable::DefineProc("contentv-make-window",(proctable_fptr)contentv_MakeContents,textviewtype,NULL,"Make a table of contents window");
    ScrollTop = environ::GetProfileSwitch("ContentsScrollTop", FALSE);

    return TRUE;
}
 void contentv::MakeWindow(class text  *txt)
{
	ATKinit;

    char buf[1024];
    class content *ct;
    class buffer *buffer;
    class frame *fr;
    class im *window;
    if((buffer = buffer::FindBufferByData((class dataobject *)txt)) != NULL) {
	sprintf(buf,"Contents_%s",(buffer)->GetName());
    }
    else sprintf(buf,"Table_of_Contents");
    if((ct = new content) == NULL) {
	fprintf(stderr,"Could not allocate enough memory.\n");
	return;
    }
    (ct)->SetSourceText(txt);
    if((buffer = buffer::Create(buf,NULL,"contentv",ct)) == NULL) {
	fprintf(stderr,"Could not allocate enough memory.\n");
	return;
    }
    if ((window = im::Create(NULL)) != NULL) {
	fr = new frame;
	(fr)->SetCommandEnable( TRUE);
	(window)->SetView( fr);
	(fr)->PostDefaultHandler( "message", (fr)->WantHandler( "message"));
	(fr)->SetBuffer( buffer, TRUE);
	(buffer)->SetScratch(TRUE);
    }
    else {
	fprintf(stderr,"Could not allocate enough memory.\n");
	if(buffer) (buffer)->Destroy();
	if(ct) (ct)->Destroy();
    }
}

void contentv::SetDotPosition(long  newpos )
{
    (this)->textview::SetDotPosition( newpos);
    locate(this, 0);
}
