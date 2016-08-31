/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1994 - All Rights Reserved
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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

/* gofig.c		

	Code for the gofig data object
*/
/*
 *    $Log: gofig.C,v $
 * Revision 1.7  1995/10/25  17:00:29  wjh
 * fix to allow no edges
 *
 * Revision 1.6  1995/08/04  18:25:41  rr2b
 * Fixed so that the observers can tell when the object has been changed.
 * BUG
 *
 * Revision 1.5  1995/06/14  12:55:59  wjh
 * > changed name from 'flex' to 'gflex'
 *
 * Revision 1.4  1994/11/11  18:07:05  wjh
 * changed index to strchr
 *
 * Revision 1.3  1994/11/10  21:57:32  wjh
 * added final newline so further checkins could happen
 * fixed compile errors on PMax
 *
 * Revision 1.2  1994/11/08  22:09:48  wjh
 * fixed to reconize "10" as part of a prefix.
 *
 * Revision 1.1  1994/10/14  21:04:02  wjh
 * Initial revision
 * 
 * Revision 1.0  94/08/17  18:52:51  wjh
 * Copied from /usr/andrew.C++/lib/null
 */

#include <andrewos.h>
#include <dataobject.H>
#include <text.H>
#include <textview.H>
#include <proctable.H>
#include <gofig.H>


#define MAXFILELINE 255

long gofig::accnum = 0;

static long parseline(gofig *self, char *s, int *prefixlen, int row, 
		int *postfix);
static long parsediagram(gofig *self, FILE *f);
static long converttext(textview *txtv, long c);


ATKdefineRegistry( gofig, dataobject, gofig::InitializeClass );

	boolean
gofig::InitializeClass() {
	struct proctable_Entry *pe = 
		proctable::DefineProc("gofig-convert", (proctable_fptr)converttext, 
			ATK::LoadClass("textview"),  "gofig", 
			"convert selected text to a gofig object");
	return (pe != NULL);
}

gofig::gofig() { 
	/* the defaults correspond to a 9x9 segment 
		in lower left of board with column spacing of 10 points*/
	ATKinit;
	width = 9;
	height = 9;
	edges = LEFTedge | BOTTOMedge;
	printcolwidth = 10000;  /* 10 points */
	stones = new gflex();
	THROWONFAILURE( TRUE );
}

gofig::gofig(int w, int h) {
	ATKinit;
	stones = new gflex();
	width = w;  height = h;
	edges = LEFTedge | BOTTOMedge;
	printcolwidth = 10000;  /* 10 points */
	THROWONFAILURE( TRUE );
}


gofig::~gofig() {
	ATKinit;
	setdimensions( 0, 0 );		/* delete all stones */
}

	static void
guessedges(gofig *self) {
	int e = 0;
	int rows, cols, i;
	self->getdimensions(&cols, &rows);
	int minrow = rows, maxrow = 0, mincol = cols, maxcol = 0;

	/* if there is a stone on the edge, it is probably a board edge */
	for (i = self->nstones(); i--; ) {
		struct stone *s = &(*self)[i];
		if (s->row < minrow) minrow = s->row;
		if (s->row > maxrow) maxrow = s->row;
		if (s->col < mincol) mincol = s->col;
		if (s->col > maxcol) maxcol = s->col;
	}
	for (i = 0; i <3; i++) {
		/* see if there is a stone near a vertical edge */
		/* give precedence to left */
		if (mincol == i)
			{e |= LEFTedge;  break;}
		else if (maxcol == cols-1-i)
			{e |= RIGHTedge;   break;}
	}
	for (i = 0; i <3; i ++) {
		/* see if there is a stone near a horizontal edge */
		/* give precendence to bottom */
		if (maxrow == rows-1-i)
			{e |= BOTTOMedge;   break;}
		else if (minrow == i)
			{e |= TOPedge;   break;}
	}
	/* assume max 19 */
	if (rows == 19) e |= TOPedge | BOTTOMedge;
	if (cols == 19) e |= LEFTedge | RIGHTedge;
	/* all edges if square standard board size */
	if (rows == cols && (rows == 9 || rows == 13))
		e |= TOPedge | RIGHTedge | BOTTOMedge | LEFTedge;
	self->setedges( e );
}

