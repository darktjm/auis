/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	   Copyright Carnegie Mellon, 1992 - All Rights Reserved          *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* null.C

	Code for the null data object
*/

#include <andrewos.h>
ATK_IMPL("null.H")
#include <dataobject.H>
/* $$$ include headers for all external references (don't use extern!) */

#include <null.H>


#define MAXFILELINE 255

/* $$$ */
	
ATKdefineRegistry(null, dataobject, null::InitializeClass);

void
null::ClearDots()
	{
	struct dotlist *d;
	for (d = (this)->GetFirstDot();  d != NULL; )  {
		struct dotlist *t = (this)->GetNextDot( d);
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
	struct dotlist *d;
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
null::Read(FILE   *file, long   id)
			{
	long result = dataobject::BADFORMAT;
	char s[MAXFILELINE + 2];
	long len;
	long sid, eid;

	if (id == 0) {
		if (fscanf(file, "\\begindata{null,%ld", &sid) != 1
				|| getc(file) != '}' || getc(file) != '\n') 
			return dataobject::NOTATKDATASTREAM;
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
			result = dataobject::PREMATUREEOF;
			break;
		}
		if (*s == '\\') {
			/* \enddata */
			result = dataobject::NOREADERROR;
			break;
		}

		nl = s + strlen(s) - 1;		/* point at last character */
		if (*nl == '\n')
			*nl = '\0';	/* delete newline*/

		/* $$$ process an input line of the data stream */
		sscanf(s, "%ld %ld", &x, &y);
		(this)->AddDot( x, y);

	}
	(this)->NotifyObservers( null_DATACHANGED);
	(this)->SetModified();	/* tell the enclosing document I have changed */


	len = strlen("\\enddata{null,");
	if (result == dataobject::NOREADERROR &&
			( strncmp(s, "\\enddata{null,", len) != 0
			  || sscanf(s+len, "%ld}\n", &eid) != 1
			  || eid != sid
			) ) 
		result = dataobject::MISSINGENDDATAMARKER;

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
		struct dotlist *d;
		this->writeID = writeID;
		sprintf(head, "data{%s, %ld}\n", (this)->GetTypeName(), id);
		fprintf(file, "\\begin%s", head);


		/* $$$ output lines of data stream */
		for (d = (this)->GetFirstDot();  d != NULL; 
				d = (this)->GetNextDot( d))  
			fprintf(file, "%ld %ld\n", (this)->GetDotX( d),
					(this)->GetDotY( d));


		fprintf(file, "\\end%s", head);
	}
	return id;
}
