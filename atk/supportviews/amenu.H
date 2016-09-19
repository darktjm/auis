#ifndef _AMENU_H
#define _AMENU_H
ATK_INTER
/* Copyright 1995 Carnegie Mellon University All rights reserved. */
#include <andrewos.h>
#include <rect.h>
#include <dataobject.H>
#include <view.H>
#include <fontdesc.H>
#include <mflex.H>
#include <sbutton.H>
#include <avalue.H>
#include <aaction.H>
#include <popupwin.H>

class AmenuGraphic {
  public:
    /* Computes the width and height of this graphic. */
    virtual void ComputeSize(view *vw, long *w, long *h)=0;
    /* Draws this graphic in the given bounds, clipping to the damaged rectangle if appropriate. */
    virtual void Draw(view *vw, const rectangle *bounds, boolean sensitive, boolean selected, const rectangle *damaged=NULL)=0;
    AmenuGraphic();
    virtual ~AmenuGraphic()=0;    
};

class AmenuLabel : public AmenuGraphic {
  public:
    AmenuLabel(const char *label, const char *font=NULL, const char *color=NULL, unsigned short pos=graphic_ATLEFT|graphic_BETWEENTOPANDBOTTOM, boolean atomize=TRUE);
    AmenuLabel(const char *label, struct sbutton_prefs *prefs, unsigned short pos=graphic_ATLEFT|graphic_BETWEENTOPANDBOTTOM, boolean atomize=TRUE);
    AmenuLabel();
    virtual ~AmenuLabel();
    void ComputeSize(view *vw, long *w, long *h);
    void Draw(view *vw, const rectangle *bounds, boolean sensitive, boolean selected, const rectangle *damaged=NULL);
    AmenuLabel &SetPosition(unsigned short spos);
    AmenuLabel &SetLabel(const char *label, boolean atomize=TRUE);
    AmenuLabel &SetPrefs(struct sbutton_prefs *prefs);
    AmenuLabel &SetColor(const char *c);
    AmenuLabel &SetFont(const char *f);
    atom *astr;
    char *str;
    unsigned short pos;
    struct sbutton_prefs *prefs;
};


class AmenuSep : public AmenuGraphic {
  public:
    AmenuSep(struct sbutton_prefs *prefs=NULL);
    AmenuSep(const char *topshadow, const char *bottomshadow);
    void SetShadows(const char *topshadow, const char *bottomshadow);
    virtual ~AmenuSep();
    void ComputeSize(view *vw, long *w, long *h);
    void Draw(view *vw, const rectangle *bounds, boolean sensitive, boolean selected, const rectangle *damaged=NULL);
    void SetPrefs(struct sbutton_prefs *prefs);
    struct sbutton_prefs *prefs;
};

struct AmenuGraphicdata {
    AmenuGraphic *ptr;
    int pos;
};

DEFINE_MFLEX_CLASS(AmenuAuxlabels_base,AmenuGraphicdata,1);
class AmenuAuxlabels : public AmenuAuxlabels_base {
    boolean sorted;
  public:
    void Sort();
    boolean Sorted();
    void MarkChanged();
};

#define amenu_ICONPOS (graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM)
class AmenuEntry  {
  public:
    // An insensitive menu item won't be highlighted or activated.
    boolean sensitive;
    /*AmenuGraphic objects on the left list will be drawn to the left of the
     label. Objects on the right list will be drawn to the right.  A future enhancement
     might allow the entries to be rotated 90 degrees for use as a menubar. */
    AmenuAuxlabels left, right;
    /* The primary label for this item. */
    AmenuGraphic *label;
    /* The arguments to be passed when this item is chosen. */
    avalueflex args;
    /* The action to be invoked. */
    aaction *act;
    /* The object on which this action will be invoked. */
    ATK *obj;
    /* The key to be used to identify this entry. */
    atom *key;
    
    /* Sets the label graphic for this key. */
    inline AmenuEntry &SetKey(atom *k) {
	key=k;
	return *this;
    }
    
    // Set a string label.
    AmenuEntry &SetLabel(const char *label, struct sbutton_prefs *prefs=NULL, unsigned short pos=graphic_ATLEFT|graphic_BETWEENTOPANDBOTTOM);
    
    AmenuEntry &SetLabel(const char *label, const char *font=NULL, const char *color=NULL, unsigned short pos=graphic_ATLEFT|graphic_BETWEENTOPANDBOTTOM);
    
