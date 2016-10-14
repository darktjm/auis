/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* lprruler.c		

	Code for the lprruler data object

*/


#include <andrewos.h>
ATK_IMPL("lprruler.H")
#include <lprruler.H>

	
ATKdefineRegistryNoInit(lprruler, dataobject);

long
lprruler::Read( FILE   *file, long   id			/* !0 if data stream, 0 if direct from file*/ )
			{
	/* reads a lprruler from -file-.  See file format in lprruler.ch */
	/* This routine reads the \enddata, if any. Its syntax is not checked */

	char s[255 + 2];
	char c;

	if ((c=getc(file)) == '\n') {}		/* COMPATIBILITY KLUDGE */
	else if (c == '\\') fgets(s, 255+2, file);	/* skip header */
	else ungetc(c, file);

	while (TRUE) {

		/* read the lines of the data stream */

		char *nl;
		if ((fgets(s, 255 + 2, file)) == 0) 
			/* EOF or error */
			break;
		if (*s == '\\') 
			/* \enddata */
			break;

		nl = s + strlen(s) - 1;		/* point at last character */
		if (*nl == '\n')
			*nl = '\0';	/* delete newline*/

		/* process an input line of the data stream */
		
	}
	(this)->NotifyObservers( lprruler_DATACHANGED);
	return dataobject::NOREADERROR;
}
	  
	long
lprruler::Write( FILE   *file, long   writeID, int   level )
		 		{
	char head[50];
	long id = (this)->GetID();
	if (this->GetWriteID() != writeID) {
		/* new instance of write, do it */
		this->SetWriteID(writeID);
		sprintf(head, "data{%s, %ld}\n", (this)->GetTypeName(), id);
		fprintf(file, "\\begin%s", head);

		/* no contents */

		fprintf(file, "\\end%s", head);
	}
	return id;
}

	lprruler::lprruler()
		{
	THROWONFAILURE( TRUE);
}

	lprruler::~lprruler()
		{
}
