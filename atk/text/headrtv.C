/* ********************************************************************** */

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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/headrtv.C,v 1.14 1995/11/07 20:17:10 robr Stab74 $";
#endif

/*         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("headrtv.H")
#include <text.H>
#include <textview.H>
#include <observable.H>
#include <view.H>
#include <fontdesc.H>
#include <style.H>
#include <graphic.H>
#include <bind.H>
#include <keymap.H>
#include <keystate.H>
#include <proctable.H>
#include <message.H>
#include <environ.H>
#include <lpair.H>
#include <rect.h>

#include <header.H>
#include <headrtv.H>

#include <ctype.h>

#define Data(self)  ((class header *)(self)->GetDataObject())
#define View(self) ((class view *)self)
#define Text(self,x) (Data(self)->texts[x])
#define Textv(self,x) ((self)->textvs[x])

#define BASEYEAR 1900

static class keymap *newKeymap;
static char **hvars=NULL;
static int hvarcount=0;

typedef char *strings[3];
static strings strs[]={
    {"LT","CT","RT"},
    {"LB","CB","RB"}
};
static char *keywords[]={
    "24hourtime",
    "blank",
    "date",
    "Edate",
    "day",
    "month",
    "page",
    "shortyear",
    "time",
    "timeofday",
    "year"
};

static char *months[]={
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

static char *day[]={
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

struct textbuf {
    char *text;
    long size;
    long pos;
};

ATKdefineRegistry(headrtv, view, headrtv::InitializeClass);
#ifndef NORCSID
#endif
struct tm *GetCurrentTime();
void TwentyFourHourTime(struct textbuf *tbuf);
void Blank(struct textbuf *tbuf);
void Date(struct textbuf *tbuf);
void Date2(struct textbuf *tbuf);
void Day(struct textbuf *tbuf);
void Month(struct textbuf *tbuf);
void Page(struct textbuf *tbuf);
void ShortYear(struct textbuf *tbuf);
void TimeofDay(struct textbuf *tbuf);
void Time(struct textbuf *tbuf);
void Year(struct textbuf *tbuf);
#if 0
static int findincommalist(char  *list,char  *sn);
#endif
static void InstallHeaderVariables();
static void PrintLine(FILE  *fp,char  *string);
static char *headrtv_GetInput(class text  *textobj);
static void DrawBorder(class headrtv  *self,struct rectangle  *vb);
static class textview *GetTextView(class view  *self);
static void headrtv_MoveOn(class headrtv  *self, long  rock);
#if 0
static void newline(class headrtv  *tv,long  rock);
#endif


struct tm *GetCurrentTime() {
    time_t t=time(0);
    return localtime(&t);
}

typedef void (*procfptr)(struct textbuf *tbuf);
static boolean printline_haspage;

static void real_ensure_another(struct textbuf *tbuf, long len)
{
    while (tbuf->pos+len+3 >= tbuf->size) {
	tbuf->size *= 2;
    }
    tbuf->text = (char *)realloc(tbuf->text, tbuf->size);
}

#define ensure_another(tbuf, len) ((tbuf->pos+len+3 >= tbuf->size) ? (real_ensure_another(tbuf, len), 0) : 1)

/* must already be ensured */
#define add_on(tbuf, str)  (strcpy((tbuf)->text+(tbuf)->pos, (str)), (tbuf)->pos += strlen(str))
#define add_char(tbuf, ch)  ((tbuf)->text[(tbuf)->pos] = (ch), (tbuf)->pos++, (tbuf)->text[(tbuf)->pos] = '\0')

static char strbuf[256];

void TwentyFourHourTime(struct textbuf *tbuf)
{
    struct tm *lt=GetCurrentTime();
    sprintf(strbuf, "%d:%s%d", lt->tm_hour, (lt->tm_min>9)?"":"0", lt->tm_min);
    ensure_another(tbuf, strlen(strbuf));
    add_on(tbuf, strbuf);
}

void Blank(struct textbuf *tbuf)
{
    /* do nothing */
}

void Date(struct textbuf *tbuf)
{
    struct tm *lt=GetCurrentTime();
    sprintf(strbuf, "%s %d, %d", months[lt->tm_mon], lt->tm_mday,BASEYEAR+lt->tm_year);
    ensure_another(tbuf, strlen(strbuf));
    add_on(tbuf, strbuf);
}

