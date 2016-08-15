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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/rofftext/RCS/roffutil.C,v 1.7 1995/07/17 21:08:55 wjh Stab74 $";
#endif


/* utility stuff for rofftext
 *
 */

#include <glist.H>
#include <rofftext.H>
#include <roffstyl.h>
#include <rofftext.h>
#include <roffutil.h>

#include <num.H>
#include <tlex.H>
#include <num.tlc>		/* lex control tables and functions */


/* magic buffers */
BUF NewBuf();
void FreeBuf(BUF  b);
void Add2Buf(BUF  b,int  c);

void CreateEnvirons(class rofftext  *self);
void DestroyEnvirons(class rofftext  *self);
void PushEnviron(class rofftext  *self,int  env);
void PopEnviron(class rofftext  *self);
struct diversionLevel *CreateDiversion(class rofftext  *self,
	struct diversionLevel  *c);
void DestroyDiversion(class rofftext  *self,struct diversionLevel  *d);
void PushDiversion(class rofftext  *self);
void PopDiversion(class rofftext  *self);
void EvalString(class rofftext *self,char *str, int *result, int scale, 
	boolean *absolute, boolean *relative);


BUF NewBuf()
{
    BUF b = (BUF)malloc(sizeof(struct MagicBuf));
    b->begin = b->ptr =  (char *)malloc(BUFSIZ);
    b->blocksize = BUFSIZ;
    b->end = b->begin + BUFSIZ - 1;
    b->size = BUFSIZ;
    return b;
}

void FreeBuf(BUF  b)
{
    free(b->begin);
    free(b);
}

/* Add2Buf  -  add a character to a buffer value
	NULCHAR gets dropped here (hope that's okay -wjh)
	because NULCHAR terminates the string
	created for ds_cmd
*/
void Add2Buf(BUF  b,int  c)
{
    char *temp;
    if (c == NULCHAR) return;
    if (b->ptr >= b->end) {
        DEBUG(1, (stderr,"<<Bumping size of buffer %x, text=%x,p=%x,end=%x>>",b,b->begin,b->ptr,b->end));
        b->size += b->blocksize;
        temp = (char *)realloc(b->begin,b->size);
        if (temp == NULL)
            DEBUG(1, (stderr,"rofftext: HELP! Buffer couldn't grow!\n"));
        b->end = (temp + b->size - 1);
        b->ptr = (b->ptr - b->begin + temp);
        b->begin = temp;
    }
    *b->ptr++ = c;
}

/* create environments with default values */

void CreateEnvirons(class rofftext  *self)
{
    struct roffEnviron *e ;
    int i;

    for (i=0;i<4;i++) {
        e = self->Environs[i] = (struct roffEnviron *)malloc(sizeof(struct roffEnviron));
        e->pointSize = 10;
	e->fill	= TRUE;
        e->font = "roman";
        e->fontStyle = 0;
        e->prevFont = e->font;
        e->indent = 0;
        e->prevIndent = 0;
        e->tempIndent = 0;
        e->NextInputTrap = 0;
        e->InputTrapCmd = NULL;
        e->controlChar = '.';
        e->NBControlChar = '\'';
    }
    self->CurrentEnviron = self->Environs[0];
}

void DestroyEnvirons(class rofftext  *self)
{
    int i; struct roffEnviron *e;

    for(i=0;i<4;i++) {
        e = self->Environs[i];
        if(e->InputTrapCmd)
            free(e->InputTrapCmd);
        free(e);
    }
}

/* push down to environment X */

void PushEnviron(class rofftext  *self,int  env)
{
    int indent;
    DEBUG(1, (stderr,"<<<Pushing Environment>>>\n"));
    (self->EnvironStack)->Push((char *)self->CurrentEnviron);

    indent = self->CurrentEnviron->indent;
    EndStyle(self,self->CurrentEnviron->fontStyle);
    self->CurrentEnviron->fontStyle = 0;

    self->CurrentEnviron = self->Environs[env];

    self->CurrentEnviron->fontStyle = BeginStyle(self,self->CurrentEnviron->font);
    if (indent != self->CurrentEnviron->indent)
        SetIndent(self,self->CurrentEnviron->indent);

}

