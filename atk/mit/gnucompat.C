/* $Author: rr2b $ */

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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/mit/RCS/gnucompat.C,v 1.5 1995/03/26 03:00:47 rr2b Stab74 $";
#endif


 

/*************************************************
 * GNU Emacs Compatiblity package.
 *************************************************/

/*************************************************
 * Copyright (C) 1990 by the Massachusetts Institute of Technology
 * Permission to use, copy, modify, distribute, and sell this
 * software and its documentation for any purpose is hereby
 * granted without fee, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.  M.I.T. makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *************************************************/

/* Contains the following functions to make ez compatible with */
/* GNU Emacs: */			

/* Fill Paragraph:  Removes hard newlines within a paragraph */

#include <andrewos.h>

#include <ctype.h>



#include <bind.H>
#include <text.H>
#include <textview.H>
#include <im.H>
#include <message.H>
#include <environ.H>
#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <txtstvec.h>

#include <gnucompat.H>

#define DIALOG 100
#define MESSAGE 0

/* Routines appear in this file in "bottom-up" order. */
/* This is so I don't have to deal with declaring forward */
/* references. */

/* Added friendly read-only behavior from txtvcmds.c */


ATKdefineRegistry(gnucompat, ATK, gnucompat::InitializeClass);
#ifndef NORCSID
#endif
static boolean ConfirmReadOnly(class textview  *self, class text  *txt);
int back_to_start (class text  *txt, int  pos);
static void gcparafill (class textview  *self, long  key);
static int is_itemp (class text  *txt, int  start , int  end);
static void insertBullets (class textview  *self, long  key);
static void removeBullets (class textview  *self, long  key);
static int parse_num (class text  *txt, int  start , int  end , int  *numret);
static void enumerate (class textview  *self, long  key);
static void denumerate (class textview  *self, long  key);
static void do_insert(class textview  *self,char  *t, boolean  nl_flag);
static void insert(class textview  *self,char  *t);
static void insert_no_nl(class textview  *self,char  *t);
static void AddDefaultTpl(class textview  *self, long  l);


static boolean ConfirmReadOnly(class textview  *self, class text  *txt)
{
    if ((txt)->GetReadOnly()) {
        message::DisplayString(self, 0,
          "Document is read only.");
        return TRUE;
    } else
        return FALSE;
}


/* Travel backward until we get a bonafide paragraph break. */
/* Return buffer position of first character of this paragraph */
/* First character is defined as first non-whitespace char */
/* unless we are at the end of the buffer, in which case */
/* we point at the end of the buffer even though it be whitespace */

int back_to_start (class text  *txt, int  pos)
{
    int cur;

    while (pos >=0 ) {
	cur = (txt)->GetChar( pos);
	if (cur == '\n') {
	    if (pos == 0) break;
	    else if ((txt)->GetChar( pos - 1)	== '\n')
		break;
	    else if (pos == (txt)->GetLength ())
		break;
	    else if ((cur = (txt)->GetChar( pos + 1))	== '\t')
		return (pos + 1);
	    else if (cur == ' ') return (pos + 1);
	}
	pos--;
    }
    /* Skip over newline if we're on it. */
    if (pos == (txt)->GetLength ()) return ((pos>0) ? pos-1 : 0);
    else if ((txt)->GetChar( pos) == '\n') return (pos + 1);
    else return pos;
}
	
/* Fill a paragraph.*/

/* This algorithm is careful to not damage an already filled */
/* paragraph. */

/* Here is the paragraphing algorithm: */
/* Backslash-newline is converted to a space */
/* Lines that begin with whitespace are preceded by a newline. */
/* Lines that contain tab or triple space after a non-white */
/*   character are preceded and followed by newlines. */
/* An empty line is given a real newline before and after. */
/* Sentence enders are : ; ? . !  If they occur at end of line,*/
/*   they will be followed by at least two blanks. */
/*  (Same if followed by quote or right parenthesis.) */

