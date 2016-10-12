/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("dialogv.H")
#include <environ.H>
#include <fontdesc.H>
#include <observable.H>
#include <text.H>
#include <textview.H>
#include <sbutton.H>
#include <sbuttonv.H>
#include <menulist.H>
#include <keystate.H>

#include <im.H>

#include <dialog.H>
#include <dialogv.H>

#define PADDING 5


ATKdefineRegistry(dialogv, view, dialogv::InitializeClass);
static void HitFunc(class sbutton  *self, class dialogv  *rock, int  button, long  brock);
static void ConfigureFunc(class im  *self, long  rock , long  customrock, class im  *parent, int  *x , int  *y, unsigned int *w , unsigned int  *h);
static void Interface(class sbutton  *b, struct dialogv::HitRock  *rock, int  ind, long  brock);


boolean dialogv::InitializeClass()
{
    return TRUE;
}

dialogv::dialogv()
{
	ATKinit;

    
    this->cont=NULL;

    this->lastchoice=(-1);
    
    this->didsizeonce=FALSE;
    
    this->client=NULL;
    
    this->extrakeys=NULL;

    this->extramenus=NULL;

    this->hr.func=NULL;
    
    this->text=new textview;
    if(!this->text) THROWONFAILURE( FALSE);

    this->buttons=new sbuttonv;
    if(!this->buttons) THROWONFAILURE( FALSE);

    this->rider=this->sidekick=NULL;

    
    this->twidth = this->theight = this->bwidth = this->bheight = this->rheight = this->rwidth=0;
    this->sheight=this->swidth=0;
    
    this->destroyall=FALSE;
    
    THROWONFAILURE( TRUE);
}

dialogv::~dialogv()
{
	ATKinit;

    class im *im=(this)->GetIM();
    /*  remove all knowledge of an object from this dialogv immediately after the object has been destroyed or dissociated with this dialogv, otherwise it may get re-associated depending on the order in which things are done in this function. */

    (this)->UnlinkTree();
    
    if(this->text) {
	(this->text)->Destroy();
	this->text=NULL;
    }

    if(this->buttons) {
	(this->buttons)->Destroy();
	this->buttons=NULL;
    }

    if(this->rider) {
	(this->rider)->UnlinkTree();
	this->rider=NULL;
    }
    if(this->sidekick) {
	(this->sidekick)->UnlinkTree();
	this->sidekick=NULL;
    }

    
    if(this->destroyall) {
	class dialog *dobj=(class dialog *)(this)->GetDataObject();
	if(dobj) (dobj)->Destroy();
	
	if(im) {
	    (im)->SetView( NULL);
	    (im)->Destroy();
	}
    }
    
}

void dialogv::SetDataObject(class dataobject  *obj)
{
    class dialog *dobj=(class dialog *)obj;
    if(dobj) {

	if((dobj)->GetText()) {
	    if(this->text) (this->text)->SetDataObject( (dobj)->GetText());
	} else {
	    fprintf(stderr,"dialogv: Warning SetDataObject called on dialog with no text!\n");
	    fflush(stderr);
	}
	if((dobj)->GetButtons()) {
	    if(this->buttons) (this->buttons)->SetDataObject( (dobj)->GetButtons());
	} else {
	    fprintf(stderr,"dialogv: Warning SetDataObject called on dialog with no buttons!\n");
	    fflush(stderr);
	}
    }
    (this)->view::SetDataObject( obj);
}

