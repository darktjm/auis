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

/*
  definitions shared between loadserv server and loadav client
*/
#include <snap.h>

#define SNAP_INTEGER long

/*opcode from client to server to ask load*/
#define OP_LOAD_REQ (1)

#define NETADDR struct sockaddr_in

/*maximum size of a request*/
#define SNAP_BUF_SIZE (8000)
extern int SNAP_debugmask;
#define getint		SNAP_ExtractIntFromMsg
extern char *SNAP_ExtractIntFromMsg();
#define getstr		SNAP_ExtractStringFromMsg
extern char *SNAP_ExtractStringFromMsg();
#define getbytes	SNAP_ExtractBytesFromMsg
extern char *SNAP_ExtractBytesFromMsg();
#define putint		SNAP_AppendIntToMsg
extern char *SNAP_AppendIntToMsg();
#define putstr		SNAP_AppendStringToMsg
extern char *SNAP_AppendStringToMsg();
#define putbytes	SNAP_AppendBytesToMsg
extern char *SNAP_AppendBytesToMsg();
