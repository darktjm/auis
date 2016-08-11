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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/texttagv.C,v 3.2 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


#include <andrewos.h>
ATK_IMPL("texttagv.H")
#include <texttag.H>
#include <texttagv.H>
#include <environ.H>
#include <fontdesc.H>
#define DataObject(A) (A->dataobject)
#define Data(A) ((class texttag *)(DataObject(A)))



ATKdefineRegistry(texttagv, fnotev, texttagv::InitializeClass);
#ifndef NORCSID
#endif


void texttagv::Print(FILE  *f, char  *process, char  *final, int  toplevel)
{
    class texttag *tag;
    char buf[256];
    if(environ::Get("IndexOnly") != NULL) return;
    tag = Data(this);
    (tag)->GetTag(255,buf);
    fprintf(f,".iy \"TEXTTAG %s\"\n",buf);
}
class view *texttagv::Hit(enum view_MouseAction  action,long  mousex ,long  mousey ,long  numberOfClicks) 
{
    return (this)->fnotev::Hit(action,mousex,mousey,numberOfClicks) ;
}
#define FONTNAME "andy"
#define FONTSIZE 12
#define OFNAME "andy"
#define OFSIZE 8
texttagv::texttagv()
{
	ATKinit;

    class fnotev *fv = (class fnotev *) this;
    (this)->SetDisplayStr("@");
    fv->fd = fontdesc::Create(FONTNAME,0,FONTSIZE);
    fv->ofd = fontdesc::Create(OFNAME,0,OFSIZE);
    THROWONFAILURE( TRUE);
}
boolean texttagv::InitializeClass()
{
    return TRUE;
}
