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

/* ************************************************************ *\
	btstat.c
	Library routines for understanding results from other routines.
	Include file ``bt.h'' declares the procedures for clients.
	Include file ``btint.h'' declares common structures for the implementation modules.
\* ************************************************************ */

#include <andyenv.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <util.h>
#ifdef WHITEPAGES_ENV  /* avoid makedepend "errors" */
#include <btint.h>
#endif /* WHITEPAGES_ENV   */


char *bt_ErrorString(codevalue)
bt_ErrorCode codevalue;
{
    static char *BTErrs[bterr_MAX + 1 + 1] = {
	"Not an error",
	"Out of available memory",
	"Not a B-tree",
	"No file names left to use",
	"No such mode for bt_Open",
	"B-tree not the current version",
	"B-tree file is damaged",
	"File given to bt_Open is not the root to a tree",
	"No permission to lock",
	"Cursor tree damaged",
	"Internally generated name of file is too long",
	"Cursor not at key",
	"No next key",
	"Empty B-tree",
	"Internal inconsistency",
	"Opened as read-only",
	"Key to insert is already present",
	"Key to replace or delete is not present",
	"New Value is too large",
	"Key must not be zero length",
	"Old value different from specified value",
	"Cursor not initialized in bt_NextCursor",
	NULL };
	static char BTErrBuff[80];

	if (codevalue <= bterr_MAX && codevalue >= 0) return BTErrs[codevalue];
	if (codevalue >= bterr_FileSystemErrorBegin &&
	    codevalue <= bterr_FileSystemErrorEnd) {
	    sprintf(BTErrBuff, "B-tree: %s",
		    UnixError(codevalue - bterr_FileSystemErrorBegin));
	    return BTErrBuff;
	}
	sprintf(BTErrBuff, "B-tree error number %d", codevalue);
	return BTErrBuff;
}
