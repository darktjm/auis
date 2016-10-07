/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
*/

#include <andrewos.h>
ATK_IMPL("phlumph.H")
#include <phlumph.H>

#include <textflow.H>

ATKdefineRegistry(phlumph, dataobject, NULL);

phlumph::phlumph()
{
    ATKinit;

    this->numpages = 0;
    this->listsize = 0;
    this->pagelist = NULL;
    this->repeat = 0;

    THROWONFAILURE( TRUE);
}

phlumph::~phlumph()
{
    ATKinit;

    if (this->pagelist)
	free(this->pagelist);

}

const char *phlumph::ViewName()
{
    return "phlumphview";
}

boolean phlumph::InsertPage(int where)
{
    class textflow *newpage;

    if (!this->pagelist) {
	this->listsize = 2;
	this->pagelist = (class textflow **)malloc(sizeof(class textflow *) * this->listsize);
	if (!this->pagelist)
	    return FALSE;
	this->numpages = 0;
    }

    if (this->numpages == this->listsize) {
	this->listsize *= 2;
	this->pagelist = (class textflow **)realloc(this->pagelist, sizeof(class textflow *) * this->listsize);
	if (!this->pagelist)
	    return FALSE;
    }

    newpage = new textflow;
    if (!newpage)
	return FALSE;

    if (where == phlumph_AtEnd)
	where = this->numpages;
    if (where < 0)
	where = 0;
    else if (where > this->numpages)
	where = this->numpages;

    if (this->numpages != where) {
	memmove(this->pagelist+(where+1), this->pagelist+where, sizeof(class textflow *) * (this->numpages-where));
    }
    this->numpages++;
    this->pagelist[where] = newpage;

    if (this->repeat != 0 && this->repeat == this->numpages-1)
	this->repeat = this->numpages;

    return TRUE;
}

void phlumph::DeletePage(int num)
{
    if (num < 0 || num >= this->numpages)
	return;

    this->pagelist[num]->Destroy();
    if (this->numpages != num+1) {
	memmove(this->pagelist+num, this->pagelist+(num+1), sizeof(class textflow *) * (this->numpages-(num+1)));
    }
    this->numpages--;
    if (this->repeat > this->numpages)
	this->repeat = this->numpages;
}

void phlumph::SetRepeat(int num)
{
    if (num < 0)
	num = 0;
    if (num > this->numpages)
	num = this->numpages;
    this->repeat = num;
}

long phlumph::Write(FILE  *fp, long  writeid, int  level)
{
    int ix;
    if ((this)->GetWriteID() != writeid) {
	(this)->SetWriteID( writeid);

	fprintf(fp, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
	fprintf(fp, "$version %d\n", phlumph_DatastreamVersion);
	fprintf(fp, "pages %d\n", this->GetNumPages());
	fprintf(fp, "repeat %d\n", this->repeat);
	for (ix=0; ix<this->GetNumPages(); ix++) {
	    this->GetPage(ix)->Write(fp, writeid, level+1);
	}
	fprintf(fp, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
    }

    return (this)->GetID();
}

long phlumph::Read(FILE  *fp, long  id)
{
    long tid, ix, jx, val1;
    int numpg;
    char namebuf[100];
#define LINELENGTH (250)
    char buf[LINELENGTH+1];
    class textflow *o;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject::PREMATUREEOF;
    ix = sscanf(buf, "$version %ld", &val1);
    if (ix!=1) return dataobject::BADFORMAT;
    if (val1 > phlumph_DatastreamVersion) {
	return dataobject::BADFORMAT; /* well, it probably will be */
    }

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject::PREMATUREEOF;
    ix = sscanf(buf, "pages %ld", &val1);
    if (ix!=1) return dataobject::BADFORMAT;
    numpg = val1;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject::PREMATUREEOF;
    ix = sscanf(buf, "repeat %ld", &val1);
    if (ix!=1) return dataobject::BADFORMAT;
    this->repeat = val1;

    if (numpg) {

	this->numpages = numpg;

	if (!this->pagelist) {
	    this->listsize = this->numpages;
	    this->pagelist = (class textflow **)malloc(sizeof(class textflow *) * this->listsize);
	    if (!this->pagelist)
		return dataobject::OBJECTCREATIONFAILED;
	}

	if (this->numpages > this->listsize) {
	    this->listsize = this->numpages;
	    this->pagelist = (class textflow **)realloc(this->pagelist, sizeof(class textflow *) * this->listsize);
	    if (!this->pagelist)
		return dataobject::OBJECTCREATIONFAILED;
	}

	for (jx=0; jx<numpg; jx++) {
	    if (fgets(buf, LINELENGTH, fp) == NULL)
		return dataobject::PREMATUREEOF;
	    ix = sscanf(buf, "\\begindata{%[^,],%ld}", namebuf, &tid);
	    if (ix!=2) return dataobject::BADFORMAT;

	    if (!ATK::IsTypeByName(namebuf, "textflow"))
		return dataobject::BADFORMAT;
	    o = (class textflow *)ATK::NewObject(namebuf);
	    if (!o) return dataobject::OBJECTCREATIONFAILED;

	    ix = (o)->Read( fp, tid);
	    if (ix!=dataobject::NOREADERROR) return ix;
	    this->pagelist[jx] = o;
	}
    }

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject::PREMATUREEOF;
    ix = sscanf(buf, "\\enddata{%[^,],%ld}", namebuf, &tid);
    if (ix!=2) return dataobject::MISSINGENDDATAMARKER;

    if ((id && tid!=id) || strcmp(namebuf, (this)->GetTypeName()))
	return dataobject::MISSINGENDDATAMARKER;

    return dataobject::NOREADERROR;     
}
