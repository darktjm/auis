/* File compress.c created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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

   compress, an object to store a hidden region of text. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static UNUSED const char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/compress.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";


#include <environment.H>
#include <viewref.H>
#include <text.H>
#include <toolcnt.h>

#include "compress.H"

#define TEXT_VIEWREFCHAR '\377'

ATKdefineRegistry(compress, text, compress::InitializeClass);

boolean compress::InitializeClass()
{
    return TRUE;
}

compress::compress()
{
    ATKinit;
    this->lines= 0;
    this->loc= -1;
    SetExportEnvironments(TRUE);
    THROWONFAILURE(TRUE);
}

compress::~compress()
{
    ATKinit;
    return;
}

/* GetModified always returns 0, so that the parent document won't realize one of its subobjects has changed when a region is compressed. */
long compress::GetModified()
{
    return 0;
}

/* override */
/* GetPosForLine "tricks" everyone who calls it by ALSO counting newlines inside NESTED compress objects */
long compress::GetPosForLine(long line)
{
    long i=1, pos=0, len=GetLength();
    int ch;

    while (i < line) {
	if (pos >= len)
	    break;
	if ((ch=GetChar(pos)) == '\012')
	    i++;
	else if (ch==TEXT_VIEWREFCHAR) { /* this check is quicker than letting IsThere figure it out */
	    compress *isthere=compress::IsThere((text *)this,pos);
	    if (isthere)
		i+= (isthere)->GetLines()-1; /* -1 to compensate for \n after box being counted */
	}
	pos++;
    }
    return pos;
}

/* override */
/* GetLineForPos "tricks" everyone who calls it by ALSO counting newlines inside NESTED compress objects */
long compress::GetLineForPos(long pos)
{
    long line=1;
    int ch;

    while (--pos >= 0)
	if ((ch= GetChar(pos)) == '\012')
	    line++;
	else if (ch==TEXT_VIEWREFCHAR) { /* this check is quicker than letting IsThere figure it out */
	    compress *isthere=compress::IsThere((text *)this,pos);
	    if (isthere)
		line+= (isthere)->GetLines()-1; /* -1 to compensate for \n after box being counted */
	}
    return line;
}

/* quickAddView is a ripoff of text_AlwaysAddView, but with all but the bare necessities ripped out of it. It also changes env_WRAPView to env_INSERTView, which is a zillion times faster, but doesn't allow nesting. */
/* text_AlwaysInsertFile calls dictionary_Insert. I have no idea what that does. I don't call it when I insert a compress object/view, and it seems to work OK, so I just left it out. -RSK*/
environment *quickAddView(text *self, long pos, char *viewtype, compress *compressobj)
{
    viewref *newviewref=viewref::Create(viewtype, (dataobject *)compressobj);
    char c=TEXT_VIEWREFCHAR;
    register environment *newenv, *root=self->rootEnvironment;

    (newviewref)->AddObserver(self);
    (self)->AlwaysInsertCharacters(pos, &c, 1);
    if (root && (root)->GetEnclosing(pos)!=root) /* can't nest environments! (in source code, anyway) */
	(self->rootEnvironment)->Remove(pos,1, environment_Style, TRUE);
    newenv= (self->rootEnvironment)->InsertView(pos, newviewref,TRUE);
    if (newenv) {
	(newenv)->SetLength(1);
	(newenv)->SetStyle(FALSE, FALSE);
    }
    return newenv;
}

void compress::Compress(text *txt, long pos, long len)
{
    ATKinit;
    compress *self;
    if (len<1) return;
    self= new compress;
    if ((self)->GetLength() > 0)
	(self)->Clear(); /* just to make sure */
    if ((txt)->GetChar(pos+len-1) == '\n')
	--len; /* don't grab trailing newline, it'd look ugly */
    (self)->CompressInFront(txt, pos,len);
    quickAddView(txt, pos, "compressv",self);
    if (!ATK::IsTypeByName((txt)->GetTypeName(), "srctext"))
	(txt)->NotifyObservers(0);
}

/* IsThere returns a pointer to the compress inset at that position, if there IS one there, or returns NULL if there isn't one there */
compress *compress::IsThere(text *txt, long pos)
{
    ATKinit;
    if ((txt)->GetChar(pos)==TEXT_VIEWREFCHAR) {
	environment *env=(environment *)(txt->rootEnvironment)->GetInnerMost(pos);
	if (env->type == environment_View) {
	    dataobject *inset= env->data.viewref->dataObject;
	    if (ATK::IsTypeByName((inset)->GetTypeName(), "compress"))
		return (compress *)inset;
	}
    }
    return NULL;
}

/* CompressInFront compresses the specified region by removing it from the parent document and stashing it into itself (in the front). */
void compress::CompressInFront(text *txt, long pos, long len)
{
    AlwaysCopyText(0, txt, pos,len);
    (txt)->AlwaysDeleteCharacters(pos,len);
}

