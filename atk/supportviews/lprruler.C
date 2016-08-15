/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/supportviews/RCS/lprruler.C,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 

/* lprruler.c		

	Code for the lprruler data object

*/


ATK_IMPL("lprruler.H")
#include <lprruler.H>

	
ATKdefineRegistry(lprruler, dataobject, NULL);
#ifndef NORCSID
#endif


long
lprruler::Read( register FILE   *file, register long   id			/* !0 if data stream, 0 if direct from file*/ )
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
	return dataobject_NOREADERROR;
}
	  
	long
lprruler::Write( FILE   *file, long   writeID, int   level )
		 		{
	char head[50];
	long id = (this)->UniqueID();
	if (this->writeID != writeID) {
		/* new instance of write, do it */
		this->writeID = writeID;
		sprintf(head, "data{%s, %d}\n", (this)->GetTypeName(), id);
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
