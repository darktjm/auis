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

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	Bush Internal Symbolics

MODULE	bush.h

VERSION	0.0

AUTHOR	TC Peters & GW Keim
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This internal symbolics for the Bush modules.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  08/21/85	Created (TCP)
  01/15/89	Convert to ATK (GW Keim)

END-SPECIFICATION  ************************************************************/
#include <stdio.h>
#include <andrewos.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <ctype.h>

#if defined(POSIX_ENV) || defined(M_UNIX)
#include <dirent.h>
#else
#define dirent direct
#endif

#include <errno.h>
#include <apt.h>






/*
    $Log: bush.h,v $
 * Revision 1.2  1993/06/30  05:04:24  rr2b
 * Removed #include of sys/dir.h, should be handled by andrwos.h
 *
 * Revision 1.1  1993/05/19  15:56:49  rr2b
 * Initial revision
 *
 * Revision 1.12  1992/12/14  23:20:33  rr2b
 * add $Logs back after disclaimerization took them out
 *
 * Revision 1.10  1992/08/11  16:38:29  gk5g
 * Small change from Todd Inglett
 * .
 *
 * Revision 1.9  1992/08/04  18:40:44  gk5g
 * Posixified
 * .ake
 *
 * Revision 1.8  1992/07/24  22:18:18  gk5g
 * Posixified
 * .ake
 *
 * Revision 1.7  1991/09/12  19:25:09  bobg
 * Update copyright notice
 *
 * Revision 1.6  1991/07/22  03:52:09  gk5g
 * #include <afs/param.h> for Ultrix4.x
 *
 * Revision 1.5  1991/04/02  16:54:02  susan
 * added SCO Unix support
 *
 * Revision 1.4  90/04/27  12:11:34  gk5g
 * Sweeping revisions including: (1) reformated, (2) added editing capability while viewing a file, (3) cleaned up unused variables, (4) additional information added to the ControlPanel (sort, detail, editor).
 * 
 * Revision 1.3  90/03/14  15:29:29  gk5g
 * macros for improved readability.
 * 
 * Revision 1.3  90/03/14  14:36:39  gk5g
 * Some added macros for improved readability.
 * 
 * Revision 1.2  89/09/01  13:30:33  gk5g
 * Made change from tree_NodeData to tree_NodeDatum.
 * 
 * Revision 1.1  89/04/28  21:24:00  tom
 * Initial revision
 * 
 * Revision 1.1  89/04/28  21:20:38  tom
 * Initial revision
 * 
*/
