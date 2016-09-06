/*								      HTWrite.c
**	Write mime obj  
**	Based on HTPlain.c
**	Writes a minimal mime header before copying input to output.
**	Supports applications that want mime output from a non-mime source
**		(ftp, gopher, etc,)
**
**		Tom Neuendorffer tpn@cs.cmu.edu
**
**	(c) COPYRIGHT CERN 1994.
**	Please first read the full copyright statement in the file COPYRIGH.
**
**	This version of the stream object just writes to a socket.
**	The socket is assumed open and left open.
**
*/

/* Library include files */
#include "tcp.h"
#include "HTUtils.h"
#include "HTStyle.h"
#include "HTomime.h"
#include "HTStream.h"




/*		HTML Object
**		-----------
*/

struct _HTStream {
	CONST HTStreamClass *	isa;
	
	HTStream *		target;
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

PRIVATE void HTomime_put_character (HTStream *me, char c)
{
    (*me->target->isa->put_character)( me->target,  c);
}



/*	String handling
**	---------------
**
*/
PRIVATE void HTomime_put_string (HTStream *me, CONST char *s)
{
    (*me->target->isa->put_string) ( me->target, s);
}


PRIVATE void HTomime_write (HTStream *me, CONST char *s, int l)
{
    (*me->target->isa->put_block) ( me->target,  s,  l);
}



/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE int HTomime_free (HTStream *me)
{
    if (PROT_TRACE) {
	printf("HTtmime: ..... In free\n");
    }
    (*me->target->isa->_free)( me->target);
    free(me);
    return 0;
}

/*	End writing
*/

PRIVATE int HTomime_abort (HTStream *me, HTError e)
{
    if (PROT_TRACE) {
	printf("HTtmime: ..... In Abort\n");
    }
     (*me->target->isa->abort)( me->target,e);
    return EOF;
}



/*		Structured Object Class
**		-----------------------
*/
PUBLIC CONST HTStreamClass HTomime =
{		
	"www/mime",
	HTomime_free,
	HTomime_abort,
	HTomime_put_character, 	HTomime_put_string, HTomime_write,
}; 


/*		New object
**		----------
*/
PUBLIC HTStream* HTomimePresent (
	HTRequest *		request,
	void *			param,
	HTFormat		input_format,
	HTFormat		output_format,
	HTStream *		output_stream)
{
    char buf[1024];
    HTStream* me = (HTStream*)malloc(sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTomime_new");
    me->isa = &HTomime;
    me->target = output_stream;
    sprintf(buf,"MIME-version: 1.0\nContent-type: %s\n\n",HTAtom_name(input_format));
    (*me->target->isa->put_block)(me->target, buf,strlen(buf));
    return (HTStream*) me;
}


