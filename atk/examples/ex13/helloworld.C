/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/config/COPYRITE

	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/examples/ex13/RCS/helloworld.C,v 1.3 1996/12/19 20:23:39 fred Exp $";
#endif

ATK_IMPL("helloworld.H")
#include <stdio.h>


#include "helloworld.H"


ATKdefineRegistry(helloworld, dataobject, NULL);
#ifndef NORCSID
#endif


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

    (this)->SetID((this)->UniqueID());

    if(fgets(buf,sizeof(buf),file)==NULL)
	return dataobject_PREMATUREEOF;
    /* the %hd tells scanf that blackOnWhite is a short, not an int */
    if(sscanf(buf,"%ld %ld %d\n",&this->x,&this->y,&this->blackOnWhite)<3)
	return dataobject_BADFORMAT;

    if(fgets(buf,sizeof(buf),file)==NULL) /* read in the \enddata{...} */
	return dataobject_MISSINGENDDATAMARKER;

    return dataobject_NOREADERROR;
}

long helloworld::Write(FILE  *file,long  writeId,int  level)
{
    if(writeId!=(this)->GetWriteID()){ /* only write a given version once */
	(this)->SetWriteID(writeId);
	fprintf(file,"\\begindata{%s,%ld}\n",
		(this)->GetTypeName(), (this)->UniqueID());
	fprintf(file,"%ld %ld %d\n",this->x,this->y,this->blackOnWhite);
	fprintf(file,"\\enddata{%s,%ld}\n",
		(this)->GetTypeName(), (this)->UniqueID());
    }

    return (this)->UniqueID();
}
