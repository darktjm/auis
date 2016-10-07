/* fontsamp.c - font sample view */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
#include <andrewos.h>
ATK_IMPL("fontsample.H")
#include <fontsample.H>

#include <fontdesc.H>

#include <fontsel.H>

#define INITTESTSTRING "Sample"


ATKdefineRegistry(fontsample, view, fontsample::InitializeClass);
static void RedrawView(class fontsample  *self);


boolean fontsample::InitializeClass()
{
    return TRUE;
}

fontsample::fontsample()
{
	ATKinit;

    this->teststring = strdup(INITTESTSTRING);
    this->dirty = TRUE;
    this->fdesc = NULL;

    THROWONFAILURE( TRUE);
}

fontsample::~fontsample()
{
	ATKinit;

    free(this->teststring);
}

static void RedrawView(class fontsample  *self)
{
    long x, y, w, h;
    class fontdesc *fdesc;

    x = (self)->GetLogicalLeft();
    y = (self)->GetLogicalTop();
    w = (self)->GetLogicalWidth();
    h = (self)->GetLogicalHeight();
    fdesc = (self)->GetFontDesc();
    if (!fdesc) 
	return;

    (self)->SetFont( fdesc); 
    (self)->MoveTo( x+w/2, y+h/2);
    (self)->DrawString( self->teststring, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM);
}

void fontsample::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    RedrawView(this);
}

void fontsample::Update()
{
    (this)->EraseVisualRect(); 
    RedrawView(this);
}

void fontsample::ObservedChanged(class observable  *dobj, long  status)
{
    if (status == observable::OBJECTDESTROYED) {
    }
    else if (status == fontsel_DATACHANGED) {
	this->dirty = TRUE;
	(this)->WantUpdate( this);
    }
}

class view *fontsample::Hit(enum view_MouseAction  action, long  x , long  y , long  clicks)
{
    return NULL;
}

void fontsample::SetString(char  *val)
{
    free(this->teststring);
    this->teststring = strdup(val);
}

class fontdesc *fontsample::GetFontDesc()
{
    class fontsel *dobj = (class fontsel *)(this)->GetDataObject();

    if (this->dirty || !this->fdesc) {
	/* You'd think you'd want to clean up around here, but no, fontdesc_Destroy() segfaults. Sometimes. I will assume this is because of font caching, and ignore it. */
	/*if (self->fdesc)
	    fontdesc_Destroy(self->fdesc);*/

	this->fdesc = fontdesc::Create((dobj)->GetFamily(), (dobj)->GetStyle(), (dobj)->GetSize());
	if (!this->fdesc) return NULL;

	this->dirty = FALSE;
    }
    return this->fdesc;
}

