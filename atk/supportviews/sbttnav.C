/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/supportviews/RCS/sbttnav.C,v 3.4 1995/04/06 02:55:07 rr2b Stab74 $";
#endif


 
#include <andrewos.h>
ATK_IMPL("sbttnav.H")
#include <sys/param.h>	/* for MAXPATHLEN */
#include <stdio.h>

#include <util.h>
#include <cursor.H>
#include <environ.H>
#include <fontdesc.H>
#include <graphic.H>
#include <im.H>
#include <menulist.H>
#include <message.H>
#include <observable.H>
#include <proctable.H>
#include <atom.H>

#include "sbutton.H"
#include "sbttnav.H"

#define PROMPTFONT "andysans12b"

/* Forward Declarations */
              

/* Global Variables */
static class menulist *sbmenulist = NULL;

struct proctable_Entry *scpe=NULL, *lcpe=NULL, *triggerpe=NULL, *bdeletepe=NULL, *gdeletepe=NULL, *renamepe=NULL, *bgpe=NULL, *labelpe=NULL, *fontpe=NULL, *stylepe=NULL, *colorpe=NULL, *dpe=NULL;


ATKdefineRegistry(sbttnav, sbuttonv, sbttnav::InitializeClass);
#ifndef NORCSID
#endif
static char *Intern(char  *str);
static struct sbutton_prefs *LookupGroupPrefs(class sbttnav  *self, char  *name);
static struct groups **LookupGroup(class sbttnav  *self, char  *name);
static void AddGroupMenu(class sbttnav  *self, struct sbutton_prefs  *prefs, int  prio);
static boolean domenus(class sbutton  *b, int  i, struct sbutton_info  *si, struct menusrock  *mr);
static void BDeleteProc(class sbttnav  *self, long  param);
static void LabelProc(class sbttnav  *self, long  param);
static void TriggerProc(class sbttnav  *self, long  param);
static void FontProc(class sbttnav  *self, long  param);
static void StyleProc(class sbttnav  *self, long  param);
static void ColorProc(class sbttnav  *self, long  param);
static void GroupProc(class sbttnav  *self, long  rock);
static void AddButtonProc(class sbttnav  *self, long  rock);
static void SetRowsProc(class sbttnav  *self, long  rock);
static void SetColsProc(class sbttnav  *self, long  rock);
static void NewGroupProc(class sbttnav  *self, long  rock);
static void RenameProc(class sbttnav  *self, long  rock);
static boolean dodeletion(class sbutton  *b, int  i, struct sbutton_info  *si, struct deleterock  *rock);
static void GDeleteProc(class sbttnav  *self, long  rock);
static void ShadowColorProc(class sbttnav  *self, long  param);
static void LabelColorProc(class sbttnav  *self, long  param);
static void DepthProc(class sbttnav  *self, long  param);


static char *Intern(char  *str)
{
    class atom *a=atom::Intern(str);
    if(a!=NULL) return (a)->Name();
    else return NULL;
}

