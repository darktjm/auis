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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/eq/RCS/eqvcmds.C,v 1.7 1994/08/12 18:50:25 rr2b Stab74 $";
#endif


 

/*
 * eqvcmds.c
 * This module handles the view commands for eq.
 */



#define AUXMODULE 1
#include <andrewos.h>
#include <eqview.H>

#include <eq.H>
#include <dataobject.H>
#include <proctable.H>
#include <keymap.H>
#include <menulist.H>
#include <bind.H>
#include <im.H>
#include <view.H>
#include <message.H>

#define MAXFILENAME 400



char *line = "{ zilch ^}";

char* eqview_cut_prefix = "{ lpile d_eqstyle { ";
char* eqview_cut_suffix = "} } ";

/*
 * Call this routine when you have changed something in
 * the data structure and you want it to get updated on
 * the screen eventually via the update mechanism.
 */

#ifndef NORCSID
#endif
void eqview_Default(class eqview  *self, char  c);
void eqview_WriteC(class eqview  *self);
void eqview_WritePS(class eqview  *self);
void eqview_WriteEqn(class eqview  *self);
void eqview_WriteTroff(class eqview  *self);
void eqview_WriteDvi(class eqview  *self);
long eqview_MoveRight(class eqview  *self, class eq  *eqptr, long  i , long  x);
void eqview_MoveForward(class eqview  *self);
void eqview_MoveBackward(class eqview  *self);
void eqview_MoveToBeginning(class eqview  *self);
void eqview_MoveToEnd(class eqview  *self);
void eqview_DeleteBackward(class eqview  *self);
void eqview_DeleteForward(class eqview  *self);
void eqview_CR(class eqview  *self);
void eqview_MoveUp(class eqview  *self);
void eqview_MoveDown(class eqview  *self);
void eqview_Bar(class eqview  *self);
void eqview_Dot(class eqview  *self);
void eqview_Prime(class eqview  *self);
#ifdef notdef
void eqview_Open(class eqview  *self, char  c);
#endif /* notdef */
void eqview_Close(class eqview  *self, char  c);
void eqview_Cut(class eqview  *self);
void eqview_Copy(class eqview  *self);
void eqview_Paste(class eqview  *self);
void eqview_Exit();
void eqview_DumpAndWrite(class eqview  *self);
void eqview_doc();
void eqview_DoSpecial(class eqview  *self, char  *s);
void eqview_Special(class eqview  *self, char  c);
void eqview_SuperScript(class eqview  *self);
void eqview_SubScript(class eqview  *self);
void eqview_AboveScript(class eqview  *self);
void eqview_BelowScript(class eqview  *self);
void eqview_String(class eqview  *self, char  *s);
void eqview_Root(class eqview  *self);
void eqview_Fraction(class eqview  *self);
void eqview_lbrace(class eqview  *self);
class keymap *eqview_InitKeyMap(struct eqview_ATKregistry_   *classInfo, class menulist  **eqviewMenus , class menulist  **eqviewCutMenus);


void eqview::Changed(enum changed  changed)
{
    if ((int)changed > (int)this->changed)
	this->changed = changed;
}

/*
 * Self-insert, basically
 */

void eqview_Default(class eqview  *self, char  c)
{
    static char s[2] = " ";
    long pos, len, added;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    if (len > 0) {
	pos += len;
	(self)->SetDotPosition( pos);
	(self)->SetDotLength( 0);
    }
    s[0] = c;
    added = (Eq(self))->InsertTokensCarefully( pos, s);
    (self)->SetDotPosition( pos+added);
    (self)->Changed( EQVIEW_eq);
}

/*
 * Parse-based output
 */

void eqview_WriteC(class eqview  *self)
{
    (Eq(self))->Parse( stdout, 'c');
}

void eqview_WritePS(class eqview  *self)
{
    (Eq(self))->Parse( stdout, 'p');
}

void eqview_WriteEqn(class eqview  *self)
{
    (Eq(self))->Parse( stdout, 'e');
}

void eqview_WriteTroff(class eqview  *self)
{
    class eq *eqptr = Eq(self);
    FILE *file = popen("eqn", "w");
    (eqptr)->Parse( file, 'e');
    pclose(file);
}

void eqview_WriteDvi(class eqview  *self)
{
    class eq *eqptr = Eq(self);
    FILE *file = popen("eqn | troff", "w");
    (eqptr)->Parse( file, 'e');
    pclose(file);
}

