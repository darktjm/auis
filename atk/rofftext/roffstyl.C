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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/rofftext/RCS/roffstyl.C,v 1.3 1993/06/09 01:59:39 rr2b Stab74 $";
#endif


 

/* styles */


#include <andrewos.h>
#include <text.H>
#include <style.H>
#include <stylesheet.H>
#include <environment.H>
#include <hash.H>
#include <rofftext.H>

/*
 *  fixed by cch@mtgzx.att.com 1/10/90
 *  program calling environment_InsertStyle with union environmentcontents 
 *  where it should be passing struct *style.
 *  This is a no-no on SPARC.
 */

#ifndef NORCSID
#endif
void WriteText(class rofftext  *self);
int ChangeStyle(class rofftext  *self,int  id,char  *st);
void CloseStyle(class rofftext  *self);
int BeginStyle(class rofftext  *self,char  *st);
void EndStyle(class rofftext  *self,int  ID);
void CloseAllStyles(class rofftext  *self);


void WriteText(class rofftext  *self)
{
    (self)->Write(stdout,(long)self,1);
}


/* change style with id='id' to new style 'st'
 * returns id of new style
 *
 */

int ChangeStyle(class rofftext  *self,int  id,char  *st)
{
    int l, newid;

    DEBUG(1, (stderr,"ChangeStyle: <current = %d>, changing %d to (%s)\n",self->stack->ID,id,(st?st:"")));

    if (id == 0) {
        return (BeginStyle(self,st));
    }

    for (l=self->stack->level;(id != self->stack->ID)&&(self->stack->level >= 0);l--) {
        self->tempstack++;
        self->tempstack->pos = self->stack->pos;
        self->tempstack->style = self->stack->env->data.style;
        self->tempstack->level = (self->tempstack-1)->level+1;
        self->tempstack->ID = self->stack->ID;
        CloseStyle(self);
    }
    CloseStyle(self);
    newid = BeginStyle(self,st);
    for(l=self->tempstack->level;l>=0;l--) {
        self->stack++;
        self->stack->pos = self->pos;
	self->stack->env = ((self->stack-1)->env)->InsertStyle( self->pos - (self->stack-1)->pos, self->tempstack->style, TRUE);
        self->stack->level = (self->stack-1)->level+1;
        self->stack->ID = self->tempstack->ID;
        self->tempstack--;
    }
    return newid;
    
}

/* close innermost style */

void CloseStyle(class rofftext  *self)
{
    DEBUG(1, (stderr,"Closing style %d (%d to %d, length %d)\n", self->stack->ID, self->stack->pos, self->pos, (self->pos - self->stack->pos)));

    if (self->stack->level < 0) {
        DEBUG(1, (stderr,"rofftext: WARNING: tried to close bottom-level style\n"));
        return;
    }

    if (self->pos > self->stack->pos) {
        (self->stack->env)->SetLength(self->pos - self->stack->pos);
        (self->stack->env)->SetStyle(FALSE,FALSE);
    }
    else {
        DEBUG(1, (stderr,"(Removing the 0-length style)\n"));
        (self->stack->env)->Delete();
    }
    self->stack--;

}

/* begin a style.  Caller must hang onto returned ID to close style */

int BeginStyle(class rofftext  *self,char  *st)
{
    class style *style;
    
    if (st == NULL)
        return 0;

    style = (self->textm->styleSheet)->Find(st);

    if (style == NULL) {
        DEBUG(1, (stderr,"BeginStyle: opening non-existent style (%s)\n",st));
        return 0; /* null style */
    }
    self->stack++;
    self->stack->pos = self->pos;
    self->stack->env = ((self->stack-1)->env)->InsertStyle( self->pos - (self->stack-1)->pos, style, TRUE);
DEBUG(4, (stderr, "Inserting style %s at pos %d offset %d\n", st, self->pos, self->pos - (self->stack-1)->pos));	
    self->stack->level = (self->stack-1)->level+1;
    self->stack->ID = self->styleID++;

    if (self->styleID == 0)   /* make sure we don't get a '0' style id */
        self->styleID = 1;

    DEBUG(1, (stderr,"BeginStyle: opening style %d (%s)\n",self->stack->ID,st));
    return self->stack->ID;
}

/* ends a style and cleans up */

void EndStyle(class rofftext  *self,int  ID)
{
    int l;

    DEBUG(1, (stderr,"EndStyle: current %d, closing %d\n",self->stack->ID,ID));

    if (ID == 0)
        return; /* null style */

    for (l=self->stack->level;(ID != self->stack->ID)&&(self->stack->level >= 0);l--) {
        self->tempstack++;
        self->tempstack->pos = self->stack->pos;
        self->tempstack->style = self->stack->env->data.style;
        self->tempstack->level = (self->tempstack-1)->level+1;
        self->tempstack->ID = self->stack->ID;
        CloseStyle(self);
    }
    CloseStyle(self);
    for(l=self->tempstack->level;l>=0;l--) {
        self->stack++;
        self->stack->pos = self->pos;
	self->stack->env = ((self->stack-1)->env)->InsertStyle( self->pos - (self->stack-1)->pos, self->tempstack->style, TRUE);
        self->stack->level = (self->stack-1)->level+1;
        self->stack->ID = self->tempstack->ID;
        self->tempstack--;
    }

} 

void CloseAllStyles(class rofftext  *self)
{
    while(self->stack->level >= 0)
        CloseStyle(self);
}
