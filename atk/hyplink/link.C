/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	Copyright Carnegie Mellon, 1996
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
ATK_IMPL("link.H")

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <pushbutton.H>
#include <view.H>
#include <environ.H>
#include <fontdesc.H>
#include <path.H>
#include <util.h>
#include <link.H>

/* Defined constants and macros */
#define MAX_LINE_LENGTH 70  /* can't be less than 6 */
#define DS_VERSION 4 /* datastream version */

/* Global variables */

ATKdefineRegistry(link, pushbutton, link::InitializeClass);
static long link_SanelyReturnReadError(class link  *self, FILE  *fp, long  id, long  code);
static long ReadOldFormat(class link  *self, FILE  *fp, long  id);
static void WriteLine(FILE  *f, const char  *l);
static char * GlomStrings(char  *s , char  *t);
static char * ReadLine(FILE  *f);
#ifdef PL8
static char * EncodeFont(class link  *self);
#endif /* PL8 */

/* 
  Initialize all the class data.
*/
	boolean
link::InitializeClass() {
	return(TRUE);
}

/*
  Inititialize the object instance data.
*/
link::link()
{
	ATKinit;
	this->rawlink = NULL;
#ifdef PL8
	this->new_ds = 0;
#endif

	this->pos = 0;
	this->len = 0;
	(this)->SetStyle(environ::GetProfileInt("linkstyle", 2));
	THROWONFAILURE((TRUE));
}


/*
  Finalize the object instance data.
*/
link::~link()
{
	ATKinit;

	if (this->rawlink) free((this)->GetRawLink());
	this->rawlink = NULL;
	return;
}


/*  Returns the filename stored in the link, except that
	substrings of the form $FOO or $(FOO) or ${FOO} are
	replaced (where possible) with the value of the
	environment variable FOO.
	Result is returned in a static buffer overwritten with each call.
*/
	const char *
link::GetResolvedLink(view *viewer) {
	static char filename[1 + MAXPATHLEN], *linkval;

	linkval = GetRawLink();
	if ( ! linkval)
		return (NULL);
	return (path::UnfoldFileName(linkval, filename,
		(viewer) ? viewer->WantInformation("filename") : NULL));
}
/*
  Write the object data out onto the datastream.

  Sample output from datastream version 1:
	\begindata{link, 1234567}
	Datastream version: 1
	This is my button label
	/afs/andrew.cmu.edu/usr5/mcinerny/target_file
	
	\enddata{link, 1234567}

  Sample output versions 2,3,4
	\begindata{link, 1234567}
	Datastream version: 2  	-- or 3 (version 2 or 3)
	4			-- data stream version 4 or higher
	/afs/andrew.cmu.edu/usr5/mcinerny/target_file
	1234					-- pos	(version >=3)
	5678					-- len	(version >=3)
	E					-- linktype  (version >= 4)
				-- versions greater than 4 add lines here
	\begindata{link, 1234567}   -- really pushbutton (superclass)
	Datastream version: 2
	This is my button label	 -- label
	2					 -- style
	andy12b				 -- font
	black					 -- foreground color
	0xFFFFFF			 -- background color (RGB)
	\enddata{link, 1234567}	 -- again, really pushbutton part
	\enddata{link, 1234567}
*/
	long
link::Write(FILE  *fp, long  id, int  level) {
	long uniqueid = (this)->UniqueID();

	if (id == (this)->GetWriteID()) 
			return uniqueid;

	/* New Write Operation */
#ifdef PL8
	if (this->new_ds == 0) {
		fprintf(fp, 
		  	"\\begindata{%s,%d}\nDatastream version: %d\n",
		  	(this)->GetTypeName(), uniqueid, 1); /* lie! */
		WriteLine(fp, (this)->GetText() ? 
				(this)->GetText() : "");
		WriteLine(fp, (this)->GetRawLink() ? 
				(this)->GetRawLink() : "");
		WriteLine(fp, (this)->GetButtonFont() ?
				EncodeFont(this) : "");
	} else
#endif /* PL8 */
	{	fprintf(fp, 
		  	"\\begindata{%s,%ld}\n%d\n",
		  	(this)->GetTypeName(), uniqueid, 
		  	DS_VERSION);
		WriteLine(fp, (this)->GetRawLink() ? 
				(this)->GetRawLink() : "");
		fprintf(fp, "%ld\n", (this)->GetPos());
		fprintf(fp, "%ld\n", (this)->GetLen());
		fprintf(fp, "%c\n", 
				(typecode == FileLink) ? 'F'
				: (typecode == RelLink) ? 'R'
				: (typecode == URLLink) ? 'U'
				: (typecode == VarFileLink) ? 'V'
				: 'E');

		(this)->pushbutton::Write( fp, id, level);
	} 		/* if (self->new_ds == 0) */

	fprintf(fp, "\\enddata{%s,%ld}\n",
		(this)->GetTypeName(), uniqueid);
	(this)->SetWriteID( id);

	return(uniqueid);
}


/*
	  Suck up the file until our enddata, then return the error code.
*/
	static long