/*
void eqview_PreviewMe(self)
struct eqview *self;
{
    register struct eq *eqptr = Eq(self);
    FILE *file;

    message_DisplayString(self, 0, "Processing preview request.");
    file = popen("eqn | troff | preview", "w");
    eq_Parse(eqptr, file, 'e');
    pclose(file);
    message_DisplayString(self, 0, "Preview window should appear soon.");
}
*/
/*
void eqview_PrintMe(self)
struct eqview *self;
{
    register struct eq *eqptr = Eq(self);
    FILE *file;

    message_DisplayString(self, 0, "Processing print request.");
    file = popen("eqn | troff | print -Tdvi", "w");
    eq_Parse(eqptr, file, 'e');
    pclose(file);
    message_DisplayString(self, 0, "Print request submitted; watch console for results.");
}
*/
/*
void eqview_WriteOutFile(self)
struct eqview *self;
{
    register struct eq *eqptr = Eq(self);
    char name[MAXFILENAME], out[500];
    long code;
    FILE *file;

    code = message_AskForString(self, 0, "Write to file: ", 0, name, sizeof(name));
    if (code < 0) {
	message_DisplayString(self, 0, "Punt!");
	return;
    }
    if (strlen(name) == 0) {
	message_DisplayString(self, 0, "No name specified.");
	return;
    }

    if (file = fopen(name, "w")) {
	eq_Write(eqptr, file, 0L, 1);
	if (self->filename)
	    self->filename = (char *) realloc(self->filename, 1 + sizeof(name));
	else
	    self->filename = (char *) malloc(1 + sizeof(name));
	strcpy(self->filename, name);
	message_DisplayString(self, 0, "Wrote file.");
    }
    else {
	sprintf(out, "Couldn't write to file %s.", name);
	message_DisplayString(self, 0, out);
    }
    fclose(file);
}
*/
/*
void eqview_Save(self)
struct eqview *self;
{
    register struct eq *eqptr = Eq(self);
    char out[500];
    FILE *file;

    if (self->filename == NULL)
	eqview_WriteOutFile(self);

    else {
	if (file = fopen(self->filename, "w")) {
	    eq_Write(eqptr, file, 0L, 1);
	    message_DisplayString(self, 0, "Wrote file.");
	}
	else {
	    sprintf(out, "Couldn't write to file %s.", self->filename);
	    message_DisplayString(self, 0, out);
	}
	fclose(file);
    }
}
*/
/*
void eqview_ReadInFile(self)
struct eqview *self;
{
    register struct eq *eqptr = Eq(self);
    char name[MAXFILENAME], out[500];
    long code;
    FILE *file;

    code = message_AskForString(self, 0, "Read file: ", 0, name, sizeof(name));
    if (code < 0) {
	message_DisplayString(self, 0, "Punt!");
	return;
    }
    if (strlen(name) == 0) {
	message_DisplayString(self, 0, "No name specified.");
	return;
    }

    if (file = fopen(name, "r")) {
	eq_Erase(eqptr);
	eq_Read(eqptr, file, 0);
	fclose(file);
	eqview_SetDotPosition(self, 4);
	if (self->filename)
	    self->filename = (char *) realloc(self->filename, 1 + sizeof(name));
	else
	    self->filename = (char *) malloc(1 + sizeof(name));
	strcpy(self->filename, name);
	eqview_Changed(self, EQVIEW_everything);
	message_DisplayString(self, 0, "Done.");
    }
    else {
	sprintf(out, "Couldn't read file %s.", name);
	message_DisplayString(self, 0, out);
    }
}
*/
/*
 * Keyboard commands
 */

/*
 * Move right in a group to position x.
 * i points to a begin group.
 */

long eqview_MoveRight(class eqview  *self, class eq  *eqptr, long  i , long  x)
{
    register int n = (eqptr)->FindEndGroup( i+1), closest = 0, distance = 1000000, j;
    for (j = i+1;  j<=n;  j++) {
	register struct formula *f = (eqptr)->Access( j);
	if (f->symbol->type == ALIGN)
	    j = (eqptr)->FindEndGroup( j);
	else if (f->symbol->type==SCRIPT && (eqptr)->Access(j+1)->symbol->type==BEGIN)
	    j = (eqptr)->FindEndGroup( j+2);
	else if (f->has_hot_spot) {
	    register int dx = f->hot.x - x;
	    dx = ABS(dx);
	    if (dx < distance) {
		closest = j;
		distance = dx;
	    }
	}
    }
    return closest;
}

void eqview_MoveForward(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long n = (eqptr)->Size(), i, pos, len;
    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    for (i = pos+len+1;  i<n;  i++) {
	struct formula *f = (eqptr)->Access( i);
	if (f->has_hot_spot)
	    break;
	else if (f->symbol->type == ALIGN)
	    i = (eqptr)->FindEndGroup( i);
	else if (f->symbol->type==SCRIPT && (eqptr)->Access(i+1)->symbol->type==BEGIN)
	    i = (eqptr)->FindEndGroup( i+2);
	else if (f->symbol->type == BEGIN) {
	    int j = (eqptr)->FindBeginGroup( i);
	    if (j!=0 && (eqptr)->Access( j+1)->symbol->type == ALIGN)
		i = (eqptr)->FindEndGroup( i);
	}
    }
    if (i < n) {
	(self)->SetDotPosition( i);
	(self)->SetDotLength( 0);
    }
    else {
	(self)->SetDotPosition( pos+len);
	(self)->SetDotLength( 0);
    }

    (self)->Changed( EQVIEW_caret);
}

void eqview_MoveBackward(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    int i, pos, len;
    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    for (i = pos+len-1;  i>0;  i--) {
	struct formula *f = (eqptr)->Access( i);
	if (f->symbol->type == END) {
	    int j = (eqptr)->FindBeginGroup( i);
	    if ((eqptr)->Access( j+1)->symbol->type == ALIGN)
		i = j+1;
	    else if ((eqptr)->Access( j-1)->symbol->type == SCRIPT)
		i = j-1;
	    else if (f->has_hot_spot)
		break;
	} else if (f->has_hot_spot) {
	    break;
	} else if (f->symbol->type == BEGIN) {
	    int j = (eqptr)->FindBeginGroup( i);
	    if (j!=0 && (eqptr)->Access( j+1)->symbol->type == ALIGN)
		i = j+1;
	}
    }
    if (i > 0) {
	(self)->SetDotPosition( i);
	(self)->SetDotLength( 0);
    }
    else {
	(self)->SetDotPosition( pos+len);
	(self)->SetDotLength( 0);
    }

    (self)->Changed( EQVIEW_caret);
}

void eqview_MoveToBeginning(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    int pos, len;
    /*eqview_MoveBackward(eqptr);*/
    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    if (len==0) {
	do
	    pos = (eqptr)->FindBeginGroup( pos);
	while ((eqptr)->Access( pos)->transparent);
	pos += 1;
	while (!(eqptr)->Access( pos) -> has_hot_spot)
	    pos++;
    }
    (self)->SetDotPosition( pos);
    (self)->SetDotLength( 0);

    (self)->Changed( EQVIEW_caret);
}