void dialogv::FullUpdate(enum view::UpdateType  type, long  left , long  top , long  width , long  height)
{
    int tpos=0;
    double topcolor[3];
    struct rectangle int1, int2, int3;
    struct rectangle r;
    class dialog *d=(class dialog *)(this)->GetDataObject();
    
    if(!(this)->GetIM() || !this->text || !this->buttons || !d) return;

    (this)->GetLogicalBounds( &r);
    if(this->sidekick) {
	int1.width=r.width/2-2;
	int1.height=r.height;
	int1.left=0;
	int1.top=0;
	int2.top=0;
	int2.left=r.width/2 + 2;
	int2.width=r.width/2 - 2;
	int2.height=r.height;
    } else {
	int1.width=r.width;
	int1.height=r.height;
	int1.left=0;
	int1.top=0;
    }
    if(!this->didsizeonce) {
	long dw, dh;
	(this)->DesiredSize( r.width, r.height, view::NoSet, &dw, &dh);
    }

    switch(type) {
	case view::Remove:
	    (this->text)->FullUpdate( type, 0, 0, 0, 0);
	    (this->buttons)->FullUpdate( type, 0, 0, 0, 0);
	    if(this->rider) {
		(this->rider)->FullUpdate( type, 0, 0, 0, 0);
	    }
	    if(this->sidekick) {
		(this->sidekick)->FullUpdate( type, 0, 0, 0, 0);
	    }
	    break;
	case view::MoveNoRedraw:
	    sbuttonv::DrawBorder(this, int1.left, int1.top, int1.width, int1.height, (d)->GetPrefs(), sbuttonv_BORDEROUT, FALSE, &int1);
	    if(this->sidekick) sbuttonv::DrawBorder(this, int2.left, int2.top, int2.width, int2.height, (d)->GetPrefs(), sbuttonv_BORDEROUT, FALSE, &int2);
	    tpos=int1.top;
	    sbuttonv::DrawBorder(this, int1.left+PADDING, int1.top, int1.width-2*PADDING, this->theight, (d)->GetPrefs(), sbuttonv_BORDERIN, FALSE, &int3);
	    (this->text)->InsertViewSize( this, int3.left, int3.top, int3.width, int3.height);
	    tpos+=this->theight+PADDING;
	    if(this->rider) {
		sbuttonv::DrawBorder(this, int1.left+PADDING, tpos, int1.width-2*PADDING, this->rheight, (d)->GetPrefs(), sbuttonv_BORDEROUT, FALSE,  &int3);
		(this->rider)->InsertViewSize( this, int3.left, int3.top, int3.width, int3.height);
		tpos+=this->rheight;
	    }
	    (this->buttons)->InsertViewSize( this, (int1.width-this->bwidth)/2+int1.left - 1, tpos, this->bwidth, this->bheight);
	    if(this->sidekick) {
		(this->sidekick)->InsertViewSize( this, int2.left, int2.top, int2.width, int2.height);
	    }
	    break;
	case view::PartialRedraw:
	    break;
	case view::LastPartialRedraw:
	case view::FullRedraw:
	    sbuttonv::InteriorBGColor(this, (d)->GetPrefs(), FALSE, topcolor);
	    sbuttonv::DrawBorder(this, int1.left, int1.top, int1.width, int1.height, (d)->GetPrefs(), sbuttonv_BORDEROUT, TRUE,  &int1);
	    if(this->sidekick) sbuttonv::DrawBorder(this, int2.left, int2.top, int2.width, int2.height, (d)->GetPrefs(), sbuttonv_BORDEROUT, TRUE,  &int2);
	    tpos=int1.top+PADDING;
	    sbuttonv::DrawBorder(this, int1.left+5, tpos, int1.width-2*PADDING, this->theight, (d)->GetPrefs(), sbuttonv_BORDERIN, TRUE,  &int3);
	    (this->text)->InsertViewSize( this, int3.left, int3.left, int3.width, int3.height);
	    tpos+=this->theight+PADDING;
	    if(this->rider) {
		sbuttonv::DrawBorder(this, int1.left + 5, tpos, int1.width - 2*PADDING, this->rheight, (d)->GetPrefs(), sbuttonv_BORDEROUT, TRUE,  &int3);
		(this->rider)->InsertViewSize( this, int3.left, int3.top, int3.width, int3.height);
		tpos+=this->rheight+1;
	    }
	    (this->buttons)->SetBGColor( topcolor[0], topcolor[1], topcolor[2]);
	    (this->buttons)->InsertViewSize( this, int1.left + (int1.width-this->bwidth)/2 - 1, tpos, this->bwidth, this->bheight);
	    if(this->sidekick) {
		(this->sidekick)->InsertViewSize( this, int2.left, int2.top, int2.width, int2.height);
	    }

	    (this->text)->SetBGColor( topcolor[0], topcolor[1], topcolor[2]);
	    (this->text)->FullUpdate( type, 0, 0, int1.width, this->theight);
	    if(this->rider) {
		(this->rider)->SetBGColor( topcolor[0], topcolor[1], topcolor[2]);
		(this->rider)->FullUpdate( type, 0, 0, this->rwidth, this->rheight);
	    }
	    if(this->sidekick) {
		(this->sidekick)->FullUpdate( type, 0, 0, int2.width, int2.height);
	    }
	    (this->buttons)->FullUpdate( type, 0, 0, this->bwidth, this->bheight);
    }
    
}