boolean sbttnav::InitializeClass()
{
    /* 
      Initialize all the class data.
      Set up the proc table entries and the menu list 
      (which is cloned for each instance of this class).
      */
    struct proctable_Entry *proc = NULL;

    if ((sbmenulist = new menulist) == NULL) return(FALSE);

    if ((bgpe = proctable::DefineProc("sbuttonv-set-button-group", (proctable_fptr)GroupProc, &sbttnav_ATKregistry_ , NULL, "Choose which group the button will take its appearance from.")) == NULL) return FALSE;
    
    if ((labelpe = proctable::DefineProc("sbuttonv-set-label", (proctable_fptr)LabelProc, &sbttnav_ATKregistry_ , NULL, "Prompts for user to set the text string of the sbutton.")) == NULL) return(FALSE);
    (sbmenulist)->AddToML( "SButton~20,Set Label~10", labelpe, 0, 0);

    if ((triggerpe = proctable::DefineProc("sbuttonv-set-trigger", (proctable_fptr)TriggerProc, &sbttnav_ATKregistry_ , NULL, "Prompts for user to set the text string of the sbutton.")) == NULL) return(FALSE);
    (sbmenulist)->AddToML( "SButton~20,Set Trigger~11", triggerpe, 0, 0);

    if ((fontpe = proctable::DefineProc("sbuttonv-set-font", (proctable_fptr)FontProc, &sbttnav_ATKregistry_ , NULL, "Prompts for user to set the font of the sbutton.")) == NULL) return(FALSE);

    if ((stylepe = proctable::DefineProc("sbuttonv-set-style", (proctable_fptr)StyleProc, &sbttnav_ATKregistry_ , NULL, "Prompts for user to set the appearance of the SButton.")) == NULL) return(FALSE);
    if ((colorpe = proctable::DefineProc("sbuttonv-set-colors", (proctable_fptr)ColorProc, &sbttnav_ATKregistry_ , NULL, "Prompts for user to set the foreground and background color of the SButton.")) == NULL) return(FALSE);

    if ((scpe = proctable::DefineProc("sbuttonv-set-shadow-colors", (proctable_fptr)ShadowColorProc, &sbttnav_ATKregistry_ , NULL, "Prompts for user to set the top, bottom and middle shadow colors of the SButton.")) == NULL) return(FALSE);

    if ((lcpe = proctable::DefineProc("sbuttonv-set-label-color", (proctable_fptr)LabelColorProc, &sbttnav_ATKregistry_ , NULL, "Prompts for user to set the foreground and background color of the SButton's label.")) == NULL) return(FALSE);
    
    if ((dpe = proctable::DefineProc("sbuttonv-set-depth", (proctable_fptr)DepthProc, &sbttnav_ATKregistry_ , NULL, "Prompts for user to set the depth of the SButton.")) == NULL) return(FALSE);
    
    if ((proc = proctable::DefineProc("sbuttonv-add-button", (proctable_fptr)AddButtonProc, &sbttnav_ATKregistry_ , NULL, "Prompts for user to name a new button.")) == NULL) return(FALSE);

    (sbmenulist)->AddToML( "SButton~20,Add Button~30", proc, 0, 0);

    if ((proc = proctable::DefineProc("sbuttonv-set-rows", (proctable_fptr)SetRowsProc, &sbttnav_ATKregistry_ , NULL, "Sets the number of rows to be used for buttons. Enough columns will be used to ensure that all buttons are visible.")) == NULL) return(FALSE);

    (sbmenulist)->AddToML( "SButton~20,Set Rows~71", proc, 0, 0);
    
    if ((proc = proctable::DefineProc("sbuttonv-set-columns", (proctable_fptr)SetColsProc, &sbttnav_ATKregistry_ , NULL, "Sets the number of columns to be used for buttons. Enough rows will be used to ensure that all buttons are visible.")) == NULL) return(FALSE);

    (sbmenulist)->AddToML( "SButton~20,Set Columns~72", proc, 0, 0);

    if ((proc = proctable::DefineProc("sbuttonv-new-group", (proctable_fptr)NewGroupProc, &sbttnav_ATKregistry_ , NULL, "Creates a new button group.")) == NULL) return FALSE;

    (sbmenulist)->AddToML( "SButton~20,New group~95", proc, 0, 0);

    if ((proc = proctable::DefineProc("sbuttonv-delete", (proctable_fptr)BDeleteProc, &sbttnav_ATKregistry_ , NULL, "Deletes the selected button.")) == NULL) return FALSE;

    (sbmenulist)->AddToML( "SButton~20,Delete button~31", proc, 0, 0);

    if ((renamepe = proctable::DefineProc("sbuttonv-rename-group", (proctable_fptr)RenameProc, &sbttnav_ATKregistry_ , NULL, "Renames the group specified by the rock given.")) == NULL) return FALSE;

    if ((gdeletepe = proctable::DefineProc("sbuttonv-delete-group", (proctable_fptr)GDeleteProc, &sbttnav_ATKregistry_ , NULL, "Deletes the group specified by the string given as a rock.")) == NULL) return FALSE;
    return(TRUE);
}

static struct themenus {
    char *name;
    struct proctable_Entry **pe;
} mymenus[]= {
    {"Set Button Group~11", &bgpe},
    {"Set Font~22", &fontpe},
    {"Set Style~23", &stylepe},
    {"Set Colors~24", &colorpe},
    {"Set Shadow Colors~25", &scpe},
    {"Set Label Colors~26", &lcpe},
    {"Set Depth~27",&dpe},
    {NULL, NULL},
    {"Rename Group~30", &renamepe},
    {"Delete Group~35", &gdeletepe},
    {NULL, NULL}
};