void eqview_MoveToEnd(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    int pos, len;
    /*eqview_MoveForward(eqptr);*/
    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    if (len==0) {
	pos = (eqptr)->FindEndGroup( pos);
	while ((eqptr)->Access( pos)->transparent)
	    pos = (eqptr)->FindEndGroup( pos+1);
    } else
	pos += len;
    (self)->SetDotPosition( pos);
    (self)->SetDotLength( 0);

    (self)->Changed( EQVIEW_caret);
}

void eqview_DeleteBackward(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    int pos, len, start, stop;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotPosition( pos+=len);
    (self)->SetDotLength( 0);

    start = pos-1,  stop = pos;

    switch ((eqptr)->Access( start)->symbol->type) {
    case BEGIN:
	if (!(eqptr)->Access( start)->deletable)
	    return;
	break;
    case END:
	if ((eqptr)->Access( start)->deletable)
	    start = (eqptr)->FindBeginGroup( start);
	else
	    return;
	break;
    }
    while (!(eqptr)->Access( start)->has_hot_spot)
	start -= 1;

    start = (eqptr)->DeleteCarefully( start, stop);
    if (start)
	(self)->SetDotPosition( start);

    (self)->Changed( EQVIEW_eq);
}

void eqview_DeleteForward(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    int pos, len, start, stop;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotPosition( pos+=len);
    (self)->SetDotLength( 0);

    start = pos,  stop = pos+1;

    switch ((eqptr)->Access( stop-1)->symbol->type) {
    case END:
	if (!(eqptr)->Access( stop-1)->deletable)
	    return;
	break;
    case BEGIN:
	if ((eqptr)->Access( stop-1)->deletable)
	    stop = (eqptr)->FindEndGroup( stop)+1;
	else
	    return;
	break;
    }
    while (!(eqptr)->Access( stop)->has_hot_spot)
	stop += 1;

    /* Inserting a zilch may have changed start */
    start = (eqptr)->DeleteCarefully( start, stop);
    (self)->SetDotPosition( start);	/* in case a zilch was inserted */
    (self)->SetDotLength( 0);

    (self)->Changed( EQVIEW_eq);
}

void eqview_CR(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long i, pos, len, added, n = (eqptr)->Size();;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    i = (eqptr)->FindEndGroup( pos+len) + 1;
    while (i<n && !(eqptr)->Access( i) -> has_hot_spot)
	i += 1;
    if (i<n) {
	(self)->SetDotPosition( i);
	(self)->SetDotLength( 0);
    }
    else if ((eqptr)->Access( 1)->symbol->type == ALIGN) {
	added = (eqptr)->InsertTokens( n-1, line);
	(self)->SetDotPosition( pos+len+added);
	(self)->SetDotLength( 0);
    }

    (self)->Changed( EQVIEW_eq);
}

void eqview_MoveUp(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long i;
    int pos, len;
    struct formula *start;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    i = pos+len;
    start = (eqptr)->Access( i);

    /* are we in front of a pile? */
    if ((eqptr)->Access( i)->symbol->type == BEGIN
      && (eqptr)->Access( i+1)->symbol->type == ALIGN)
    {
	while (!(eqptr)->Access( ++i) -> has_hot_spot) ;
	(self)->SetDotPosition( i);
	(self)->SetDotLength( 0);
	return;
    }

    /* are we in front of a scripted atom? */
    else if ((eqptr)->Access( i+1)->symbol->type == SCRIPT) {
	/* move to top script of following object */
	if ((eqptr)->Access( i+3)->hot.y < start->hot.y) {
	    while(!(eqptr)->Access( ++i)->has_hot_spot) ;
	    (self)->SetDotPosition( i);
	    (self)->SetDotLength( 0);
	    return;
	}
    }

    /* no script or pile to move into; find a higher sibling */
    while ( (i=(eqptr)->FindBeginGroup( i)) != 0) {
#ifdef notdef
	/* is our parent higher? */
	if ((eqptr)->Access( i)->hot.y < start->hot.y)
	    break;
#endif /* notdef */
	if ((eqptr)->Access( i-1)->symbol->type == SCRIPT) {
	    /* find sibling script */
	    if ((eqptr)->Access( i-2)->symbol->type == END) {
		i = (eqptr)->FindBeginGroup( i-2);
		if ((eqptr)->Access( i-1)->symbol->type == SCRIPT)
		    break;
	    }
	} else {
	    /* find sibling in pile */
	    enum type t;
	    while ((t=(eqptr)->Access( --i)->symbol->type) != END && t != BEGIN) ;
	    if (t==END) {
		i = (eqptr)->FindBeginGroup( i);
		if ((eqptr)->Access( i-1)->symbol->type != SCRIPT
		  && (eqptr)->Access( i)->hot.y < start->hot.y)
		    break;
	    } else
		i += 1;
	}
    }
    if (i != 0) {
	(self)->SetDotPosition( eqview_MoveRight(self, eqptr, i, start->hot.x));
	(self)->SetDotLength( 0);
    }

    (self)->Changed( EQVIEW_caret);
}

