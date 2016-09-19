#ifndef _table_H_
#define _table_H_
ATK_INTER
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



NO_DLL_EXPORT extern int table_DefaultPrecision;

/* table.ch - table data object definition and interface */

#define table_VERSION 1
#define SafeFromNull(s) (((s) != NULL) ? (s) : "(null)")

/* information about an entire row or column */

struct slice {
    short thickness;
    unsigned short flags;  // only one currently: 1 if default, 0 if set explicitly
};
#define TABLE_DEFAULT_ROW_THICKNESS 0
#define TABLE_DEFAULT_COLUMN_THICKNESS 99
#define FLAG_DEFAULT_SIZE 1

/* a dimension determines whether we are working
    horizontally or vertically at the moment                  */

enum dimension { ROWS, COLS };
typedef enum dimension Dimension;

/* "colors" of cell boundary lines */

typedef char Color;
#define SUPPRESSED 0	/* suppressed when not input focus */
#define JOINED 1	/* joined cells */
#define GHOST 2		/* dotted line, not printed */
#define BLACK 3		/* black line */

/* a chunk is a subrectangle of the spreadsheet */

struct chunk {
    short LeftCol;		/* first column involved */
    short RightCol;		/* last column involved */
    short TopRow;		/* first row involved */
    short BotRow;		/* last row involved */
};
typedef struct chunk * Chunk;

enum extended_type {extended_STANDARD, extended_BOGUS};
struct extended_double_struct {
    union {
	double standard;
	const char *bogus;
    } extended_value;
    enum extended_type type;
};

typedef struct extended_double_struct extended_double;

#define ExtendedType(x) ((x)->type)
#define StandardValue(x) ((x)->extended_value.standard)
#define IsStandard(x) ((x)->type == extended_STANDARD)
#define IsBogus(x) ((x)->type == extended_BOGUS)
#define ExtractBogus(x) ((x)->extended_value.bogus)

/* list of views (of a dataobject) */

struct viewlist {
    struct viewlist *next;	/* next view on list */
    class view	*child;		/* child view - viewing this data object */
    class view	*parent;	/* parent of the child */
};

/* fundamental cell content types */

enum CellTypes {table_EmptyCell, table_TextCell, table_ValCell, table_ImbeddedObject };

/* a cell is an element of a spreadsheet */

struct cell {
    char format;		/* format letter */
    char precision;		/* precision parameter of format */
    char lock;			/* cell is locked */
    int	lastcalc;		/* time stamp of last re-evaluation */
    enum CellTypes celltype;
    union {			/* cell type dependent information */
	int EmptyCell;
	struct {
	    char *textstring;	    /* string preceeded by justification char */
	} TextCell;
	struct {
	    extended_double value;  /* numeric value or nan("message") */
	    char *formula;	    /* symbolic formula */
	} ValCell;
	struct {
	    class dataobject *data; /* embedded object */
	    struct viewlist *views;  /* views of embedded object */
	} ImbeddedObject;
    } interior;
};

/* format codes */

#define GENERALFORMAT 'G'
#define CURRENCYFORMAT '$'
#define PERCENTFORMAT '\045'	/* class preprocessor mangles the real thing */
#define EXPFORMAT 'E'
#define FIXEDFORMAT 'F'
#define HORIZONTALBARFORMAT 'H'
#define VERTICALBARFORMAT 'V'
#define DDMMMYYYYFORMAT 'Q'
#define MMMYYYYFORMAT 'R'
#define DDMMMFORMAT 'S'

#define adjustbyone(n) (n <= 0 ? 0 : n + 1)
#define max(n, m) (n < m ? m : n)

#define getDataObject(T) (*(T))

