#ifndef _wpeditapp_H_
#define _wpeditapp_H_
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


 
#ifndef _INCLUDED_WPI_H_
#define boolean dumbdumbboolean
#include <wpi.h>
#define _INCLUDED_WPI_H_ 1
#undef boolean
#endif

#include <application.H>
class wpeditapp : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();
	static boolean atk_inited;

    
	wpeditapp();
    
	virtual ~wpeditapp();

     boolean Start()  ;
     boolean ParseArgs(int argc, const char **argv)  ;

    class chlist *admin, *user; /* the admin and user panes */
    class text *desc;		/* the description pane */
    int adminflag;		/* is requestor an administrator? */
    WPI_entry_t entry;		/* WPI record for user */
    char EditUser[200];		/* user we're editing */
    char requestor[200];	/* user making changes */
    char reqdomain[200];	/* changer's domain */
};

#endif /* _wpeditapp_H_ */
