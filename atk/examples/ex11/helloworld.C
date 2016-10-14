/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("helloworld.H")
#include <stdio.h>


#include "helloworld.H"


ATKdefineRegistryNoInit(helloworld, dataobject);

helloworld::helloworld()
{
    this->x=POSUNDEF;
    this->y=POSUNDEF;
    this->blackOnWhite=TRUE;
    THROWONFAILURE( TRUE);
}

long helloworld::Read(FILE  *file,long  id)
{
    char buf[100];

    if(fgets(buf,sizeof(buf),file)==NULL)
	return dataobject::PREMATUREEOF;
    /* the %hd tells scanf that blackOnWhite is a short, not an int */
    if(sscanf(buf,"%ld %ld %d\n",&this->x,&this->y,&this->blackOnWhite)<3)
	return dataobject::BADFORMAT;

    if(fgets(buf,sizeof(buf),file)==NULL) /* read in the \enddata{...} */
	return dataobject::MISSINGENDDATAMARKER;

    return dataobject::NOREADERROR;
}

long helloworld::Write(FILE  *file,long  writeId,int  level)
{
    if(writeId!=(this)->GetWriteID()){ /* only write a given version once */
	(this)->SetWriteID(writeId);
	fprintf(file,"\\begindata{%s,%ld}\n",
		(this)->GetTypeName(), (this)->GetID());
	fprintf(file,"%ld %ld %d\n",this->x,this->y,this->blackOnWhite);
	fprintf(file,"\\enddata{%s,%ld}\n",
		(this)->GetTypeName(), (this)->GetID());
    }

    return (this)->GetID();
}