static void gcparafill (class textview  *self, long  key)
{
    class text *txt = (class text *)self->dataobject;
    int pos, len, cur, npos, count, nextc, end;
    int one_only = 0, modified = 0, found_table;

    if (ConfirmReadOnly(self, txt))
        return;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    end = pos + len;

    if (len == 0) {
	/* No region, fill current paragraph. */
	pos = back_to_start (txt, pos);
	end = (txt)->GetLength ();
	len = end - pos;
	one_only = 1;
    }

    while (pos < end) {
	/* for each newline */
	cur = (txt)->GetChar( pos);
	if (cur == '\n') {
	    /* If this is the last char in the region */
	    /* we're done */
	    if (pos + 1 == end)
		break;
	    /* keep newline preceded by newline */
	    nextc = (txt)->GetChar( pos + 1);
	    if ((pos > 0) && ((txt)->GetChar( pos-1) == '\n')) {
		pos++;
		if (one_only) break;
	    }
	    /* or followed by tab (save char in cur for next else) */
	    else if (nextc == '\t') {
		pos++;
		if (one_only) break;
	    }
	    /* or followed by space */
	    else if (nextc == ' ') {
		pos++;
		if (one_only) break;
	    }
	    /* or followed by a newline */
	    else if (nextc == '\n') {
		pos++;
		if (one_only) break;
	    }
	    /* replace backslash newline pair with a space */
	    else if ((pos > 0) &&
		     ((txt)->GetChar( pos-1) == '\\')) {
		(txt)->ReplaceCharacters( pos-1, 2, " ", 1);
		len--;
		end--;
		modified = 1;
	    }
	    else {
		npos = pos + 1;
		found_table = 0;
		while ((npos <= end) &&
		       ((cur = (txt)->GetChar( npos)) != '\n')){
		    if ((cur == '\t') ||
			((cur == ' ') &&
			 ((txt)->GetChar( npos+1 ) == ' ') &&
			 ((txt)->GetChar( npos+2 ) == ' '))) {
			found_table = 1;
		    }
		    npos++;
		}
		if ((cur == '\n') && (found_table == 1)) {
		    /* we're in a table keep newline at pos */
		    /* and process the one at npos */
		    pos = npos;
		    if (one_only) break;
		}
		else {
		    /* At last!  Our purpose for existance! */
		    /* compress whitespace */
		    count = 0;
		    while ((pos > 0) &&
			   ((cur = (txt)->GetChar( pos-1) == ' '))) {
			    pos--;
			    count++;
		    }
		    if (count>0) {
			(txt)->DeleteCharacters( pos, count);
			modified = 1;
			len -= count;
			end -= count;
		    }
		    /* find out how many spaces to replace */
		    /* newline with: punctuation followed by these: */
		    switch (cur) {
			case '\'':
			case '\\':
			case '"':
			case '}':
			case ']':
			case ')':
			    cur = (txt)->GetChar( pos-2);
			    break;
		    }
		    /* consistinge of these: */
		    switch (cur) {
			case '.':
			case ':':
			case ';':
			case '?':
			case '!':
			    /* get two spaces. */
			    (txt)->ReplaceCharacters( pos, 1, "  ", 2);
			    len++;
			    end++;
			    modified = 1;
			    break;
			default:
			    /* otherwise one space */
			    (txt)->ReplaceCharacters(	pos, 1,	" ", 1);
			    modified = 1;
		    }
		    pos = npos;
		}
	    }
	}
	else pos++;
    }
    if (modified) {
	(txt)->NotifyObservers( 0);
	if (!one_only) (self)->SetDotLength ( len);
    }
}

/*
 * Is what we're pointing at an item tag?
 *
 * It will return the count of characters scan'ed.
 * If this is not an item tag, the count is returned as zero.
 * A tag is currently defined as "7 ".
 * We could also test for the symbola environment, but users
 * might have done plainest before discovering de-itemize.
 */

static int is_itemp (class text  *txt, int  start , int  end)
{
    long c;

    if (((txt)->GetChar( start) == '7') &&
	 (start < end) &&
	 ((c = (txt)->GetChar( start + 1)) == ' ' || c == '\t')) return 2;
    else return 0;
}


