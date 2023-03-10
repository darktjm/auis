/* Copyright 1994 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
ATK_IMPL("unknown.H")
#include <stdio.h>
#include <text.H>
#include <util.h>
#include <unknown.H>
#include <ctype.h>

static char keywordbuf[32];


ATKdefineRegistryNoInit(unknown, text);

const char *unknown::GetRealClass()
{
    return this->realclass?this->realclass:"unknown";
}

void unknown::SetRealClass(char  *rclass)
{
    char buf[256];
    if(rclass && ATK::IsTypeByName(rclass, "unknown")) {
	/* assume that the first subobject in the 'unknown' dataobject set the real
	 class */
	this->wrapped=TRUE;
	return;
    }
    if(this->realclass) free(this->realclass);
    /* delete the old message */
    (this)->AlwaysDeleteCharacters( 0, (this)->GetLength());
    if(rclass) {
	this->realclass=strdup(rclass);
	/* tell the user that the inset is not supported */
	sprintf(buf, "? (see 'help unknown')\nThe %s inset is not supported in this version of AUIS.\n", this->realclass);
    } else {
	this->realclass=NULL;
	/* tell the user there was some inset here that wasn't supported but we don't know what
	 it was supposed to be. */
	sprintf(buf, "? (see 'help unknown')\nAn unidentified inset was here but could not be displayed.\n");
    }
    fprintf(stderr, "%s", buf);
    (this)->AlwaysInsertCharacters( 0, buf, strlen(buf));
}
    
unknown::unknown()
{
    (this)->SetReadOnly( TRUE);
    this->odata=NULL;
    this->wrapped=FALSE;
    this->realclass=NULL;
    THROWONFAILURE( TRUE);
}

unknown::~unknown()
{
    if(this->odata) {
	(this->odata)->Destroy();
	this->odata=NULL;
    }
    if(this->realclass) {
	free(this->realclass);
	this->realclass=NULL;
    }
}

/* get a character from the file and put it in the text */
static int tgetc(class text  *self, FILE  *file)
{
    int ch=getc(file);
    char charch=ch;
    if(ch!=EOF && self) (self)->AlwaysInsertCharacters( (self)->GetLength(), &charch, 1);
    return ch;
}

/* put a character back in the file and remove it from the text. */
static void tungetc(class text  *self, int ch, FILE  *file)
{
    ungetc(ch, file);
    if(self) (self)->AlwaysDeleteCharacters( (self)->GetLength()-1, 1);
}

/* Read one unknown dataobject, putting the raw datastream into self, and checking
 that the enddata id matches id.  If lev is 0 or 1 the "real" class name for this object
 will be set on the unknown inset uself. */
static long RealRead(class unknown  *uself, class text  *self, FILE  *file, long  id, int  lev)
{
    int ch;
    int sawslash=0;
    long res=0;
    while((ch=tgetc(self, file))!=EOF) {
	if(sawslash) {
	    char *p=keywordbuf;
	    sawslash=0;
	    if(ch=='{' || ch=='}' || ch=='\\') continue;
	    /* else look for keyword{data} */
	    *p++=ch;
	    while((ch=tgetc(self, file))!=EOF && ch!='{'  && ch!='\\' && ch!='}') {
		if(p-keywordbuf<(int)sizeof(keywordbuf)-1) {
		    *p++=ch;
		}
	    }
	    if(ch==EOF) return dataobject::PREMATUREEOF;
	    if(ch=='\\' || ch=='}') {
		/* well this may have LOOKED like a keyword but somebody wrote
		 some strange datastream.  ungetting the \ or } and starting
		 the loop again will ensure that we don't miss an enddata. */
		tungetc(self, ch, file);
		continue;
	    }
	    *p='\0';
	    if(ch=='{') {
		if(strcmp(keywordbuf, "begindata")==0) {
		    /* read the start of a subobject, and then call ourselves recursively. */
		    long bid=0;
		    /* everything after the '{' and before the ',' is the classname */
		    while((ch=tgetc(self, file))!=EOF && ch!=',');
		    if(ch==EOF) return dataobject::PREMATUREEOF;
		    /* skip any space after the comma */
		    while((ch=tgetc(self, file))!=EOF && isspace(ch));
		    if(ch==EOF) return dataobject::PREMATUREEOF;
		    /* put back the first non-space character after the comma */
		    tungetc(self, ch, file);
		    /* read the dataobject id */
		    while((ch=tgetc(self, file))!=EOF && isdigit(ch)) bid=bid*10+(ch-'0');
		    if(ch==EOF) return dataobject::PREMATUREEOF;
		    /* discard any chunk after the id and before the '}' */
		    if(ch!='}') while((ch=tgetc(self, file))!=EOF && ch!='}');
		    if(ch==EOF) return dataobject::PREMATUREEOF;
		    /* if there is a newline after the '}' slurp it up, otherwise leave the character
		     for the subobject. */
		    if((ch=tgetc(self, file))!=EOF && ch!='\n') tungetc(self, ch, file);
		    else if(ch==EOF) return dataobject::PREMATUREEOF;
		    /* recurse to read the subobject */
		    res=RealRead(uself, self, file, bid, lev+1);
		    if(res!=0) return res;
		    continue;
		} else if(strcmp(keywordbuf, "enddata")==0) {
		    /* exit this subobject, checking that the dataobject id matches the
		     begindata.  If a mismatch is detected just warn the user. */
		    int eid=0;
		    char cbuf[100];
		    long cpos=(self)->GetLength(), cepos;
		    /* scan the classname */
		    while((ch=tgetc(self, file))!=EOF && ch!=',');
		    if(lev==0 || lev==1) {
			/* if this is the top level object or the one directly below it
			 remember the class name so we  can tell the user about it.
			 If the name is "unknown" then we will assume that the the
			 second level subobject set the name appropriately */
			cepos=(self)->GetLength()-1;
			if(ch==',' && cepos>cpos) cepos--;
			if(cepos-cpos+1>(int)sizeof(cbuf)) {
			    cepos=cpos+sizeof(cbuf)-1;
			}
			(self)->CopySubString( cpos, cepos-cpos+1, cbuf, FALSE);
			(uself)->SetRealClass( cbuf);
		    }
		    if(ch==EOF) return dataobject::PREMATUREEOF;
		    /* skip the space after the comma in \enddata{classname, 328} */
		    while((ch=tgetc(self, file))!=EOF && isspace(ch));
		    if(ch==EOF) return dataobject::PREMATUREEOF;
		    /* put back the first non-space character, it should be the first digit
		     of the dataobject id. */
		    tungetc(self, ch, file);
		    /* read the enddata id */
		    while((ch=tgetc(self, file))!=EOF && isdigit(ch)) eid=eid*10+(ch-'0');
		    if(ch==EOF) return dataobject::PREMATUREEOF;
		    /* slurp up garbage after the id and before the '}' */
		    if(ch!='}') while((ch=tgetc(self, file))!=EOF && ch!='}');
		    if(ch==EOF) return dataobject::PREMATUREEOF;
		    /* slurp up the newline if it is there. */
		    if((ch=tgetc(self, file))!=EOF && ch!='\n') tungetc(self, ch, file);
		    else if(ch==EOF) return dataobject::PREMATUREEOF;
		    if(eid!=id) {
			fprintf(stderr, "warning: %s__Read: enddata id %d doesn't match begindata id %ld.\n", (self)->GetTypeName(), eid, id);
		    }
		    return dataobject::NOREADERROR;
		}
	    }


	} else {
	    if(ch=='\\') sawslash=1;
	}
    }
    return dataobject::PREMATUREEOF;
}

