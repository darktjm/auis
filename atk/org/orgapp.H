ATK_INTER
#ifndef _orgapp_H_
#define _orgapp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Org Application-Class

MODULE	orga.ch

VERSION	0.0

NOTICE	IBM Internal Use Only

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Org Application-Class.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  01/19/89	Created (TCP)
  05/02/89	Change class-name to orgapp (TCP)

END-SPECIFICATION  ************************************************************/

#include <application.H>
class orgapp  : public  application 
  {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    orgapp();
	virtual ~orgapp();

  
     boolean Start()				     ;
     boolean ParseArgs( int argc, const char **argv )	     ;

  
    char				    source[512];
    class org				   *org_data_object;
    class orgv				   *org_view_object;
    class im				   *imp;
    class frame			   *framep;
  };


/*
    $Log: orgapp.H,v $
// Revision 1.5  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.4  1994/08/11  20:58:37  rr2b
// new fiasco
//
// Revision 1.3  1993/06/10  20:34:28  rr2b
// Great atk_inited removal.
//
 * Revision 1.2  1993/05/18  16:46:32  rr2b
 * Fixed SYNTAX_ERROR on InitializeObject (by removing it.)
 * Fixed argc argument to ParseArgs to be an int.
 *
*Revision 1.5  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.3  1991/09/12  19:45:34  bobg
Update copyright notice

Revision 1.2  1989/05/02  21:16:20  tom
Added BackgroundShade Attribute;
Added SetHitHandler method;
Added NodeName method;
ReSpelled class-name from orga to orgapp.

Revision 1.1  89/05/01  16:23:52  tom
Initial revision

*/

#endif /* _orgapp_H_ */