link_SanelyReturnReadError(class link  *self, FILE  *fp, 
			long  id, long  code)  {
	char *buf, buf2[255];

	buf = NULL;
	sprintf(buf2, "\\enddata{%s,%ld}", (self)->GetTypeName(), id);
	do {
		if (buf != NULL) free(buf);
		if ((buf = ReadLine(fp)) == NULL)
		  return(dataobject_PREMATUREEOF);
	} while (strncmp(buf, "\\enddata{", 9) != 0); /* find an enddata */

	if (strcmp(buf, buf2) != 0) {
		free(buf);
		return(dataobject_MISSINGENDDATAMARKER); /* not ours! */
	}
	free(buf);

	return(code);
}

	static long
ReadOldFormat(class link  *self, FILE  *fp, long  id) {
	char *buf;

	if ((buf = ReadLine(fp)) == NULL) 
		return(link_SanelyReturnReadError(self, fp, id, 
				dataobject_PREMATUREEOF));
	(self)->SetText((*buf) ? buf : NULL);
	free(buf);
	
	if ((buf = ReadLine(fp)) == NULL) 
		return(link_SanelyReturnReadError(self, fp, id, 
				dataobject_PREMATUREEOF));
	(self)->SetLink((*buf) ? buf : NULL);
	free(buf);
	
	if ((buf = ReadLine(fp)) == NULL) 
		return(link_SanelyReturnReadError(self, fp, id, 
				dataobject_PREMATUREEOF));
	char name[255];
	long style, size;
	if (! *buf || !fontdesc::ExplodeFontName(buf,name,sizeof(name),
						  &style, &size)) {
		strcpy(name,"andy");
		style = fontdesc_Bold;
		size = 12;
	}
	(self)->SetButtonFont(fontdesc::Create(name,style,size));
	free(buf);
	return(dataobject_NOREADERROR);
}

/*
  Read in the object from the file.
*/
	long
link::Read(FILE  *fp, long  id)	{
	char *buf;
	int ds_version; 
	long error;
	
	(this)->SetID( (this)->UniqueID());

	// read data stream version number
	if ((buf = ReadLine(fp)) == NULL) {
		free(buf);
		return(link_SanelyReturnReadError(this, fp, id, 
				dataobject_PREMATUREEOF));
	}
	if (sscanf(buf, "%d", &ds_version) == 1) {
		// version 4 or above
	}
	else {
		if (strncmp(buf,"Datastream version:",19)) {
			free(buf);
			return(link_SanelyReturnReadError(this, fp, id, 
				dataobject_BADFORMAT));
		}
		ds_version = atoi(buf+19);
	}
	if (ds_version < 1  ||  (ds_version > 3 && !isdigit(*buf))) {
		free(buf);
		return(link_SanelyReturnReadError(this, fp, id, 
					dataobject_BADFORMAT));
	}
	free(buf);

	if (ds_version == 1)
		return ReadOldFormat(this, fp, id);

	// all other formats 
#ifdef PL8
	this->new_ds = 1;	// preserve new data on write
#endif /* PL8 */

	if ((buf = ReadLine(fp)) == NULL) 
		return(link_SanelyReturnReadError(this, fp, id, 
			dataobject_PREMATUREEOF));
	(this)->SetLink((*buf) ? buf : NULL);
	free(buf);
	
	if (ds_version >= 3) {
		if ((buf = ReadLine(fp)) == NULL) {
			free(buf);
			return(dataobject_PREMATUREEOF);
		}
		(this)->SetPos( atol(buf));
		free(buf);

		if ((buf = ReadLine(fp)) == NULL) {
			free(buf);
			return(dataobject_PREMATUREEOF);
		}
		(this)->SetLen( atol(buf));
		free(buf);
	}
	if (ds_version >= 4) {
		if ((buf = ReadLine(fp)) == NULL) {
			free(buf);
			return(dataobject_PREMATUREEOF);
		}
		SetTypeCode(
			(*buf == 'F') ? FileLink
			: (*buf == 'R') ? RelLink
			: (*buf == 'U') ? URLLink
			: (*buf == 'V') ? VarFileLink
			: NoLink
		);
		free(buf);
	}

	// skip to leading `\';  this ignores attributes for versions > 4
	while (TRUE) {
		buf = ReadLine(fp);
		if (!buf)
			return(link_SanelyReturnReadError(this, fp, id,
				dataobject_PREMATUREEOF));
		if (*buf == '\\') 
			break;
		free(buf);
	}

	if (strncmp(buf,"\\begindata{", 11) != 0 ) {
		return(link_SanelyReturnReadError(this, fp, id, 
				dataobject_BADFORMAT));
	}
	free(buf);
	(this)->pushbutton::Read(fp, id);

	return(link_SanelyReturnReadError(this, fp, id, 
		dataobject_NOREADERROR));
}



/*  Set the link target (a filename) for this object.  */
	void
link::SetLink(char *link) {
	if ((this)->GetRawLink()) free((this)->GetRawLink());
	if (link) {
		this->rawlink = NewString(link);
		SetTypeCode((strchr(link, '$')) ? VarFileLink : FileLink);
	}
	else {
		this->rawlink = NULL;
		SetTypeCode(NoLink);
	}
	(this)->SetModified();
	(this)->NotifyObservers( 0);	// (technically WRONG)
}