/*
	Alternate format (if no begindata):
	Assume board is encoded in ASCII with O, 0, or o for white 
	and x, X, or # for black.  Assume . or + for empty location.
	Also allow letters (a...n) and digits (1...9) in empty locations 
		(except not at start of first line).
	Also accept 
	Lines may begin with a prefix including whitespace, letters,
	digits, and |.  The prefix on the first line will determine the 
	prefix length and that number of characters are skipped on 
	subsequent lines.  Any illegal character (including |) terminates 
	reading on line.

	The characters ^, @, %, and - standalone and yield empty locations
	with the indicated note.  (# and + can be in empty locations, 
		but must be coded as  .# or .+   (similarly for ~_&

	A multidigit sequence is parsed as a single empty, numbered location.
	A position character (XxOo0#.+) may be followed (without space) by 
		a digit sequence to put that number in that space
		a letter to appear as a note  (-a-zA-Z^#@%+-~_&)

	Ignore '>'.  Convert '<' to a '^' note.
	Terminate at '|' or '\t'
*/
/* parseline parses a single line for the alternate format
	results are inserted in 'self'
	line is taken from 's'
	prefix length is computed if *prefixlen is -1
		otherwise *prefixlen is taken as length of prefix
	row is required so stones can be added
	*postfix is assigned the index of the next character to process
		but if got no elements for the row, *postfix is 0
	Returns the number of elements found for the row.
*/
	static long
parseline(gofig *self, char *s, int *prefixlen, int row, int *postfix) {
	int col = 0;
	int i;
	int letkludge = 0;

	/* determine prefixlen from first line */
	for (i = 0; *prefixlen < 0; i++) {
		switch (s[i]) {
		case '|':
			*prefixlen = i+1;
			break;
		case 'x': case 'X': case'#':
		case 'o': case 'O': case '.':
			*prefixlen = i;
			break;
		case'0': 	/* test for leading "10" */
			if (i <= 0 || s[i-1] != '1')
				*prefixlen = i;
			break;
		case 'A': case 'B': letkludge ++; break;
		case 'C': if (letkludge != 2) break;
			/* assume A B C ... */
			/* DROP THRU */
		case '\n': case '\0':
			if (postfix) *postfix = 0;
			return 0;
		}
	}

	for(i = *prefixlen; ; i++) {
		struct stone *newstone = NULL;
		switch (s[i]) {
		case ' ': case '>': case '\t': 	/* skip space and '>' */
			break;
		case 'o': case 'O': case '0':
			newstone = self->addstone(row, col);
			newstone->color = 'W';
			goto checkfornote;
		case 'x': case 'X': case'#':
			newstone = self->addstone(row, col);
			newstone->color = 'B';
			goto checkfornote;
		case '.': case '+':
	checkfornote:
			col++;
			if (s[i+1] == '\0') {}
			else if (isdigit( s[i+1] )) {
				if (newstone == NULL)
					newstone = self->addstone( 
							row, col-1 );
				i++;
				while ('0'<=s[i] && s[i]<='9') {
					newstone->note = 
						10*newstone->note 
						+ s[i]-'0';
					i++;
				}
			}
			else if (strchr( " \txoXO.", s[i+1] ) != NULL) {}
			else if (isprint( s[i+1] )) {
				/* letter as a note */
				i++;		
				if (newstone == NULL)
					newstone = self->addstone(
							row, col-1);
				newstone->note = 
					- ((s[i]=='<') ? '^' : s[i]);
			}
			break;
		case '1': case'2': case '3': case'4': case '5':
		case'6': case '7': case'8': case '9': 
			newstone = self->addstone(row, col);
			while ('0'<=s[i] && s[i]<='9') 
				newstone->note = 10*newstone->note 
						+ s[i++]-'0';
			col++;
			break;
		case 'a': case 'b': case 'c': case 'd': case 'e': 
		case 'f': case 'g': case 'h': case 'i': case 'j': 
		case 'k': case 'l': case 'm': case 'n': 
		case '^': case '@': case '%': case '-':
			newstone = self->addstone(row, col);
			newstone->note = - s[i];
			col++;
			break;
		default:
			if (postfix) 
				*postfix = (col) ? i : 0;
			return col;
		}
	}
}

	static long
