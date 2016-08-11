#ifndef _gofigapp_H_
#define _gofigapp_H_
/* ********************************************************************** *\
 *         Copyright Carnegie Mellon University, 1992 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/* version number  */
#define MAJORVERSION 1
#define MINORVERSION 6


#include <application.H>
class gofigapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

	 boolean ParseArgs(int argc, char **argv)  ;
	 boolean Start()  ;
	
	static boolean InitializeClass();
	gofigapp();
	virtual ~gofigapp();

	char *inputfile;
	class gofig *dobj;
};

#endif /* _gofigapp_H_ */