void eqview_MoveDown(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long i;
    int pos, len, j;
    struct formula *start;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    i = pos+len;
    start = (eqptr)->Access( i);

    /* are we in front of a pile? */
    if ((eqptr)->Access( i)->symbol->type == BEGIN
      && (eqptr)->Access( i+1)->symbol->type == ALIGN)
    {
	/* find last of pile */
	enum type t;
	i = (eqptr)->FindEndGroup( i+1);
	while ((t=(eqptr)->Access( --i)->symbol->type) != BEGIN && t != END) ;
	if (t==END) {
	    i = (eqptr)->FindBeginGroup( i);
	    while (!(eqptr)->Access( ++i)->has_hot_spot) ;
	    (self)->SetDotPosition( i);
	    (self)->SetDotLength( 0);
	    return;
	}
    }

    /* are we in front of a scripted atom? */	
    else if ((eqptr)->Access( i+1)->symbol->type == SCRIPT) {
	/* find last script */
	int j = i;
	do {
	    i = j;
	    j = (eqptr)->FindEndGroup( i+3);
	} while ((eqptr)->Access( j+1)->symbol->type == SCRIPT);
	if ((eqptr)->Access( i+3)->hot.y > start->hot.y) {
	    while(!(eqptr)->Access( ++i)->has_hot_spot) ;
	    (self)->SetDotPosition( i);
	    (self)->SetDotLength( 0);
	    return;
	}
    }

    /* find a lower sibling */
    j = i;
    while ( (j=(eqptr)->FindBeginGroup( j)) != 0) {
	i = (eqptr)->FindEndGroup( i);
#ifdef notdef
	/* is our parent lower? */
	if ((eqptr)->Access( j)->hot.y > start->hot.y) {
	    i = j;
	    break;
	}
#endif /* notdef */
	if ((eqptr)->Access( j-1)->symbol->type == SCRIPT) {
	    /* find sibling script */
	    if ((eqptr)->Access( i+1)->symbol->type == SCRIPT) {
		i = i+2;
		break;
	    } else
		i += 1;
	} else {
	    /* find sibling in pile */
	    enum type t;
	    while ((t=(eqptr)->Access( ++i)->symbol->type) != BEGIN && t != END) ;
	    if (t==BEGIN) {
		if ((eqptr)->Access( i-1)->symbol->type != SCRIPT
		  && (eqptr)->Access( i)->hot.y < start->hot.y)
		    break;
	    }
	}
    }
    if (j != 0) {
	(self)->SetDotPosition( eqview_MoveRight(self, eqptr, i, start->hot.x));
	(self)->SetDotLength( 0);
    }
    (self)->Changed( EQVIEW_caret);
}

/*
 * Diacritical marks
 */

void eqview_Bar(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long pos, len, start, stop;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    if (len==0) {
	stop = pos;
	eqview_MoveBackward(self /*eqptr*/);
	start = (self)->GetDotPosition();
    } else {
	start = pos;
	stop = pos+len;
    }
    (self)->SetDotPosition( stop);
    (self)->SetDotLength( 0);
    (eqptr)->InsertTokens( stop, "} above bar");
    (eqptr)->InsertTokens( start, "{ord");

    (self)->Changed( EQVIEW_eq);
}

void eqview_Dot(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long n, pos, len, added = 0;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotLength( 0);
    (self)->SetDotPosition( pos = (pos + len));

    if ((n = (eqptr)->DoScript( pos, ABOVE, "above { dot }")) == pos) {
	message::DisplayString(self, 0, "Bad command.");
	return;
    }
    else if (n == pos+3) {
	(self)->SetDotPosition( n+1);
    }
    else {
	added = (eqptr)->InsertTokensCarefully( n, "dot");
	(self)->SetDotPosition( n+added+1);
    }

    (self)->Changed( EQVIEW_eq);
}

void eqview_Prime(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long n, pos, len, added = 0;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotLength( 0);
    (self)->SetDotPosition( pos = (pos + len));

    if ((n=(eqptr)->DoScript( pos, SUP, "sup { ' }")) == pos) {
	message::DisplayString(self, 0, "Bad command.");
	return;
    }
    else if (n == pos+3) {
	(self)->SetDotPosition( n+1);
    }
    else {
	added = (eqptr)->InsertTokensCarefully( n, "'");
	(self)->SetDotPosition( n+added+1);
    }

    (self)->Changed( EQVIEW_eq);
}

/*
 * Parens
 */

#ifdef notdef
void eqview_Open(class eqview  *self, char  c)
{
    register class eq *eqptr = Eq(self);
    long pos, len, added;
    char buf[20];

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotPosition( pos+len);
    (self)->SetDotLength( 0);
    if (c=='{')
	buf = "lbrace";
    sprintf(buf, "%c", c);
    added = (eqptr)->InsertTokensCarefully( pos+len, buf);
    (self)->SetDotPosition( pos+added);

    (self)->Changed( EQVIEW_eq);
}
#endif /* notdef */

void eqview_Close(class eqview  *self, char  c)
{
    register class eq *eqptr = Eq(self);
    long pos, len, added, level = 0, i, matched = 0;
    struct formula *f;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    pos += len;

    for (i=pos-1; i>=0 && level>=0; i--) {
	f = (eqptr)->Access( i);
	if (f->symbol->type == END)
	    i = (eqptr)->FindBeginGroup( i);
	else if (f->symbol->type == BEGIN)
	    break;
	else if (f->symbol->genre == CLOSE)
	    level++;
	else if (f->symbol->genre == OPEN && f->symbol != root) {	
	    if (level != 0)
		level--;
	    else {
		matched = 1;
		break;
	    }
#ifdef notdef
		f = (eqptr)->Access( i+1);	/* XXX - following necessary? */
		if (! (f->symbol->type==BEGIN && f->transparent) )
		    break;
#endif /* notdef */
	}
    }
    if (matched) {
	char buf[20];
	if (c=='}')
	    sprintf(buf, "} rbrace");
	else
	    sprintf(buf, "} %c", c);
	(eqptr)->InsertTokens( pos, buf);
	(eqptr)->InsertTokens( i+1, "{");
	(self)->SetDotPosition( i);
	(self)->SetDotLength( pos-i+3);
    } else {
	char buf[20];
	if (c=='}')
	    sprintf(buf, "rbrace");
	else
	    sprintf(buf, "%c", c);
	if (len>0) {
	    (self)->SetDotPosition( pos);
	    (self)->SetDotLength( 0);
	}
	added = (eqptr)->InsertTokensCarefully( pos, buf);
	(self)->SetDotPosition( pos+added);
    }

    (self)->Changed( EQVIEW_eq);
}