static struct sbutton_prefs *LookupGroupPrefs(class sbttnav  *self, char  *name)
{
    struct groups *g=self->groups;
    while(g) {
	if(g->prefs->name!=NULL && !strcmp(g->prefs->name, name)) return g->prefs;
	g=g->next;
    }
    return NULL;
}

static struct groups **LookupGroup(class sbttnav  *self, char  *name)
{
    struct groups **g=(&self->groups);
    while(*g) {
	if((*g)->prefs->name!=NULL && !strcmp((*g)->prefs->name, name)) return g;
	g=(&(*g)->next);
    }
    return NULL;
}

static void AddGroupMenu(class sbttnav  *self, struct sbutton_prefs  *prefs, int  prio)
{
    char buf[256];
    struct groups *g;
    class menulist *ml;
    int len=strlen(prefs->name);
    
    g=(struct groups *)malloc(sizeof(struct groups));
    if(g==NULL) return;

    g->prefs=prefs;
    prefs->refcount++; /* declare our interest in this set of preferences */
    
    ml=menulist::Create(self);
    if(ml==NULL) {
	free(g);
	return;
    }
    
    if(prio<0) {
	g->prio=22+self->groupcount;
	self->groupcount++;
    } else g->prio=prio;
    
    g->ml=ml;
    if(len<sizeof(buf)-32) {
	struct themenus *tm=mymenus;
	while(tm->name) {
	    strcpy(buf, prefs->name);
	    sprintf(buf+len, "~%d,", g->prio);
	    strcat(buf,tm->name);
	    (ml)->AddToML( buf, *tm->pe, (long)prefs->name, 0);
	    tm++;
	}
	if(strcmp(prefs->name, "Default")) {
	    tm++;
	    while(tm->name) {
		strcpy(buf, prefs->name);
		sprintf(buf+len, "~%d,", g->prio);
		strcat(buf,tm->name);
		(ml)->AddToML( buf, *tm->pe, (long)prefs->name, 0);
		tm++;
	    }
	}
	g->next=self->groups;
	self->groups=g;
    } else {
	delete ml;
	free(g);
    }
    return;
}
    
    
struct menusrock {
    class sbttnav *self;
    long count;
};

static boolean domenus(class sbutton  *b, int  i, struct sbutton_info  *si, struct menusrock  *mr)
{
    struct groups *g=mr->self->groups;
    while(g) {
	if(g->prefs==si->prefs) {
	    (g->ml)->ClearChain();
	    (mr->self->ml)->ChainAfterML( g->ml, (long)g->ml);
	    return FALSE;
	}
	g=g->next;
    }
    if(si->prefs->name!=NULL) AddGroupMenu(mr->self, si->prefs, -1);
    return FALSE;
}
    
void sbttnav::PostMenus(class menulist  *ml)
{
    class sbutton *b=(this)->ButtonData();
    struct menusrock mr;
    struct groups *g;
    mr.self=this;
    mr.count=0;
    (this->ml)->ClearChain();
    (b)->Enumerate( (sbutton_efptr)domenus, (long)&mr);
    g=this->groups;
    while(g) {
	(g->ml)->ClearChain();
	(this->ml)->ChainAfterML( g->ml, (long)g->ml);
	g=g->next;
    }
    if (ml) (this->ml)->ChainAfterML( ml, (long)ml);
    (this)->sbuttonv::PostMenus( this->ml);
}

boolean sbttnav::Touch(int  ind, enum view_MouseAction  action)
{
    class sbutton *b=(this)->ButtonData();
    switch(action) {
	case view_RightDown:
	    this->dragging=(this)->LastButton();
	    (this)->WantInputFocus( this);
	    if((this)->GetIM()) {
		this->cursor=cursor::Create(this);
		if(this->cursor) {
		    (this->cursor)->SetStandard( Cursor_LeftPointer);
		    ((this)->GetIM())->SetWindowCursor( this->cursor);
		}
	    }
	    break;
	case view_RightUp:
	    (b)->DeActivateButton( (this)->LastButton());
	    if((this)->LastButton()!=this->dragging) (b)->Swap( (this)->LastButton(), this->dragging);
	    if((this)->GetIM() && this->cursor) {
		((this)->GetIM())->SetWindowCursor( NULL);
		delete this->cursor;
	    }
		
    }
    return (this)->sbuttonv::Touch( ind, action);
}