/*
 * itemize
 *
 * Put a bullet in front of every paragraph in the given region.
 * (Unless one is already there.)
 * If no region is given, put one at the begining of the current
 * paragraph
 */

static void insertBullets (class textview  *self, long  key)
{
    class text *txt = (class text *)self->dataobject;
    class style *stylep;
    
    int dot, pos, len, end;
    long cur, indent, left;
    int one_only = 0, modified = 0;
    struct text_statevector sv;

    if (ConfirmReadOnly(self, txt))
        return;

    stylep = (txt->styleSheet)->Find( "symbola");
    if (stylep == NULL)  {
        stylep = new style;
	(stylep)->SetFontFamily ( "AndySymbola");
	(stylep)->SetName( "symbola");
        (txt->styleSheet)->Add( stylep); 
    }

    dot = pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    end = pos + len;
    (void) (self)->GetStyleInformation( &sv, pos, NULL);
    indent = sv.CurIndentation;
    left = sv.CurLeftMargin;

    /* if len is zero, do current paragraph */
    if (len == 0) {
	/* back up to begining of paragraph */
	if (pos > 0) pos--;
	while (pos > 0 && ((txt)->GetChar( pos - 1)) != '\n') pos--;
	end = (txt)->GetLength ();
	len = end - pos;
	one_only = 1;
    }

    while (pos < end) {
	cur = (txt)->GetChar( pos);
	if (cur == ' ' || cur == '\n' || cur == '\t') {
	    pos++;
	    continue;	/* go to first non-whitespace character */
	}
	else {
	    /* Only itemize lines at the same level */
	    (void) (self)->GetStyleInformation( &sv, pos, NULL);
	    if (sv.CurLeftMargin == left && sv.CurIndentation == indent)
		if (is_itemp(txt, pos, end) == 0) {
		    /* Not already itemized */
		    char c = (char) cur;

		    (txt)->AlwaysInsertCharacters( pos+1, &c, 1);
		    (txt)->AlwaysInsertCharacters( pos+1, "7\t", 2);
		    (txt)->AlwaysDeleteCharacters( pos, 1);
		    (txt->rootEnvironment)->WrapStyle(	pos, 1, stylep);
		    modified = 1;
		    len += 2;
		    end += 2;
		}
	}
	if (one_only) break;
	/* go to end of paragraph */
	while (pos < end) {
	    pos++;	/* always move at least one character */
	    if ((txt)->GetChar( pos) == '\n') break;
	}
	pos++;
    }
    if (modified) {
	(txt)->NotifyObservers( 0);
	if (!one_only) {
	    (self)->SetDotPosition ( dot);
	    (self)->SetDotLength ( len);
	}
    }
}

static void removeBullets (class textview  *self, long  key)
{
    class text *txt = (class text *)self->dataobject;
    struct text_statevector sv;
    int pos, count, len, end;
    long cur, indent, left;
    int one_only = 0, modified = 0;

    if (ConfirmReadOnly(self, txt))
        return;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    end = pos + len;

    /* if len is zero, do current paragraph */
    if (len == 0) {
	/* back up to begining of paragraph */
	if (pos > 0) pos--;
	while (pos > 0 && ((txt)->GetChar( pos - 1)) != '\n') pos--;
	end = (txt)->GetLength ();
	len = end - pos;
	one_only = 1;
    }

    (void) (self)->GetStyleInformation( &sv, pos, NULL);
    indent = sv.CurIndentation;
    left = sv.CurLeftMargin;

    while (pos < end) {
	cur = (txt)->GetChar( pos);
	if (cur == ' ' || cur == '\n' || cur == '\t') {
	    pos++;
	    continue;	/* go to first non-whitespace character */
	}
	else {
	    (void) (self)->GetStyleInformation( &sv, pos, NULL);	    
	    if (sv.CurLeftMargin == left && sv.CurIndentation == indent)
		if ((count = is_itemp (txt, pos, end)) > 0) {
		    (txt)->DeleteCharacters( pos, count);
		    end -= count;
		    len -= count;
		    modified = 1;
		}
	}
	if (one_only) break;
	/* go to end of paragraph */
	while (pos < end) {
	    pos++;	/* always move at least one character */
	    if ((txt)->GetChar( pos) == '\n') break;
	}
	pos++;
    }
    if (modified) {
	(txt)->NotifyObservers( 0);
	if (!one_only) (self)->SetDotLength ( len);
    }
}
/*
 * Parse a number out of a text object.
 *
 * Call this with start pointing at the first digit.
 * It will store the number parsed in numret.
 * It will return the count of characters scan'ed.
 * If the number is not immediately followed by a 
 * period and then a space or tab, the count is
 * returned as zero
 * signifying that this number is to be ignored.
 */

