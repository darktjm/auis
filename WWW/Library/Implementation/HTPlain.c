/*								      HTWrite.c
**	PLAIN TEXT OBJECT
**
**	(c) COPYRIGHT CERN 1994.
**	Please first read the full copyright statement in the file COPYRIGH.
**
**	This version of the stream object just writes to a socket.
**	The socket is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/

/* Library include files */
#include "tcp.h"
#include "HTUtils.h"
#include "HText.h"
#include "HTStyle.h"
#include "HTPlain.h"

#define BUFFER_SIZE 4096;	/* Tradeoff */

extern HTStyleSheet * styleSheet;



/*		HTML Object
**		-----------
*/

struct _HTStream {
	CONST HTStreamClass *	isa;

	HText * 		text;
};

/*	Write the buffer out to the socket
**	----------------------------------
*/


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

PRIVATE void HTPlain_put_character (HTStream * me, char c)
{
    HText_appendCharacter(me->text, c);
}



/*	String handling
**	---------------
**
*/
PRIVATE void HTPlain_put_string (HTStream * me, CONST char* s)
{
    HText_appendText(me->text, s);
}


PRIVATE void HTPlain_write (HTStream * me, CONST char* s, int l)
{
    CONST char* p;
    CONST char* e = s+l;
    for (p=s; p<e; p++) HText_appendCharacter(me->text, *p);
}



/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE int HTPlain_free (HTStream * me)
{
    free(me);
    return 0;
}

/*	End writing
*/

PRIVATE int HTPlain_abort (HTStream * me, HTError e)
{
    HTPlain_free(me);
    return EOF;
}



/*		Structured Object Class
**		-----------------------
*/
PUBLIC CONST HTStreamClass HTPlain =
{		
	"PlainText",
	HTPlain_free,
	HTPlain_abort,
	HTPlain_put_character, 	HTPlain_put_string, HTPlain_write,
}; 


/*		New object
**		----------
*/
PUBLIC HTStream* HTPlainPresent (
	HTRequest *		request,
	void *			param,
	HTFormat		input_format,
	HTFormat		output_format,
	HTStream *		output_stream)
{

    HTStream* me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTPlain_new");
    me->isa = &HTPlain;       

    me->text = HText_new2(request->anchor, output_stream);
    HText_beginAppend(me->text);
    HText_setStyle(me->text, HTStyleNamed(styleSheet, "Example"));

    return (HTStream*) me;
}