void PopEnviron(class rofftext  *self)
{
    int indent;
    DEBUG(1, (stderr,"<<<Popping environment>>>\n"));
    indent = self->CurrentEnviron->indent;
    EndStyle(self,self->CurrentEnviron->fontStyle);
    self->CurrentEnviron = (struct roffEnviron *)(self->EnvironStack)->Pop();
    if (self->CurrentEnviron == NULL)
        self->CurrentEnviron = self->Environs[0];
    self->CurrentEnviron->fontStyle = BeginStyle(self,self->CurrentEnviron->font);
    if (indent != self->CurrentEnviron->indent)
        SetIndent(self,self->CurrentEnviron->indent);
}

struct diversionLevel *CreateDiversion(class rofftext  *self,struct diversionLevel  *c)
{
    struct diversionLevel *d = (struct diversionLevel *)malloc(sizeof(struct diversionLevel));

    if (c == NULL) {
        d->NextDiversionTrap = 0;
        d->name = NULL;
        d->level = 0;
        d->NoSpaceMode = FALSE;
        d->OutputDone = 0;
	d->SnarfOutput = NULL;
    }
    else {
        d->NextDiversionTrap = c->NextDiversionTrap;
        if (c->name)
            d->name = StrDup(c->name);
        else
            d->name = NULL;
        d->level = c->level+1;
        d->NoSpaceMode = c->NoSpaceMode;
        d->OutputDone = c->OutputDone;
	d->SnarfOutput = c->SnarfOutput;

    }
    return d;
}

void DestroyDiversion(class rofftext  *self,struct diversionLevel  *d)
{
    if (d->name)
        free(d->name);
    free(d);
}

void PushDiversion(class rofftext  *self)
{
    (self->DiversionStack)->Push((char *)self->CurrentDiversion);
    self->CurrentDiversion = CreateDiversion(self,self->CurrentDiversion);
    self->v_DiversionLevel++;
}

void PopDiversion(class rofftext  *self)
{
    if (self->v_DiversionLevel > 0) {
        DestroyDiversion(self,self->CurrentDiversion);
        self->v_DiversionLevel--;
        self->CurrentDiversion = (struct diversionLevel *)(self->DiversionStack)->Pop();
    }
}


static struct evalstrdata *Data = NULL;		/* instance of data block */
	/* if multiple simultaneous parses are to be allowed, we need only 
		make a list of evalstrdata blocks */


	void 
EvalString(class rofftext *self, char *str, int *result, int scale, 
	boolean *absolute, boolean *relative) 
{
	int sign = 0, temp;
	boolean ab,rel;

	if (Data == NULL) {
		Data = (struct evalstrdata *)malloc(sizeof(struct evalstrdata));

		/* make linked list of dbls to use on value stack */
		Data->nextdbl = 0;
		Data->dummydbl = 0.0;
		int i = (int)(sizeof(Data->dbls)/sizeof(*Data->dbls)) - 1;
		Data->dbls[i] = (double)(-1);
		while ( --i >= 0)
			Data->dbls[i] = (double)(i +1);
			
		Data->numtext = new text;
		Data->numlex = tlex::Create(&num_tlex_tables, Data, 
				Data->numtext, 0, 0);
		Data->numparse = new num;
		(Data->numparse)->SetRock((void *)Data);
	}

	switch (*str) {
	case '-':
		rel = 1;
		sign = 1;
		ab = 0;
		str++;
		break;
	case '+':
		rel = 1;
		ab = 0;
		str++;
		break;
	case '|':
		ab = 1;
		rel = 0;
		str++;
		break;
	default:
		ab = 0;
		rel = 0;
	}

	Data->v_DefaultScale = scale;
	Data->rt = self;
	Data->SF = self->ScaleFactor;
	(Data->numtext)->Clear();
	(Data->numtext)->InsertCharacters(0, str, strlen(str)+1 /* include the nul */);
	(Data->numlex)->SetText(Data->numtext, 0,
			(Data->numtext)->GetLength()-1);

	if ((Data->numparse)->Parse(tlex::LexFunc, Data->numlex) == parser_OK){
		temp = (int)((sign) ? (-Data->v_Result) : (Data->v_Result));
		*result = temp;
	}
	else {
		*result = 0;
	}

	if (absolute)
		*absolute = ab;
	if (relative)
		*relative = rel;
}
