/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/config/COPYRITE

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
ATK_IMPL("hwview.H")
#include <stdio.h>


#include "hwview.H"

#include "graphic.H"
#include "fontdesc.H"
#include "rect.h"
#include "point.h"
#include "keymap.H"
#include "keystate.H"
#include "menulist.H"
#include "scroll.H"
#include "bind.H"
#include "message.H"
#include "im.H"
#include "observable.H"
#include "dataobject.H"
#include "view.H"
#include "cursor.H"

#include "helloworld.H"

#define TOTALSIZE 1500

static void xgetinfo(class helloworldview  *hwv, struct range  *total , struct range  *seen , struct range  *dot);
static void ygetinfo(class helloworldview  *hwv, struct range  *total , struct range  *seen , struct range  *dot);
static void xsetframe(class helloworldview  *hwv, int  posn, long  cord , long  outof);
static void ysetframe(class helloworldview  *hwv, int  posn, long  cord , long  outof);
static long xwhat(class helloworldview  *hwv, long  cord , long  outof);
static long ywhat(class helloworldview  *hwv, long  cord , long  outof);

static const struct scrollfns horizInterface = {
    (scroll_getinfofptr) xgetinfo,
    (scroll_setframefptr) xsetframe,
    (scroll_endzonefptr) NULL,
    (scroll_whatfptr) xwhat
};

static const struct scrollfns vertInterface = {
    (scroll_getinfofptr) ygetinfo,
    (scroll_setframefptr) ysetframe,
    (scroll_endzonefptr) NULL,
    (scroll_whatfptr) ywhat
};

static class keymap *helloworldviewKeymap;
static class menulist *helloworldviewMenulist;


ATKdefineRegistry(helloworldview, view, helloworldview::InitializeClass);

static void setSubView(class helloworldview  *hwv,class dataobject  *dobj);
static void Center(class helloworldview  *hwv,long  rock);
static void Invert(class helloworldview  *hwv, long  rock);
static void relocate(class helloworldview  *hwv,long  rock);
static void readHW(class helloworldview  *hwv,long  rock);
static void writeHW(class helloworldview  *hwv,long  rock);
static void changeObj(class helloworldview  *hwv,long  rock);


helloworldview::helloworldview()
{
	ATKinit;

    this->haveInputFocus=FALSE;
    this->HaveDownTransition=FALSE;
    this->keystate=keystate::Create(this,helloworldviewKeymap);
    this->menulistDup=(helloworldviewMenulist)->DuplicateML(this);
    this->newFrameX=0;
    this->newFrameY=0;
    this->view=NULL;
    this->applayer=NULL;
    this->redrawSubView=TRUE;
    this->idleCursor=cursor::Create(this);
    (this->idleCursor)->SetStandard(Cursor_Cross);
    this->placeCursor=cursor::Create(this);
    (this->placeCursor)->SetStandard(Cursor_UpperLeftCorner);
    this->dragCursor=cursor::Create(this);
    (this->dragCursor)->SetStandard(Cursor_Octagon);
    THROWONFAILURE( TRUE);
}

helloworldview::~helloworldview()
{
	ATKinit;

    if(this->view!=NULL){
	(this->view)->DeleteApplicationLayer(this->applayer);
	(this->view)->Destroy();
    }
    delete this->idleCursor;
    delete this->placeCursor;
    delete this->dragCursor;
}

void helloworldview::LinkTree(class view  *parent)
{
    (this)->view::LinkTree(parent);
    if((this)->GetIM() && this->applayer!=NULL)
	(this->applayer)->LinkTree(this);
}