void Date2(struct textbuf *tbuf)
{
    struct tm *lt=GetCurrentTime();
    sprintf(strbuf, " %d %s %d",lt->tm_mday, months[lt->tm_mon], BASEYEAR+lt->tm_year);
    ensure_another(tbuf, strlen(strbuf));
    add_on(tbuf, strbuf);
}

void Day(struct textbuf *tbuf)
{
    struct tm *lt=GetCurrentTime();
    ensure_another(tbuf, 16);
    add_on(tbuf, day[lt->tm_wday]);
}

void Month(struct textbuf *tbuf)
{
    struct tm *lt=GetCurrentTime();
    ensure_another(tbuf, 16);
    add_on(tbuf, months[lt->tm_mon]);
}

void Page(struct textbuf *tbuf)
{
    ensure_another(tbuf, 6);
    add_on(tbuf, "\\\\n%");
    printline_haspage = TRUE;
}

void ShortYear(struct textbuf *tbuf)
{
    struct tm *lt=GetCurrentTime();
    sprintf(strbuf, "%d", lt->tm_year);
    ensure_another(tbuf, strlen(strbuf));
    add_on(tbuf, strbuf);
}

void TimeofDay(struct textbuf *tbuf)
{
    struct tm *lt=GetCurrentTime();
    boolean am=TRUE;
    if(lt->tm_hour>12) {
	am=FALSE;
	lt->tm_hour-=12;
    }
    else if (lt->tm_hour == 12) am = FALSE;

    sprintf(strbuf, "%d:%s%d %s", lt->tm_hour, (lt->tm_min>9)?"":"0", lt->tm_min, am?"AM":"PM");
    ensure_another(tbuf, strlen(strbuf));
    add_on(tbuf, strbuf);
}

void Time(struct textbuf *tbuf)
{
    time_t t=time(0);
    char *ct=ctime(&t),*i;
    i=strchr(ct,'\n');
    if(i) *i='\0';
    ensure_another(tbuf, strlen(ct));
    add_on(tbuf, ct);
}

void Year(struct textbuf *tbuf)
{
    struct tm *lt=GetCurrentTime();
    sprintf(strbuf, "%d", BASEYEAR+lt->tm_year);
    ensure_another(tbuf, strlen(strbuf));
    add_on(tbuf, strbuf);
}
   
static procfptr procs[]={
    TwentyFourHourTime,
    Blank,
    Date,
    Date2,
    Day,
    Month,
    Page,
    ShortYear,
    Time,
    TimeofDay,
    Year
};


#define NKEYWORDS (sizeof(keywords)/sizeof(char *))

#if 0
static int findincommalist(char  *list,char  *sn)
{
    int i,count=0;
    char buf[256],*p;
    for(i=0;i<strlen(list);i++,count++) {
	p=buf;
	while(list[i] && list[i]!=',' && p-buf<256) *(p++)=list[i],i++;
	*p='\0';
	if(!strcmp(buf,sn)) return count;
    }
    return -1;
}
#endif

#define DEFAULTHEADERVARS 2
static void InstallHeaderVariables()
{
    char *headervars, *p;
    int count=0,i;

    headervars = environ::GetProfile("headervars");
    if (headervars) {
	p=headervars;
	while(p && *p) {
	    count++;
	    p=strchr(p+1,',');
	}
	if(DEFAULTHEADERVARS+count>9) {
	    fprintf(stderr,"Warning too many header variables maximum allowable is 10.\n");
	    return;
	}

	p=(char *)malloc(strlen(headervars)+1);
	if(!p) return;
	strcpy(p,headervars);
    } else {
	p = (char *)malloc(1);
	if (!p) return;
	p[0] = '\0';
    }
    hvars=(char **)malloc(sizeof(char *)*(DEFAULTHEADERVARS+count));
    if(!hvars) return;
    hvars[0]="chapter";
    hvars[1]="section";
    for(i=0;i<count;i++) {
	hvars[DEFAULTHEADERVARS+i]=p;
	while(*p && *p!=',') p++;
	*p++='\0';
    }
    hvarcount=DEFAULTHEADERVARS+count;
}
    
	
static void PrintLine(struct textbuf *tbuf, char  *string)
{
    long pos,len=strlen(string),c,ix=0;
    char *ptr;
    boolean keyword=FALSE,pass=FALSE;
    printline_haspage = FALSE; /* this global will contain whether the resultant string has a $page ("//n%") sequence. */
    for(pos=0;pos<len;pos++) {
	ensure_another(tbuf, 10);
	c=string[pos];
	if(keyword) {
	    keyword=FALSE;
	    if(c=='\\') pass=!pass;
	    if(c=='$') add_char(tbuf, c);
	    else {
		for(ix=NKEYWORDS-1;ix>=0;ix--) {
		    ptr = string + pos;
		    if(*keywords[ix]==c && !strncmp(ptr,keywords[ix], strlen(keywords[ix]))) {
			if(procs[ix])
			    procs[ix](tbuf);
			pos+=strlen(keywords[ix])-1;
			break;
		    }
		}
		if(ix<0) {
		    for(ix=hvarcount-1;ix>=0;ix--) {
			ptr = string + pos;
			if(*hvars[ix]==c && !strncmp(ptr,hvars[ix], strlen(hvars[ix]))) {
			    sprintf(strbuf, "\\*(H%d\n", ix);
			    ensure_another(tbuf, strlen(strbuf));
			    add_on(tbuf, strbuf);
			    pos+=strlen(hvars[ix])-1;
			    break;
			}
		    }

		}
	    }
	} else {
	    switch(c) {
		case '\n':
		    break;
		case '\\':
		    if(!pass) add_on(tbuf, "\\\\\\\\");
		    else add_char(tbuf, c);
		    break;
		case '$':
		    keyword=TRUE;
		    break;
		default:
		    add_char(tbuf, c);
	    }
	}
    }
}