class view *dialogv::Hit(enum view::MouseAction  action, long  x, long  y, long  numberOfClicks)
{

    if(this->rider) {
	long rx=(this->rider)->EnclosedXToLocalX( x);
	long ry=(this->rider)->EnclosedYToLocalY( y);
	if( rx>=0 && rx<(this->rider)->GetLogicalWidth() && ry>=0 && ry<(this->rider)->GetLogicalHeight()) {
	    return (this->rider)->Hit( action, (this->rider)->EnclosedXToLocalX( x),(this->rider)->EnclosedYToLocalY( y), numberOfClicks);
	}
    }
    if(this->sidekick) {
	long sx=(this->sidekick)->EnclosedXToLocalX( x);
	long sy=(this->sidekick)->EnclosedYToLocalY( y);
	
	if(sx>=0 && sx<(this->sidekick)->GetLogicalWidth() && sy>=0 && sy<(this->sidekick)->GetLogicalHeight()) {
	    return (this->sidekick)->Hit( action, sx, sy, numberOfClicks);
	}
    }
    if(this->buttons) return (this->buttons)->Hit( action, (this->buttons)->EnclosedXToLocalX( x), (this->buttons)->EnclosedYToLocalY( y), numberOfClicks);
    return (class view *)this;
}

#define PERCENTAGE(x, p) (((x)*(p))/100)

view::DSattributes dialogv::DesiredSize(long  width, long  height, enum view::DSpass  pass, long  *dWidth, long  *dHeight)
{
    long iw, ih;
    long dummy;
    long  oldheight;
    class dialog *d=(class dialog *)(this)->GetDataObject();
    
    this->didsizeonce=TRUE;
    
    if(!this->buttons || !this->text || !d) {
	*dWidth = width;
	*dHeight = (height > 2048) ? 256 :height;
	return (view::DSattributes) (view::HeightFlexible | view::WidthFlexible);
    }

    if(width<0 || width>32767) width=((class view *)this)->parent?(((class view *)this)->parent)->GetLogicalWidth(): 500;
    
    
    (void) (this->buttons)->DesiredSize( 0, 0, view::NoSet, &this->bwidth, &this->bheight);

    
    if(this->rider || this->sidekick) width=PERCENTAGE(width, 90);
    else {
	long low=this->bwidth, high=PERCENTAGE(width, 90);
	/* Please, there MUST be a better way! */
	(void) (this->text)->DesiredSize( high, height-this->bheight, view::WidthSet, &this->twidth, &oldheight);
	sbuttonv::SizeForBorder(this->text, sbuttonv_Enclosing, d->prefs, FALSE, this->twidth, oldheight, &this->twidth, &oldheight);
	while(low<high-20) {
	    sbuttonv::SizeForBorder(this->text, sbuttonv_Interior, d->prefs, FALSE, (low+high)/2-2*PADDING, height-this->bheight, &this->twidth, &this->theight);
	    (void) (this->text)->DesiredSize( this->twidth, this->theight, view::WidthSet, &this->twidth, &this->theight);
	    sbuttonv::SizeForBorder(this->text, sbuttonv_Enclosing, d->prefs, FALSE, this->twidth, this->theight, &this->twidth, &this->theight);
	    if(this->theight>oldheight) low=(low+high)/2+1;
	    else high=(low+high)/2;
	}
	width=high+10; /* SORRY.... this is a magic fudge factor, please fix if you can see how. */
    }
    
    if(width < this->bwidth) width=this->twidth=this->bwidth;
    else this->twidth=width;

    if(this->sidekick) {
	this->twidth=width/2;
	this->bwidth=width/2;
	this->swidth=width/2;
    }
    
    sbuttonv::SizeForBorder(this->text, sbuttonv_Interior, d->prefs, FALSE, this->twidth, height-this->bheight, &this->twidth, &this->theight);
    (void) (this->text)->DesiredSize( this->twidth, height-this->bheight, view::WidthSet, &dummy, &this->theight);
    sbuttonv::SizeForBorder(this->text, sbuttonv_Enclosing, d->prefs, FALSE, this->twidth, this->theight, &dummy, &this->theight);

    this->twidth-=2*PADDING;
    
    if(this->rider) {
	sbuttonv::SizeForBorder(this->rider, sbuttonv_Interior, d->prefs, TRUE,  this->twidth-2*PADDING, height-this->bheight-this->theight, &iw, &ih);
	(void) (this->rider)->DesiredSize( iw, ih, view::WidthSet, &dummy, &this->rheight);
	sbuttonv::SizeForBorder(this->rider, sbuttonv_Enclosing, d->prefs, TRUE, dummy, this->rheight,  &dummy, &this->rheight);
    }

    if(this->sidekick) {
	sbuttonv::SizeForBorder(this->sidekick, sbuttonv_Interior, d->prefs, FALSE, width/2, this->theight + this->bheight + this->rheight, &iw, &ih);
	(void) (this->sidekick)->DesiredSize( iw, ih, view::WidthSet, &dummy, &this->sheight);
	sbuttonv::SizeForBorder(this->sidekick, sbuttonv_Enclosing, d->prefs, FALSE, dummy, this->sheight, &dummy, &this->sheight);
	
    }
    sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, d->prefs, FALSE, width, this->theight + this->bheight +  this->rheight +2*PADDING, dWidth, dHeight);
    return view::Fixed;
}

