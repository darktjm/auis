/* File rawtext.c created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1992-1995.  All rights reserved.

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

   rawtext, a style-less mode for ATK. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1992-1995.  All rights reserved.";
static UNUSED const char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/rawtext.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";


#include <environment.H>
#include <environ.H>
#include <attribs.h>
#include "rawtext.H"


ATKdefineRegistry(rawtext, text, NULL);
boolean MakeSureNotOverstrikingView(class rawtext  *d, long  pos , long  len);


rawtext::rawtext()
{
    this->NewlineEOF= environ::GetProfileSwitch("rawtext.newlineateof",FALSE);
    this->OverstrikeMode= FALSE; /*RSK92overstrike*/
    (this)->SetCopyAsText( TRUE); /* otherwise regions get pasted as INSETS */
    (this)->SetObjectInsertionFlag( FALSE); /* don't let insets get inserted here */
    (this)->SetExportEnvironments( FALSE);
    THROWONFAILURE( TRUE);
}

rawtext::~rawtext()
{
    return;
}

/* override */
long rawtext::Write(FILE  *file, long  writeID, int  level)
{
    long temp, len=(this)->GetLength();
    if (len>0 && (this)->PutNewlineAtEOF() && (this)->GetChar( len-1)!='\n')
	/* add a newline char to the end */
        (this)->InsertCharacters( len, "\n", 1);
    temp=(this)->text::Write( file, writeID, level);
    return temp;
}

/* override */
void rawtext::SetAttributes(struct attributes  *atts)
{
    (this)->text::SetAttributes(atts);
    while (atts!=NULL) {
	if (strcmp(atts->key,"newline-at-eof")==0)
	    this->NewlineEOF= atoi(atts->value.string);
	else if (strcmp(atts->key,"overstrike")==0) /*RSK92overstrike*/
	    (this)->ChangeOverstrikeMode(atoi(atts->value.string));
	atts=atts->next;
    }
}

/*RSK92overstrike: this is a duplicate of the original simpletext_InsertCharacters; it's used by the TABOVER macro to ignore overstrike mode*/
boolean rawtext::JustInsertCharacters(long  pos,const char  *str,long  len)
{
    if (pos >= (this)->GetFence()) {
        (this)->AlwaysInsertCharacters( pos, str, len);
	return TRUE;
    }
    else
        return FALSE;
}

/*RSK91overstrike: mostly snagged from Patch10's ConfirmViewDeletion in atk/text/txtvcmod.c*/
#define TEXT_VIEWREFCHAR '\377'
boolean MakeSureNotOverstrikingView(class rawtext  *d, long  pos , long  len)
{
    boolean hasViews;
    class environment *env;

    for (hasViews = FALSE; len--; pos++)
	if ((d)->GetChar( pos) == TEXT_VIEWREFCHAR) {
	    env = (environment *) (d->text::rootEnvironment)->GetInnerMost( pos);
	    if (env->type == environment_View) {
		hasViews = TRUE;
		break;
	    }
	}
    if (! hasViews)
	return TRUE;
    return FALSE; /*can't type over insets*/
}

/*RSK91overstrike: this routine was based on Patch10's textview_ViDeleteCmd (atk/text/txtvcmod.c).*/
void rawtext::OverstrikeAChar(long  pos)
{
    int	dsize;
    char c;

    if ( (this)->GetChar( pos - 1) == '\n' && (this)->GetChar( pos) == '\n' )
	return;
    dsize = (this)->GetLength();
    if (MakeSureNotOverstrikingView(this, pos, 1)) {
	if ( (c=(this)->GetChar( pos)) != '\n' && pos < dsize) {
	    if (c!='\t')
		(this)->DeleteCharacters( pos, 1);
	}		
	(this)->NotifyObservers( observable_OBJECTCHANGED);
    }
}

/*RSK91overstrike: override simpletext's normal character insertion*/
boolean rawtext::InsertCharacters(long  pos, const char  *str, long  len)
{
    if ((this)->IsInOverstrikeMode()) {
	int i= 0;
	do  {
	    if (*(str+i)!='\n' && *(str+i)!='\t')
		(this)->OverstrikeAChar(pos+i);
	    i++;
	} while (i<len);
    }
    return (this)->text::InsertCharacters(pos,str,len);
}
