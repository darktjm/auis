/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("helloworld.H")
#include <stdio.h>


#include "helloworld.H"

#include "dataobject.H"
#include "text.H"
#include "fontdesc.H"
#include "style.H"


ATKdefineRegistryNoInit(helloworld, dataobject);


static class dataobject *createInitialDobj()
{
    class text *theText=new text;
    class style *bold=new style,*italic=new style;

    (bold)->SetName("bold");
    (bold)->AddNewFontFace(fontdesc_Bold);

    (italic)->SetName("italic");
    (italic)->AddNewFontFace(fontdesc_Italic);

    (theText)->InsertCharacters(0,"Hello world!",sizeof("Hello world!")-1);

    (theText)->AddStyle(0,5,bold);
    (theText)->AddStyle(6,5,italic);

    return (class dataobject *)theText;
}

helloworld::helloworld()
{
    this->x = POSUNDEF;
    this->y = POSUNDEF;
    this->blackOnWhite = TRUE;
    this->dobj=createInitialDobj();

    THROWONFAILURE( TRUE);
}

helloworld::~helloworld()
{
    (this->dobj)->Destroy();
}

long helloworld::Read(FILE  *file,long  id)
{
    char buf[100],classNameBuf[100];
    long retVal,dobjObjId;

    if(fgets(buf,sizeof(buf),file)==NULL ||
       /* the %hd tells scanf that blackOnWhite is a short, not an int */
       sscanf(buf,"%ld %ld %d\n",&this->x,&this->y,&this->blackOnWhite)<3 ||
       fgets(buf,sizeof(buf),file)==NULL ||
       sscanf(buf,"\\begindata{%[^,],%ld}\n",classNameBuf,&dobjObjId)<2)
	retVal=dataobject::PREMATUREEOF;
    else{
	if(strcmp(classNameBuf,(this->dobj)->GetTypeName())!=0){
	    /* the type of the sub-object has changed */
	    class dataobject *oldDobj;

	    if(!ATK::IsTypeByName(classNameBuf,"dataobject"))
		return dataobject::BADFORMAT;

	    oldDobj=this->dobj;
	    this->dobj=(class dataobject *)ATK::NewObject(classNameBuf);

	    (this)->NotifyObservers(helloworld_SubObjectChanged);

	    (oldDobj)->Destroy();
	}

	retVal=(this->dobj)->Read(file,id);
	if(retVal==dataobject::NOREADERROR)
	    if(fgets(buf,sizeof(buf),file)==NULL) /* read in the \enddata{...} */
		retVal=dataobject::MISSINGENDDATAMARKER;
    }

    return retVal;
}

long helloworld::Write(FILE  *file,long  writeId,int  level)
{
    if(writeId!=(this)->GetWriteID()){ /* only write a given version once */
	(this)->SetWriteID(writeId);
	fprintf(file,"\\begindata{%s,%ld}\n",
		(this)->GetTypeName(), (this)->GetID());
	fprintf(file,"%ld %ld %d\n",this->x,this->y,this->blackOnWhite);
	(this->dobj)->Write(file,writeId,level);
	fprintf(file,"\\enddata{%s,%ld}\n",
		(this)->GetTypeName(), (this)->GetID());
    }

    return (this)->GetID();
}
