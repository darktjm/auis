/*									 HTee.c
**	TEE CLASS STREAM DEFINITION
**
**	(c) COPYRIGHT CERN 1994.
**	Please first read the full copyright statement in the file COPYRIGH.
**
**	The Tee class just writes to two streams.  Easy.
**	See also the Black Hole stream which is even easier.
**
** HISTORY:
**	 8 Jul 94  FM	Insulate free() from _free structure element.
**
*/

/* Library include files */
#include "tcp.h"
#include "HTUtils.h"
#include "HTTee.h"

/*		Stream Object
**		------------
*/

struct _HTStream {
	CONST HTStreamClass *	isa;
	
	HTStream *		s1;
	HTStream *		s2;
};


PRIVATE void HTTee_put_character (HTStream * me, char c)
{
    (*me->s1->isa->put_character)(me->s1, c);
    (*me->s2->isa->put_character)(me->s2, c);
}
PRIVATE void HTTee_put_string (HTStream * me, CONST char* s)
{
    (*me->s1->isa->put_string)(me->s1, s);
    (*me->s2->isa->put_string)(me->s2, s);
}
PRIVATE void HTTee_write (HTStream * me, CONST char* s, int l)
{
    (*me->s1->isa->put_block)(me->s1, s, l);
    (*me->s2->isa->put_block)(me->s2, s, l);
}
PRIVATE int HTTee_free (HTStream * me)
{
    (*me->s1->isa->_free)(me->s1);
    (*me->s2->isa->_free)(me->s2);
    free(me);
    return 0;
}
PRIVATE int HTTee_abort (HTStream * me, HTError e)
{
    (*me->s1->isa->abort)(me->s1, e);
    (*me->s2->isa->abort)(me->s2, e);
    free(me);
    return EOF;
}


/*	Tee stream
**	----------
*/
PRIVATE CONST HTStreamClass HTTeeClass =
{		
	"Tee",
	HTTee_free,
	HTTee_abort,
	HTTee_put_character, 	HTTee_put_string,
	HTTee_write
}; 


/*	Tee creation
*/
PUBLIC HTStream * HTTee (HTStream * s1,HTStream * s2)
{
    HTStream * me = (HTStream*)malloc(sizeof(*me));
    if (!me) outofmem(__FILE__, "HTTee");
    me->isa = &HTTeeClass;
    me->s1 = s1;
    me->s2 = s2;
    return me;
}