static int parse_num (class text  *txt, int  start , int  end , int  *numret)
{
    int cur_num = 0, count = 0;
    long cur;

    while (start < end && isdigit (cur = (txt)->GetChar( start))) {
	cur_num = 10 * cur_num + (cur - '0');
	start++;
	count++;
    }
    if (count == 0) return 0;
    /* skip over trailing period and whitespace if present */
    if (start < end && (cur = (txt)->GetChar( start))	!= '.') return 0;	/* wrong format */
    if (start < end && ((cur = (txt)->GetChar( start+1))
			 != ' ' && cur != '\t')) 
	return 0;	/* wrong format */
    count += 2;
    *numret = cur_num;
    return count;
}

/*
 * enumerate
 *
 * Put a number in front of every paragraph in the given region.
 *
 * The tricky part is deciding how to sequence the numbers.
 */

static void enumerate (class textview  *self, long  key)
{
    class text *txt = (class text *)self->dataobject;
    struct text_statevector sv;
    int dot, pos, npos, count, tlen, len, end;
    long cur, indent, left;
    int one_only = 0, modified = 0;
    int cur_num, the_number = 0;
    int found_the_number = 0;
    char numstring[20];

    if (ConfirmReadOnly(self, txt))
	return;

    removeBullets(self, 0L); /* Enumeration overrides bullets */

    dot = pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    end = pos + len;

    /* if len is zero, do current paragraph */
    if (len == 0) {
	/* back up to begining of paragraph */
	if (pos > 0) pos--;
	while (pos > 0 && ((txt)->GetChar( pos - 1)) != '\n') pos--;
	end = (txt)->GetLength ();
	len = end - pos;
	one_only = 1;

	/* See if the previous paragraph begins with a number. If so, set the_number */
	npos = pos;
	if (npos > 0) npos--; /* back up over newline we just saw */

	/* back up over additional whitespace between paragraphs */
	while (npos > 0) {
	    cur = (txt)->GetChar( npos);
	    if (!(cur == ' ' || cur == '\n' || cur == '\t'))
		break;
	    npos--;
	    continue;
	}
	    
	/* Now go to begining of the paragraph */
	while (npos > 0 && ((txt)->GetChar( npos - 1)) != '\n') npos--;
	if (parse_num (txt, npos, pos, &cur_num) > 0) {
	    found_the_number = 1;
	    the_number = cur_num + 1;
	}
    }
    
    (void) (self)->GetStyleInformation( &sv, pos, NULL);
    indent = sv.CurIndentation;
    left = sv.CurLeftMargin;

    while (pos < end) {
	cur = (txt)->GetChar( pos);
	if (cur == ' ' || cur == '\n' || cur == '\t') {
	    pos++;
	    continue;	/* go to first non-whitespace character */
	}
	else {
	    (void) (self)->GetStyleInformation( &sv, pos, NULL);
	    if (sv.CurLeftMargin == left && sv.CurIndentation == indent)
		if ((count = parse_num (txt, pos, end, &cur_num)) > 0) {
		    if (found_the_number) {
			if (the_number != cur_num) {
			    sprintf (numstring, "%d.\t", the_number);
			    tlen = strlen(numstring);
			    (txt)->ReplaceCharacters( pos, count, numstring, tlen);
			    count = tlen - count;
			    end += count;
			    len += count;
			    pos += tlen;
			    modified = 1;
			}
			the_number++;
		    } else {
			the_number = cur_num + 1;
			found_the_number = 1;
			pos += count;
		    }
		} else {
		    char c = (char) cur;

		    if (!found_the_number) {
			/* There was no digit seen Set the_number */
			the_number = 1;
			found_the_number = 1;
		    }
		    /* this paragraph has no number, add it */
		    sprintf (numstring, "%d.\t", the_number);
		    tlen = strlen(numstring);
		    (txt)->AlwaysInsertCharacters( pos+1, &c, 1);
		    (txt)->AlwaysInsertCharacters( pos+1, numstring, tlen);
		    (txt)->AlwaysDeleteCharacters( pos, 1);
		    pos += tlen;
		    len += tlen;
		    end += tlen;
		    modified = 1;
		    the_number++;
		}
	}
	if (one_only) break;
	/* go to end of paragraph */
	while (pos < end) {
	    pos++;	/* always move at least one character */
	    if ((txt)->GetChar( pos) == '\n') break;
	}
	pos++;
    }
    if (modified) {
	(txt)->NotifyObservers( 0);
	if (!one_only) {
	    (self)->SetDotPosition ( dot);
	    (self)->SetDotLength ( len);
	}
    }
}

