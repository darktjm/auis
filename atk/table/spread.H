#ifndef _spread_H_
#define _spread_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtable
 * @{ */
/* spread.H - table view definition and interface */
#define spread_VERSION 1

/* internal dimensions */

#define	spread_LINEWIDTH 1	    /* width of lines between cells */
#define	spread_SPACING 3	    /* total additional space between cells */
#define	spread_CELLMARGIN 1	    /* white border stolen from cell contents */
#define	spread_BORDER(V) (V->sborder)	    /* width of table border */

/* reference to the associated data object */

#define MyTable(V)  ((class table *)((V)->GetDataObject()))

#define CtoX(V, j) (spread_Width(V, 0, j) + spread_BORDER(V) - V -> hOffset)
#define RtoY(V, j) (spread_Height(V, 0, j) + spread_BORDER(V) - V -> vOffset)

/* loops over cells */

#define FirstX(V, j, z) j = 0, z = spread_BORDER(V) - V -> hOffset
#define FirstY(V, j, z) j = 0, z = spread_BORDER(V) - V -> vOffset
#define NextX(V, j, z) z += (j<MyTable(V)->NumberOfColumns()?V-> colInfo[j++].computedWidth:0) + spread_SPACING
#define NextY(V, j, z) z += ((j<MyTable(V)->NumberOfRows())?V -> rowInfo[j++].computedHeight:V->rowInfo[MyTable(V)->NumberOfRows()-1].computedHeight)

/* information about row displays */

struct rowInformation {
    int	computedHeight;		/* most recently computed thickness */

};

struct colInformation {
    int computedWidth; /* most recently computed thickness */
};

/* Interface definition */
#include <table.H>
#include <view.H>
class spread: public  view {
    public:
      virtual ATKregistryEntry *ATKregistry();


      void WantInputFocus(class view *requestor);
      void Print(FILE *f, const char *processor, const char *format, boolean toplevel);
      void *GetPSPrintInterface(const char *printtype);
      void PrintPSDoc(FILE *outfile, long pagew, long pageh);
      void PrintPSRect(FILE *file, long logwidth, long logheight, struct rectangle *visrect);
      void FullUpdate(enum view::UpdateType how, long left, long top, long width, long height);
      void Update();
      void WantUpdate(class view *requestor);
      class view * Hit(enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
      view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
      void WantNewSize(class view *requestor);
      void ReceiveInputFocus();
      void LoseInputFocus();
      class view * GetApplicationLayer()  ;
      const void * GetInterface(const char *type)  ;
      void LinkTree(class view * parent);
      void ObservedChanged(class observable *changed, long status);
      void UnlinkNotification(class view *unlinkedTree);
      void ExposeChild(class view *v);
      boolean RecSearch(class search *pat, boolean toplevel);
      boolean RecSrchResume(class search *pat);
      boolean RecSrchReplace(class dataobject *text, long pos, long len);
      void ChildLosingInputFocus();
      void ChildReceivingInputFocus();
      void WantExposure(class view *requestor, struct rectangle *childrect);

      void RecSrchExpose(const struct rectangle &logical, struct rectangle &hit);
      
      inline void SetHideOnReadOnly(boolean o) { this->hidero=o; }
      inline boolean GetHideOnReadOnly() { return this->hidero; }
      inline void SetBlockOnReadOnly(boolean o) { this->blockro=o; }
      inline boolean GetBlockOnReadOnly() { return this->blockro; }

      static boolean InitializeClass();
      /* initialize class */

      spread();	/* initialize instance */

      virtual ~spread();	/* clean up instance */
      static boolean WantLimitedHighlighting()  ;

      virtual void ComputeSizes();
      
      boolean hasInputFocus;		/* res ipse dixit */
      boolean childhasfocus;
      int updateRequests;		/* update requests pending */
      boolean borderDrawn;		/* border, etc are visible */
      int rowInfoCount;		/* number of entries in rowInfo */
      struct rowInformation *rowInfo;	/* row display information */
      int colInfoCount;
      struct colInformation *colInfo;
      char movemode;			/* in midst of a line move */
      short currentslice;		/* current (dim ? column : row) index */
      short currentoffset;		/* loc (pels in inset) of current slice */
      short icx,icy;			/* hit location */
      int lastTime;			/* timestamp of last update */
      long vOffset;			/* vertical scroll offset */
      long hOffset;			/* horizontal scroll offset */
      struct chunk anchor;		/* chunk selected by downpress */
      struct chunk selection;		/* currently selected chunk */
      boolean selectionvisible;		/* selection has been drawn */
      char bufferstatus;		/* what is in keysin */
#define BUFFEREMPTY 0			/* nothing there */
#define BUFFERHASINPUT 1		/* reading new formula */
#define BUFFERHASFORMULA 2		/* displaying selected cell formula */
#define BUFFERHASMESSAGE 3		/* displaying message */
#define BUFFERHASPARAM 4		/* reading parameter */
      class keystate *keystatep;	/* keyboard input tracker */
      class menulist *menulistp;	/* menus for this view */
      class cursor *tableCursor;	/* cursor within the table */
      short recsearchposh, recsearchposv; /* last cell searched */
      short recsearchsubstart, recsearchsublen; /* segment of label which matched RecSearch. Only defined if recsearchvalid && !recsearchchild. */
      class view *recsearchchild;	/* child in which last RecSearch found something */
      boolean recsearchvalid;		/* was last RecSearch successful? */
      class fontdesc *writingFont;	/* font for table entries */
      long standardHeight;		/* height of a digit or letter */
      long zeroWidth;			/* width of zero in writingfont */
      long dotWidth;			/* width of dot in writingfont */

      class graphic *grayPix;	/* halftone filler */
      class graphic *blackPix;	/* pure black filler */
      boolean	finalizing;		/* being finalized? */
      class keystate *readonlykeystate;
      class menulist *readonlymenulist;
      boolean readonlyposted;
      boolean blockro;
      boolean hidero;
      long sborder;
      struct rectangle crect;
      boolean sizescached;
};
/** @} */
/* end of spread.H */


#endif /* _spread_H_ */