void dialogv::InstallRider(class view  *rider)
{
    long dumbw, dumbh;
    this->roffset=PADDING;
    this->rider=rider;
    if(rider) (rider)->LinkTree( this);
    if(this->client) (void) (this)->DesiredSize( (this->client)->GetLogicalWidth(), (this->client)->GetLogicalHeight(), view::WidthSet, &dumbw, &dumbh);
    if(rider) (rider)->WantInputFocus( rider);
}

void dialogv::InstallSidekick(class view  *sidekick)
{
    long dumbw, dumbh;
    this->sidekick=sidekick;
    if(sidekick) (sidekick)->LinkTree( this);
    if(this->client) (void) (this)->DesiredSize( (this->client)->GetLogicalWidth(), (this->client)->GetLogicalHeight(), view::WidthSet, &dumbw, &dumbh);
}
	
void dialogv::LinkTree(class view  *parent)
{
    class im *im;
    class dialog *d=(class dialog *)(this)->GetDataObject();
    
    (this)->view::LinkTree( parent);
    
    im=(this)->GetIM();
    
    if(d && im) {
	(this)->SetForegroundColor( (d)->GetForeground(), 0, 0, 0);
	(this)->SetBackgroundColor( (d)->GetBackground(), 0, 0, 0);
	(this)->SetForegroundColor( (d)->GetForeground(), 0, 0, 0);
    }
    
    if(this->buttons) (this->buttons)->LinkTree( this);
    
    if(this->text) {
	(this->text)->LinkTree( this);
	if(d && im) {
	    (this->text)->SetForegroundColor( (d)->GetForeground(), 0, 0, 0);
	    (this->text)->SetBackgroundColor( (d)->GetBackground(), 0, 0, 0);
	    (this->text)->SetForegroundColor( (d)->GetForeground(), 0, 0, 0);
	}
    }
    if(this->rider) {
	(this->rider)->LinkTree( this);
	if(d && im) {
	    (this->rider)->SetForegroundColor( (d)->GetForeground(), 0, 0, 0);
	    (this->rider)->SetBackgroundColor( (d)->GetBackground(), 0, 0, 0);
	    (this->rider)->SetForegroundColor( (d)->GetForeground(), 0, 0, 0);
	}
    }
    if(this->sidekick) {
	(this->sidekick)->LinkTree( this);
	if(d && im) {
	    (this->sidekick)->SetForegroundColor( (d)->GetForeground(), 0, 0, 0);
	    (this->sidekick)->SetBackgroundColor( (d)->GetBackground(), 0, 0, 0);
	    (this->sidekick)->SetForegroundColor( (d)->GetForeground(), 0, 0, 0);
	}
    }
}