    /* Sets the label graphic for this entry. */
    inline AmenuEntry &SetLabel(AmenuGraphic *new_label) {
	label=new_label;
	return *this;
    }
    inline AmenuEntry &SetSensitivity(boolean sense) {
	sensitive=sense;
	return *this;
    }
    inline AmenuEntry &SetArgs(const avalueflex &a) {
	args=a;
	return *this;
    }
    AmenuEntry &SetAction(ATK *o, const aaction *a);
    AmenuEntry &SetAction(ATK *o, const aaction *a, const avalueflex &av);
    /* Adds a graphic to be displayed to the left or right of the label. Lower priority values place the graphic closer to the label. */
    AmenuEntry &AddLeftGraphic(AmenuGraphic *g, int priority=10);
    AmenuEntry &AddRightGraphic(AmenuGraphic *g, int priority=10);

    AmenuEntry &AddLeftIcon(const char *icon, const char *color=NULL, int priority=10, unsigned short pos=amenu_ICONPOS);
    AmenuEntry &AddRightString(const char *str, const char *font=NULL, const char *color=NULL, int priority=10, unsigned short pos=graphic_ATLEFT|graphic_BETWEENTOPANDBOTTOM);

    /* Returns the size of the label. */
    virtual void ComputeSize(view *vw, long *labelw, long *leftw, long *rightw, long *h);

    /* Draws the item.  The label  is drawn within labelbounds.  The left and right graphics are drawn within  the bounds, to the left and right of the labelbounds respectively. Drawing will be restricted to the damaged rectangle. */
    virtual void Draw(view *vw, const rectangle *bounds, const rectangle *labelbounds, boolean sensitive, boolean selected, const rectangle *damaged=NULL);
    AmenuEntry();
    virtual ~AmenuEntry();
    virtual void Dispatch();
};

class AmenuCard;
struct AmenuEntrydata {
    AmenuEntry *ptr;
    int pos;
};

DEFINE_MFLEX_CLASS(AmenuEntrylist_base,AmenuEntrydata,5);
class AmenuEntrylist : public AmenuEntrylist_base {
    boolean sorted;
  public:
    AmenuEntrylist() : sorted(FALSE) {
    }
    void Sort();
    boolean Sorted();
    void MarkChanged();
};

class AmenuCard : public dataobject {
  public:
    virtual ATKregistryEntry *ATKregistry();
    AmenuCard();
    ~AmenuCard();
    AmenuEntrylist entries;
    struct sbutton_prefs *prefs;
    virtual AmenuEntry &AddEntry(AmenuEntry *e, int priority=10);
    virtual AmenuEntry &AddEntry(int priority=10);
    virtual AmenuEntry &AddSeparator(int piority, struct sbutton_prefs *prefs=NULL);
    virtual AmenuEntry &AddSeparator(int priority, const char *topshadow, const char *bottomshadow);
    virtual void RemoveEntry(AmenuEntry *e);
    virtual void RemoveEntry(atom *key); 
    inline void Sort() { entries.Sort(); }
    inline AmenuEntry &operator[](size_t i) { return *entries[i].ptr; }
    inline size_t GetN() { return entries.GetN(); }
};

struct AmenuEntriesdata {
    struct rectangle r;
    AmenuEntry *e;
};

DEFINE_MFLEX_CLASS(AmenuEntries, AmenuEntriesdata, 5);

class AmenuCardv : public view {
  public:
    virtual ATKregistryEntry *ATKregistry();
    AmenuCardv();
    ~AmenuCardv();
    view_DSattributes DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *desired_width, long  *desired_height);
    void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
    void Update();
    class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
    AmenuEntries ed;
    AmenuEntry *highlight, *newhighlight;
};


class AmenuOption : public AmenuCard {
  public:
    virtual ATKregistryEntry *ATKregistry();
    AmenuEntry *selected;
    struct sbutton_prefs *prefs;
    AmenuOption();
    ~AmenuOption();
};

class AmenuOptionCardv;
class AmenuOptionv : public view {
  public:
    virtual ATKregistryEntry *ATKregistry();
    AmenuOptionv();
    ~AmenuOptionv();
    view_DSattributes DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *desired_width, long  *desired_height);
    void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
    void Update();
    class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;

    AmenuOptionCardv *card;
    popupwin *win;
};

    
#endif
