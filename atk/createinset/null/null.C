/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	   Copyright Carnegie Mellon, 1992 - All Rights Reserved
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

/* null.c		

	Code for the null data object
*/
/*
 *    $Log: null.C,v $
 * Revision 1.3  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.2  1993/06/05  00:32:29  rr2b
 * Converted to C++
 *
 * Revision 1.1  1993/06/04  23:26:51  rr2b
 * Initial revision
 *
 * Revision 1.6  1992/12/15  21:32:24  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.5  1992/12/14  20:40:31  rr2b
 * disclaimerization
 *
 * Revision 1.4  1992/06/22  21:48:52  wjh
 * fixed so it doesn't give MISSINGENDDATAMARKER
 * revised description of __Read
 * .
 *
 * Revision 1.3  1991/09/12  16:11:39  bobg
 * Update copyright notice and rcsid
 *
 * Revision 1.2  1991/08/22  19:03:51  wjh
 * fix comparison in if(*nl=='\n')
 *
 * .
 *
 * Revision 1.1  1989/07/31  15:34:47  wjh
 * Initial revision
 *
 *  
 * Revision 1.0  88/05/14  15:40:33  wjh
 * Copied from /usr/andrew/lib/genericinset
 */

#include <andrewos.h>
ATK_IMPL("null.H")
#include <dataobject.H>
/* $$$ include  xxx.ih  for any routine called below as xxx_routine() */

#include <null.H>


#define MAXFILELINE 255

/* $$$ */
	
ATKdefineRegistry(null, dataobject, null::InitializeClass);

void
null::ClearDots()
	{
	register struct dotlist *d;
	for (d = (this)->GetFirstDot();  d != NULL; )  {
		register struct dotlist *t = (this)->GetNextDot( d);
		free(d);
		d = t;
	}
	this->dots = NULL;
	(this)->SetModified();	/* tell the enclosing document I have changed */
}

/* $$$ */
	void
null::AddDot(long  x , long  y)
		{
	register struct dotlist *d;
	d = (struct dotlist *)malloc(sizeof (struct dotlist));
	d->next = (this)->GetFirstDot();
	d->x = x;
	d->y = y;
	this->dots = d;
	(this)->SetModified();	/* tell the enclosing document I have changed */
}


	boolean
null::InitializeClass()
	{
	/* initialize any global data used in this module */
	return TRUE;
}

	null::null()
		{
	ATKinit;

	/* here we give initial values to any fields that need them */
	
	this->dots = NULL;	/* $$$ */

	THROWONFAILURE( TRUE);
}

	null::~null()
		{
	ATKinit;

	/* free any storage allocated to self */
	(this)->ClearDots();	/* $$$ */
}

/* null__Read(self, file, id)
	Reads the data for 'self' from 'file',  see format in null.H
	If 'id' is 0, the data stream is at the outermost level and thus might
	be in an alternate format (see null__Write).  In this data object,
	the format is always the same.

	If 'id' is non-zero, the first line of the data stream will have been read
	by the caller and the value found in the line is passed as 'id'.  This must
	match the id value found in the enddata line.

	Returns an int indication of success.  (See dataobj.ch)
	Fails if syntax of headers is incorrect.
*/
	long
null::Read(register FILE   *file, register long   id)
			{
	long result = dataobject_BADFORMAT;
	char s[MAXFILELINE + 2];
	long len;
	long sid, eid;

	if (id == 0) {
		if (fscanf(file, "\\begindata{null,%ld", &sid) != 1
				|| getc(file) != '}' || getc(file) != '\n') 
			return dataobject_NOTBE2DATASTREAM;
	}
	else {
		int c;
		sid = id;

		/*  The parent is supposed to have read the entire 
			header including the final \n.
		    Some ancient insets did not read the final \n,
			so we check and discard it here.
		    This must be removed if the internal data stream,
			might begin with \n.
		*/
		if ((c=getc(file)) != '\n')
			ungetc(c, file);
	}

	(this)->ClearDots();	/* $$$  initialize data structure */

	while (TRUE) {
		/* read the lines of the data stream */
		long x, y;

		char *nl;
		if ((fgets(s, MAXFILELINE + 2, file)) == 0) {
			/* EOF or error */
			result = dataobject_PREMATUREEOF;
			break;
		}
		if (*s == '\\') {
			/* \enddata */
			result = dataobject_NOREADERROR;
			break;
		}

		nl = s + strlen(s) - 1;		/* point at last character */
		if (*nl == '\n')
			*nl = '\0';	/* delete newline*/

		/* $$$ process an input line of the data stream */
		sscanf(s, "%d %d", &x, &y);
		(this)->AddDot( x, y);

	}
	(this)->NotifyObservers( null_DATACHANGED);
	(this)->SetModified();	/* tell the enclosing document I have changed */


	len = strlen("\\enddata{null,");
	if (result == dataobject_NOREADERROR &&
			( strncmp(s, "\\enddata{null,", len) != 0
			  || sscanf(s+len, "%d}\n", &eid) != 1
			  || eid != sid
			) ) 
		result = dataobject_MISSINGENDDATAMARKER;

	return result;
}


/* null__Write(self, file, writeID, level)
	Writes a data stream representation of 'self' to 'file'
	The same 'writeID' value is passed in for all writes during a given
	datastream construction.  Once this object has been written, it need 
	not be written again.

	The 'level' can be ignored.  (A level of 0 indicates that the
	output is to be the sole contents of a file, so the format could be 
	non-datastream.  This is only used to support data streams 
	defined prior to the base editor.)
	
	Returns the identifying number written into the header.  The caller will 
	use this number in referring to the object from the dictionary.
*/
	long
null::Write(FILE   *file, long   writeID, int   level)
		 		{
	char head[50];
	long id = (this)->GetID();
	if (this->writeID != writeID) {
		/* new instance of write, do it */
		register struct dotlist *d;
		this->writeID = writeID;
		sprintf(head, "data{%s, %d}\n", (this)->GetTypeName(), id);
		fprintf(file, "\\begin%s", head);


		/* $$$ output lines of data stream */
		for (d = (this)->GetFirstDot();  d != NULL; 
				d = (this)->GetNextDot( d))  
			fprintf(file, "%d %d\n", (this)->GetDotX( d),
					(this)->GetDotY( d));


		fprintf(file, "\\end%s", head);
	}
	return id;
}
