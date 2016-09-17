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

/*
 * eq.c
 * This module handles the eq data object.
 */


#include <andrewos.h>
ATK_IMPL("eq.H")
#include "eq.H"

#include <dataobject.H>
#include <mark.H>

#define SIZE 10

static const char init_string[] = "{ lpile d_eqstyle { zilch ^} }";

/* Create a new equation */

ATKdefineRegistry(eq, dataobject, eq::InitializeClass);
#ifdef notyet
void eq_SetCursor(class eq  *self, long  left , long  top , long  width , long  height);
void eq_GetCursor(class eq  *self, long  *leftp , long  *topp , long  *widthp , long  *heightp);
#endif /* notyet */


eq::eq()
{
	ATKinit;
 
    struct formula *f = (struct formula *) malloc(SIZE*sizeof(struct formula));

    this->f = f;
    this->p1 = 0;
    this->gap = SIZE;
    this->p2 = 0;
    this->markList = NULL;
    this->fence = (this)->CreateMark( 0, 0);
    (this->fence)->SetStyle( TRUE, FALSE);
    (this)->InsertTokensCarefully( 0, init_string);

    THROWONFAILURE( TRUE);
}

eq::~eq()
{
	ATKinit;

    class mark *mark;

    free(this->f);
    if (this->markList != NULL)  {
	for (mark = this->markList; mark != NULL; mark = (mark)->GetNext())
	    (mark)->SetObjectFree( TRUE);
    }
    delete this->fence;
}

/* Insert a formula in an equation at position n  */
void eq::Insert(long  pos, struct formula  *f)
{
    if (pos > this->p1 + this->p2)
	pos = this->p1 + this->p2;

    if (this->gap == 0) {
	int size = (this->p1 + this->gap + this->p2) * 3 / 2;
	this->f = (struct formula *) realloc(this->f, size*sizeof(struct formula));
	this->p1 = this->p1 + this->p2;
	this->p2 = 0;
	this->gap = size - this->p1;
    }

    if (this->p1 != pos) {
	int i, gap = this->gap;
	struct formula *newf = this->f;
	for (i=this->p1-1; i>=pos; i--)
	    newf[i+gap] = newf[i];
	for (i=this->p1; i<pos; i++)
	    newf[i] = newf[i+gap];
	this->p2 = this->p2 + this->p1 - pos;
	this->p1 = pos;
    }

    this->f[pos] = *f;
    this->p1 += 1;
    this->gap -= 1;
    (this->markList)->UpdateMarks( pos , 1);
}

/* Insert tokens */

long eq::InsertTokens(long  pos, const char  *s)
{
    char buf[100], *p;
    int inserted = 0;
    static struct formula f;

    while (*s) {
	while (*s==' ')
	    s++;
	p = buf;
	while (*s!=' ' && *s!='\0')
	    *p++ = *s++;
	if (p != buf) {
	    *p = '\0';
	    p = buf;
	    if (p[0]=='#' && p[1]!='\0') {
		p += 1;
	    }
	    else if (p[0]=='^' && p[1]!='\0')
		p += 1;
	    f.symbol = eq::Lookup(p);
	    if (!f.symbol) {
		fprintf(stderr, "Unknown symbol '%s'\n", p);
		return 0;
	    }
	    else {
		(this)->Insert( (pos + inserted), &f);
		inserted += 1;
	    }
	}
    }

    return inserted;
}

/* generally useful.  Initialized in eqview_InitializeClass. */
const struct symbol *eq_zilch;
const struct symbol *eq_root;

/*
 * Take account of zilches when inserting.
 * Caution: a near copy of this is in eqview_Paste, but
 * the two are inconsistent wrt scripted zilches.
 */

long eq::InsertTokensCarefully(long  pos, const char  *s)
{
    long evenup = 0, inserted = 0;
    short zilch_removed = 0;
    struct formula *f;

    if ((f = (this)->Access( pos)) != NULL && f->symbol == eq_zilch) {
	(this)->Delete( pos);
	zilch_removed = 1;
    }
    if ((f = (this)->Access( pos-1)) != NULL && f->symbol == eq_zilch) {
	(this)->Delete( --pos);
	zilch_removed = 1;
	evenup = 1;
    }
/* the following may not handle replacing a scripted zilch properly */
    if (!(inserted = (this)->InsertTokens( pos, s)) && (zilch_removed))
	inserted = (this)->InsertTokens( pos, "zilch");

    return (inserted-evenup);
}

