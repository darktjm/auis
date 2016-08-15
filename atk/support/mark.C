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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/support/RCS/mark.C,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


ATK_IMPL("mark.H")
#include <mark.H>

/* Crank out marks in 4k blocks. */
#define DESIREDBLOCKSIZE 4096
/* Number of marks per block */
#define NUMPERBLOCK DESIREDBLOCKSIZE / sizeof(class mark)
#define BLOCKSIZE NUMPERBLOCK * sizeof(class mark)

static class mark *freeMarks = NULL;
static class mark *lastBlock = NULL;


ATKdefineRegistry(mark, ATK, NULL);
#ifndef NORCSID
#endif


class mark *mark::Allocate()
{

    static int lastIndex = NUMPERBLOCK; /* Force a block malloc on first call. */

    if (freeMarks) {
        class mark *tempMark = freeMarks;
        freeMarks = freeMarks->next;
        return tempMark;
    }
    if (lastIndex >= NUMPERBLOCK) {
        lastBlock = (class mark *) malloc(BLOCKSIZE);
        lastIndex = 0;
    }
    return &lastBlock[lastIndex++];
}

void mark::Deallocate(class mark  *self)
    {

    self->next = freeMarks;
    freeMarks = self;
}

mark::mark()
{
    this->next = NULL;
    this->pos = 0;
    this->length = 0;
    this->modified = FALSE;
    this->objectFree = FALSE;
    this->includeBeginning = FALSE;
    this->includeEnding = TRUE;

    THROWONFAILURE( TRUE);
}

void mark::SetStyle(boolean  beginning, boolean  ending  )
{
    this->includeBeginning = beginning;
    this->includeEnding = ending;
}

class mark *mark::NewWithStyle(boolean  beginning, boolean  ending  )
{
    class mark *nmark;

    nmark = new mark;
    nmark->includeBeginning = beginning;
    nmark->includeEnding = ending;
    return nmark;
}

void mark::UpdateMarks(long  pos, long  size)
            {

    class mark *mark;
    long tpos;
    long tsize;
    long endPos;

    if (size == 0) return;
    
    for (mark = this; mark != NULL; mark = (mark)->GetNext())  {
	tpos = pos;
	tsize = size;
	if (tpos <= (endPos = (mark)->GetEndPos()))  {
	    if (tsize > 0)  {
		if (tpos == endPos)  {
		    if ((mark)->IncludeEnding())  {
			(mark)->SetModified( TRUE);
			(mark)->SetLength( (mark)->GetLength() + tsize);
		    }
		}
		else if (tpos < (mark)->GetPos() || tpos == (mark)->GetPos() && ! (mark)->IncludeBeginning())
		    (mark)->SetPos( (mark)->GetPos() + tsize);
		else {
		    (mark)->SetLength( (mark)->GetLength() + tsize);
		    (mark)->SetModified( TRUE);
		}
	    }
	    else if (tpos < endPos) {
		if (tpos-tsize <= (mark)->GetPos())  {
/* 		    Deleted region is before the mark
 */		    
		    (mark)->SetPos( (mark)->GetPos() + tsize);
		}
		else  {
		    if (tpos <= (mark)->GetPos())  {
/* 			Delete portion before the mark
 */			
		    tsize += (mark)->GetPos() - tpos;
			(mark)->SetPos( tpos);
		    }
		    
/* 		    Reset the size of the deleted region to only include the mark
 */		    
		    if (tpos - tsize > endPos)
			tsize = tpos - endPos;
			
/* 		    Delete the characters from the mark
 */		    
		    (mark)->SetLength( (mark)->GetLength()+ tsize);
		    if ((mark)->GetLength() < 0) (mark)->SetLength( 0);
		    (mark)->SetModified( TRUE);
		}
	    }
	}
    }
}
