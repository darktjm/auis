/*							              HTWrite.c
**	FILE WRITER BASED ON A SOCKFD
**
**	(c) COPYRIGHT CERN 1994.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/* Library include files */
#include "tcp.h"
#include "HTUtils.h"
#include "HTString.h"
#include "HTWriter.h"

#define BUFFER_SIZE 4096	/* Tradeoff */

/*		HTML Object
**		-----------
*/

struct _HTStream {
	CONST HTStreamClass *	isa;

	SOCKFD	soc;
	char	*write_pointer;
	char 	buffer[BUFFER_SIZE];
	BOOL	leave_open;
#ifdef NOT_ASCII
	BOOL	make_ascii;	/* Are we writing to the net? */
#endif
};


/*	Write the buffer out to the socket
**	----------------------------------
*/

PRIVATE void flush (HTStream * me)
{
    char *read_pointer 	= me->buffer;
    char *write_pointer = me->write_pointer;

#ifdef NOT_ASCII
    if (me->make_ascii) {
    	char * p;
	for(p = me->buffer; p < me->write_pointer; p++)
	    *p = TOASCII(*p);
    }
#endif
    while (read_pointer < write_pointer) {
        int status = NETWRITE(me->soc, read_pointer,
			      write_pointer - read_pointer);
	if (status < 0) {
	    if (PROT_TRACE)
		fprintf(TDEST, "WriteStream.. Error writing to target\n");
	    return;
	}
	read_pointer += status;
    }
    me->write_pointer = me->buffer;
    return;
}


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

PRIVATE void HTWriter_put_character (HTStream * me, char c)
{
    if (me->write_pointer == &me->buffer[BUFFER_SIZE]) flush(me);
    *me->write_pointer++ = c;
}



/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/
PRIVATE void HTWriter_put_string (HTStream * me, CONST char* s)
{
    int l = strlen(s);
    if (me->write_pointer + l > &me->buffer[BUFFER_SIZE]) flush(me);
    strcpy(me->write_pointer, s);
    me->write_pointer = me->write_pointer + l;
}


/*	Buffer write.  Buffers can (and should!) be big.
**	------------
*/
PRIVATE void HTWriter_write (HTStream * me, CONST char* s, int l)
{
 
    CONST char *read_pointer 	= s;
    CONST char *write_pointer = s+l;

    flush(me);		/* First get rid of our buffer */

#ifdef NOT_ASCII
    if (me->make_ascii) {
    	char * p;
	for(p = me->buffer; p < me->write_pointer; p++)
	    *p = TOASCII(*p);
    }
#endif
    while (read_pointer < write_pointer) {
        int status = NETWRITE(me->soc, read_pointer,
			write_pointer - read_pointer);
	if (status<0) {
	    if(TRACE) fprintf(TDEST,
	    "HTWriter_write: Error on socket output stream!!!\n");
	    return;
	}
	read_pointer = read_pointer + status;
    }
}




/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE int HTWriter_free (HTStream * me)
{
    flush(me);
    if (!me->leave_open)
	NETCLOSE(me->soc);
    free(me);
    return 0;
}

PRIVATE int HTWriter_abort (HTStream * me, HTError e)
{
    HTWriter_free(me);
    return EOF;
}


/*	Structured Object Class
**	-----------------------
*/
PRIVATE CONST HTStreamClass HTWriter = /* As opposed to print etc */
{		
	"SocketWriter",
	HTWriter_free,
	HTWriter_abort,
	HTWriter_put_character, HTWriter_put_string,
	HTWriter_write
}; 


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStream* HTWriter_new (SOCKFD soc)
{
    HTStream* me = (HTStream*)calloc(1,sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTWriter_new");
    me->isa = &HTWriter;       
    
#ifdef NOT_ASCII
    me->make_ascii = NO;
#endif    
    me->soc = soc;
    me->write_pointer = me->buffer;
    return me;
}

PUBLIC HTStream* HTWriter_newNoClose (SOCKFD soc)
{
    HTStream * me = HTWriter_new(soc);
    if (me) me->leave_open = YES;
    return me;
}


/*	Subclass-specific Methods
**	-------------------------
*/

PUBLIC HTStream* HTASCIIWriter (SOCKFD soc)
{
    HTStream* me = (HTStream*)calloc(1,sizeof(*me));
    if (me == NULL) outofmem(__FILE__, "HTML_new");
    me->isa = &HTWriter;       

#ifdef NOT_ASCII
    me->make_ascii = YES;
#endif    
    me->soc = soc;
    me->write_pointer = me->buffer;
    return me;
}