sbttnav::sbttnav()
{
	ATKinit;

    (void) (this)->SetActiveMouseButtons( sbuttonv_LEFTBUTTON, sbuttonv_RIGHTBUTTON);
  
    this->ml = (sbmenulist)->DuplicateML( this);
    if(this->ml == NULL) THROWONFAILURE( FALSE);
    this->groups=NULL;
    this->groupcount=0;
    this->dragging=(-1);
    this->cursor=NULL;
    THROWONFAILURE( TRUE);
}

sbttnav::~sbttnav()
{
	ATKinit;

    struct groups *g=this->groups;
    if(this->ml!=NULL) {
	delete this->ml;
	this->ml=NULL;
    }
    while(g) {
	struct groups *h=g->next;
	sbutton::FreePrefs(g->prefs);
	delete g->ml;
	free(g);
	g=h;
    }
}


static void BDeleteProc(class sbttnav  *self, long  param)
{
    char buf[1024];
    class sbutton *b=(self)->ButtonData();
    int i=(self)->LastButton();
    char *button=(b)->GetLabel( i);

    if(button==NULL) button="Push Me";
    (b)->Delete( i);

    strcpy(buf, "Deleted button ");
    strncat(buf, button, sizeof(buf)-strlen(buf)-8);
    strcat(buf, ".");
    message::DisplayString(self, 0, buf);
}

static void LabelProc(class sbttnav  *self, long  param)
{
    /*
      This is the routine which asks the user for a new text label. */

    char buf[MAXPATHLEN];
    class sbutton *b = (self)->ButtonData();
    char *oldtext;

    oldtext = (b)->GetLabel( (self)->LastButton());
    if (message::AskForString(self,50,"Enter new label for button: ", oldtext, buf, sizeof(buf)) >= 0) {
	(b)->SetLabel( (self)->LastButton(), buf);
	message::DisplayString(self, 10, "Changed button label.");
    }
}

static void TriggerProc(class sbttnav  *self, long  param)
{
    char buf[MAXPATHLEN];
    class sbutton *b = (self)->ButtonData();
    char *oldtext;

    if((b)->GetTrigger( (self)->LastButton())) oldtext = ((b)->GetTrigger( (self)->LastButton()))->Name();
    else oldtext=NULL;
    if (message::AskForString(self,50,"Enter new trigger for button: ", oldtext, buf, sizeof(buf)) >= 0) {
	(b)->SetTrigger( (self)->LastButton(), buf);
	message::DisplayString(self, 10, "Changed button trigger.");
    }
}

static void FontProc(class sbttnav  *self, long  param)
{
/*
  This is the routine which asks the user for a new font.
  It sucks, but I don't know how to smoothly integrate this button
  with a textview-like font change.  Oh well.
*/

  char buf[MAXPATHLEN], name[MAXPATHLEN];
  long style, size;
  class sbutton *b = (self)->ButtonData();
  class fontdesc *fd;
  struct sbutton_prefs *prefs;
  prefs=(b)->GetDefaultPrefs();
  if(param>255) {
      /* we have an arg assume it is the preferences group to modify. */
      prefs=LookupGroupPrefs(self, (char *)param);
      if(prefs==NULL) {
	  message::DisplayString(self, 50, "Bad group name given.");
	  return;
      }
  }
  sprintf(name, "Enter new fontname for group %s:", (prefs->name!=NULL)?prefs->name:"Default");
  if (message::AskForString(self, 50, name, PROMPTFONT, buf, sizeof(buf)) >= 0) {
      if (!fontdesc::ExplodeFontName(buf, name, sizeof(name), &style, &size)) {
	  message::DisplayString(self, 50, "Couldn't parse fontname.");
	  return;
      }
      if ((fd = fontdesc::Create(name,style,size))!=NULL) {
	  sbutton::GetFont(prefs) = fd;
	  message::DisplayString(self, 10, "Changed font.");
      } else {
	  message::DisplayString(self, 50, "Font change failed.  Using old font.");
      }
  }
  (b)->SetModified();
  (b)->SetChangeFlag( sbutton_ALLCHANGED|sbutton_SIZECHANGED);
  (b)->NotifyObservers( observable_OBJECTCHANGED);
}