static void setSubView(class helloworldview  *hwv,class dataobject  *dobj)
{
    if(hwv->view!=NULL){
	(hwv->applayer)->UnlinkTree();
	if(hwv->view!=hwv->applayer)
	    (hwv->view)->DeleteApplicationLayer(hwv->applayer);
	(hwv->view)->Destroy();
    }

    if(dobj!=NULL){
	hwv->view=(class view *)ATK::NewObject((dobj)->ViewName());

	hwv->applayer=(hwv->view)->GetApplicationLayer();
	if(hwv->applayer==NULL)
	    hwv->applayer=hwv->view;

	(hwv->view)->SetDataObject(dobj);

	(hwv->applayer)->LinkTree(hwv);
    }else
	hwv->view=hwv->applayer=NULL;

    hwv->redrawSubView=TRUE;
}

void helloworldview::SetDataObject(class dataobject *d)
{
    class helloworld *hw = (class helloworld *) d;
    this->x=hw->x;
    this->y=hw->y;
    this->blackOnWhite=hw->blackOnWhite;
    setSubView(this,hw->dobj);
    (this)->view::SetDataObject(hw);
}

class view *helloworldview::GetApplicationLayer()
{
    return (class view *)scroll::Create(this,scroll_LEFT+scroll_BOTTOM);
}

void helloworldview::DeleteApplicationLayer(class view *v)
{
    class scroll *scrollbar = (class scroll *) v;
    (scrollbar)->Destroy();
}

void helloworldview::ObservedChanged(class observable *obj, long  val)
{
    class helloworld *changed = (class helloworld *) obj;
    class helloworld *hw=(class helloworld *)this->dataobject;

    if(changed==hw)
	switch(val){
	    case helloworld_SubObjectChanged:
		setSubView(this,hw->dobj);
		break;
	    case observable_OBJECTDESTROYED:
		setSubView(this,NULL);
		break;
	}

    (this)->view::ObservedChanged(changed,val);
}

#define WIDTH 100
#define HEIGHT 100

void helloworldview::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height )
{
    class helloworld *hw=(class helloworld *)this->dataobject;
    struct rectangle myVisualRect,rec;

    (this)->GetVisualBounds(&myVisualRect);
    this->vrWidth=rectangle_Width(&myVisualRect);
    this->vrHeight=rectangle_Height(&myVisualRect);

    (this)->PostCursor(&myVisualRect,this->idleCursor);

    if (this->newFrameX+this->vrWidth>TOTALSIZE)
	this->newFrameX=TOTALSIZE-this->vrWidth;
    if (this->newFrameY+this->vrHeight>TOTALSIZE)
	this->newFrameY=TOTALSIZE-this->vrHeight;

    this->frameX=this->newFrameX;
    this->frameY=this->newFrameY;

    if(hw->x==POSUNDEF){
	hw->x=this->frameX+(this->vrWidth-WIDTH)/2;
	hw->y=this->frameY+(this->vrHeight-HEIGHT)/2;
    }

    this->x=hw->x;
    this->y=hw->y;
    this->blackOnWhite=hw->blackOnWhite;

    rectangle_SetRectSize(&rec,this->x-this->frameX-1,
	this->y-this->frameY-1,WIDTH+1,HEIGHT+1);

    (this)->SetTransferMode(graphic_COPY);

    if(hw->blackOnWhite){
	(this)->FillRect(&myVisualRect,
		(this)->WhitePattern());
	/* if on white background, draw a rectangle around it */
	(this)->DrawRect(&rec);
    }else{
	(this)->FillRect(&myVisualRect,
		(this)->BlackPattern());
	(this)->FillRect(&rec, 		(this)->WhitePattern());
    }

    rec.top++;
    rec.left++;
    rec.width--;
    rec.height--;

    (this->applayer)->InsertView(this,&rec);
	
    /* clearing the childs cursors is bogus-- views should clear their own.
     * hopefully this convention will change some day
     */
    (this)->RetractViewCursors(this->applayer);

    (this->applayer)->FullUpdate(view_FullRedraw,0,0,WIDTH,HEIGHT);
    this->redrawSubView=FALSE;
}

