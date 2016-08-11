

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/rpc3.9/demo/dir/RCS/dir_xdr.c,v 1.3 1992/12/15 21:53:21 rr2b Stab74 $";
#endif
#include <rpc/rpc.h>
#include "dir.h"


bool_t
xdr_nametype(xdrs, objp)
	XDR *xdrs;
	nametype *objp;
{
	if (!xdr_string(xdrs, objp, MAXNAMELEN)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_namelist(xdrs, objp)
	XDR *xdrs;
	namelist *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(struct namenode), xdr_namenode)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_namenode(xdrs, objp)
	XDR *xdrs;
	namenode *objp;
{
	if (!xdr_nametype(xdrs, &objp->name)) {
		return (FALSE);
	}
	if (!xdr_namelist(xdrs, &objp->next)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_readdir_res(xdrs, objp)
	XDR *xdrs;
	readdir_res *objp;
{
	if (!xdr_int(xdrs, &objp->errno)) {
		return (FALSE);
	}
	switch (objp->errno) {
	case 0:
		if (!xdr_namelist(xdrs, &objp->readdir_res_u.list)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}