/* this function will read and ignore any dataobject. */
long unknown::Read(FILE  *file, long  id)
{
    long ret;
    if(this->odata==NULL) {
	/* make the text to keep the raw datastream, make it readonly so it is less likely to
	 be modified by the user if they ever get their hands on it. */
	this->odata=new text;
	if(this->odata) (this->odata)->SetReadOnly( TRUE);
    } else {
	/* remove any old data left over from a previous read. */
	if((this->odata)->GetLength()) (this->odata)->AlwaysDeleteCharacters( 0, (this->odata)->GetLength());
    }
    /* actually read all the data up to the appropriate enddata */
    ret=RealRead(this, this->odata, file, id, 0);
    if(this->odata) {
	char buf[256];
 /* if the unknown object was at the top level when we read it in, we
	     now wrap it in the unknown inset since upon writing the data contained
	     in it may become invalid.  This would occur for example if it contains
	     references to dataobject::GetID's of objects outside itself.  There is
	     also the potential for conflict between id's in the object and objects
	     outside. Manual intervention will be required to */
	if(!this->wrapped) {
	    /* put in the begindata which we missed the first time because the caller of __Read
	     had to slurp it up. */
	    sprintf(buf, "\\begindata{%s,%ld}\n", (this)->GetRealClass(), id);
	    (this->odata)->AlwaysInsertCharacters( 0, buf, strlen(buf));
	} else {
	    /* it was already wrapped, so we search for the \enddata{unknown,9282}\n that was stuffed into the
	     text and delete it, the write routine will put in the right begindata/enddata pair around the unknown
	     inset's begindata/enddata. */
	    long pos=(this->odata)->GetLength()-1;
	    while(pos>=0) {
		if((this->odata)->GetChar( pos)=='\\') break;
		pos--;
	    }
	    if(pos>=0) (this->odata)->AlwaysDeleteCharacters( pos, (this->odata)->GetLength()-pos);
	    (this)->AlwaysDeleteCharacters( 0, (this)->GetLength());
	    /* display a message to the user indicating that the data contained here may have been
	     corrupted by a write as an unknown inset and point them at the documentation. */
	    sprintf(buf, "? (see 'help unknown') The %s inset included here may have been corrupted.\nSee the help file for 'unknown' for recovery options.\n", (this)->GetRealClass());
	    (this)->AlwaysInsertCharacters( 0, buf, strlen(buf));
	}
    }
    return ret;
}  

/* wrap the unknown inset in an 'unknown' inset to protect future readers from a possibly corrupted
 version of the data. */
long unknown::Write(FILE  *file, long  writeid, int  level)
{
    if((this)->GetWriteID()!=writeid) {
	long pos=0, len;
	/* output the raw inset data wrapped in an 'unknown' dataobject. */
	fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
	if(this->odata) {
	    /* write out the data verbatim! */
	    while(pos<(this->odata)->GetLength()) {
		char *buf=(this->odata)->GetBuf( pos, (this->odata)->GetLength()-pos, &len);
		if(len==0 || (long)fwrite(buf, 1, len, file)!=len) return -1;
		pos+=len;
	    }
	} else fprintf(file, "\n");
	fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
    }
    return (this)->GetID();
}