static void denumerate (class textview  *self, long  key)
{
    class text *txt = (class text *)self->dataobject;
    struct text_statevector sv;
    int pos, count, len, end;
    long cur, indent, left;
    int one_only = 0, modified = 0;
    int cur_num;

    if (ConfirmReadOnly(self, txt))
        return;

    pos = (self)->GetDotPosition();
    len = (self)->GetDotLength();

    end = pos + len;

    /* if len is zero, do current paragraph */
    if (len == 0) {
	/* back up to begining of paragraph */
	if (pos > 0) pos--;
	while (pos > 0 && ((txt)->GetChar( pos - 1)) != '\n') pos--;
	end = (txt)->GetLength ();
	len = end - pos;
	one_only = 1;
    }

    (void) (self)->GetStyleInformation( &sv, pos, NULL);
    indent = sv.CurIndentation;
    left = sv.CurLeftMargin;

    while (pos < end) {
	cur = (txt)->GetChar( pos);
	if (cur == ' ' || cur == '\n' || cur == '\t') {
	    pos++;
	    continue;	/* go to first non-whitespace character */
	}
	else {
	    (void) (self)->GetStyleInformation( &sv, pos, NULL);
	    if (sv.CurLeftMargin == left && sv.CurIndentation == indent)
		if ((count = parse_num (txt, pos, end, &cur_num)) > 0) {
		    (txt)->DeleteCharacters( pos, count);
		    end -= count;
		    len -= count;
		    modified = 1;
		}
	}
	if (one_only) break;
	/* go to end of paragraph */
	while (pos < end) {
	    pos++;	/* always move at least one character */
	    if ((txt)->GetChar( pos) == '\n') break;
	}
	pos++;
    }
    if (modified) {
	(txt)->NotifyObservers( 0);
	if (!one_only) (self)->SetDotLength ( len);
    }
}

/*
 * Explicit object insertion code.
 *
 * This is so we can make menu items to insert objects
 * by name rather than prompting and asking for them.
  * for example:
      addmenu gnucompat-insert-inset "Special~4,Insert equation~41" textview gnucompat inherit "eq"
      addmenu gnucompat-insert-inset "My Cart, Insert Eq" textview gnucompat inherit "eq"
 * The flag is true if we should make sure there are newlines around the inset.
 */