class dialogv *dialogv::Create(const char  * const *list, const char  *font, int  style)
{
	ATKinit;

    class sbutton *s;
    class dialog *d=new dialog;
    class dialogv *dv=new dialogv;
    struct sbutton_list *sl=NULL;
    int count=0, i;
    struct sbutton_prefs *prefs;

    
    if(!d || !dv) {
	if(d) (d)->Destroy();
	if(dv) (dv)->Destroy();
	return NULL;
    }
    
    prefs=((d)->GetButtons())->GetDefaultPrefs();
    
    dv->destroyall=TRUE;
    
    while(*list && list[count]) count++;

    if(!count) {
	(d)->Destroy();
	(dv)->Destroy();
	return NULL;
    }

    sl=(struct sbutton_list *)malloc(sizeof(struct sbutton_list)*(count+1));

    if(!sl) {
	(d)->Destroy();
	(dv)->Destroy();
	return NULL;
    }

    for(i=0;i<count;i++) {
	sl[i].label=list[count-i-1];
	sl[i].trigger=NULL;
	sl[i].rock=0;
	sl[i].lit=FALSE;
    }
    sl[count].label=NULL;
    
    s=sbutton::CreateFilledSButton(prefs, sl);
    free(sl);
    
    if(!s) {
	(d)->Destroy();
	(dv)->Destroy();
	return NULL;
    }
    
    (d)->SetButtons( s);
        
    (dv)->SetDataObject( d);

    if((dv)->GetTextView() && (dv)->GetTextData()) ((dv)->GetTextData())->Clear();

    
    return dv;
}


static void HitFunc(class sbutton  *self, class dialogv  *rock, int  button, long  brock)
{
    rock->lastchoice=(self)->GetCount()-button-1;
    *(rock->cont)=FALSE;
}

void dialogv::PostMenus(class menulist  *ml)
{
    if(!ml) {
	(this)->view::PostMenus( this->extramenus);
	return;
    }
    if(this->extramenus && this->extramenus!=ml) {
	(this->extramenus)->ClearChain();
	(this->extramenus)->ChainAfterML( ml, (long)ml);
	(this)->view::PostMenus( this->extramenus);
    } else (this)->view::PostMenus( ml);
}

void dialogv::ReceiveInputFocus()
{
    if(this->rider) (this->rider)->ReceiveInputFocus();
    else {
	if(this->extrakeys) {
	    this->extrakeys->next = NULL;
	    (this)->PostKeyState( this->extrakeys);
	}
	if(this->extramenus) (this)->PostMenus( this->extramenus);
    }
}

void dialogv::SetExtraMenus(class menulist  *ml)
{
    this->extramenus=ml;
}

void dialogv::SetExtraKeyState(class keystate  *ks)
{
    this->extrakeys=ks;
}

void dialogv::ActivateButton(int  ind)
{
    class sbutton *sb=(this)->GetButtonsData();
    (sb)->ActivateButton( sb->count-ind-1);
    this->lastchoice=ind;
}

void dialogv::DeActivateButton(int  ind)
{
    class sbutton *sb=(this)->GetButtonsData();
    if(sb && (this)->GetButtonsView()) {
	(sb)->DeActivateButton( sb->count-ind-1);
	this->lastchoice=(-1);
    }
}

static void ConfigureFunc(class im  *self, long  rock , long  customrock, class im  *parent, int  *x , int  *y, unsigned int  *w , unsigned int  *h)
{
    long lw, lh;
    (void) ((class dialogv *)customrock)->DesiredSize( (parent)->GetLogicalWidth(), (parent)->GetLogicalHeight(), view::NoSet, &lw, &lh);
    *w=(unsigned int)lw;
    *h=(unsigned int)lh;
    (self)->NormalConfiguration( rock, customrock, parent, x, y,w, h);
}
    
