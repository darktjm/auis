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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/rofftext/RCS/mmtext.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


/* ********************************************************************** *\
 *         Copyright AT&T Bell Laboratories - All Rights Reserved         *
 *        For full copyright information see:'andrew/config/COPYRITE.att' *
\* ********************************************************************** */

/* mmtext: link to rofftext -mm. */



ATK_IMPL("mmtext.H")
#include <buffer.H>
#include <environ.H>
#include <mark.H>
#include <rofftext.H>
#include <text.H>

#include <mmtext.H>

#define INITIAL 100
#define INCREMENT 1000


ATKdefineRegistry(mmtext, rofftext, NULL);
#ifndef NORCSID
#endif


mmtext::mmtext()
{
    this->nLines = 0;
    this->nAlloc = INITIAL;
    this->filename[0] = '\0';
    this->lineMark = (class mark **)malloc(this->nAlloc*sizeof(class mark *));
    if (this->lineMark == NULL) THROWONFAILURE( FALSE);
    (this)->SetLinePos( 0L, 0L);
    THROWONFAILURE( TRUE);
}

long mmtext::Read(FILE  *file, long  id )
{
    long tmpRetValue;
    class rofftext *r = (class rofftext *)this;
    class buffer *buf = buffer::FindBufferByData(this);
    char **t;

    /* copy the filename that was put into rofftext via SetAttributes */
    if (r->filename != NULL) strcpy(this->filename, r->filename);
    r->inputfiles = t = (char **)malloc(2 * sizeof(char *));
    t[0] = NULL;
    t[1] = NULL;
    r->filename = NULL;

    r->macrofile = environ::AndrewDir("/lib/tmac/tmac.m");
    r->RoffType = FALSE;
    r->HelpMode = FALSE;

    tmpRetValue = (r)->rofftext::Read( file, (long)r); 

    /* set read-only for buffer */
    if (buf != NULL) {
	class text *text = (class text *)(buf)->GetData();
	(buf)->SetReadOnly( TRUE);
	if (ATK::IsTypeByName((text)->GetTypeName(), "text")) {
	    (text)->SetReadOnly( TRUE);
	}
    }

    return tmpRetValue;

}

long mmtext::GetLinePos(long  line )
{
    line -= 2;
    if (line > this->nLines) line = this->nLines;
    if (line < 0) line = 0;
    return (this->lineMark[line])->GetPos();
}

void mmtext::SetLinePos(long  line, long  pos )
{
    if (line < 0) return;
    if (line >= this->nAlloc) {
	this->nAlloc += INCREMENT;
	this->lineMark = (class mark **)realloc(this->lineMark, this->nAlloc*sizeof(class mark *));
    }
    if (this->lineMark == NULL) return;
    this->lineMark[line] = (this)->CreateMark( pos, 0);
    this->nLines = line;
    /*  must make sure we have not skipped some */
}


void mmtext::GetFilename(char  *filename )
{
    strcpy(filename, this->filename);
}