parsediagram(gofig *self, FILE *f) {
	char s[MAXFILELINE + 2];
	int prefixlen = -1;
	int row, cols, maxcols;
	int i;

	/* delete all stones */
	self->setdimensions( 0, 0 );

	row = 0;
	maxcols = 0;
	while (TRUE) {
		/* read the lines of the data stream */
		if ((fgets( s, MAXFILELINE + 2, f )) == 0) {
			/* EOF or error */
			break;
		}

		cols = parseline( self, s, &prefixlen, row, &i );

		if (cols > maxcols) maxcols = cols;
		row++;
	}
	if (cols < maxcols-2) row --;  /* no last row */
	if (row == 0)
		return  dataobject_BADFORMAT;
	self->setdimensions( maxcols, row );

	guessedges(self);

	(self)->SetModified(); /* tell the enclosing document I have changed */
	(self)->NotifyObservers( gofig_SIZECHANGED );

	return dataobject_NOREADERROR;
}


/* gofig__Read(self, file, id)
	Reads the data for 'self' from 'file',  see format in gofig.H
	If 'id' is 0, the data stream is at the outermost level and thus might
	be in an alternate format (see parsediagram).  

	If 'id' is non-zero, the first line of the data stream will have been 
	read by the caller and the value found in the line is passed as 'id'. 
	This must match the id value found in the enddata line.

	Returns an int indication of success.  (See dataobj.ch)
	Fails if syntax of headers is incorrect.

*/
	long
gofig::Read( register FILE *file, register long id ) {
	long result = dataobject_BADFORMAT;
	char s[MAXFILELINE + 2];
	long len;
	long sid, eid;

	if (id == 0) {
		if (fscanf(file, "\\begindata{gofig,%ld", &sid) != 1
				|| getc(file) != '}' || getc(file) != '\n')
			return parsediagram( this, file );
	}
	else {
		int c;
		sid = id;
		if ((c=getc(file)) != '\n')
			ungetc(c, file);
	}

	/* delete all stones */
	setdimensions( 0, 0 );

	int v, w, h, p, e,  r, c, ch;
	/* read header line */
	if (fscanf( file, " %d %d %d %d %d ",
			&v, &w, &h, &p, &e) != 5) 
		return dataobject_PREMATUREEOF;

	if (v != 1 || w<1 || w>25 || h<1 || h>25 || p < 2000 || p > 200000 
			|| e<0 || e>31) 
		return dataobject_BADFORMAT;

	width = w;  height = h;
	printcolwidth = p;
	edges = e;

	while (TRUE) {
		/* read the lines of the data stream */

		char *nl;
		if ((fgets( s, MAXFILELINE + 2, file )) == 0) {
			/* EOF or error */
			result = dataobject_PREMATUREEOF;
			break;
		}
		if (*s == '\\') {
			/* \enddata */
			result = dataobject_NOREADERROR;
			break;
		}

		nl = s + strlen(s) - 1;		/* point at last character */
		if (*nl == '\n')
			*nl = '\0';	/* delete newline*/

		char piece, ch;
		if (sscanf( s, " %d %d %c %d %c ", &r, &c, &piece, &v, &ch ) != 5) 
			break;
		if (r < 0 || r >= height || c < 0 || c > width
				|| (piece!='W' && piece!='B' && piece!='/')
				|| v < 0 || v > 999)
			break;
		struct stone &st = *addstone( r, c );
		st.color = piece;
		st.note = (v != 0) ? v : (ch == '.')  ?  0 : -ch;
	}
	(this)->SetModified(); /* tell the enclosing document I have changed */
	(this)->NotifyObservers( gofig_SIZECHANGED );

	len = strlen("\\enddata{gofig,");
	if (result == dataobject_NOREADERROR &&
			( strncmp( s, "\\enddata{gofig,", len ) != 0
			  || sscanf(s+len, "%ld}\n", &eid) != 1
			  || eid != sid
			) ) 
		result = dataobject_MISSINGENDDATAMARKER;

	return result;
}


