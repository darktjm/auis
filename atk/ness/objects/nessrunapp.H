ATK_INTER

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
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
#ifndef _nessrunapp_H_
#define _nessrunapp_H_

#include <application.H>
class nessrunapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

	boolean ParseArgs(int argc, char **argv)  ;
	boolean Start()  ;
	int Run()  ;
	
	nessrunapp();
	virtual ~nessrunapp();

	char *inputfile;
	class ness *theNess;
	boolean dump;
};

#endif /* _nessrunapp_H_ */