static char *headrtv_GetInput(class text  *textobj)
{
    int len,pos;
    char *string;

    len = (textobj)->GetLength();
    pos = (textobj)->GetFence();
    len = len - pos;
    string = (char *) malloc(len + 1);

    (textobj)->CopySubString( pos, len, string, TRUE);
    
    return string;
}

void headrtv::Print(FILE  *file,char  *processor,char  *finalFormat, boolean  topLevel)
{
    char *string;
    int i;
    struct textbuf tbuf;

    tbuf.size = 40;
    tbuf.text = (char *)malloc(tbuf.size);

    if(!strcmp(processor,"troff")) {
	for(i=header_ltext;i<header_TEXTS;i++) {
	    string = headrtv_GetInput(Text(this, i));
	    if (string && *string != '\0') {
		fprintf(file,".ds %s ",strs[Data(this)->where][i]);
		strcpy(tbuf.text, "");
		tbuf.pos = 0;
		PrintLine(&tbuf, string);
		fputs(tbuf.text, file);
		fputs("\n", file);
	    }
	    if (string) free(string);
	}
    }

    free(tbuf.text);
}

void *headrtv::GetPSPrintInterface(char *printtype)
{
    static struct textview_insetdata dat;

    if (!strcmp(printtype, "text")) {
	dat.type = textview_Header;
	dat.u.Header = this;
	return (void *)(&dat);
    }

    return NULL;
}

char *headrtv::GetExpandedString(int pos, boolean *rethaspage)
{
    char *string;
    int i = pos;
    struct textbuf tbuf;

    string = headrtv_GetInput(Text(this, i));
    if (string && *string != '\0') {
	tbuf.size = 40;
	tbuf.text = (char *)malloc(tbuf.size);
	strcpy(tbuf.text, "");
	tbuf.pos = 0;
	PrintLine(&tbuf, string);
	if (rethaspage)
	    *rethaspage = printline_haspage;
	/*printf("### rethaspage(%d) == %d\n", pos, printline_haspage);*/
	if (string) free(string);
	return tbuf.text;
    }
    else {
	if (string) free(string);
	if (rethaspage)
	    *rethaspage = FALSE;
	return NULL;
    }
}

view_DSattributes headrtv::DesiredSize(long  width , long  height, enum view_DSpass  pass, long  *desiredwidth , long  *desiredheight)
{
    view_DSattributes result;
    result=(this->sections)->DesiredSize(width, height, pass, desiredwidth, desiredheight);
    *desiredwidth = width;
    if (*desiredwidth<100) *desiredwidth= 100; /* make sure it doesn't vanish if it's adjacent to another header */
    if(this->open) *desiredheight+=this->top*4;
    else *desiredheight=this->top;
    if(*desiredheight>this->top*5) {
	*desiredheight=this->top*5;
    }
    return result;
}

void headrtv::GetOrigin(long width, long height, long *originX, long *originY)
{
    *originX = 0;
    *originY = height; /* fit into surrounding text, don't drop down to baseline */
}