/*  Set the link target (a filename) for this object.  */
	void
link::SetLink(enum link_type code, char  *link) {
	SetLink(link);
	if (link) 
		SetTypeCode(code);
}

 /*  Set the link position for this object. */
	void 
link::SetPos(long  pos) {
	this->pos = pos;
	(this)->NotifyObservers( 0);
}

 /*  Set the link length for this object. */
	void 
link::SetLen(long  len ) {
	this->len = len;
	(this)->NotifyObservers( 0);
}


static void
WriteLine(FILE  *f, const char  *l)
{
/* 
	Output a single line onto the data stream, quoting
	back slashes and staying within line length limits.
	Warning:  this routine wasn't meant to handle embedded
	newlines.
*/

	char buf[MAX_LINE_LENGTH];
	int i = 0;

	for (;*l != '\0'; ++l) {
	if (i > (MAX_LINE_LENGTH - 5)) {
	  buf[i++] = '\\';  /* signal for line continuation */
	  buf[i++] = '\n';
	  buf[i++] = '\0';
	  fputs(buf,f);
	  i = 0;
	} /* if (i > ...) */
	switch (*l) {
	case '\\': 
	  /* if a backslash, quote it. */
	  buf[i++] = '\\';
	  buf[i++] = *l;
	  break;
	default:
	  buf[i++] = *l;
	} /* switch (*l) */
	} /* for (; *l != ... ) */

	/* Need to empty buffer */
	if ((i > 0) && (buf[i-1]==' ')) {
	/* don't allow trailing whitespace */
	buf[i++] = '\\';
	buf[i++] = '\n';
	buf[i++] = '\0';
	fputs(buf,f);
	fputs("\n",f);
	} else {
	buf[i++] = '\n';
	buf[i++] = '\0';
	fputs(buf,f);
	}
}


static char *
GlomStrings(char  *s , char  *t)
{
/* 
	Safely (allocs more memory) concatenates the two strings, 
	freeing the first.  Meant to build a new string of unknown length.
*/

	char *r;

	r = (char *)malloc(strlen(s)+strlen(t)+1);
	*r = '\0';
	strcpy(r,s);
	free(s);
	strcat(r,t);
	return r;
}


static char *
ReadLine(FILE  *f)
{
/* 
	Reads from the datastream, attempting to return a single string.
	Undoes quoting and broken lines.
	Warning:  this routine wasn't meant to handle embedded
	newlines.
	Warning:  possible source of memory leaks;  remember to 
	free the returned string when finished with it!
*/

	char buf[MAX_LINE_LENGTH], /* (BUG) What if the datastream is broken? */
	   buf2[MAX_LINE_LENGTH],
	  *result;
	int i,j;

	
	result = (char *)malloc(1);
	*result = '\0';

	while (fgets(buf,sizeof(buf),f)) {
	for (i = 0, j = 0; buf[i] != '\0'; ++i) {
	  switch (buf[i]) {
	  case '\\':
		/* Unquote backslash or splice line */
		switch (buf[++i]) {
		case '\\':
		  /* Unquote the backslash */
		  buf2[j++] = buf[i];
		  break;
		case '\n':
		  /* broke long line */
		  break;
		default:
		  /* things like \enddata come through here */
		  buf2[j++] = '\\';
		  buf2[j++] = buf[i];
		  break;
		} /* switch (buf[++i]) */
		break;
	  case '\n':
		/* An unquoted newline means end of string */
		buf2[j++] = '\0';
		result = GlomStrings(result, buf2);
		return(result);
	  default:
		buf2[j++] = buf[i];
		break;
	  } /* switch (buf[i]) */
	} /* for (i = 0, ...) */
	buf2[j++] = '\0';
	result = GlomStrings(result, buf2);
	} /* while (fgets...) */
	/* Should not get here... it means we went off the end
	 of the data stream.  Ooops. */
	return(NULL);
}

#ifdef PL8

static char *
EncodeFont(class link  *self)
{
/*
	Returns a string representing the name of the font for this object.
	(BUG) I shouldn't have to do this function, it should be a method
	of the fontdesc object.  In any case, I handle only Bold, Italic,
	and fixed styles.
*/

	char *buf, type[15];
	long myfonttype, myfontsize;
	char *myfontname;

	*type = '\0';
	myfontname = ((self)->GetButtonFont())->GetFontFamily();
	myfontsize = ((self)->GetButtonFont())->GetFontSize();
	myfonttype = ((self)->GetButtonFont())->GetFontStyle();
	if (myfonttype & fontdesc_Bold) strcpy(type,"b");
	if (myfonttype & fontdesc_Italic) strcpy(type,"i");
	if (myfonttype & fontdesc_Fixed) strcpy(type,"f");
	if (buf = (char *)malloc(strlen(myfontname)+25)) {
	sprintf(buf,"%s%d%s", myfontname, myfontsize, type);
	return (buf);
	} else {
	return(NULL);
	}
}

#endif /* PL8 */