static void StyleProc(class sbttnav  *self, long  param)
{
    /*
      This is the routine which asks the user for a new sbutton appearance.
	  */

    class sbutton *b = (self)->ButtonData();
    struct sbutton_prefs *prefs = (b)->GetDefaultPrefs();
    static char *style_menu[] = {
	"Plain Text",
	"Boxed Text",
	"Three Dimensional",
	"Simple Boxed Text",
	"OSF/Motif",
	NULL
    };
    long choice;
     if(param>255) {
	/* we have an arg assume it is the preferences group to modify. */
	prefs=LookupGroupPrefs(self, (char *)param);
	if(prefs==NULL) {
	    message::DisplayString(self, 50, "Bad group name given.");
	    return;
	}
    }
    choice=sbutton::GetStyle(prefs);
    if (message::MultipleChoiceQuestion(self,99,"Pick a new style:", choice, &choice, style_menu, NULL)>= 0) {
	sbutton::GetStyle(prefs) =choice;
	message::DisplayString(self, 10, "Changed button style.");
    } else {
	message::DisplayString(self, 10, "Choice cancelled.");
    }
    (b)->SetModified();
    (b)->SetChangeFlag( sbutton_ALLCHANGED|sbutton_SIZECHANGED);
    (b)->NotifyObservers( observable_OBJECTCHANGED);
}

static void ColorProc(class sbttnav  *self, long  param)
{
    /*
      This is the routine which asks the user for  new sbutton colors.
	  */

    char buf1[MAXPATHLEN], buf2[MAXPATHLEN];
    class sbutton *b = (self)->ButtonData();
    char *oldcolor;
    struct sbutton_prefs *prefs=(b)->GetDefaultPrefs();

    if(param>255) {
	/* we have an arg assume it is the preferences group to modify. */
	prefs=LookupGroupPrefs(self, (char *)param);
	if(prefs==NULL) {
	    message::DisplayString(self, 50, "Bad group name given.");
	    return;
	}
    }
    oldcolor = sbutton::GetForeground(prefs);
    
    if (message::AskForString(self,50,"Enter new foreground color for button: ", oldcolor?oldcolor:"black", buf1, sizeof(buf1)) >= 0) {
	char buf3[1024];
	if(buf1[0]=='\0') {
	    sbutton::GetForeground(prefs)=NULL;
	    message::DisplayString(self, 10, "Restored button foreground color to the default.");
	} else {
	    sbutton::GetForeground(prefs) = Intern(buf1);
	    if(sbutton::GetForeground(prefs)!=NULL) {
		sprintf(buf3, "Changed button foreground color to %s.", buf1);
	    } else {
		sbutton::GetForeground(prefs)=oldcolor;
		sprintf(buf3, "Failed to change button foreground color.");
	    }
	}
	message::DisplayString(self, 10, buf3);
    } else return;

    oldcolor = sbutton::GetBackground(prefs);

    if (message::AskForString(self,50,"Enter new background color for button: ", oldcolor?oldcolor:"white", buf2, sizeof(buf2)) >= 0) {
	char buf3[1024];
	if(buf2[0]=='\0') {
	    sbutton::GetBackground(prefs) = NULL;
	    message::DisplayString(self, 10, "Restored button background color to the default.");
	} else {
	    sbutton::GetBackground(prefs) = Intern(buf2);
	    if(sbutton::GetBackground(prefs)!=NULL) {
		sprintf(buf3, "Changed button background color to %s.", buf2);
	    } else {
		sbutton::GetBackground(prefs)=oldcolor;
		sprintf(buf3, "Failed to change button background color.");
	    }
	    message::DisplayString(self, 10, buf3);
	}
    } else return;
    (b)->SetModified();
    (b)->SetChangeFlag( sbutton_ALLCHANGED);
    (b)->NotifyObservers( observable_OBJECTCHANGED);
}

static void GroupProc(class sbttnav  *self, long  rock)
{
    class sbutton *b=(self)->ButtonData();
    struct sbutton_prefs *prefs=(b)->GetDefaultPrefs();
    if(rock>255) {
	/* we have an arg assume it is the preferences group to modify. */
	prefs=LookupGroupPrefs(self, (char *)rock);
	if(prefs==NULL) {
	    message::DisplayString(self, 50, "Bad group name given.");
	    return;
	}
    }
    (b)->SetPrefs( (self)->LastButton(), prefs);
    message::DisplayString(self, 0, "Set button group.");
}