/* Delete formula at position n from an equation */
void eq::Delete(long  pos)
{
    if (pos == this->p1 - 1) {
	this->p1 -= 1;
    } else if (pos == this->p1) {
	this->p2 -= 1;
    } else {
	int i, gap = this->gap;
	struct formula *f = this->f;
	for (i=this->p1-1; i>=pos; i--)
	    f[i+gap] = f[i];
	for (i=this->p1; i<pos; i++)
	    f[i] = f[i+gap];
	this->p2 = this->p2 + this->p1 - pos - 1;
	this->p1 = pos;
    }

    this->gap += 1;
    (this->markList)->UpdateMarks( pos , -1);
}

/*
 * Delete an item but preserve structure
 */

long eq::DeleteCarefully(long  start , long  stop)
{
    long was_zilch;
    int i, j;
    enum type type;
    struct formula *f;

    /* remember whether we deleted a zilch */
    f = (this)->Access( stop-1);
    was_zilch = (f != NULL && f->symbol == eq_zilch);

    /* eliminate unmatched groups */
    i = start;
    j = (this)->FindEndGroup( start);
    while (j < stop) {
	i = (this)->FindBeginGroup( i);
	(this)->Delete( j);
	(this)->Delete( i);
	start -= 1,  stop -= 2,  j -= 1;
	j = (this)->FindEndGroup( j);
    }
    i = (this)->FindBeginGroup( stop);
    j = stop;
    while (i >= start) {
	j = (this)->FindEndGroup( j);
	(this)->Delete( j);
	i = (this)->FindBeginGroup( i);
    }

    /* do the delete */
    for (i=start; i<stop; i++)
	(this)->Delete( start);

    /* did we just delete the last item, a zilch, in a group? */
    if ((this)->Access( start)->symbol->type==END
      && ((type=(this)->Access( start-1)->symbol->type)==BEGIN
      || type==ALIGN || type==EQSTYLE)   &&  was_zilch) {
	/* find an enclosing group to delete */
	int i, b=start, e;
	while ((this)->Access( --b)->symbol->type != BEGIN) ;
	while (b!=0 && !(this)->Access( b)->has_hot_spot
	  && (this)->Access( b-1)->symbol->type!=SCRIPT)
	    b = (this)->FindBeginGroup( b);
	if (b!=0) {
	    e = (this)->FindEndGroup( b+1);
	    for (i=b+1;  i<e;  i++) {
		struct formula *f = (this)->Access( i);
		if (f->symbol->type!=END && f->has_hot_spot
		  && f->symbol != eq_zilch)
		    break;
	    }
	    if (i==e) {
		start = b,  stop = e+1;
		if ((this)->Access( b-1)->symbol->type==SCRIPT)
		    start -= 1;
		for (i=start;  i<stop;  i++)
		    (this)->Delete( start);
	    }
	}
    }
    
    /* Check again */
    if ((this)->Access( start)->symbol->type==END
      && ((type=(this)->Access( start-1)->symbol->type)==BEGIN
      || type==ALIGN || type==EQSTYLE)) {
	i = (this)->FindBeginGroup( start);
	if (!(this)->Access( i)->deletable)
	    (this)->InsertTokens( start, "zilch");
    }
    return start;
}

/*
 * DoScript.
 * Looks for specified script; if found, sets dot to end of
 * script and returns position of end of script.  If not found,
 * inserts string and returns new position.
 */

long eq::DoScript(long  pos, enum script  script, const char  *string)
{
    long i, added, found = -1;

    for (i=pos-1; ; i--) {
	struct formula *f = (this)->Access( i);
	if (f->symbol == eq_zilch)
	    return pos;
	if (f->symbol->type==END)
	    i = (this)->FindBeginGroup( i);
	else if (f->symbol->type==SCRIPT) {
	    if (((enum script)f->symbol->what)==script) {
		found = i;
		break;
	    }
	}
	else if (f->symbol->type==BEGIN) {
	    return pos; 
	}
	else {
	    break;
	}
    }
    if (found >= 0) {
	return ((this)->FindEndGroup( found));
    } else {
	added = (this)->InsertTokens( pos, string);
	return pos+added-1;
    }
}