void helloworldview::Update()
{    
    class helloworld *hw=(class helloworld *)this->dataobject;

    (this)->SetTransferMode( graphic_COPY);

    if(this->x!=hw->x ||
       this->y!=hw->y ||
       this->frameX!=this->newFrameX ||
       this->frameY!=this->newFrameY ||
       this->blackOnWhite!=hw->blackOnWhite ||
       this->redrawSubView){
	struct rectangle rec;

	if(this->x!=hw->x ||
	   this->y!=hw->y){
	    static char buf[100];
	    sprintf(buf,"Hello world at (%ld,%ld)",hw->x,hw->y);
	    message::DisplayString(this,0,buf);
	}	    

	if(hw->blackOnWhite!=this->blackOnWhite){
	    struct rectangle vr;
	    (this)->GetVisualBounds(&vr);
	    if(hw->blackOnWhite)
		(this)->FillRect(&vr,
			(this)->WhitePattern());
	    else
		(this)->FillRect(&vr,
			(this)->BlackPattern());
	    this->blackOnWhite=hw->blackOnWhite;
	}

	/* includes 1 pixel border */
	rectangle_SetRectSize(&rec,this->x-this->frameX-1,
		this->y-this->frameY-1,WIDTH+2,HEIGHT+2);

	if(hw->blackOnWhite)
	    (this)->FillRect(&rec,
		(this)->WhitePattern());
	else
	    (this)->FillRect(&rec,
		(this)->BlackPattern());

	this->x=hw->x;
	this->y=hw->y;
  	this->frameX=this->newFrameX;
  	this->frameY=this->newFrameY;
  
	rectangle_SetRectSize(&rec,this->x-this->frameX,
		this->y-this->frameY,WIDTH,HEIGHT);
	(this->applayer)->InsertView(this,&rec);

	if(hw->blackOnWhite)
	    (this)->DrawRectSize(this->x-this->frameX-1,
		this->y-this->frameY-1,WIDTH+1,HEIGHT+1);

	(this)->RetractViewCursors(this->applayer);

	(this->applayer)->FullUpdate(view_FullRedraw,0,0,
		WIDTH,HEIGHT);
	this->redrawSubView=FALSE;
    }else
	(this->applayer)->Update();
}


class view *helloworldview::Hit(enum view_MouseAction  action,long  x,long  y,long  numberOfClicks)
{
    class helloworld *hw=(class helloworld *)this->dataobject;

    if(!this->HaveDownTransition &&
       x>=(this->x-this->frameX) && x<(this->x-this->frameX+WIDTH) &&
	y>=(this->y-this->frameY) && y<(this->y-this->frameY+HEIGHT))
	return (this->applayer)->Hit(
			action,
			x-(this->x-this->frameX),
			y-(this->y-this->frameY),
			numberOfClicks);

    if(this->HaveDownTransition){
	switch(action){
	    case view_RightUp:
		this->HaveDownTransition=FALSE;
		/* fall through */
	    case view_RightMovement:
		hw->x+=x-this->hitX;
		hw->y+=y-this->hitY;
		this->hitX=x;
		this->hitY=y;
		break;
	    case view_LeftUp:
		this->HaveDownTransition=FALSE;
		hw->x=x+this->frameX;
		hw->y=y+this->frameY;
		break;
	    case view_LeftMovement:
		/* do nothing */
		break;
	    default:
		/* re-synchronize mouse */
		this->HaveDownTransition=FALSE;
	}

	if(!this->HaveDownTransition)
	    ((this)->GetIM())->SetWindowCursor(NULL);
    }

    if(!this->HaveDownTransition)
	switch(action){
		class cursor *activeCursor;
	    case view_RightDown:
	    case view_LeftDown:
		if(action==view_RightDown){
		    this->hitX=x;
		    this->hitY=y;
		    activeCursor=this->dragCursor;
		}else
		    activeCursor=this->placeCursor;

		this->HaveDownTransition=TRUE;
		((this)->GetIM())->SetWindowCursor(activeCursor);
		(this)->WantInputFocus(this);
		break;
	    default:
		break;
	}

    (hw)->NotifyObservers(0);

    return (class view *)this;
}


