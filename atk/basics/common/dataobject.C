/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
ATK_IMPL("dataobject.H")
#include <dataobject.H>
#include <attribs.h>
#include <view.H>
/* #include "dict.ih" */


ATKdefineRegistry(dataobject, observable, NULL);

dataobject::dataobject()
        {
    this->id = (this)->UniqueID();
    this->writeID = dataobject_UNDEFINEDID;
    this->modified = 0;
    this->properties = NULL;
    THROWONFAILURE( TRUE);
}

static boolean FreeProps(long rock, class Namespace *self, int x)
{
    char *val=(char *)self->ValueAt(x);
    free(val);
    return TRUE;    
}

dataobject::~dataobject()
{
    if(this->properties) {
	this->properties->Enumerate(FreeProps, 0);
	delete this->properties;
	this->properties=NULL;
    }
}

long dataobject::Read(FILE  *file, long  id)
            {

/*  The following may be used as a template for creating real read routines 
    for objects that may contain other objects */

    long endcount = 1;
    boolean begindata;
    char *s;
    const char *be;
    long c;
    long status;
    char objectname[200];
    long objectid;
    class dataobject *newobject;

    (this)->SetID((this)->UniqueID());/* change id to unique number */
    while (endcount != 0)  {
        while ((c = getc(file)) != EOF && c != '\\')  {
	    if(endcount == 1){
		/* Place actual read code here */
	    }
        }
        if (c == EOF) return dataobject_NOREADERROR;
        if ((c = getc(file)) == EOF)
            return dataobject_PREMATUREEOF;
        if (c == 'b')  {
            begindata = TRUE;
            be = "egindata";
        }
        else if (c == 'e')  {
            begindata = FALSE;
            be = "nddata";
        }
        else  {
	    if(endcount == 1){
		/* Place handling of \x characters here */
	    }
            continue;
        }
        while ((c = getc(file)) != EOF && c == *be) be++;
        if (c == '{' && *be == '\0')  {
            if (begindata) {
                s = objectname;
                while ((c = getc(file)) != EOF && c != ',')
                    *s++ = c;
                if (c == EOF) return dataobject_PREMATUREEOF;
                *s = '\0';
                objectid = 0;
                while ((c = getc(file)) != EOF && c != '}')
                    if(c >= '0' && c <= '9')objectid = objectid * 10 + c - '0';
                if (c == EOF) return dataobject_PREMATUREEOF;
		if((c = getc(file))!= '\n') ungetc(c,file);
                /* Call the New routine for the object */
                if ((newobject = (class dataobject *) ATK::NewObject(objectname)))  {
                    /* Register the object with the dictionary */
		    /* This call should be made, It is only commented out here
		     to avoid making the basics directory dependent on the support directory */
/*		    dictionary_Insert(NULL,(char *)objectid, (char *)newobject); */
                    /* Call the read routine for the object */
                    status = (newobject)->Read( file, objectid);
		    if (status != dataobject_NOREADERROR) return status;
		}
                else {
                    endcount += 1;
		    /* return dataobject_OBJECTCREATIONFAILED; */
		}

	    }
            else  {
                endcount -= 1;
                while ((c = getc(file)) != EOF && c != '}');
		if((c = getc(file))!= '\n') ungetc(c,file);
            }
        }
        else if(endcount == 1){
	    
        /* 	    Place Handling of characters following \  
           */	}
    }
    return dataobject_NOREADERROR;
}


long dataobject::Write(FILE  *file, long  writeID, int  level)
                {
    if ((this)->GetWriteID() != writeID)  {
        (this)->SetWriteID(writeID);
        fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
/*	place file writing code here */
        fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
    }

    return (this)->GetID();
}


/* 
 * GetModified/SetModified
 * 
 * With every call to SetModified, a dataobject is assigned a 
 * modification timestamp.  The timestamp is not related to the time 
 * of day.  It is simply a long integer.  Timestamps are issued in 
 * increasing order, starting with 1.  Therefore, timestamps can be 
 * compared with the normal integer comparison functions (<, >, etc.)
 * 
 */

