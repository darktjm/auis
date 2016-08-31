/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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
\* ********************************************************************** */
/* toksym.c		

	Code for the toksym object
*/
/*
 *    $Log: toksym.C,v $
// Revision 1.3  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.2  1993/05/18  17:08:29  rr2b
// Removed ::s before local variables.
//
// Revision 1.1  1993/05/06  16:57:06  rr2b
// Initial revision
//
 * Revision 1.10  1992/12/14  20:57:48  rr2b
 * disclaimerization
 *
 * Revision 1.9  1992/12/02  20:00:53  rr2b
 * fixed to not use xgetchar.h, and to include text.ih so it can use getunsignedchar
 * on the text
 * .
 *
 * Revision 1.8  1992/11/26  02:03:49  wjh
 * updated header
 * .
 *
. . .
 * Revision 1.0  88/07/14  13:22:05 WJHansen
 * Copied from sym.c and discarded EVERYTHING
 */



#include <andrewos.h>
ATK_IMPL("toksym.H")
#include <toksym.H>
#include <text.H>


	
ATKdefineRegistry(toksym, sym, NULL);
#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
#endif


toksym::toksym()
		{
	this->loc = 0;	/* (because error terminator set uses -3 as flag) */
	this->len = 0;
	this->info.obj = NULL;
	THROWONFAILURE( TRUE);
}


	toksym::~toksym()
		{
}

/* toksym__ToC(t, buf, maxlen)
	copies the token from the text t to the buffer buf, up to maxlen bytes
	returns buf 
*/
	char *
toksym::ToC(class text  *t, char  *buf , long  maxlen)
				{
	char *cx;
	long loc, len;
	cx = buf;
	loc = this->loc;
	len = this->len;
	if (len > maxlen-1) 
		len = maxlen - 1;
	for ( ;  len > 0;  len--, loc++)
		*cx++ = (t)->GetUnsignedChar( loc);
	*cx = '\0';
	return buf;
}