/* gofig__Write(self, file, writeID, level)
	Writes a data stream representation of 'self' to 'file'
	The same 'writeID' value is passed in for all writes during a given
	datastream construction.  Once this object has been written, it need 
	not be written again.

	The 'level' can be ignored.  (A level of 0 indicates that the
	output is to be the sole contents of a file, so the format could be 
	non-datastream.  This is only used to support data streams 
	defined prior to the base editor.)
	
	Returns the identifying number written into the header.  The caller 
	uses this number in referring to the object from the dictionary.
*/
	long
gofig::Write( FILE *file, long writeID, int level ) {
	char head[50];
	long id = (this)->GetID();
	int i;
	if (this->writeID != writeID) {
		/* new instance of write, do it */
		this->writeID = writeID;
		sprintf( head, "data{%s, %ld}\n", (this)->GetTypeName(), id );
		fprintf( file, "\\begin%s", head );

		/* prune empty intersections */
		i = 0;
		while (i < nstones()) {
			struct stone &s = (*stones)[i];
			if (s.color == '/' && s.note == 0)
				stones->erase( i );
			else i++;
		}

		/* output lines of data stream */
		fprintf( file, " 1   %d %d   %ld   %d\n",
			width, height, printcolwidth, edges );
		int i;
		for (i = 0; i < nstones(); i++) {
			struct stone &s = (*stones)[i];
			fprintf( file, "%d %d %c %d %c\n", 
				s.row, s.col, s.color, 
				(s.note >= 0) ? s.note : 0, 
				(s.note < 0) ? -s.note : '.' );
		}
		fprintf( file, "\\end%s", head );
	}
	return id;
}


/* write the diagram as ASCII 
	follow the conventions expected by the reading routine
*/
	void
gofig::WriteASCII(FILE *file) {
	char board[25][25];	/* MAX board size is 25 */
	int note[25][25];
	int i, j;
	/* init board and note to empty */
	for (i = 0; i < height; i++) for (j = 0; j < width; j++) {
		board[i][j] = '.';
		note[i][j] = 0;
	}
	/* copy stones to board (to sort in order) */
	for (i = nstones(); i--; ) {
		struct stone &s = (*stones)[i];
		board[s.row][s.col] = (s.color == 'W') ? 'O' 
				: (s.color == 'B') ? '#' : '.';
		note[s.row][s.col] = s.note;
	}
	/* output ASCII from board and note */
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			int nij = note[i][j];
			if (board[i][j] == '.' && nij > 0 && nij < 10)
				fprintf( file, " %d", nij );	/* single digit */
			else if (board[i][j] == '.' 
					&&'a' <= -nij && -nij <= 'n')
				fprintf( file, " %c", -nij );	/* l.c. letter */
			else {
				fprintf( file, " %c", board[i][j] );  /* stone */
				if (nij < 0) 
					fprintf( file, "%c", -nij ); /* letter */
				else if (nij > 0) 
					fprintf( file, "%d", nij );  /* integer */
			}
		}
		fprintf( file, "\n" );
	}

}


/* change width and height.  
	  Any out-of-bounds stones are removed. 
*/
	void 
gofig::setdimensions( int w, int h ) {
	width = w;
	height = h;
	int i;
	for (i = nstones(); i--; ) {
		struct stone &s = (*stones)[i];
		if (s.row >= height || s.col >= width)
			deletestone( s );
	}
}
	
/* add a stone at the indicated location 
	If location is within 0...24, but outside current
	board bounds, the board bounds are adjusted upward.
	Returns a pointer to the new stone so client can adjust it 
		Returns NULL for invalid row or col.
*/
	struct stone *
gofig::addstone( int row, int col ) {
	if (row < 0 || col < 0 || row > 24 || col > 24) 
		return NULL;
	int w = col+1, h = row+1;

	/* if necessary, make the board bigger */
	if (w < width) w = width;
	if (h < height) h = height;
	if (h > height || w > width)
		setdimensions(w, h);

	struct stone *s = (stones)->append();
	s->row = row;
	s->col = col;
	s->note = 0;
	s->color = '/';
	s->accnum = accnum++;
	return s;
}