/* CompressInBack compresses the specified region by removing it from the parent document and stashing it into itself (at the end). */
void compress::CompressInBack(text *txt, long pos, long len)
{
    AlwaysCopyText(GetLength(), txt, pos,len);
    (txt)->AlwaysDeleteCharacters(pos,len);
}

/* global pointers that store a list of compresses to run through, immediately after doupdate is done */
static compress **compresslist=NULL, **endcompresslist=NULL;

static boolean doupdate(compress *self, text *txt, long pos, environment *env)
{
    boolean retval=FALSE;
    if (env->type == environment_View){
	viewref *vr=env->data.viewref;
	const char *name=(vr->dataObject)->GetTypeName();
	if (self == NULL) {
	    if (!ATK::IsTypeByName(name,"compress"))
		return FALSE;
	    self= (compress *)vr->dataObject;
	    if (compresslist!=NULL && compresslist<endcompresslist)
		*compresslist++= self;
	} else {
	    if (self != (compress *)vr->dataObject)
		return FALSE;
	    retval= TRUE;
	}
	/* we have a compress */
	self->loc= pos;
    }
    return retval;
}

/* DoAll updates all the data held by the compresses in this text object, and will enumerate through them as well if callBack is non-NULL */
static void DoAll(text *txt, boolean (*callBack)(compress *, text *))
{
#define MAXCOMPRESSES 1024 /* MAXCOMPRESSES just determines the maximum number that can have the callBack() executed on them. ALL of the compresses in the text will have their positions updated, no matter HOW many there are, so don't worry about it. */
    compress *compresses[MAXCOMPRESSES];
    compresslist= compresses;
    endcompresslist= compresses + MAXCOMPRESSES;
    if (txt) {
	(txt)->EnumerateEnvironments(0, (txt)->GetLength(), (text_eefptr)doupdate, 0);
	if (callBack != NULL) {
	    /* do in reverse order. this makes sure the positions stay valid if we're decompressing as we go */
	    while (--compresslist >= compresses)
		(*(callBack))(*compresslist,txt);
	    if (!ATK::IsTypeByName((txt)->GetTypeName(), "srctext"))
		(txt)->NotifyObservers(0);
	}
    }
}

static boolean decompress(compress *self, text *txt)
{
    (txt)->AlwaysCopyText(self->loc+1, self,0,(self)->GetLength());
    (txt)->AlwaysDeleteCharacters(self->loc,1); /* this deletes the character, which reduces the view env's length to zero.  When the env removes itself, it destroys its viewref, which in turn destroys the dataobject it was looking at */
    return TRUE; /* (not used anyway) */
}

/* DecompressBox puts the compressed text back into the parent document */
void compress::DecompressBox(text *txt)
{
    DoAll(txt,NULL); /* make sure everything's updated */
    decompress(this,txt);
}

/* PartialDecompress decompresses the part that needs decompressing, and re-compresses the rest */
void compress::PartialDecompress(text *txt, long pos, long len)
{
    long cprslen=GetLength();
    if (pos==0 && len>=cprslen) {
	DecompressBox(txt);
	return;
    }
    if (len<1)
	return;
    DoAll(txt,NULL); /* update compress boxes' positions */
    if (pos==0 || pos+len>=cprslen) {
	/* just decompress out of the front or back */
	(txt)->AlwaysCopyText(this->loc+(pos>0), this,pos,len);
	(this)->AlwaysDeleteCharacters(pos,len);
	(this)->SetLines( (this)->GetLineForPos( (this)->GetLength()));
	if (!ATK::IsTypeByName((txt)->GetTypeName(), "srctext"))
	    NotifyObservers(0);
    } else {
	/* decompress out of the middle */
	long refpos=this->loc+1, reflen=cprslen-pos;
	/* first, decompress the ENTIRE last part of the box */
	(txt)->AlwaysCopyText(refpos, this,pos,reflen);
	(this)->AlwaysDeleteCharacters(pos,reflen);
	(this)->SetLines( (this)->GetLineForPos( (this)->GetLength()));
	if (!ATK::IsTypeByName((txt)->GetTypeName(), "srctext"))
	    NotifyObservers(0);
	/* then, REcompress the part at the end we didn't WANT decompressed */
	compress::Compress(txt,refpos+len,reflen-len);
    }
}

/* DecompressAll decompresses everything in the parent document */
void compress::DecompressAll(text *txt)
{
    ATKinit;
    DoAll(txt,decompress);
}

/* static variables used for limited-range operations */
static long startrange, endrange;

static boolean decompressIfInRange(compress *self, text *txt)
{
    if (self->loc >= startrange && self->loc <= endrange)
	return decompress(self,txt);
    return TRUE; /* (not used anyway) */
}

/* DecompressRange decompresses everything in the parent document that falls within len after pos */
void compress::DecompressRange(text *txt, long pos, long len)
{
    ATKinit;
    startrange= pos;
    endrange= pos+len;
    DoAll(txt,decompressIfInRange);
}
