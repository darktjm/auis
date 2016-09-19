/* fontsel.c - font selection inset dataobject */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
#include <andrewos.h>
ATK_IMPL("fontsel.H")
#include <fontsel.H>

#include <fontdesc.H>




ATKdefineRegistry(fontsel, dataobject, fontsel::InitializeClass);
static char *CopyString(const char  *str);


boolean fontsel::InitializeClass()
{
    return TRUE;
}

fontsel::fontsel()
{
	ATKinit;

    this->active = ~(unsigned long)0;

    this->family = CopyString(fontsel_default_Family);
    this->size = fontsel_default_Size;
    this->style = fontsel_default_Style;

    THROWONFAILURE( TRUE);
}

fontsel::~fontsel()
{
	ATKinit;

    free(this->family);
}

void fontsel::SetStyle(long  mask)
{
    this->style = mask;
    this->active |= ((unsigned long)1<<fontsel_Style);
}

void fontsel::SetSize(short  newsize)
{
    this->size = newsize;
    this->active |= ((unsigned long)1<<fontsel_Size);
}

void fontsel::SetFamily(const char  *newfam)
{
    if (strcmp(newfam, this->family)) {
	free(this->family);
	this->family = CopyString(newfam);
    }
    this->active |= ((unsigned long)1<<fontsel_Family);
}

static char *CopyString(const char  *str)
{
    if (str==NULL)
	return NULL;
    return strdup(str);
}