void helloworldview::ReceiveInputFocus()
{
    this->haveInputFocus=TRUE;
    this->keystate->next=NULL;
    (this)->PostKeyState(this->keystate);
    (this)->PostMenus(this->menulistDup);
}


void helloworldview::LoseInputFocus()
{
    this->haveInputFocus=FALSE;
}

static void Center(class helloworldview  *hwv,long  rock)
{
    class helloworld *hw=(class helloworld *)hwv->dataobject;

    hw->x = hwv->newFrameX + hwv->vrWidth / 2;
    hw->y = hwv->newFrameY + hwv->vrHeight / 2;

    (hw)->NotifyObservers(0);
}


static void Invert(class helloworldview  *hwv, long  rock)
{
    class helloworld *hw=(class helloworld *)hwv->dataobject;

    hw->blackOnWhite=!hw->blackOnWhite;
    (hw)->NotifyObservers(0);
}


static void relocate(class helloworldview  *hwv,long  rock)
{
    class helloworld *hw=(class helloworld *)hwv->dataobject;
    char buf[100];
    int x,y;

    message::AskForString(hwv,0,"New location (x,y): ",NULL,buf,sizeof(buf));

    if(sscanf(buf,"%d,%d",&x,&y)!=2)
	message::DisplayString(hwv,1,"Bad format; must be: number,number");
    else{
	hw->x=x;
	hw->y=y;

	(hw)->NotifyObservers(0);
    }
}


static void readHW(class helloworldview  *hwv,long  rock)
{
    char file[100], msgBuf[100];
    FILE *fp;

    message::AskForString(hwv,0,"Read file: ",NULL,file,sizeof(file));
    fp=fopen(file,"r");
    if(fp==NULL){
	sprintf(msgBuf,"Couldn't open %s for reading.", file);
	message::DisplayString(hwv,1,msgBuf);
    }else{
	char header[100];

	if(fgets(header,sizeof(header),fp)==NULL){
	    sprintf(msgBuf,"Premature end-of-file in %s.",file);
	    message::DisplayString(hwv,1,msgBuf);
	}else{
	    char name[20];
	    int id;

	    if(sscanf(header,"\\begindata{%[^,],%d}\n",name,&id)!=2){
		sprintf(msgBuf,"%s doesn't contain a valid datastream header.",file);
		message::DisplayString(hwv,1,msgBuf);
	    }else{
		class helloworld *hw=
		  (class helloworld *)hwv->dataobject;

		if(strcmp(name,(hw)->GetTypeName())!=0){
		    sprintf(msgBuf,"%s doesn't contain a helloworld dataobj.", file);
		    message::DisplayString(hwv,1,msgBuf);
		}else{
		    /* FINALLY, read the object in... */
		    (hw)->Read(fp,id);
		    fclose(fp);
		    (hw)->NotifyObservers(0);
		}
	    }
	}
    }
}


static void writeHW(class helloworldview  *hwv,long  rock)
{
    char file[100], msgBuf[100];
    FILE *fp;

    message::AskForString(hwv,0,"Write file: ",NULL,file,sizeof(file));
    fp=fopen(file,"w");
    if(fp==NULL){
	sprintf(msgBuf,"Couldn't open %s for writing.",file);
	message::DisplayString(hwv,1,msgBuf);
    }else{
	class helloworld *hw=
	  (class helloworld *)hwv->dataobject;
	
	(hw)->Write(fp,im::GetWriteID(),0);
	fclose(fp);
    }
}


static void changeObj(class helloworldview  *hwv,long  rock)
{
    char objtype[100],msgbuf[100];
    class helloworld *hw=(class helloworld *)hwv->dataobject;
    class dataobject *oldDobj=hw->dobj;

    message::AskForString(hwv,0,"Type of new object: ",NULL,objtype,sizeof(objtype));

    if(!ATK::IsTypeByName(objtype,"dataobject")){
	sprintf(msgbuf,"%s is not a dataobject!",objtype);
	message::DisplayString(hwv,1,msgbuf);
	return;
    }

    hw->dobj=(class dataobject *)ATK::NewObject(objtype);
    if(hw->dobj==NULL){
	message::DisplayString(hwv,1,"Error creating the object!");
	hw->dobj=oldDobj;
	return;
    }

    (hw)->NotifyObservers(helloworld_SubObjectChanged);

    (oldDobj)->Destroy(); /* get rid of the old one */
}

