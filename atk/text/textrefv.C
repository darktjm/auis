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

#include <andrewos.h>
ATK_IMPL("textrefv.H")
#include <textref.H>
#include <textrefv.H>
#include <textview.H>
#include <text.H>
#include <viewref.H>
#include <environ.H>
#include <environment.H>
#include <texttag.H>
#include <fontdesc.H>
#define DataObject(A) (A->dataobject)
#define Data(A) ((class textref *)(DataObject(A)))

/*
#define PROCESSOR "/afs/andrew.cmu.edu/usr20/tpn/itc/amos/text/ezpostprocess"
#define TFILE "/afs/andrew.cmu.edu/usr20/tpn/itc/amos/text/tmac.arf"
*/

#define PROCESSOR environ::AndrewDir("/bin/ezpostprocess")
#define TFILE environ::AndrewDir("/lib/tmac/tmac.arf")


ATKdefineRegistry(textrefv, fnotev, textrefv::InitializeClass);
static boolean findtag(class textrefv  *self,class text  *text,long  pos,class environment  *env);


static boolean findtag(class textrefv  *self,class text  *text,long  pos,class environment  *env)
{
    char *foo,buf[256];
    const char *name;
    class viewref *vr;
    if(self->cref != NULL && *self->cref != '\0' && env->type == environment_View){
	vr = env->data.viewref;
	name = (vr->dataObject)->GetTypeName();
	if(!ATK::IsTypeByName(name,"texttag"))
		return FALSE;
	foo = ((class texttag *)vr->dataObject)->GetTag(255,buf);
	if(strcmp(foo,self->cref) == 0) {
	    self->loc = pos;
	    return TRUE;
	}
    }
    return FALSE;
}
void textrefv::Print(FILE  *f, const char  *process, const char  *final, int  toplevel)
{
    class textref *ref;
    char buf[256];
    ref = Data(this);
    (ref)->GetRef(255,buf);
    fprintf(f,"XXX  \\\"TEXTREF %s\n",buf);
    if(environ::Get("troffpostprocessor") == NULL){
	sprintf(buf,"%s ",PROCESSOR);
	strcat(buf,TFILE);
	environ::Put("troffpostprocessor",buf);
    }
}

class view *textrefv::Hit(enum view_MouseAction  action,long  mousex ,long  mousey ,long  numberOfClicks) 
{
    class textref *ref;
    char buf[256];
    class text *txt;
    class textview *tv;
 /*   long cpos,clen; */
    ref = Data(this);
    this->loc = -1;
    if( action == view_LeftUp && 
	((ref)->IsOpen() == FALSE) &&
	((txt = (this)->GetParentText()) != NULL) &&
	((tv = (this)->GetParentTextview()) != NULL) &&
	((this->cref = (ref)->GetRef(255,buf)) != NULL) && 
       ((txt)->EnumerateEnvironments(0,(txt)->GetLength(),(text_eefptr)findtag,(long) this) != NULL) &&
	this->loc >= 0){
/*
	cpos = textview_GetDotPosition(tv);
	clen = textview_GetDotLength(tv);
*/
	(tv)->SetDotPosition(this->loc);   
	(tv)->SetDotLength(1);   
/*
	textview_FrameDot(tv);
	textview_SetDotPos(tv,cloc);   
	textview_SetDotLength(tv,clen);   
*/
	(tv)->FrameDot(this->loc);
	(tv)->WantUpdate(tv);
	this->cref = NULL;
	return((class view *)this);
    }
    this->cref = NULL;
    return (this)->fnotev::Hit(action,mousex,mousey,numberOfClicks) ;
}
#define FONTNAME "andy"
#define FONTSIZE 12
#define OFNAME "andy"
#define OFSIZE 8
textrefv::textrefv()
{
	ATKinit;

    class fnotev *fv = (class fnotev *) this;
    (this)->SetDisplayStr("?");
    fv->fd = fontdesc::Create(FONTNAME,0,FONTSIZE);
    fv->ofd = fontdesc::Create(OFNAME,0,OFSIZE);
    THROWONFAILURE( TRUE);
}
boolean textrefv::InitializeClass()
{   
    return TRUE;
}
