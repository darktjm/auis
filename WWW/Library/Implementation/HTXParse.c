/*								   HTXParse.c
**	EXTPARSE CLASS
**
**	(c) COPYRIGHT CERN 1994.
**	Please first read the full copyright statement in the file COPYRIGH.
**
**  AUTHORS:
**	HWL 23/8/94
*/

/* Library include files */
#include "tcp.h"
#include "HTUtils.h"
#include "HTFormat.h"			/* defines INPUT_BUFFER_SIZE */
#include "HTXParse.h"                 /* defines HTStreamClass */
#include "HTEPtoCl.h"               /* defines dummy routine for talking to client */

#if 0
/*extern void  (*HTCallClient)(HTExtParseStruct *eps);*/
#endif

extern CallClient HTCallClient;

struct _HTStream {
	CONST HTStreamClass *	isa;
	HTExtParseStruct *      eps;
};

PRIVATE void HTExtParse_put_character (HTStream * me, char c)
{
    while ((me->eps->used + 1) > me->eps->length) {
	me->eps->length += INPUT_BUFFER_SIZE;
    }
    me->eps->buffer = (char *) realloc(me->eps->buffer, me->eps->length);
    *(me->eps->buffer + me->eps->used) = c;
    me->eps->used++;
}

PRIVATE void HTExtParse_put_string (HTStream * me, CONST char* s)
{
    int l = strlen(s);

    if (TRACE) fprintf(TDEST, "HTExtParse_put_string, %s\n",s);

    while ((me->eps->used + l) > me->eps->length) {
	me->eps->length += INPUT_BUFFER_SIZE;
    }
    me->eps->buffer = (char *) realloc(me->eps->buffer, me->eps->length);
    memcpy( (me->eps->buffer + me->eps->used), s, l); 
    me->eps->used += l;
}

PRIVATE void HTExtParse_write (HTStream * me, CONST char* s, int l)
{
    while ((me->eps->used + l) > me->eps->length) {
	me->eps->length += INPUT_BUFFER_SIZE;
    }
    me->eps->buffer = (char *) realloc(me->eps->buffer, me->eps->length);
    memcpy( (me->eps->buffer + me->eps->used), s, l); 
    me->eps->used += l;
    (*me->eps->call_client)(me->eps);         /* so that client can give status info */
    
    if (TRACE)
	fprintf(TDEST, "HTExtParse_write, l=%d, used = %d\n",l,me->eps->used);
}


PRIVATE int HTExtParse_free (HTStream * me)
{
    if (TRACE) fprintf(TDEST, "HTExtParse_free\n");
    me->eps->finished = YES;
    (*me->eps->call_client)(me->eps);         /* client will free buffer */
    free(me->eps);
    free(me);
    return 0;
}

PRIVATE int HTExtParse_abort (HTStream * me, HTError e)
{
    if (TRACE)
	fprintf(TDEST, "HTExtParse_abort\n");
    HTExtParse_free(me);				  /* Henrik Nov 2 94 */
    return EOF;
}


/*	ExtParse stream
**	-----------------
*/


PRIVATE CONST HTStreamClass HTExtParseClass =
{		
	"ExtParse",
	HTExtParse_free,
	HTExtParse_abort,
	HTExtParse_put_character,
	HTExtParse_put_string,
	HTExtParse_write
}; 

/*
extern void SetBufferPt (char * p, int l);
extern void GiveReadStatus (char * p, int l);
*/

PUBLIC HTStream* HTExtParse (
	HTRequest *		request,
	void *			param,
	HTFormat		input_format,
	HTFormat		output_format,
	HTStream *		output_stream)
{
    HTStream* me;
  
    if (TRACE) {
	fprintf(TDEST, "HTExtConvert..");
	if (input_format && input_format->name)
            fprintf(TDEST, ".. input format is %s",input_format->name);
	if (output_format && output_format->name)
            fprintf(TDEST, ".. output format is %s",output_format->name);
	fprintf(TDEST, "\n");
    }

    me = (HTStream*)calloc(1, sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTExtConvert");
    me->isa = &HTExtParseClass;

    me->eps = (HTExtParseStruct *) calloc(1, sizeof(HTExtParseStruct));
    if (me->eps == NULL) outofmem(__FILE__, "HTExtConvert");

    me->eps->content_type = input_format->name;
    me->eps->call_client = HTCallClient;
    me->eps->buffer = (char *)calloc(INPUT_BUFFER_SIZE,1);
    me->eps->used = 0;
    me->eps->finished = NO;
    me->eps->length = INPUT_BUFFER_SIZE;
    me->eps->request = request;
    return me;
}