static char *hdrtv_where[]={"Header","Footer"};
static char *hdrtv_close="Close";
static char *hdrtv_open="Open";

#define ALIGNMENT (graphic_ATLEFT|graphic_ATBASELINE)
static void DrawBorder(class headrtv  *self,struct rectangle  *vb)
{
    long width,junk,left;
    char *type;
   
    type=hdrtv_where[Data(self)->where];
    (self)->SetTransferMode(graphic_COPY);
    (self)->FillRectSize( vb->left+1, vb->top+1, vb->width-2, self->top-1,(self)->WhitePattern());
    (self)->SetTransferMode(graphic_BLACK);
    (self)->DrawRectSize(vb->left, vb->top,  vb->width-1, vb->height-1);
    (self)->MoveTo(0,self->top-1);
    (self)->DrawLineTo( vb->width-1,self->top-1); 
    (self)->SetFont(self->font);
    (self)->MoveTo(5,self->top-5);
    (self)->DrawString(type,ALIGNMENT);
    (self->font)->TextSize((self)->GetDrawable(), type, strlen(type),&width,&junk);
    left=15+width;
    (self)->MoveTo( width + 10, 0);
    (self)->DrawLineTo( width + 10, self->top-1);
    self->wherebox = left;
   
    type=(self->open)?hdrtv_close:hdrtv_open;
    (self->font)->StringSize((self)->GetDrawable(),type, &width, &junk);
    self->closebox=vb->width-width-5;
    (self)->MoveTo(self->closebox,self->top-5);
    (self)->SetFont(self->font);
    (self)->DrawString(type,ALIGNMENT);
    (self)->MoveTo( self->closebox - 5, 0);
    (self)->DrawLineTo( self->closebox - 5, self->top-1);
}

static class textview *GetTextView(class view  *self)
{
    while(self) {
	if(ATK::IsTypeByName((self)->GetTypeName(),"textview")) return (class textview *)self;
	self=self->parent;
    }
    return (class textview *)self;
}

static void headrtv_MoveOn(class headrtv  *self, long  rock)
/* This is the routine bound to the Return key - it moves the focus from
  self->name to self->number, or, if self->number already has the focus,
      then it calls turnin_TurninGo
      */
{
    if (self->my_focus >= header_rtext) self->my_focus = header_ltext;
    else self->my_focus++;
    
    (self)->WantInputFocus( Textv(self, self->my_focus));
}

class view *headrtv::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    if(y>this->top) {
	int i;
	long testy = this->top;
	class view *tmpv;
	struct rectangle r;

	(this)->WantInputFocus(this);
	tmpv = (class view *)(this->sections)->Hit( action, x, y-this->top, numberOfClicks);
	for(i=header_ltext;i<header_TEXTS;i++) {
	    (Textv(this, i))->GetEnclosedBounds( &r);
	    testy += r.height;
	    if (y < testy) {
		this->my_focus = i;
		break;
	    }
	}
	return tmpv;
	
    }
    switch(action) {
	case view_LeftUp:
	case view_RightUp:
	    if(x>this->closebox||!this->open) {
		if(this->open) {
		    struct rectangle mvb;
		    class textview *tv=GetTextView(View(this));
		    (tv)->WantInputFocus(tv);
		    rectangle_SetRectSize(&mvb,0,0,0,0);
		    this->open=FALSE;
		    (this)->WantNewSize(this);
		    (this->sections)->InsertView(this,&mvb);
		} else (this)->WantInputFocus(this);
		return View(this);
	    }
	    if(x<this->wherebox) {
		if(Data(this)->where==header_HEADER)
		    Data(this)->where=header_FOOTER;
		else Data(this)->where=header_HEADER;
	    }
	    (Data(this))->NotifyObservers(0);
	    break;
    }
    return View(this);
}

void headrtv::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
{
    struct rectangle mvb;
   
    switch(type) {
	case view_PartialRedraw:
	case view_LastPartialRedraw:
	case view_FullRedraw:
	    (this)->GetVisualBounds(&mvb);
	   
	    DrawBorder(this,&mvb);
	    if(this->open) {
		mvb.top+=this->top;
		mvb.left++;
		mvb.width-=2;
		mvb.height-=this->top+1;
		(this->sections)->InsertView(this,&mvb);
		(this->sections)->FullUpdate( type, left,top, width, height);
	    }
    }
}

