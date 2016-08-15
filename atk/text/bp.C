/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/bp.C,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $ */
/* $ACIS:$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/bp.C,v $ */

#include <andrewos.h>

#ifndef lint
static UNUSED const char rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/bp.C,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* lint */

ATK_IMPL("bp.H")
#include <bp.H>

ATKdefineRegistry(bp, dataobject, NULL);
#ifndef lint
#endif /* lint */


bp::bp()
{
    this->haspagenum = FALSE;
    this->pagenum = 0;
    THROWONFAILURE( TRUE);
}

char *bp::ViewName()
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

    (this)->SetID((this)->UniqueID()); /* change id to unique number */
    this->ClearPageNum();

    if (!fgets(buf, 200, file))
	return dataobject_PREMATUREEOF;

    if (!strncmp(buf, "Version 2", 9)) {
	/* version 2: */
	if (!fgets(buf, 200, file))
	    return dataobject_PREMATUREEOF;
	ix = sscanf(buf, "%c %ld", &flag, &val);
	if (ix != 2)
	    return dataobject_BADFORMAT;
	this->haspagenum = (flag=='y');
	this->pagenum = val;
	if (!fgets(buf, 200, file))
	    return dataobject_PREMATUREEOF;
    }

    /* if the first line was not "Version 2", we expect an enddata immediately */
    if (!strncmp(buf, "\\enddata{", 9)) {
	ix = sscanf(buf, "\\enddata{%[^,],%ld}", name, &val);
	if (ix != 2)
	    return dataobject_BADFORMAT;
	if (val != id || strcmp(name, (this)->GetTypeName()))
	    return dataobject_BADFORMAT;
	return dataobject_NOREADERROR;
    }

    return dataobject_BADFORMAT;
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