static void
do_insert(class textview  *self,char  *t, boolean  nl_flag)
{
    class text *txt;
    char buf[80];
    int result;
    long pos;

    if ((txt = (class text *) (self)->GetDataObject()) == NULL) return;

    if (ConfirmReadOnly(self, txt))
	return;

    if((txt)->GetObjectInsertionFlag() == FALSE){
	message::DisplayString(self, 0, "Object Insertion Not Allowed!");
	return;
    }
    /* Check to see if we are interactive or not... njw*/
    if (!t || (int)(long) t < 256 /* Sigh */) {
	/* This is a bit of overkill, using completedString...
	 * But at some point, it might be nice to have either
	 * completion or help...
	 */
	result = message::AskForStringCompleted(self,
					       0,
					       "Object to insert: " ,
					       NULL, /* no default */
					       buf,
					       sizeof(buf),
					       NULL, /* no special keymap */
					       NULL,/* no completion */
					       NULL, /* no help */
					       0L,
					       message_NoInitialString);
	if (result<0) {
	    message::DisplayString(self, 0, "Object insertion cancelled.");
	    return;
	}
	t = buf;
    }

    pos = (self)->GetDotPosition() + (self)->GetDotLength();
    /* for user convenience, if we don'txt have newlines before and after, we insert them. */
    if (nl_flag) {
	if ((txt)->GetChar( pos) != '\n') {
	    (txt)->InsertCharacters( pos, "\n", 1);
	}
	if ((txt)->GetChar( pos -	1) != '\n'){
	    (txt)->InsertCharacters( pos, "\n", 1);
	    pos++;
	}
    }
    self->currentViewreference = (txt)->InsertObject( pos, t, NULL);
    (txt)->NotifyObservers( 0);
}

static void
insert(class textview  *self,char  *t)
{
    do_insert (self, t, TRUE);
}

static void
insert_no_nl(class textview  *self,char  *t)
{
    do_insert (self, t, FALSE);
}

static void
AddDefaultTpl(class textview  *self, long  l)
{
    class text *txt;

    if ((txt = (class text *) (self)->GetDataObject()) == NULL) return;

    if (ConfirmReadOnly(self, txt))
        return;

    if ((txt)->ReadTemplate( "default", (txt)->GetLength() == 0) < 0)
        message::DisplayString(self, 100, "Could not read template file.");
    else {
	(txt)->RegionModified( 0, (txt)->GetLength());
	(txt)->NotifyObservers( 0);
        message::DisplayString(self, 0, "Enabled Basic Styles.");
    }
}



boolean gnucompat::InitializeClass()
{
    static struct bind_Description compat_fns[] = {
	{"gnucompat-fill-para", NULL, 0, NULL, 0, 0, (proctable_fptr)gcparafill, "Fill Paragraph.", "gnucompat"},
	{"gnucompat-insert-bullets", NULL, 0, NULL, 0, 0, (proctable_fptr)insertBullets, "Put a bullet at the begining of each paragraph in the region", "gnucompat"},
	{"gnucompat-remove-bullets", NULL, 0, NULL, 0, 0, (proctable_fptr)removeBullets, "Remove bullets from the begining of each paragraph in the region", "gnucompat"},
	{"gnucompat-enumerate", NULL, 0, NULL, 0, 0, (proctable_fptr)enumerate, "Put number in sequence at the begining of each paragraph in the region", "gnucompat"},
	{"gnucompat-denumerate", NULL, 0, NULL, 0, 0, (proctable_fptr)denumerate, "Delete number from the begining of each paragraph in the region", "gnucompat"},
	/* Added by njw so that you can keybind all this junk: */
	{"gnucompat-insert-inset", NULL, 0, NULL, 0, 0, (proctable_fptr)insert, "Insert a named inset here", "gnucompat"},
	{"gnucompat-insert-inset-no-nl", NULL, 0, NULL, 0, 0, (proctable_fptr)insert_no_nl, "Insert a named inset without newlines around it", "gnucompat"},
	{"gnucompat-add-default-tpl", NULL, 0, NULL, 0, 0, (proctable_fptr)AddDefaultTpl, "Enable basic styles by adding the template named default", "gnucompat"},

        {NULL},
    };
    struct ATKregistryEntry  *textviewClassinfo;

    textviewClassinfo = ATK::LoadClass("textview");
    if (textviewClassinfo != NULL) {
        bind::BindList(compat_fns, NULL, NULL, textviewClassinfo);
        return TRUE;
    }
    else
        return FALSE;
}