static void AddButtonProc(class sbttnav  *self, long  rock)
{
    char buf[1024];
    class sbutton *b=(self)->ButtonData();
    if (message::AskForString(self,50,"Enter label for new button: ", NULL, buf, sizeof(buf)) >= 0) {
	(b)->SetLabel( b->count, buf);
    }
    message::DisplayString(self, 0, "Added new button.");
}

static void SetRowsProc(class sbttnav  *self, long  rock)
{
    char buf[1024];
    class sbutton *b=(self)->ButtonData();
    if (message::AskForString(self,50,"Rows: ", NULL, buf, sizeof(buf)) >= 0) {
	int i=atoi(buf);
	if(i<=0) {
	    message::DisplayString(self, 50, "The number of rows should be a number greater than zero.");
	    return;
	}
	(b)->SetLayout( i, (b)->GetCols(), sbutton_GrowColumns);
    }
}

static void SetColsProc(class sbttnav  *self, long  rock)
{
    char buf[1024];
    class sbutton *b=(self)->ButtonData();
    if (message::AskForString(self,50,"Columns: ", NULL, buf, sizeof(buf)) >= 0) {
	int i=atoi(buf);
	if(i<=0) {
	    message::DisplayString(self, 50, "The number of columns should be a number greater than zero.");
	    return;
	}
	(b)->SetLayout( (b)->GetRows(), i, sbutton_GrowRows);
    }
}

static void NewGroupProc(class sbttnav  *self, long  rock)
{
    char buf[1024];
    class sbutton *b=(self)->ButtonData();
    if (message::AskForString(self,50,"Name for new button group: ", NULL, buf, sizeof(buf)) >= 0) {
	struct sbutton_prefs *new_c;
	new_c=sbutton::DuplicatePrefs((b)->GetDefaultPrefs(), buf);
	new_c->refcount=1;
	if(new_c->name!=NULL) {
	    AddGroupMenu(self, new_c, -1);
	}
    }
    (self)->PostMenus( NULL);
}

static void RenameProc(class sbttnav  *self, long  rock)
{
    char buf[1024];
    class sbutton *b=(self)->ButtonData();
    struct sbutton_prefs *prefs=(b)->GetDefaultPrefs();
    struct groups **g, *h=NULL;
    int prio;
    if(rock<=255) {
	message::DisplayString(self, 50, "Bad group name given.");
	return;
    }
    /* we have an arg assume it is the preferences group to modify. */
    g=LookupGroup(self, (char *)rock);
    if(g==NULL) {
	message::DisplayString(self, 50, "Bad group name given.");
	return;
    }
    if (message::AskForString(self,50,"New name for button group: ", NULL, buf, sizeof(buf)) >= 0) {
	if(LookupGroup(self, buf)!=NULL) {
	    message::DisplayString(self, 75, "That name is already in use.");
	    return;
	}
	/* get out the important data then destroy the old structures... */
	prio=(*g)->prio;
	prefs=(*g)->prefs;
	((*g)->ml)->ClearChain();
	delete (*g)->ml;
	h=(*g)->next;
	free(*g);
	*g=h;
	if(prefs->name!=NULL) free(prefs->name);
	prefs->name=NewString(buf);
	if(prefs->name!=NULL) AddGroupMenu(self, prefs, prio);
    }
    (self)->PostMenus( NULL);
}

struct deleterock {
    struct sbutton_prefs *def, *prefs;
};

static boolean dodeletion(class sbutton  *b, int  i, struct sbutton_info  *si, struct deleterock  *rock)
{
    if(si->prefs==rock->prefs) {
	rock->prefs->refcount--;
	si->prefs=rock->def;
	rock->def->refcount++;
    }
    return FALSE;
}

