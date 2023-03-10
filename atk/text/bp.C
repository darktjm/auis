/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("bp.H")
#include <bp.H>

ATKdefineRegistryNoInit(bp, dataobject);
#ifndef lint
#endif /* lint */


bp::bp()
{
    this->haspagenum = FALSE;
    this->pagenum = 0;
    THROWONFAILURE( TRUE);
}

const char *bp::ViewName()
{
    return "bpv";
}

void bp::SetPageNum(long newval)
{
    this->haspagenum = TRUE;
    this->pagenum = newval;
    this->SetModified();
}

void bp::ClearPageNum()
{
    this->haspagenum = FALSE;
    this->SetModified();
}

long bp::Read(FILE  *file, long  id)
{
    char buf[200], name[200];
    char flag;
    long val;
    int ix;

    this->ClearPageNum();

    if (!fgets(buf, 200, file))
	return dataobject::PREMATUREEOF;

    if (!strncmp(buf, "Version 2", 9)) {
	/* version 2: */
	if (!fgets(buf, 200, file))
	    return dataobject::PREMATUREEOF;
	ix = sscanf(buf, "%c %ld", &flag, &val);
	if (ix != 2)
	    return dataobject::BADFORMAT;
	this->haspagenum = (flag=='y');
	this->pagenum = val;
	if (!fgets(buf, 200, file))
	    return dataobject::PREMATUREEOF;
    }

    /* if the first line was not "Version 2", we expect an enddata immediately */
    if (!strncmp(buf, "\\enddata{", 9)) {
	ix = sscanf(buf, "\\enddata{%[^,],%ld}", name, &val);
	if (ix != 2)
	    return dataobject::BADFORMAT;
	if (val != id || strcmp(name, (this)->GetTypeName()))
	    return dataobject::BADFORMAT;
	return dataobject::NOREADERROR;
    }

    return dataobject::BADFORMAT;
}

long bp::Write(FILE  *file, long  writeID, int  level)
{
    if ((this)->GetWriteID() != writeID)  {
        (this)->SetWriteID(writeID);
        fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());

	fprintf(file, "Version 2\n");
	fprintf(file, "%c %ld\n", (this->haspagenum ? 'y' : 'n'), this->pagenum);

	fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
    }

    return (this)->GetID();
}