/*
 * Cut, copy, paste.
 *
 * The cut buffer is set up to contain an autonomous equation
 * in case we are pasting into something other than an eq
 * object.  For this we use eqview_cut_prefix before and
 * eqview_cut_suffix after the sub-equation we put into 
 * the cut buffer.
 */

void eqview_Cut(class eqview  *self)
{
    class eq *eqptr = Eq(self);
    long pos, len;
    FILE *cutFile;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    cutFile = (((class view *) self)->GetIM())->ToCutBuffer();
    fprintf(cutFile, "\\begindata{%s,%d}\n", (eqptr)->GetTypeName(), (eqptr)->UniqueID());
    fprintf(cutFile, "%s", eqview_cut_prefix);
    eqptr->writeID = im::GetWriteID();
    (eqptr)->WriteFILE( cutFile, pos, pos+len, ' ');
    fprintf(cutFile, "%s", eqview_cut_suffix);
    fprintf(cutFile, "\\enddata{%s,%d}\n", (eqptr)->GetTypeName(), (eqptr)->UniqueID());
    (((class view *) self)->GetIM())->CloseToCutBuffer( cutFile);
    (eqptr)->DeleteCarefully( pos, pos+len);
    (self)->SetDotLength( 0);

    (self)->Changed( EQVIEW_eq);
}

void eqview_Copy(class eqview  *self)
{
    class eq *eqptr = Eq(self);
    long pos, len;
    FILE *cutFile;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    cutFile = (((class view *) self)->GetIM())->ToCutBuffer();
    fprintf(cutFile, "\\begindata{%s,%d}\n", (eqptr)->GetTypeName(), (eqptr)->UniqueID());
    fprintf(cutFile, "%s", eqview_cut_prefix);
    eqptr->writeID = im::GetWriteID();
    (eqptr)->WriteFILE( cutFile, pos, pos+len, ' ');
    fprintf(cutFile, "%s", eqview_cut_suffix);
    fprintf(cutFile, "\\enddata{%s,%d}\n", (eqptr)->GetTypeName(), (eqptr)->UniqueID());
    (((class view *) self)->GetIM())->CloseToCutBuffer( cutFile);
}

/*
 * Paste takes account of zilches.
 * Caution: this is a near copy of eq_InsertTokensCarefully,
 * but the two are inconsistent wrt scripted zilches.
 */

void eqview_Paste(class eqview  *self)
{
    class eq *eqptr = Eq(self);
    register int i;
    long pos, len;
    FILE *pasteFile;
    long ct;
    struct formula *f;

    i = 0;
    ct = (self->imPtr)->Argument();

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    pos += len;
    len = 0;

    if ((f = (eqptr)->Access( pos)) != NULL && f->symbol == zilch)
	(eqptr)->Delete( pos);
    if ((f = (eqptr)->Access( pos-1)) != NULL && f->symbol ==  zilch)
	(eqptr)->Delete( --pos);

    while(i < ct) {
	char s[256], name[64];
	long inid;

	pasteFile = (((class view *) self)->GetIM())->FromCutBuffer();
	while (fgets(s, 256, pasteFile) != NULL) {
	    if (*s == '\\') {
		if (strncmp(s, "\\enddata{", 9) == 0) {
		    sscanf(s, "\\enddata{%s,%ld}\n", name, &inid);
		    break;
		}
		if (strncmp(s, "\\begindata{", 11) == 0)
		    sscanf(s, "\\begindata{%s,%ld}\n", name, &inid);
	    }
	    len = (eqptr)->ReadFILE( pasteFile, pos);
	    break;
	}
	i++;
	(((class view *) self)->GetIM())->CloseFromCutBuffer( pasteFile);
    }


    /* The cut buffer is set up to contain an autonomous equation
      in case we are pasting into something other than an eq
      object.

      When we paste into eq, we remove the additional crud:
      "{ lpile d_eqstyle { " before and "} } " after. */

    /* first confirm the crud is there */
    if ((len >= 5) &&
	 ((eqptr)->Access( pos)->symbol->type == BEGIN) &&
	 ((eqptr)->Access( pos + 1)->symbol->type == ALIGN) &&
	 ((eqptr)->Access( pos + 2)->symbol->type == EQSTYLE) &&
	 ((eqptr)->Access( pos + 3)->symbol->type == BEGIN)) {
	for (i = 3; i >= 0; i --) {
	    (eqptr)->Delete( pos + i);
	    len -= 1;
	}
    }

    /* eliminate unmatched groups */
    i = (eqptr)->FindEndGroup( pos);
    while (i < pos+len) {
	(eqptr)->Delete( i);
	len -= 1;
	i = (eqptr)->FindEndGroup( i);
    }
    i = (eqptr)->FindBeginGroup( pos+len);
    while (i >= pos) {
	(eqptr)->Delete( i);
	len -= 1;
	i = (eqptr)->FindBeginGroup( i);
    }
    (self)->SetDotPosition( pos);
    (self)->SetDotLength( len);

    (self)->Changed( EQVIEW_eq);
}

/*
 * Perhaps this should go away
 */

void eqview_Exit()
{
    exit(0);
}

/*
 * For debugging
 */

void eqview_DumpAndWrite(class eqview  *self)
{
    FILE *file;
    class eq *eqptr = Eq(self);

    if (file = fopen("new.eq", "w"))
	(eqptr)->Write( file, 0L, 1);
    else
	message::DisplayString(self, 0, "Couldn't write to file new.eq.");
    fclose(file);
    (eqptr)->Dump( "/tmp/eq");
}

/*
 * Online documentation
 */

void eqview_doc()
{
    printf("not yet\n");
}

/*
 * Special symbols prefixed with ESC
 */

