/* fontsel.c - font selection inset dataobject */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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
char *fontsel_c_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/fontsel.C,v 3.2 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

#include <andrewos.h>
ATK_IMPL("fontsel.H")
#include <fontsel.H>

#include <fontdesc.H>




ATKdefineRegistry(fontsel, dataobject, fontsel::InitializeClass);
#ifndef NORCSID
#endif
static char *CopyString(char  *str);


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

void fontsel::SetFamily(char  *newfam)
{
    if (strcmp(newfam, this->family)) {
	free(this->family);
	this->family = CopyString(newfam);
    }
    this->active |= ((unsigned long)1<<fontsel_Family);
}

static char *CopyString(char  *str)
{
    char *tmp;

    if (str==NULL)
	return NULL;
    tmp = (char *)malloc(strlen(str)+1);
    if (!tmp)
	return NULL;
    strcpy(tmp, str);
    return tmp;
}