void headrtv::Update()
{
    struct rectangle mvb;
    (this)->view::Update();
    (this)->GetVisualBounds(&mvb);
    DrawBorder(this,&mvb);
}

void headrtv::WantInputFocus(class view  *requestor)
{
    if(View(this)!=requestor || !this->open) (this)->view::WantInputFocus(requestor);
}

void headrtv::LoseInputFocus()
{
    if(this->sections) (this->sections)->LoseInputFocus();
}

void headrtv::PostKeyState(class keystate  *keystate)
{
    class keystate *k;
    k=(this->keystate)->AddBefore(keystate);
    (this)->view::PostKeyState(k);
}
    
void headrtv::ReceiveInputFocus()
{
    
    if(!this->open) {
	this->open=TRUE;
	(this)->WantNewSize(this);
	if (Textv(this, header_ltext)) {
	    this->my_focus = header_ltext;
	    (this)->WantInputFocus( Textv(this, header_ltext));
	}
    } else
	if(this->sections) {
	    (this->sections)->ReceiveInputFocus();
	}
}

#if 0
/* never declared in the .ch (or .H) file, maybe it should be? */
boolean headrtv::CanView(char  *name)
{
    if(!strcmp(name,"header")) return TRUE;
    return FALSE;
}
#endif

void headrtv::LinkTree(class view  *parent)
{
    (this)->view::LinkTree(parent);
    if(this->sections) (this->sections)->LinkTree(this);
}

void headrtv::SetDataObject(class dataobject  *object)
{
    int i, pos;
    (this)->view::SetDataObject(object);
    
    for(i=header_ltext;i<header_TEXTS;i++) {
	(Textv(this, i))->SetDataObject( Text(this, i));
	pos = (Text(this, i))->GetFence();
	(Textv(this, i))->SetDotPosition( pos);
    }
}

headrtv::headrtv()
{
	ATKinit;

    struct FontSummary *fontSummary;
    class lpair *bottom;
    int i;

    this->open= environ::GetProfileSwitch("HeadersInitiallyOpen", FALSE);
    this->bfont=fontdesc::Create("andysans",fontdesc_Bold,12);
    this->font=fontdesc::Create("andysans",fontdesc_Plain,12);
    this->bifont=fontdesc::Create("andysans",fontdesc_Bold | fontdesc_Italic,12);
    this->ifont=fontdesc::Create("andysans",fontdesc_Italic,12);
    if(!this->ifont || !this->font || !this->bfont || !this->bifont) THROWONFAILURE( FALSE);
    if ((fontSummary = (this->font)->FontSummary( (this)->GetDrawable())) == NULL)
	THROWONFAILURE( FALSE);
    this->top=fontSummary->maxHeight+5;

    this->my_focus = header_ltext;
    for(i=header_ltext;i<header_TEXTS;i++) {
	Textv(this, i) = new textview;
	if(!Textv(this, i)) {
	    for(i--;i>=header_ltext;i--) (Textv(this, i))->Destroy();
	    THROWONFAILURE( FALSE);
	}
    }

    bottom = new lpair;
    this->sections = new lpair;
    (this->sections)->VSplit ( Textv(this, header_ltext), bottom, 66, FALSE);
    (bottom)->VSplit ( Textv(this, header_ctext), Textv(this, header_rtext), 50, FALSE);
    
    this->keystate = keystate::Create(this,newKeymap);
    if(!this->keystate) THROWONFAILURE( FALSE);

    (this->sections)->LinkTree( this);

    THROWONFAILURE( TRUE);
}

#if 0
static void newline(class headrtv  *tv,long  rock)
{
    message::DisplayString(tv,0,"Headers and footers can be only one line long.");
}
#endif

boolean headrtv::InitializeClass()
{
    struct proctable_Entry *tempProc;
    struct ATKregistryEntry  *textvClassInfo=ATK::LoadClass("textview");
    if(!textvClassInfo) return FALSE;
    newKeymap = new keymap;
    if(!newKeymap) return FALSE;
    tempProc = proctable::DefineProc("headrtv-newline", (proctable_fptr)headrtv_MoveOn ,&headrtv_ATKregistry_ ,NULL, "Goes to next section of the header/footer.");
    (newKeymap)->BindToKey("\n",tempProc,0);
    (newKeymap)->BindToKey("\r",tempProc,0);
    InstallHeaderVariables();
    return TRUE;
}