class mark *eq::CreateMark(long  pos, long  length  )
{
    class mark *markp;
    
    markp = new mark;
    (markp)->SetPos( pos);
    (markp)->SetLength( length);
    (markp)->SetNext( this->markList);
    (markp)->SetObject( this);
    this->markList = markp;
    return markp;
}

void eq::RemoveMark(class mark  *markp  )
{
    class mark *mp;
    class mark *tp;

    if (markp == NULL) return;
    
    for (mp = this->markList, tp = NULL; mp != NULL && mp != markp; tp = mp, mp = (mp)->GetNext());
    
    if (mp != NULL)  {
	if (tp == NULL)  {
/* 	    First element on the list
 */	    
	    this->markList = (mp)->GetNext();
	}
	else  {
	    (tp)->SetNext( (mp)->GetNext());
	}
    }
}

#ifdef notyet
/* Maintain the current cursor geometry */
void eq_SetCursor(class eq  *self, long  left , long  top , long  width , long  height)
{
    self->cursor.left = left;
    self->cursor.top = top;
    self->cursor.width = width;
    self->cursor.height = height;
}

void eq_GetCursor(class eq  *self, long  *leftp , long  *topp , long  *widthp , long  *heightp)
{
    *leftp = self->cursor.left;
    *topp = self->cursor.top;
    *widthp = self->cursor.width;
    *heightp = self->cursor.height;
}
#endif /* notyet */

/* Forget a whole equation */
void eq::Erase()
{
    this->gap = this->p1 + this->gap + this->p2;
    this->p1 = 0;
    this->p2 = 0;
}

/* Get formula n from an equation */
struct formula *eq::Access(long  n)
{
    if (n < 0) return 0;
    if (n < this->p1)
	return &(this->f[n]);
    else if (n < this->p1+this->p2)
	return &(this->f[n+this->gap]);
    else
	return 0;
}

/* Determine the number of formulas in an equation */
long eq::Size()
{
    return this->p1 + this->p2;
}

/* Given an equation and a formula, return the next formula. */
struct formula *eq::NextFormula(struct formula  *f)
{
    int n, p1 = this->p1, gap = this->gap, p2 = this->p2;

    f += 1;
    n = f - this->f;
    if (n == p1)
	return f + gap;
    else if (n < p1 + gap + p2)
	return f;
    else
	return 0;
}

/* Convert to printable representation.  Returns length in bytes, including trailing null. */
long eq::GetTokens(long  *startp , long  stop, char  *string, long  size)
{
    char *s = string;
    int i, len;
    for (i= *startp; i<stop; i++) {
	struct formula *f = (this)->Access( i);
	len = strlen(f->symbol->name) + 1;
	if (len > size)
	    break;
	sprintf(s, "%s ", f->symbol->name);
	s += len,  size -= len;
    }
    *startp = i;
    if (s>string)
	s[-1] = '\0';
    else
	*s++ = '\0';
    return s - string;
}

/*
 * Find end of group i is in.    { . . . }
 * x's at right find the }.	   x x x x
 */

long eq::FindEndGroup(long  i)
{
    int j, level = 0;
    for (j=i; ; j++) {
	switch ((this)->Access( j)->symbol->type) {
	case BEGIN:
	    level++;
	    break;
	case END:
	    if (level==0)
		return j;
	    level--;
	    break;
	default: // only care about above two
	    break;
	}
    }
}


/*
 * Find beginning of group i is in.	{ . . . }
 * x's at right find the {.		  x x x x
 */

long eq::FindBeginGroup(long  i)
{
    int j, level = 0;
    for (j=i-1; ; j--) {
	switch ((this)->Access( j)->symbol->type) {
	case END:
	    level++;
	    break;
	case BEGIN:
	    if (level==0)
		return j;
	    level--;
	    break;
	default: // only care about above two
	    break;
	}
    }
}


/*
 * Find siblings.  Assumes i points to a begin group.
 * Returns pointer to a begin group.
 */