/* remove the indicated stone 
*/
	void 
gofig::deletestone( int row, int col ) {
	int j;
	for (j = nstones(); j--; ) {
		struct stone &s = (*stones)[j];
		if (s.row == row && s.col == col) {
			stones->erase( j );
			return;
		}
	}
}

/* converttext
	convert the selected portion of the text to a gofig diagram
		(it had better be one)
	excess characters at ends of lines are retained after the diagram
	Strategy:  find length of row on first line
		create a gofig and fill it by calling parseline for each line
		insert the diagram at the beginning in the text
		if there is a capital letter in prefix and it is repeated after
			the diagram on the same line, both are deleted
	Returns the number of rows.
*/
	static long
converttext(textview *txtv, long c) {
	char buf[100];  /* board must be in first 100 bytes of line */
	int prefixlen = -1;
	gofig *newfig = new gofig;
	text *txt = (text *)txtv->GetDataObject();

	/* insert 'newfig' at start of text */
	int startpos = (txtv)->GetDotPosition();
	int len = (txtv)->GetDotLength();

	int pos = startpos;
	int row = 0;
	int cols, maxcols = 0;
	boolean undeletednewline = FALSE;

	while (len > 0) {
		char buf[101], *bx;
		int gotten,  nextch,  linepos = pos;

		/* get 1st 100 chars of line of text to buf & skip the rest */
		gotten = 0;
		bx = buf;
		while (len > 0) {
			int c = (txt)->GetChar(pos);
			pos++; len--;
			if (c == '\n') break;
			if (gotten < 100) {
				gotten++;
				*bx++ = c;
			}
		}
		*bx = '\0';

		/* process the line */
		if (prefixlen >= 0 && bx-buf <= prefixlen)
			continue;	/* nothing on this line */
		cols = parseline( newfig, buf, &prefixlen, row, &nextch );
		if (cols <= 0) 
			continue;  /* nothing on this line */

		if (cols > maxcols) maxcols = cols;
		row++;
		/* delete processed text.  insert tab if text remains after figure */
		/* in prefix, delete immediately preceding whitespace, digits, and |
				(but once find other text, keep it 
				and its preceding whitespace)
		*/
		for (bx = buf + prefixlen; bx-- > buf; ) 
			if ( ! isdigit( *bx ) && ! isspace( *bx ) && *bx != '|') 
				break;
		int startdel = linepos + bx+1-buf;   	/* deletion starts here */

		/*  in postfix, delete W, W|W, or W|WdW
			where W is zero or one space or tab and d is 1 or two digits
			the first W is looked for at buf[nextch]
		*/
		bx = buf+nextch;
		if (*bx == ' ' || *bx == '\t') bx++;
		if (*bx == '|') {
			bx++; 
			if (*bx == ' ' || *bx == '\t') bx++;
			if (*bx == ' ' || *bx == '\t') bx++;
		}
		if (isdigit(*bx)) {
			while (isdigit( *bx )) bx++;
			if (*bx == ' ' || *bx == '\t') bx++;
		}
		/* bx is first char to keep */
		int dellen = linepos + (bx-buf)-startdel;
		if (startdel == linepos && len > 0 
				&& (txt)->GetChar(startpos+dellen) == '\n')
			dellen ++;	/* empty line, delete newline */
		else undeletednewline = TRUE;
		txt->DeleteCharacters( startdel,  dellen );
		pos -= dellen;

		if (*bx && *bx != '\n') {
			txt->InsertCharacters(startdel, "\t", 1);  /* tab */
			pos ++;
		}
	}  /* while TRUE */
	if (row == 0) {
		delete newfig;
		return 0;
	}

	/* finish up newfig and insert in text */
	newfig->setdimensions( maxcols, row );
	guessedges(newfig);

	char *vname = strdup("gofigview");
	(txt)->AddView( startpos, vname, (class dataobject *)newfig);
	if (undeletednewline)
		(txt)->InsertCharacters(startpos+1, "\n", 1);
	(txt)->NotifyObservers(1);

	return row;
}
