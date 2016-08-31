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

#include <andrewos.h>
ATK_IMPL("dialog.H")
#include <stdio.h>
#include <environ.H>
#include <fontdesc.H>
#include <observable.H>
#include <sbutton.H>
#include <text.H>
#include <util.h>
#include <graphic.H>

#include <dialog.H>


ATKdefineRegistry(dialog, dataobject, dialog::InitializeClass);
static long SanelyReturnReadError(class dialog  *self, FILE  *fp, long  id, long  code);


boolean dialog::InitializeClass()
{
    return TRUE;
}


dialog::dialog()
{
	ATKinit;

    struct sbutton_prefs *prefs;
    
    this->prefs=sbutton::GetNewPrefs("dialog");

    prefs=sbutton::GetNewPrefs(NULL);
    
    if(this->prefs == NULL || prefs == NULL) {
	if(this->prefs!=NULL) sbutton::FreePrefs(this->prefs);
	if(prefs!=NULL) sbutton::FreePrefs(prefs);
	THROWONFAILURE( FALSE);
    }
    
    sbutton::InitPrefs(this->prefs, "dialog");

    *prefs=(*this->prefs);

    prefs->refcount=0;

    sbutton::InitPrefs(prefs, "dialogbutton");    
    
    this->textp=new text;
    if(this->textp==NULL) THROWONFAILURE( FALSE);

    this->buttons=sbutton::CreateSButton(prefs);
    if(this->buttons==NULL) {
	sbutton::FreePrefs(this->prefs);
	(this->textp)->Destroy();
	THROWONFAILURE( FALSE);
    }        
    THROWONFAILURE( TRUE);
}

dialog::~dialog()
{
	ATKinit;

    if(this->prefs) {
	sbutton::FreePrefs(this->prefs);
	this->prefs=NULL;
    }
    
    if(this->textp) {
	(this->textp)->Destroy();
	this->textp=NULL;
    }
    if(this->buttons) {
	(this->buttons)->Destroy();
	this->buttons=NULL;
    }
}


const char *dialog::ViewName()
{
    return "dialogv";
}

long dialog::Write(FILE  *fp, long  id, int  level)
{
    long uniqueid = (this)->UniqueID();

    if (id != (this)->GetWriteID()) {
	/* New Write Operation */
	(this)->SetWriteID( id);
	
	fprintf(fp, "\\begindata{%s,%ld}\nDatastream version: %d\n",
		(this)->GetTypeName(), 
		uniqueid, dialog_DS_VERSION);

	if(!this->textp) {
	    class text *t=new text;
	    (t)->Write( fp, id, level+1);
	    (t)->Destroy();
	} else {
	    (this->textp)->Write( fp, id, level+1);
	}

	if(!this->buttons) {
	    class sbutton *s=new sbutton;
	    (s)->Write( fp, id, level+1);
	    (s)->Destroy();
	} else {
	    (this->buttons)->Write( fp, id, level+1);
	}
	fprintf(fp, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), uniqueid);
    }
    return(uniqueid);
}

static long SanelyReturnReadError(class dialog  *self, FILE  *fp, long  id, long  code)
{
    /*
      Suck up the file until our enddata, then return the error code.
      */
    char buf[1024], buf2[255];

    sprintf(buf2, "\\enddata{%s,%ld}\n", (self)->GetTypeName(), id);
    do {
	if ((fgets(buf, sizeof(buf)-1, fp)) == NULL)
	    return(dataobject_PREMATUREEOF);
    } while (strncmp(buf, "\\enddata{", 9) != 0); /* find an enddata */

    if (strcmp(buf, buf2) != 0) {
	return(dataobject_MISSINGENDDATAMARKER); /* not ours! */
    }

    return(code);
}

static const char DatastreamHeader[]="Datastream version:";

long dialog::Read(FILE  *fp, long  id)
{
    char buf[1024], *p;
    long err;
    long textid;
    
    (this)->SetID( (this)->UniqueID());

    p=fgets(buf, sizeof(buf)-1, fp);
    if(!p) return dataobject_PREMATUREEOF;
    if (strncmp(buf, DatastreamHeader, sizeof(DatastreamHeader) - 1))
	return(SanelyReturnReadError(this, fp, id, dataobject_BADFORMAT));
    
    if ((atoi(buf+sizeof(DatastreamHeader)-1)) > dialog_DS_VERSION) return(SanelyReturnReadError(this, fp, id, dataobject_BADFORMAT));

    p=fgets(buf, sizeof(buf)-1, fp);
    if(!p) return SanelyReturnReadError(this, fp, id, dataobject_PREMATUREEOF);
    
    if(sscanf(buf, "\\begindata{text,%ld}", &textid )!=1)  SanelyReturnReadError(this, fp, id, dataobject_BADFORMAT);
    
    if(this->textp) (this->textp)->Clear();
    else this->textp=new text;

    if(!this->textp) return  SanelyReturnReadError(this, fp, id, dataobject_PREMATUREEOF);
    err=(this->textp)->Read( fp, textid);

    if(err!=dataobject_NOREADERROR) return SanelyReturnReadError(this, fp, id, err);
    
    if(!this->buttons) return SanelyReturnReadError(this, fp, id, dataobject_PREMATUREEOF);
    
    p=fgets(buf, sizeof(buf)-1, fp);
    if(!p) return SanelyReturnReadError(this, fp, id, dataobject_PREMATUREEOF);
    
    if(sscanf(buf, "\\begindata{sbutton,%ld}", &textid )!=1)  return SanelyReturnReadError(this, fp, id, dataobject_BADFORMAT);

    err=(this->buttons)->Read( fp, textid);
    
    if(err!=dataobject_NOREADERROR) return SanelyReturnReadError(this, fp, id, err);

    return SanelyReturnReadError(this, fp, id, dataobject_NOREADERROR); 
}

void dialog::SetText(class text  *textp)
{
    if(this->textp) (this->textp)->Destroy();
    this->textp=textp;
}

void dialog::SetButtons(class sbutton  *buttons)
{
    if(this->buttons) (this->buttons)->Destroy();
    this->buttons=buttons;
}

const char *dialog::GetForeground()
{
    const char *fg, *bg;
    graphic::GetDefaultColors(&fg, &bg);
    if(this->prefs->colors[sbutton_FOREGROUND]) return this->prefs->colors[sbutton_FOREGROUND];
    else return fg?fg:"black";
}

const char *dialog::GetBackground()
{
    const char *fg, *bg;
    graphic::GetDefaultColors(&fg, &bg);
    if(this->prefs->colors[sbutton_BACKGROUND]) return this->prefs->colors[sbutton_BACKGROUND];
    else return bg?bg:"white";
}