long eq::FindLeftSibling(long  i)
{
    int j;
    if (i==0)
	return -1;
    for (j=i-1; ; j--) {
	switch ((this)->Access( j)->symbol->type) {
	case END:
	    return (this)->FindBeginGroup( j);
	case BEGIN:
	    return -1;
	default: // only care about above two
	    break;
	}
    }
}

long eq::FindRightSibling(long  i)
{
    int j;
    if (i==0)
	return -1;
    for (j=(this)->FindEndGroup( i+1)+1; ; j++) {
	switch ((this)->Access( j)->symbol->type) {
	case END:
	    return -1;
	case BEGIN:
	    return j;
	default: // only care about above two
	    break;
	}
    }
}

/* Write to a file */
void eq::WriteFILE(FILE  *f, long  start , long  stop, char  sep)
{
    while (start<stop) {
	char buf[4000];
	int n = (this)->GetTokens( &start, stop, buf, sizeof(buf));
	if (n<=0)
	    break;
	buf[n-1] = sep;
	fwrite(buf, 1, n, f);
    }
}

/* Read from a FILE */
long eq::ReadFILE(FILE  *file, long  start)
{
    char word[100];
    long n = start, i, c;
    char name[64];
    long inid;

    c = getc(file);
    while (c != EOF && c != '\0') {
	char *ptr = word;

	while (c != ' ' && c != '\t' && c != '\n' && c != EOF) {
	    *ptr++ = c;
	    c = getc(file);
	}
	*ptr++ = '\0';
	if (c == EOF)
	    break;
	if (strncmp(word, "\\begindata{", 11) == 0) {
	    sscanf(word, "\\begindata{%s,%ld}", name, &inid);
	    if (strcmp(name, "eq") == 0)
		this->id = inid;
	    else
		break;
	}
	if (strncmp(word, "\\enddata{", 9) == 0) {
	    sscanf(word, "\\enddata{%s,%ld}", name, &inid);
	    break;
	}
	if (word[0] != '\0') {
	    i = (this)->InsertTokens( n, word);
	    if (i==0) {
		fprintf(stderr, "Unknown symbol '%s'\n", word);
		break;
	    }
	    n += i;
	}
	while (c == ' ' || c == '\t' || c == '\n') 
	    c = getc(file);
    }

    return n-start;
}

long eq::Read(FILE  *file, long  id)
{
    (this)->Erase();
    if (id != 0L)
	this->id = (this)->UniqueID();
    this->modified = 0;
    (this)->ReadFILE( file, 0);
    return dataobject_NOREADERROR; /* probably should get a status value from eq_ReadFILE */
}

long eq::Write(FILE  *file, long  writeid, int  level)
{
    if ((this->writeID != writeid)) {
	this->writeID = writeid;
	fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->UniqueID()); 
	(this)->WriteFILE( file, 0, (this)->Size(), ' ');
	fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->UniqueID());
    }
    return (this)->UniqueID();
}

/* Dump info */
void eq::Dump(const char  *name)
{
    FILE *file = NULL;
    int n = (this)->Size(), i;

    if (name != NULL && name[0] != '\0')
	file = fopen(name, "w");
    else
	file = stdout;
    if (file==NULL) {
	perror(name);
	exit(-1);
    }

    for (i=0; i<n; i++) {
	struct formula *f = (this)->Access( i);
	fprintf(file, "%s pos (%d,%d) hot (%d,%d) %c%c%c min (%d,%d) max (%d,%d) ",
	    f->symbol->name,
	    f->posp.x, f->posp.y,
	    f->hot.x, f->hot.y,
	    (f->has_hot_spot? 'H': 'x'),
	    (f->transparent? 'T': 'x'),
	    (f->deletable? 'D': 'x'),
	    f->min.x, f->min.y,
	    f->max.x, f->max.y
	);
	fprintf(file, "sup_y %d, sub_y %d, kern %d\n",
	    f->sup_y,
	    f->sub_y,
	    f->kern
	);
	fflush(file);
    }
    fprintf(file, "\n");

    if (file != stdout)
	fclose(file);
}

boolean eq::InitializeClass()
{
    if (eq_zilch == NULL)
	eq_zilch = eq::Lookup("zilch");
    if (eq_root == NULL)
	eq_root = eq::Lookup("root");

    return TRUE;
}