/*
 * Called as a result of eqview_Special
 */

void eqview_DoSpecial(class eqview  *self, char  *s)
{
    long pos, len, added;

    if (strcmp(s, "}") == 0)
	eqview_Close(self, '}');
    else {
	pos = (self)->GetDotPosition();
	len = (self)->GetDotLength();

	pos += len;
	if (len>0) {
	    (self)->SetDotPosition( pos);
	    (self)->SetDotLength( 0);
	}

	if (strcmp(s, "bar") == 0)
	    eqview_Bar(self);
	else if (strcmp(s, "dot") == 0)
	    eqview_Dot(self);
	else if (strcmp(s, "prime") == 0)
	    eqview_Prime(self);
	else {
	    if (strcmp(s, "{") == 0)
		s = "lbrace";
	    added = (Eq(self))->InsertTokensCarefully( pos, s);
	    (self)->SetDotPosition( pos+added);
	    (self)->Changed( EQVIEW_eq);
	}
    }
}

struct eq_helpsplot {
    message_workfptr HelpWork;
    char *hrock;
    char *keywd;
    int keywdlen;
    long nummatches;
};

struct eq_completesplot {
    int sofar, len;
    char *origstr; 
    char *buffer;
    int buffersize;
    enum message_CompletionCode res;
};

/* like strcmp(), but it returns the index of the first character where s1 and s2 differ. If s1 and s2 are identical, returns -1. */
static int lenstrcmp(char *s1 , char *s2)
{
    int ix;
    for (ix=0; TRUE; ix++, s1++, s2++) {
	if (!*s1 && !*s2) return (-1);
	if (*s1 != *s2) return ix;
    }
}

static void CompletionSplot(char  *name , char  *original, struct eq_completesplot  *rock)
{
    int ix, jx;

    ix = lenstrcmp(name, rock->origstr);
    if (ix == (-1)) {
	/* exactly the same as the input */
	strncpy(rock->buffer, name, rock->buffersize);
	rock->buffer[rock->buffersize-1] = '\0';
	if (rock->res==message_Valid || rock->res==message_CompleteValid)
	    rock->res = message_CompleteValid;
	else
	    rock->res = message_Complete;
	rock->sofar = strlen(rock->buffer);
    }
    else if (ix == rock->len) {
	/* matched all of the input, but there's more */
	if (rock->sofar < rock->len)  {
	    /* first one found that does -- use all of it */
	    strncpy(rock->buffer, name, rock->buffersize);
	    rock->buffer[rock->buffersize-1] = '\0';
	    rock->sofar = strlen(rock->buffer);
	    rock->res = message_Complete;
	}
	else {
	    /* another -- use the greatest common prefix */
	    jx = lenstrcmp(name, rock->buffer);
	    if (jx==(-1)) {
		/* this is the same alias -- ignore it! */
	    }
	    else if (jx==rock->sofar)
		rock->res = message_CompleteValid;
	    else {
		rock->buffer[jx] = '\0';
		rock->sofar = jx;
		rock->res = message_Valid;
	    }
	}
    }
    else if (ix > rock->sofar) {
	/* matched part of input, but more than we have so far */
	strncpy(rock->buffer, name, ix);
	rock->buffer[ix] = '\0';
	rock->sofar = ix;
	rock->res = message_Invalid;
    }

}

static enum message_CompletionCode EqCompletionProc(char  *string, class eqview  *self, char  *buffer, int  buffersize)
{
    struct helpAlias *ta;
    int ix, jx;
    char origstr[64];
    struct eq_completesplot hcsplot;

    strncpy(origstr, string, 64);
    origstr[64-1] = '\0';

    hcsplot.origstr = origstr;
    hcsplot.len = strlen(origstr);
    hcsplot.sofar = (-1);
    hcsplot.buffer = buffer;
    hcsplot.buffersize = buffersize;
    strcpy(hcsplot.buffer, "");
    hcsplot.res = message_Invalid;

    eq::EnumerateSymbols((eq_enumfptr)CompletionSplot, (void *)&hcsplot);

    message::DeleteCharacters(self, strlen(buffer), 999);

    return hcsplot.res;
}


static void EqEnumProc(char *name, char  *desc, struct eq_helpsplot *rock)
{
    int ix = lenstrcmp(name, rock->keywd);
    char tmp[64], *tmpptr;

    if (ix==(-1) || ix==rock->keywdlen) {
	if (desc && desc[0]) {
	    sprintf(tmp, "\t\"%s\"", desc);
	    tmpptr = tmp;
	}
	else {
	    tmpptr = NULL;
	}
	(*(rock->HelpWork))((long)rock->hrock, message_HelpListItem, name, tmpptr);
	rock->nummatches++;
    }
}

static void EqHelpProc(char  *partialKeyword, class eqview  *rock, message_workfptr HelpWork, char  *hrock)
{
    struct eq_helpsplot hhsplot;

    if (!HelpWork) return;
    hhsplot.HelpWork = HelpWork;
    hhsplot.hrock = hrock;
    hhsplot.keywd = partialKeyword;
    hhsplot.keywdlen = strlen(partialKeyword);
    hhsplot.nummatches = 0;

    if (hhsplot.keywdlen) {
	(*HelpWork)((long)hrock, message_HelpGenericItem, "Possible completions for \"", NULL);
	(*HelpWork)((long)hrock, message_HelpGenericItem, partialKeyword, NULL);
	(*HelpWork)((long)hrock, message_HelpGenericItem, "\" (note that capitalization is important):\n\n", NULL);
    }
    else {
	(*HelpWork)((long)hrock, message_HelpGenericItem, "Symbols available (note that capitalization is important):\n\n", NULL);
    }
    eq::EnumerateSymbols((eq_enumfptr)EqEnumProc, (void *)&hhsplot);

    if (hhsplot.nummatches == 0) {
	(*HelpWork)((long)hrock, message_HelpGenericItem, "There are no symbols that begin with that string.\n", NULL);
    }
}