static void xgetinfo(class helloworldview  *hwv, struct range  *total , struct range  *seen , struct range  *dot)
{
    class helloworld *hw=(class helloworld *)hwv->dataobject;

    total->beg = 0;
    total->end = TOTALSIZE;
    seen->beg = hwv->frameX;
    seen->end = hwv->frameX + hwv->vrWidth;
    dot->beg = dot->end = hw->x;
}

static void ygetinfo(class helloworldview  *hwv, struct range  *total , struct range  *seen , struct range  *dot)
{
    class helloworld *hw=(class helloworld *)hwv->dataobject;

    total->beg = 0;
    total->end = TOTALSIZE;
    seen->beg = hwv->frameY;
    seen->end = hwv->frameY + hwv->vrHeight;
    dot->beg = dot->end = hw->y;
}

static void xsetframe(class helloworldview  *hwv, int  posn, long  cord , long  outof)
{
    hwv->newFrameX = posn - hwv->vrWidth * cord / outof;
    if (hwv->newFrameX + hwv->vrWidth > TOTALSIZE)
	hwv->newFrameX = TOTALSIZE - hwv->vrWidth;
    else if (hwv->newFrameX < 0)
	hwv->newFrameX = 0;
    (hwv)->WantUpdate( hwv);
}

static void ysetframe(class helloworldview  *hwv, int  posn, long  cord , long  outof)
{
    hwv->newFrameY = posn - hwv->vrHeight * cord / outof;
    if (hwv->newFrameY + hwv->vrHeight > TOTALSIZE)
	hwv->newFrameY = TOTALSIZE - hwv->vrHeight;
    else if (hwv->newFrameY < 0)
	hwv->newFrameY = 0;
    (hwv)->WantUpdate( hwv);
}

static long xwhat(class helloworldview  *hwv, long  cord , long  outof)
{
    return hwv->frameX + hwv->vrWidth * cord / outof;
}

static long ywhat(class helloworldview  *hwv, long  cord , long  outof)
{
    return hwv->frameY + hwv->vrHeight * cord / outof;
}


const void *helloworldview::GetInterface(const char  *type)
{
    if (strcmp(type, "scroll,vertical") == 0)
	return &vertInterface;
    else if (strcmp(type, "scroll,horizontal") == 0)
	return &horizInterface;
    else
	return NULL;
}


static struct bind_Description helloworldviewBindings[]={
    {"helloworldview-center", "\003",0, "Hello World,Center",0,0, (proctable_fptr) Center, "Center the helloworldview string."},
    {"helloworldview-invert", "\011",0, "Hello World,Invert",0,0, (proctable_fptr) Invert, "Invert the helloworldview string."},
    {"helloworldview-relocate", "\022",0, "Hello World,Relocate",0,0, (proctable_fptr) relocate, "Relocate the helloworld string."},
    {"helloworldview-read", NULL,0, "Hello World,Read",0,0, (proctable_fptr) readHW, "Read in a new hello world."},
    {"helloworldview-write", NULL,0, "Hello World,Write",0,0, (proctable_fptr) writeHW, "Write out the current hello world to a file."},
    {"helloworldview-change-obj",NULL,0, "Hello World,Change Object",0,0, (proctable_fptr) changeObj, "Change the object in the square."},
    NULL
};

boolean helloworldview::InitializeClass()
{
	
    helloworldviewMenulist = new menulist;
    helloworldviewKeymap=new keymap;
    bind::BindList(helloworldviewBindings,helloworldviewKeymap, 	
		helloworldviewMenulist,&helloworldview_ATKregistry_ );

    return TRUE;
}
