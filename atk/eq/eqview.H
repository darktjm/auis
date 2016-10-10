#ifndef _eqview_H_
#define _eqview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libeq
 * @{ */
/*
 * eqv.H
 * class header file for eqv.
 */

enum changed {
    EQVIEW_nothing,		/* nada */
    EQVIEW_caret,		/* just the caret */
    EQVIEW_eq,		/* the equation, maybe including caret */
    EQVIEW_everything	/* redraw the whole inset */
};
	 
#include <view.H>
struct formula;
class eqview : public  view {
  public:
    virtual ATKregistryEntry *ATKregistry();


    view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *widthp, long *heightp)  ;
    void Update();
    void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
    void ReceiveInputFocus();
    void LoseInputFocus();
    class view * Hit (enum view::MouseAction action, long x, long y, long clicks)  ;
    void SetDataObject(class dataobject *dataObject);
    void Print(FILE *f, const char *process, const char *final, int toplevel);
    void *GetPSPrintInterface(const char *printtype);
    void DesiredPrintSize(long width, long height, enum view::DSpass pass, long *desiredwidth, long *desiredheight);
    void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);


    virtual struct formula * Draw(class eq *eqptr, struct formula *f, long x, long y)  ;
    virtual long Find(class eq *eqptr, long mx, long my, long restrict)  ;
    virtual void CalculateCaret();
    virtual void DrawCaret();
    virtual void SetDotPosition(long newp);
    virtual void SetDotLength(long newl);
    virtual long GetDotPosition()  ;
    virtual long GetDotLength()  ;
    virtual void Changed(enum changed changed);



    eqview();

    virtual ~eqview();

    static boolean InitializeClass();



    /*
     * The equation pointer in the data object is not view-independent,
     * so the eqview cheats and simply contains a pointer to an equation.
     * Since the equation has markers in it, it doesn't qualify as a bona
     * fide view-independent data object.
     */
    long off_x, off_y;		/* offset from 0,0 to origin of equation */
    enum changed changed;			/* what changed for Update */
    long			/* caret or selection as follows: */
      caret_x,		/* left if selection, x position if caret */
      caret_y,		/* top if selection, y position if caret */
      selection_width,	/* width if selection, 0 if caret */
      selection_height;	/* height if selection, 0 if caret */
    boolean hasinputfocus;	/* doc'n says this is in struct view! */
    boolean embedded;	/* true if GetApplicationLayer is not called */
    class keystate *keystate;	/* for keymap routines */
    class menulist *normalMenus;  /* for menu routines */
    class menulist *cutMenus;  /* for menu routines */
    class mark	*dot;
    char *filename;		/* name of file read in */
};

#define Eq(self) ((class eq *) ((self)->dataobject))
/** @} */
#endif /* _eqview_H_ */