void eqview_Special(class eqview  *self, char  c)
{
    char name[100];
    long code;

    code = message::AskForStringCompleted(self, 0, "Symbol? ", 0, name, sizeof(name), NULL, (message_completionfptr)EqCompletionProc, (message_helpfptr)EqHelpProc, (long)self, message_MustMatch);
    if (code < 0) {
	message::DisplayString(self, 0, "Punt!");
	return;
    }
    if (strlen(name) == 0) {
	message::DisplayString(self, 0, "No name specified.");
	return;
    }
    eqview_DoSpecial(self, name);
    message::DisplayString(self, 0, "Done.");
}

/*
void eqview_Punt(self)
struct eqview *self;
{
    message_DisplayString(self, 0, "Bad command.");
    return;
}
*/

/*
 * Scripts
 */

void eqview_SuperScript(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long pos, len, n;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotLength( 0);
    (self)->SetDotPosition( pos = (pos + len));

    if (n = (eqptr)->DoScript( pos, SUP, "sup { zilch ^}"))
	(self)->SetDotPosition( n);
    (self)->Changed( EQVIEW_eq);
}

void eqview_SubScript(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long pos, len, n;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotLength( 0);
    (self)->SetDotPosition( pos = (pos + len));

    if (n = (eqptr)->DoScript( pos, SUB, "sub { zilch ^}"))
	(self)->SetDotPosition( n);
    (self)->Changed( EQVIEW_eq);
}

void eqview_AboveScript(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long pos, len, n;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotLength( 0);
    (self)->SetDotPosition( pos = (pos + len));

    if (n = (eqptr)->DoScript( pos, ABOVE, "above { zilch ^}"))
	(self)->SetDotPosition( n);
    (self)->Changed( EQVIEW_eq);
}

void eqview_BelowScript(class eqview  *self)
{
    register class eq *eqptr = Eq(self);
    long pos, len, n;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();
    (self)->SetDotLength( 0);
    (self)->SetDotPosition( pos = (pos + len));

    if (n = (eqptr)->DoScript( pos, BELOW, "below { zilch ^}"))
	(self)->SetDotPosition( n);
    (self)->Changed( EQVIEW_eq);
}

/*
 * Special symbols that require a string of symbols
 * There should be a third parameter to keymap-called routines!
 */

void eqview_String(class eqview  *self, char  *s)
{
    long pos, len, added;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    if (len > 0) {
	pos += len;
	(self)->SetDotPosition( pos);
	(self)->SetDotLength( 0);
    }

    added = (Eq(self))->InsertTokensCarefully( pos, s);
    (self)->SetDotPosition( pos+added);
    (self)->Changed( EQVIEW_eq);
}

void eqview_Root(class eqview  *self)
{
    long pos;

    eqview_String(self, "root { ^zilch }");
    pos = (self)->GetDotPosition();
    (self)->SetDotPosition( pos-2);
}

void eqview_Fraction(class eqview  *self)
{
    long pos;

    eqview_String(self, "{ cpile { zilch ^} over { zilch } }");
    pos = (self)->GetDotPosition();
    (self)->SetDotPosition( pos-7);
}

void eqview_lbrace(class eqview  *self)
{
    eqview_String(self, "lbrace");
}

static struct bind_Description eqviewBindings[]={
    /* FILE menu card */
/*    {"eqview-write-out-file", "\030\027",0,"File,Save As~21",0,0,(proctable_fptr)eqview_WriteOutFile, "Write equation to specified file."}, */

/*    {"eqview-read-in-file", "\030\022",0,"File,Read File~31",0,0,(proctable_fptr)eqview_ReadInFile, "Read in specified file."}, */

    {"eqview-write-c", "\032c",0,"File,C~41",0,0,(proctable_fptr)eqview_WriteC, "Write equation in C format."},
    /*{"eqview-write-ps", "\032p",0,"File,PS~41",0,0,(proctable_fptr)eqview_WritePS, "Write equation in PostScript format."},*/
    {"eqview-write-eqn", "\032e",0,"File,EQN~42",0,0,(proctable_fptr)eqview_WriteEqn, "Write equation in EQN format."},
    {"eqview-write-troff", "\032t",0,"File,TROFF~43",0,0,(proctable_fptr)eqview_WriteTroff, "Write equation in troff format."},
    {"eqview-write-dvi", "\032d",0,"File,DVI~44",0,0,(proctable_fptr)eqview_WriteDvi, "Write equation in DVI format."},

/*    {"eqview-preview-me", "\032v",0,"File,Preview~51",0,0,(proctable_fptr)eqview_PreviewMe, "Preview equation."}, */
/*    {"eqview-print-me", "\032p",0,"File,Print~52",0,0,(proctable_fptr)eqview_PrintMe, "Print equation."}, */

    /* FORMULA menu card */
    {"eqview-root", "\022",0,"Formulae,Root~11",0,0,(proctable_fptr)eqview_Root, "Insert a root."},
    {"eqview-root", "\033root\n",0,0},

    {"eqview-fraction", "/",0,"Formulae,Fraction~21",0,0,(proctable_fptr)eqview_Fraction,"Insert a fraction."},

    {"eqview-superscript", "^",0,"Formulae,Superscript~31",0,0,(proctable_fptr)eqview_SuperScript, "Insert a superscript."},
    {"eqview-subscript", "_",0,"Formulae,Subscript~32",0,0,(proctable_fptr)eqview_SubScript, "Insert a subscript."},

    {"eqview-above-script", "!",0,"Formulae,Above~41",0,0,(proctable_fptr)eqview_AboveScript, "Insert an abovescript."},
    {"eqview-below-script", "#",0,"Formulae,Below~42",0,0,(proctable_fptr)eqview_BelowScript, "Insert a belowscript."},

