/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ossupport/aix3/RCS/setreuid.c,v 1.6 1992/12/15 21:16:51 rr2b Stab74 $";
#endif



/* can't do this since we have to build this
 before install, which is needed to install andrewos.h... #include <andrewos.h> */
#include <sys/types.h>
#include <sys/priv.h>
#include <sys/id.h>

priv_t priv;

atk_setuid(id)
  uid_t id;
{
    if(!id) {
	getpriv(PRIV_MAXIMUM, &priv,sizeof(priv_t));
	setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv,sizeof(priv_t));
	return (setuidx(ID_EFFECTIVE|ID_REAL|ID_SAVED, id));

    }
    else {
	priv.pv_priv[0] = 0;
	priv.pv_priv[1] = 0;
	setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv,sizeof(priv_t));
	return(setuidx(ID_EFFECTIVE|ID_REAL|ID_SAVED, id));
    }
}

