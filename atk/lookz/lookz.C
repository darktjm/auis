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

/* lookz.c		

	Code for the lookz data object
*/

#include <andrewos.h>
ATK_IMPL("lookz.H")
#include <dataobject.H>	/* for dataobject_NOREADERROR */
#include <text.H>
#include <lookz.H>

#define MAXFILELINE 255

	
ATKdefineRegistry(lookz, dataobject, NULL);

lookz::lookz()
		{
	this->visible = TRUE;
	this->canClose = TRUE;
	this->text = NULL;
	THROWONFAILURE( TRUE);
}

	lookz::~lookz()
		{
	if(this->text != NULL) {
	    (this->text)->RemoveObserver( this);
	    this->text = NULL;
	}
}

	long
lookz::Read( register FILE   *file, register long   id			/* !0 if data stream, 0 if direct from file*/ )
			{
	/* reads a lookz from -file-.  See file format in lookz.ch */
	/* This routine reads the \enddata, if any. Its syntax is not checked */

	char s[MAXFILELINE + 2];
	int c;

	if ((c=getc(file)) == '\n') {}		/* COMPATIBILITY KLUDGE */
	else if (c == '\\')
	    fgets(s, MAXFILELINE+2, file);	/* skip header */
	else ungetc(c, file);

	while (TRUE) {
		/* read the lines of the data stream */
		if ((fgets(s, MAXFILELINE + 2, file)) == 0) 
			/* EOF or error */
			break;
		if (*s == '\\') 
			/* \enddata */
			break;

		(this)->SetVisibility( strncmp(s, "visible", strlen("visible")) == 0);	
	}
	return dataobject_NOREADERROR;
}
	  
	long
lookz::Write( FILE   *file, long   writeID, int   level )
		 		{
	char head[50];
	long id = (this)->UniqueID();
	if (this->writeID != writeID) {
		/* new instance of write, do it */
		this->writeID = writeID;
		sprintf(head, "data{%s, %ld}\n", (this)->GetTypeName(), id);
		fprintf(file, "\\begin%s", head);

		fprintf(file, "%s\n", ((this)->GetVisibility() ? "visible" : "hidden"));

		fprintf(file, "\\end%s", head);
	}
	return id;
}

	void
lookz::SetVisibility(boolean  visibility)
		{
	/* this routine ensures that self->visible will be either TRUE or FALSE */
	if (this->visible != (visibility ? TRUE : FALSE)) {
		this->visible = (visibility ? TRUE : FALSE);
		(this)->NotifyObservers( lookz_VISIBILITYCHANGED);
	}
}

void lookz::SetCanClose(boolean  canClose)
{
    if (this->canClose != canClose) {
	this->canClose = canClose;
	(this)->SetVisibility( TRUE);
	(this)->NotifyObservers( lookz_CANCLOSECHANGED);
    }
}

void lookz::SetTextObject(class text  *text)
{
    if (this->text != text) {
	if (this->text != NULL) {
	    (this->text)->RemoveObserver( this);
	}
	this->text = text;
	if (text != NULL) {
	    (text)->AddObserver( this);
	}
	(this)->NotifyObservers( lookz_TEXTOBJECTCHANGED);
    }
}

void lookz::ObservedChanged(class observable  *dobj, long  value)
{
    if (value == observable_OBJECTDESTROYED &&
	 (class observable *) this->text == dobj) {
	(this->text)->RemoveObserver( this);
	this->text = NULL;
	(this)->NotifyObservers( lookz_TEXTOBJECTCHANGED);
    }
}

	
