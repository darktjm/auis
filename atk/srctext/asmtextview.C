/* File asmtextview.C created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   asmtextview, a view for editing Assembly Language code. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <im.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <proctable.H>
#include <bind.H>
#include <message.H>

#include "asmtext.H"
#include "asmtextview.H"

static class keymap *asm_Map;
static class menulist *asm_Menus;

static struct bind_Description asmtextBindings[]={
    {"srctextview-start-comment", "*",'*'},
    {"srctextview-end-comment", "/",'/'},
    NULL
};

ATKdefineRegistry(asmtextview, srctextview, asmtextview::InitializeClass);

boolean asmtextview::InitializeClass()
{
    asm_Menus = new menulist;
    asm_Map = new keymap;
    bind::BindList(asmtextBindings,asm_Map,asm_Menus,ATK::LoadClass("srctextview"));
    return TRUE;
}

asmtextview::asmtextview()
{
	ATKinit;
    this->asm_state = keystate::Create(this, asm_Map);
    this->asm_menus = (asm_Menus)->DuplicateML(this);
    SetBorder(5,5);
    THROWONFAILURE(TRUE);
}

asmtextview::~asmtextview()
{
	ATKinit;
    delete this->asm_state;
    delete this->asm_menus;
}

void asmtextview::PostMenus(menulist *menulist)
{
    (this->asm_menus)->ChainBeforeML(menulist, (long)this);
    (this)->srctextview::PostMenus(this->asm_menus);
}

class keystate *asmtextview::PrependKeyState()
{
    this->asm_state->next= NULL;
    return (this->asm_state)->AddBefore((this)->srctextview::PrependKeyState());
}

/* override */
/* hook all those bang-comment characters that were captured from .ezinit to proctable entries */
/* NOTE! asm_Map, the keymap, is shared by ALL asmtextview objects.  Since THIS asmtext object might not want bang-comment characters that some OTHER asmtext put in the keymap, we must double-check this in bangcomment() */
void asmtextview::SetDataObject(class dataobject *dataobj)
{
    char *bang= ((asmtext *)dataobj)->bangComments;
    (this)->srctextview::SetDataObject(dataobj);

    if (*bang!='\0') {
	/* add bang-comment keys to keymap */
	char ch[2]; ch[1]='\0';
	while (*bang!='\0') {
	    ch[0]= *bang;
	    (asm_Map)->BindToKey(ch, proctable::Lookup("srctextview-start-linecomment"), *ch);
            ++bang;
	}
/*asmtextview_PostKeyState(self,kstate);*/
    }
}

/* override */
/* only check if c-comments are on */
void asmtextview::StartComment(char key /* must be char for "&" to work. */)
{
    asmtext *ct=(asmtext *)this->view::dataobject;
    if ((ct)->UseCComments())
	(this)->srctextview::StartComment(key);
    else
	SelfInsert(key);
}

/* override */
/* only check if c-comments are on */
void asmtextview::EndComment(char key /* must be char for "&" to work. */)
{
    asmtext *ct=(asmtext *)this->view::dataobject;
    if ((ct)->UseCComments())
	(this)->srctextview::EndComment(key);
    else
	SelfInsert(key);
}

/* override */
void asmtextview::StartLineComment(char key /* must be char for "&" to work. */)
{
    asmtext *ct=(asmtext *)this->view::dataobject;
    /* hold it! make sure THIS asmtext object is the one that mapped this key to be a bang-comment! */
    if (strchr(ct->bangComments,key))
	(this)->srctextview::StartLineComment(key);
    else
	SelfInsert(key);
}

/* override */
void asmtextview::Reindent()
{
    asmtext *ct= (asmtext *)this->view::dataobject;
    long pos= GetDotPosition(), len= GetDotLength();

    if (len>0 && (ct)->IndentingEnabled()) {
	/* a region is selected; grab whole lines and pass to external filter */
	long end= pos+len;
	if ((ct)->GetReadOnly()) {
	    message::DisplayString(this, 0, "Document is read only.  Cannot reindent.");
	    return;
	}
	pos= (ct)->GetBeginningOfLine(pos);
	end= (ct)->GetEndOfLine(end)+1;
	SetDotPosition(pos);
	SetDotLength(end-pos);
	if ((ct)->HasReindentFilter()) {
	    WaitCursorOn();
	    if (proctable::Lookup("filter-filter-region-thru-command")==NULL) {
		/* apparently filter gets loaded every time ez is invoked. */
		/* therefore, this never gets called (and never got tested) */
		/* (I'm not even sure class_Load *does* what needs doing in this case */
		ATK::LoadClass("filter");
	    }
	    (GetIM())->HandleMenu(proctable::Lookup("filter-filter-region-thru-command"), this, (long)(ct)->ReindentFilterName());
	    WaitCursorOff();
	} else
	    message::DisplayString(this,0,"No reindentation filter name was specified in .ezinit");
    } else
	/* no region selected; move cursor to next tab stop */
	(this)->srctextview::Reindent();
    (ct)->NotifyObservers(0);
}
