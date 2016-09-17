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

#define class_StaticEntriesOnly

#include <andrewos.h>
#include <buffer.H>
#include <completion.H>
#include <dictionary.H>
#include <environment.H>
#include <mark.H>
#include <nestedmark.H>
#include <rectlist.H>
#include <style.H>
#include <stylesheet.H>
#include <tree23int.H>
#include <viewref.H>

int support_Initialize();


int support_Initialize()
{
    buffer_StaticEntry;
    completion_StaticEntry;
    dictionary_StaticEntry;
    environment_StaticEntry;
    mark_StaticEntry;
    nestedmark_StaticEntry;
    rectlist_StaticEntry;
    style_StaticEntry;
    stylesheet_StaticEntry;
    tree23int_StaticEntry;
    viewref_StaticEntry;

    return 1;
}