int dialogv::PostChoice(class im  *im, class view  *client, boolean  *cflag, int  deflt, boolean  block, long  pos)
{
    class im *new_c;
    class sbutton *sb=(this)->GetButtonsData();
    im_configurefptr oldconfigfunc;
    long oldconfigrock, oldcustomrock;

    if(pos==0) pos=im_InMiddle|im_Centered;
    this->client=client;
    
      /* Horrible hack to get desired size for the dialog box! We link the dialogview into the view tree of the client  and then after we have created the window unlink it so that it can be linked into the new im.  This is all because in order to get an appropriate size it really needs to be able to get at info about the actual display and its fonts */
    (this)->LinkTree( client);

    /* Set the default configuration function temporarily so that the configuration function here will get called to determine the desired size for the window */
    oldconfigfunc=im::DefaultConfigureFunction(ConfigureFunc);
    oldconfigrock=im::DefaultConfigureRock(pos);
    oldcustomrock=im::DefaultConfigureCustomRock((long)this);

    if(block) new_c=im::CreateOverride(im);
    else new_c=im::CreateTransient(im);
    
    im::DefaultConfigureFunction(oldconfigfunc);
    im::DefaultConfigureRock(oldconfigrock);
    im::DefaultConfigureCustomRock(oldcustomrock);

    /* unlink the dialogview from the client view tree so that it can be re-linked into the new im's view tree, since the new im is on the same screen and the same display all the computations done while under the client view tree should still be valid */
    (this)->UnlinkTree();
    
    if(!new_c) return -1;

    (new_c)->SetBorderWidth( 0);

    (new_c)->SetView( this);
    
    (this)->WantInputFocus( this);
    (sb)->GetHitFunc()=(sbutton_hitfptr)HitFunc;
    (sb)->GetHitFuncRock()=(long)this;
    if(deflt>=0 && deflt<sb->count) (this)->ActivateButton( deflt);
    this->lastchoice=(-1);
    this->cont=cflag;
    while(*cflag) im::Interact(TRUE);

    if(this->lastchoice>=0) return this->lastchoice;
    else return -1;
}

static void Interface(class sbutton  *b, struct dialogv::HitRock  *rock, int  ind, long  brock)
{
    if(rock->func) rock->func(rock->rock, b->count-ind-1, brock);
}

int dialogv::PostInput(class im  *im, class view  *client, dialogv_choicefptr  choicefunc, long  choicerock, boolean  block, long  pos)
{
    class im *new_c;
    class sbutton *sb=(this)->GetButtonsData();
    im_configurefptr oldconfigfunc;
    long oldconfigrock, oldcustomrock;
    
    if(pos==0) pos=im_InMiddle|im_Centered;
    
    this->client=client;
    
    this->hr.func=choicefunc;
    this->hr.rock=choicerock;
    /* Horrible hack to get desired size for the dialog box! */
    (this)->LinkTree( client);

    oldconfigfunc=im::DefaultConfigureFunction(ConfigureFunc);
    oldconfigrock=im::DefaultConfigureRock(pos);
    oldcustomrock=im::DefaultConfigureCustomRock((long)this);
    
    if(block) new_c=im::CreateOverride(im);
    else new_c=im::CreateTransient(im);
    
    im::DefaultConfigureFunction(oldconfigfunc);
    im::DefaultConfigureRock(oldconfigrock);
    im::DefaultConfigureCustomRock(oldcustomrock);

    (this)->UnlinkTree();
    if(!new_c) return -1;
    
    (new_c)->SetBorderWidth( 0);

    (new_c)->SetView( this);

    (sb)->GetHitFunc()=(sbutton_hitfptr)Interface;
    (sb)->GetHitFuncRock()=(long)&this->hr;

    return 0;
}

void dialogv::Vanish()
{
    if((this)->GetIM()) ((this)->GetIM())->VanishWindow();
}

void dialogv::UnVanish()
{
    if((this)->GetIM()) ((this)->GetIM())->ExposeWindow();
}

void dialogv::SetLayout(int  rows , int  cols)
{
    if(!(this)->GetButtonsData()) return;
    ((this)->GetButtonsData())->SetLayout( rows, cols, sbutton_GrowRows);
}

void dialogv::CancelQuestion()
{
    *(this->cont)=FALSE;
    this->lastchoice=(-1);
}
