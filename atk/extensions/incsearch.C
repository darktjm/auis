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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/incsearch.C,v 3.2 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 



ATK_IMPL("incsearch.H")
#include <bind.H>
#include <text.H>
#include <textview.H>
#include <mark.H>
#include <search.H>
#include <message.H>
#include <im.H>
#include <environ.H>

#include <incsearch.H>

static boolean useSelectionRegion;

#define MAXSTRING 256

static char LastString[MAXSTRING] = {0};


ATKdefineRegistry(incsearch, ATK, incsearch::InitializeClass);
#ifndef NORCSID
#endif
void strappend(char  *dst , char  *src, int  ch , int  len);
static boolean dosearch(class textview  *tv, class text  *txt, class mark  *pos, char  *string , boolean  forwardp, boolean  contForward, boolean  contBackward, char  *errmsg);
static long incsearchf(ATK *tva, long  key);


void strappend(char  *dst , char  *src, int  ch , int  len)
{
    /* copy as much of the source as we can. */
    while (--len > 0 && (*dst = *src++) != '\0')
        dst++;

    /* If there is room for the character, add it. */
    if (len > 0)
        *dst++ = ch;

    /* Add the null. Note, the above tests should leave us room. */
    *dst = '\0';
}

static boolean dosearch(class textview  *tv, class text  *txt, class mark  *pos, char  *string , boolean  forwardp, boolean  contForward, boolean  contBackward, char  *errmsg)
{
    int c, fudge;
    boolean results;
    class mark *newpos;
    char newstring[MAXSTRING], *newerr;
    static char prompt[MAXSTRING+256];
    long loc;
    struct SearchPattern *pattern;
    boolean newContForward = contForward;
    boolean newContBackward = contBackward;

    do {
	if (strlen (string) > 0) strcpy(LastString, string);
        if (pos != NULL) {
            (tv)->SetDotPosition( (pos)->GetPos());
            (tv)->SetDotLength( (pos)->GetLength());
            (tv)->FrameDot( (pos)->GetPos());
            sprintf(prompt, "Search %s: %s", forwardp ? "forward" : "backward", string);
        }
        else {
            if (errmsg != NULL)
                sprintf(prompt, "Search %s: %s [%s]", forwardp ? "forward" : "backward", string, errmsg);
            else
                sprintf(prompt, "Failing search %s: %s", forwardp ? "forward" : "backward", string);
        }
        message::DisplayString(tv, 0, prompt);

        im::ForceUpdate();

        c = ((tv)->GetIM())->GetCharacter();

        if (c == '\r')
            c = '\n';

        if ((c < ' ' && c != '\n') || c == '\177') {
            switch (c) {
                case -1:
                case 'G'-64:
                    /* Punt. */
                    message::DisplayString(tv, 0, "Cancelled.");
                    return FALSE;
                case 'S'-64:
                case 'R'-64:
                    forwardp = (c == 'S'-64);
		    strcpy(newstring, LastString);
		    newContForward = TRUE;
		    newContBackward = TRUE;
                    break;
                case 'H'-64:
                case '\177':
		    LastString[0] = '\0';   /* Let caller set it if it is to be non-empty */
                    return TRUE;
                default:
                    message::DisplayString(tv, 0, "");
                    ((tv)->GetIM())->DoKey( c);
                    return FALSE;
            }
        }
        else
            strappend(newstring, string, c, sizeof(newstring));

	if (newstring[0] != '\0') {
	    loc = -1;
            newpos = NULL;
            pattern = NULL;
            newerr = search::CompilePattern(newstring, &pattern);
            if (newerr == NULL) {
                if (strcmp(string, newstring) == 0)
                    fudge = 1;
                else
                    fudge = 0;

		if (forwardp) {
		    if (contForward) {
			loc = search::MatchPattern(txt, (tv)->GetDotPosition() + fudge, pattern);
		    }
		}
		else {
		    if (contBackward) {
			loc = search::MatchPatternReverse(txt, (tv)->GetDotPosition() - fudge, pattern);
		    }
		}

                if (loc >= 0) {
		    newpos = (txt)->CreateMark( loc, search::GetMatchLength());
		    newContForward = TRUE;
		    newContBackward = TRUE;
		}
		else {
		    if (forwardp) {
			newContForward = FALSE;
		    }
		    else {
			newContBackward = FALSE;
		    }
		}
            }

            results = dosearch(tv, txt, newpos, newstring, forwardp, newContForward, newContBackward, newerr);
            if (newpos != NULL) {
                (txt)->RemoveMark( newpos);
                delete newpos;
            }
        }
        else
            results = dosearch(tv, txt, pos, newstring, forwardp, TRUE, TRUE, NULL);
    } while (results);
    return FALSE;
}

static long incsearchf(ATK *tva, long  key)
{
    class textview  *tv=(class textview *)tva;
    class text *txt = (class text *)(tv->dataobject);
    long pos = (tv)->GetDotPosition();
    long len = (tv)->GetDotLength();
    class mark *mark = (txt)->CreateMark( pos, len);
    char *searchString = NULL;

    if (useSelectionRegion && len != 0) {
	char unquotedString[1000];

	if (len >= sizeof(unquotedString) - 1) {
	    message::DisplayString(tv, 0, "Search string too long - continuing with truncating string.");
	    len = sizeof(unquotedString) -1;
	}
	    
	(txt)->CopySubString( pos, len, unquotedString, FALSE);
	searchString = search::GetQuotedSearchString(unquotedString, NULL, 0);
    }
	    
    while (dosearch(tv, txt, mark, (searchString != NULL) ? searchString : (char *)"", key == 'S'-64, TRUE, TRUE, NULL))
        ;
    
    if (searchString != NULL) {
	free(searchString);
    }
    (txt)->RemoveMark( mark);
    delete mark;
    return 0;
}


boolean incsearch::InitializeClass()
{
    static struct bind_Description fns[] = {
	{"incsearch-forward", NULL, 0, NULL, 0, 0, (proctable_fptr)incsearchf, "Search forward incrementally.", "incsearch"},
	{"incsearch-backward", NULL, 0, NULL, 0, 0, (proctable_fptr)incsearchf, "Search backward incrementally.", "incsearch"},
        {NULL},
    };
    struct ATKregistryEntry  *textviewClassinfo;

    useSelectionRegion = environ::GetProfileSwitch("incsearchUseSelectionRegion", TRUE);

    textviewClassinfo = ATK::LoadClass("textview");
    if (textviewClassinfo != NULL) {
        bind::BindList(fns, NULL, NULL, textviewClassinfo);
        return TRUE;
    }
    else
        return FALSE;
}