static long mod_time = 1;
#define NextTime()  (mod_time++)

long dataobject::GetModified()
    {
    return this->modified;
}

void dataobject::SetModified()
    {
    this->modified = NextTime();
}
void dataobject::RestoreModified(long  oldmodified)
        {   /* allow the reseting of the modified flag to an old value. 
      This is NOT a supported function */
    this->modified = oldmodified;
}

static char viewname[200];
static const char * const fmts[] = {
	"%sview",  "%sv",  "%sView",  "%sV",  NULL
};
ATK_CLASS(view);

const char *dataobject::ViewName() {
	const char * const *fx;
	const char *tnm = (this)->GetTypeName();
	for (fx = fmts; *fx; fx++) {
		sprintf(viewname, *fx, tnm);
		ATKregistryEntry *are = ATK::LoadClass(viewname);
		if (are && are->IsType(class_view))
			return are->ClassName;
	}
	return "view";
}

void dataobject::SetAttributes(struct attributes  *attributes) {
}


void dataobject::Put( class atom  * property, class atom  * type, long  value )
{
    struct property *oldprop = NULL;
    struct property * newprop =
      (struct property *)malloc(sizeof(struct property));

    newprop->type = type;
    newprop->data = value;
    if (this->properties == NULL)
	this->properties = new Namespace;
    else {
	if(this->properties->Boundp(property, (long *)&oldprop)) {
	    if(oldprop) free(oldprop);
	}
    }
    (this->properties)->SetValue(  property, (long) newprop );
}


short dataobject::Get( class atom  * property, class atom  **type, long  * value )
                    {
  struct property * prop;

  /* we found the property if: the name is bound, and the types match or 
     the specified type is NULL.  If type is not NULL, but *type is, then 
     we fill in the actual type. */

  if (this->properties != NULL && (this->properties)->Boundp( property, (long *) &prop)
		       && (type == NULL || *type == NULL || *type == prop->type))
    {
      if (value != NULL)
	*value = prop->data;
      if (type != NULL && *type == NULL)
	*type = prop->type;
      return TRUE;
    }
  else
    return FALSE;
}

int dataobject::ListCurrentViews(class view  **array,int  size)
{ 
    /* fills in the array (of size 'size') with a list of views
          observing this dataobj.
      Returns the number of views found, which may be greater than size.
      Note that to just get a count of views,
         this routine may be called with array = NULL and size = 0 */
    int i,count;
    class observable *ob,**observers;
    ob = (class observable *)this;
    count = 0; 
    for (i = 0, observers = ob->observers; i < ob->nObservers ; i++, observers++)
	if(ATK::IsTypeByName((*observers)->GetTypeName(),"view") &&
	   ((class view *)(*observers))->dataobject == this){
	    if(count < size) *array++ = (class view *)*observers;
	    count++;
	}
    if(count < size)  *array = NULL;
    return count;
}

long dataobject::WriteOtherFormat(FILE  *file, long  writeID, int  level, int  usagetype, char  *boundary)
{
    if (usagetype != dataobject_OTHERFORMAT_MAIL) return(dataobject_BADFORMAT);
#ifdef THREEPART
    /* This is if we want to go the three-versions mail-stream route */
    fprintf(file, "\n<nl>[An <bold>Andrew</bold> object (a <italic>'%s'</italic> inset)\nwas included here in the original message,\nbut could not be translated to a non-Andrew mail format.]<nl>\n", (this)->GetTypeName());
    return 0; /* 0 return means we did NOT write out a real multipart piece.
		If we DID write it out properly, we should return 
		dataobject_GetID(self), as in the commented out line above */
#else
    fprintf(file, "\n--%s\nContent-type: application/andrew-inset\n\n", boundary);
    (this)->Write( file, writeID, 1); /* Make sure it isn't top-level */
    return (this)->GetID();
#endif
}

boolean
dataobject::ReadOtherFormat(FILE  *file, char  *fmt , char  *encoding, char  *description)
{
    return(FALSE); /* couldn't read it */
}

