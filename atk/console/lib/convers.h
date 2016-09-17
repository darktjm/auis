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


 

/* IMPORTANT  -- IF YOU CHANGE THE VERSION NUMBER BELOW, YOU NOW ALSO HAVE TO CHANGE IT IN ../cmd/consolea.c, in the InitializeObject procedure.  */
#define CONSOLE_VERSION " [7.6]" 
/* Don't forget leading space in version number*/
/* version information is contained in the next two defines, 
 * bump the minor version when we get significant bug fixes,
 * major version changes require negotiation- whoever is in charge of version control
 * should choose this in accordance with the version chosen for the system. as a whole
 * NOTE: The way this is written, you also need to make lib/convers.h conform to 
 * these numbers explicitly.
 */
#define MINORVERSION 6
#define MAJORVERSION 7