    {"eqview-special", "\033a",0,"Formulae,Insert Symbol...~51",0,0,(proctable_fptr)eqview_Special, "Special symbol (dialog)."},

    /* FRONT menu card */
    {"eqview-cut", "\027",0,NULL,0,0,(proctable_fptr)eqview_Cut, "Cut~11"},
    {"eqview-copy", "\033w",0,NULL,0,0,(proctable_fptr)eqview_Copy, "Copy~12"},
    {"eqview-paste", "\031",0,"Paste~10",0,0,(proctable_fptr)eqview_Paste, "Paste~13"},

/*    {"eqview-save", "\030\023",0,"Save~31",0,0,(proctable_fptr)eqview_Save, "Save equation in default file."}, */

/*    {"eqview-exit", "\030\003",0,"Quit~50",0,0,(proctable_fptr)eqview_Exit, "Exit eq."}, */
/*    {"eqview-exit", EOF,0,0}, */

    /* Debugging commands */
    {"eqview-dump-and-write", "\030\030",0,NULL,0,0,(proctable_fptr)eqview_DumpAndWrite, "Dump equation and write it to tmp."},

    /* Special commands */
/*    {"eqview-punt", "\007",0,NULL,0,0,(proctable_fptr)eqview_Punt, "Abort current command."}, */
/*    {"eqview-symbol-documentation", "\023",0,"List symbols~30",0,0,(proctable_fptr)eqview_doc, "Documentation for symbols."}, */

    /* Positioning commands */
    {"eqview-move-forward", "\006",0,NULL,0,0,(proctable_fptr)eqview_MoveForward, "Move forward."},
    {"eqview-move-backward", "\002",0,NULL,0,0,(proctable_fptr)eqview_MoveBackward, "Move backward."},
    {"eqview-move-to-beginning", "\001",0,NULL,0,0,(proctable_fptr)eqview_MoveToBeginning, "Move to the beginning."},
    {"eqview-move-to-end", "\005",0,NULL,0,0,(proctable_fptr)eqview_MoveToEnd, "Move to the end."},
    {"eqview-move-up", "\020",0,NULL,0,0,(proctable_fptr)eqview_MoveUp, "Move up."},
    {"eqview-move-down", "\016",0,NULL,0,0,(proctable_fptr)eqview_MoveDown, "Move down"},
    {"eqview-carriage-return", "\015",0,NULL,0,0,(proctable_fptr)eqview_CR, "Handle carriage returns."},

    /* Other commands */
    {"eqview-delete-forward", "\004",0,NULL,0,0,(proctable_fptr)eqview_DeleteForward, "Delete forward from cursor."},
    {"eqview-delete-backward", "\010",0,NULL,0,0,(proctable_fptr)eqview_DeleteBackward, "Delete backward from cursor."},
    {"eqview-delete-backward", "\177",0},
    /*{"eqview-undelete", "\025",0,NULL,0,0,(proctable_fptr)eqview_UnDelete, "Undelete."}, */

    {"eqview-left-brace", "{",0,NULL,0,0,(proctable_fptr)eqview_lbrace, "Insert a left brace."},
    {"eqview-close-paren", ")",')',NULL,0,0,(proctable_fptr)eqview_Close, "Insert a close paren."},
    {"eqview-close-bracket", "]",']',NULL,0,0,(proctable_fptr)eqview_Close, "Insert a close bracket."},
    {"eqview-close-brace", "}",'}',NULL,0,0,(proctable_fptr)eqview_Close, "Insert a close brace."},

    {"eqview-prime", "\'",0,NULL,0,0,(proctable_fptr)eqview_Prime,"Insert a prime."}, 
    NULL
};

static struct bind_Description eqviewCutBindings[]={
    /* FILE menu card */
/*    {"eqview-write-out-file", NULL,0,"File,Save As~21",0,0}, */

    {"eqview-read-in-file", NULL,0,"File,Read File~31",0,0},

    {"eqview-write-c", NULL,0,"File,C~41",0,0},
    {"eqview-write-eqn", NULL,0,"File,EQN~42",0,0},
    {"eqview-write-troff", NULL,0,"File,TROFF~43",0,0},
    {"eqview-write-dvi", NULL,0,"File,DVI~44",0,0},

/*    {"eqview-preview-me", NULL,0,"File,Preview~51",0,0}, */
/*    {"eqview-print-me", NULL,0,"File,Print~52",0,0}, */

    /* FRONT menu card */
    {"eqview-cut", NULL,0,"Cut~10",0,0},
    {"eqview-copy", NULL,0,"Copy~11",0,0},

/*    {"eqview-save", NULL,0,"Save~31",0,0}, */

/*    {"eqview-exit", NULL,0,"Quit~50",0,0}, */

    NULL
};

class keymap *eqview_InitKeyMap(struct ATKregistryEntry   *classInfo, class menulist  **eqviewMenus , class menulist  **eqviewCutMenus)
{
    class keymap *keymapp = new keymap;
    char str[2];
    register int i;
    struct proctable_Entry *def;

    if (eqviewMenus != NULL)
	*eqviewMenus = new menulist;
    if(eqviewCutMenus!=NULL)
	*eqviewCutMenus=new menulist;

    def=proctable::DefineProc("eqview-default", (proctable_fptr)eqview_Default, classInfo, NULL, "Insert a character.");

    str[0] = ' ';
    str[1] = '\0';

    for (i = 32; i<127; i++) {
	(keymapp)->BindToKey( str, def, i);
	str[0]++;
    }
    bind::BindList(eqviewBindings, keymapp, *eqviewMenus, classInfo);
    bind::BindList(eqviewCutBindings, NULL, *eqviewCutMenus, classInfo);

    return keymapp;
}
