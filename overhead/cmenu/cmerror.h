/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/* Some of the values in this file are obsolete. Take them with a grain of salt. */

#define cm_FAILURE		-1
#define cm_SUCCESS		1
#define cm_NO_SELECT		2
#define cm_IA_SELECT		3

#define cmE_CODE_COUNT		13

#define cmE_NO_ERROR		0
#define cmE_NOT_INIT		1
#define cmE_ARG_BOUNDS		2
#define cmE_P_NOT_FOUND		3
#define cmE_S_NOT_FOUND		4
#define cmE_CREATE_GC           5
#define cmE_CALLOC		6
#define cmE_CREATE_ASSOC	7
#define cmE_MAKE_PIXMAP		8
#define cmE_CREATE_CURSOR	9
#define cmE_OPEN_FONT		10
#define cmE_CREATE_WINDOW	11
#define cmE_CREATE_INPUTONLY	12

/*
 * cmenu error code and error list definitions.
 */
extern int _cmErrorCode;
extern const char * const _cmErrorList[];