/* actual interface definition */
#include <attribs.h>
#include <dataobject.H>
class table: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();


  void SetAttributes(struct attributes *attributes);
   void ObservedChanged (class observable *changed, long value);
   const char * ViewName()  ;
   long Read (FILE * f, long id)  ;
   long Write (FILE * f, long writeid, int level)  ;
   long GetModified()  ;


  virtual void ToggleDebug ();
  virtual class table * FindName (const char * name)  ;
  virtual class table * SetName (const char * name)  ;
  virtual void ChangeSize (int nrows, int ncols);
  virtual class table * ExtractData (Chunk chunk)  ;
  virtual void InsertData (class table * T, Chunk chunk);
  virtual void WriteASCII (FILE * f, Chunk chunk);
  virtual class table * ReadASCII (FILE * f)  ;
  virtual void FormatCell (struct cell * cell, char **buff);
  virtual void ParseCell (struct cell * cell, char * buff);
  virtual void ChangeThickness (Dimension dim, int i, int thickness);
  virtual void FindBoundary (Chunk chunk);
  virtual void SetInterior (Chunk chunk, Color color);
  virtual void SetBoundary (Chunk chunk, Color color);
  virtual void ReEval (int r, int c);
  virtual void SetFormat (char ch, Chunk chunk);
  virtual void SetPrecision (int precision, Chunk chunk);
  virtual void Imbed (char *name, Chunk chunk);
  virtual void Lock (char ch, Chunk chunk);
  virtual void FormatDate (double fdate, char *buf, char format);
  
  virtual void RemoveViewFromTable(class view *v);


  inline const char * Name() { return SafeFromNull(this->tablename); }
  inline short &NumberOfRows() { return this->rows; }
  inline short &NumberOfColumns() { return this->cols; }
  inline short &RowHeight(short r) {return this->row[r].thickness; }
  inline short &ColumnWidth(short c) { return this->col[c].thickness; }
  inline struct cell * GetCell(short r,short c) { return (this->cells+((r)*this->cols+(c))); }
  inline char ColorAbove(short r,short c) { return *(this->aboveedge+((r)*(this->cols+1)+(c))); }
  inline char ColorToLeft(short r,short c) { return *(this->leftedge+((r)*(this->cols+1)+(c))); }
  inline char SetAboveColor(short r, short c, char color) { return *(this->aboveedge+((r)*(this->cols+1)+(c))) = color; }
  inline void SetLeftColor(short r, short c, char color) { *(this->leftedge+((r)*(this->cols+1)+(c))) = color; }
  inline boolean IsJoinedAbove(short r, short c) { return ((this)->ColorAbove( r, c) == JOINED); }
  inline boolean IsJoinedToLeft(short r,short c) { return ((this)->ColorToLeft( r, c) == JOINED); }
  inline boolean IsJoinedToAnother(short r,short c) { return ((this)->IsJoinedAbove( r, c) || (this)->IsJoinedToLeft( r, c)); }
  inline int StampEverything() { return this->everythingChanged = ++(this->timeStamper); }
  inline int CurrentTimestamp() { return (this->timeStamper); }
  inline int EverythingTimestamp() { return (this->everythingChanged); }
  inline int CellsTimestamp() { return (this->cellChanged); }
  inline int EdgesTimestamp() { return (this->edgeChanged); }
  inline int WriteTimestamp() { return (this->fileWritten); }
  virtual void SetReadOnly(boolean ro);
  inline boolean GetReadOnly() { return this->readonly; }
  
	static boolean InitializeClass();

  
	table();	/* initialize contents */
  
	virtual ~table();	/* clean up contents */


    char *tablename;			/* tag for write routine */
    class table * NextTable;		/* next table in memory */
    short rows, cols;			/* numbers of rows and columns */
    struct slice *row, *col;		/* row and column information */
    struct cell *cells;			/* rectangular array of cells */
    char *leftedge;			/* edge array (1 extra for end) */
    char *aboveedge;			/* edge array (1 extra for end) */
    int everythingChanged;		/* timestamp when whole grid changed */
    int edgeChanged;			/* timestamp when edges changed */
    int cellChanged;			/* timestamp when cell contents changed */
    int fileWritten;			/* timestamp when file read/written */
    int timeStamper;			/* current timestamp */
    boolean inGetModified;		/* recursion loop stopper for GetModified */
    boolean readonly;
};

/* end of table.ch */

#endif /* _table_H_ */
