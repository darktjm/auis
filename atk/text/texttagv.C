/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("texttagv.H")
#include <texttag.H>
#include <texttagv.H>
#include <environ.H>
#include <fontdesc.H>
#define DataObject(A) (A->dataobject)
#define Data(A) ((class texttag *)(DataObject(A)))



ATKdefineRegistry(texttagv, fnotev, texttagv::InitializeClass);

void texttagv::Print(FILE  *f, const char  *process, const char  *final, int  toplevel)
{
    class texttag *tag;
    char buf[256];
    if(environ::Get("IndexOnly") != NULL) return;
    tag = Data(this);
    (tag)->GetTag(255,buf);
    fprintf(f,".iy \"TEXTTAG %s\"\n",buf);
}
class view *texttagv::Hit(enum view::MouseAction  action,long  mousex ,long  mousey ,long  numberOfClicks) 
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