static void GDeleteProc(class sbttnav  *self, long  rock)
{
    class sbutton *b=(self)->ButtonData();
    struct sbutton_prefs *prefs=(b)->GetDefaultPrefs();
    struct groups **g, *h=NULL;
    struct deleterock deleterock;
    
    if(rock<=255) {
	message::DisplayString(self, 50, "Bad group name given.");
	return;
    }
    /* we have an arg assume it is the preferences group to delete. */
    g=LookupGroup(self, (char *)rock);
    if(g==NULL) {
	message::DisplayString(self, 50, "Bad group name given.");
	return;
    }
    h=(*g);
    if(h==NULL || h->prefs==prefs) return;
    deleterock.prefs=h->prefs;
    deleterock.def=prefs;
    (b)->Enumerate( (sbutton_efptr)dodeletion, (long)&deleterock);
    *g=h->next;
    sbutton::FreePrefs(h->prefs);
    (h->ml)->ClearChain();
    delete h->ml;
    free(h);
    (b)->SetModified();
    (b)->SetChangeFlag( sbutton_SIZECHANGED|sbutton_ALLCHANGED);
    (b)->NotifyObservers( observable_OBJECTCHANGED);
    (self)->PostMenus( NULL);
}

static void ShadowColorProc(class sbttnav  *self, long  param)
{
    /*
      This is the routine which asks the user for  new sbutton colors.
	  */

    char buf1[MAXPATHLEN], buf2[MAXPATHLEN];
    class sbutton *b = (self)->ButtonData();
    char *oldcolor;
    struct sbutton_prefs *prefs=(b)->GetDefaultPrefs();

    if(param>255) {
	/* we have an arg assume it is the preferences group to modify. */
	prefs=LookupGroupPrefs(self, (char *)param);
	if(prefs==NULL) {
	    message::DisplayString(self, 50, "Bad group name given.");
	    return;
	}
    }
    oldcolor = sbutton::GetTopShadow(prefs);    
    
    if (message::AskForString(self,50,"Enter new top shadow color for button: ", NULL, buf1, sizeof(buf1)) >= 0) {
	char buf3[1024];
	if(buf1[0]=='\0') {
	    sbutton::GetTopShadow(prefs)=NULL;
	    message::DisplayString(self, 10, "Restored button top shadow color to the default.");
	}
	else {
	    sbutton::GetTopShadow(prefs) = Intern(buf1);
	    (b)->SetModified();
	    (b)->SetChangeFlag( sbutton_ALLCHANGED);
	      (b)->NotifyObservers( observable_OBJECTCHANGED);
	    if(sbutton::GetTopShadow(prefs)!=NULL) {
		sprintf(buf3, "Changed button top shadow color to %s.", buf1);
	    } else {
		sbutton::GetTopShadow(prefs)=oldcolor;
		sprintf(buf3, "Failed to change button top shadow color.");
	    }
	    message::DisplayString(self, 10, buf3);
	}
    } else return;

    oldcolor = sbutton::GetBottomShadow(prefs);

    if (message::AskForString(self,50,"Enter new bottom shadow color for button: ", NULL, buf2, sizeof(buf2)) >= 0) {
	char buf3[1024];
	if(buf2[0]=='\0') {
	    sbutton::GetBottomShadow(prefs) = NULL;
	    message::DisplayString(self, 10, "Restored button bottom shadow color to the default.");
	}
	else {
	    sbutton::GetBottomShadow(prefs) = Intern(buf2);
	    (b)->SetModified();
	    (b)->SetChangeFlag( sbutton_ALLCHANGED);
	      (b)->NotifyObservers( observable_OBJECTCHANGED);
	    if(sbutton::GetBottomShadow(prefs)!=NULL) {
		sprintf(buf3, "Changed button bottom shadow color to %s.", buf2);
	    } else {
		sbutton::GetBottomShadow(prefs)=oldcolor;
		sprintf(buf3, "Failed to change button bottom shadow color.");
	    }
	    message::DisplayString(self, 10, buf3);
	}
    } else return;
    
    oldcolor = sbutton::GetTop(prefs);

    if (message::AskForString(self,50,"Enter new top color for button: ", oldcolor?oldcolor:"White", buf2, sizeof(buf2)) >= 0) {
	char buf3[1024];
	if(buf2[0]=='\0') {
	    sbutton::GetTop(prefs)=NULL;
	    message::DisplayString(self, 10, "Restored button top color to the default.");
	}
	else {
	    sbutton::GetTop(prefs)=Intern(buf2);
	    (b)->SetModified();
	    (b)->SetChangeFlag( sbutton_ALLCHANGED);
	      (b)->NotifyObservers( observable_OBJECTCHANGED);
	    if(sbutton::GetTop(prefs)!=NULL) {
		sprintf(buf3, "Changed button top color to %s.", buf2);
	    } else {
		sbutton::GetTop(prefs)=oldcolor;
		sprintf(buf3, "Failed to change button top color.");
	    }
	    message::DisplayString(self, 10, buf3);
	}
    } else return;
    
}


static void DepthProc(class sbttnav  *self, long  param)
{
    class sbutton *b = (self)->ButtonData();
    struct sbutton_prefs *prefs=(b)->GetDefaultPrefs();

    if(param>255) {
	prefs=LookupGroupPrefs(self, (char *)param);
	if(prefs==NULL) {
	    message::DisplayString(self, 50, "Bad group name given.");
	    return;
	}
    }
    char buf1[MAXPATHLEN];
    if (message::AskForString(self,50,"Enter new depth for buttons: ", NULL, buf1, sizeof(buf1)) >= 0) {
	if(prefs) prefs->bdepth=atoi(buf1);
	(b)->SetModified();
	(b)->SetChangeFlag( sbutton_ALLCHANGED|sbutton_SIZECHANGED);
	(b)->NotifyObservers( observable_OBJECTCHANGED);
    }
}

static void LabelColorProc(class sbttnav  *self, long  param)
{
    /*
      This is the routine which asks the user for  new sbutton colors.
	  */

    char buf1[MAXPATHLEN], buf2[MAXPATHLEN];
    class sbutton *b = (self)->ButtonData();
    char *oldcolor;
    struct sbutton_prefs *prefs=(b)->GetDefaultPrefs();

    if(param>255) {
	/* we have an arg assume it is the preferences group to modify. */
	prefs=LookupGroupPrefs(self, (char *)param);
	if(prefs==NULL) {
	    message::DisplayString(self, 50, "Bad group name given.");
	    return;
	}
    }
    oldcolor = sbutton::GetLabelFG(prefs);
    
    if (message::AskForString(self,50,"Enter new foreground color for button label: ", oldcolor?oldcolor:"black", buf1, sizeof(buf1)) >= 0) {
	char buf3[1024];
	if(buf1[0]=='\0') {
	    sbutton::GetLabelFG(prefs)=NULL;
	    (b)->SetModified();
	    (b)->SetChangeFlag( sbutton_ALLCHANGED);
	      (b)->NotifyObservers( observable_OBJECTCHANGED);
	    message::DisplayString(self, 10, "Restored button label foreground color to the default.");
	} else {
	    sbutton::GetLabelFG(prefs)=Intern(buf1);
	    (b)->SetModified();
	    (b)->SetChangeFlag( sbutton_ALLCHANGED);
	      (b)->NotifyObservers( observable_OBJECTCHANGED);
	    if(sbutton::GetLabelFG(prefs)!=NULL) {
		sprintf(buf3, "Changed button label foreground color to %s.", buf1);
	    } else {
		sbutton::GetLabelFG(prefs)=oldcolor;
		sprintf(buf3, "Failed to change button label foreground color.");
	    }
	    message::DisplayString(self, 10, buf3);
	}
    } else return;

    oldcolor = sbutton::GetLabelBG(prefs);

    if (message::AskForString(self,50,"Enter new background color for button label: ", oldcolor?oldcolor:"white", buf2, sizeof(buf2)) >= 0) {
	char buf3[1024];
	if(buf2[0]=='\0') {
	    sbutton::GetLabelBG(prefs)=NULL;
	    (b)->SetModified();
	    (b)->SetChangeFlag( sbutton_ALLCHANGED);
	      (b)->NotifyObservers( observable_OBJECTCHANGED);
	    message::DisplayString(self, 10, "Restored button label background color to the default.");
	} else {
	    sbutton::GetLabelBG(prefs)=Intern(buf2);
	    (b)->SetModified();
	    (b)->SetChangeFlag( sbutton_ALLCHANGED);
	      (b)->NotifyObservers( observable_OBJECTCHANGED);
	    if(sbutton::GetLabelBG(prefs)!=NULL) {
		sprintf(buf3, "Changed button label background color to %s.", buf2);
	    } else {
		sbutton::GetLabelBG(prefs)=oldcolor;
		sprintf(buf3, "Failed to change button label background color.");
	    }
	    message::DisplayString(self, 10, buf3);
	}
    } else return;
}
