#ifndef _AMENU_H
#define _AMENU_H
/* Copyright 1995 Carnegie Mellon University All rights reserved. */
/** \addtogroup libsupviews
 * @{ */
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

/** The graphical representation of a menu item.
 *  Derivations of this abstract class draw a menu item's contents. */
class AmenuGraphic {
  public:
    /** Compute the width and height of this graphic.
     *  This should return the minimum width and height required to
     *  render this graphic in a view on the same display as \p vw.
     *  \param vw A view on the display on which this is to be rendered.
     *            This is usually the view into which the graphic will be drawn.
     *  \param w The returned width.
     *  \param h The returned height.
     */
    virtual void ComputeSize(view *vw, long *w, long *h)=0;
    /** Draw this graphic.
     *  The graphic should be positioned in the given bounds, clipping to
     *  the damaged rectangle if appropriate.  The background will already
     *  have been filled.
     *  \param vw The view into which to draw.
     *  \param bounds The region into which to draw.  The entire region must
     *                be filled.
     *  \param sensitive FALSE if insensitive (grayed/ghosted out).
     *  \param selected TRUE if the item is currently selected.
     *  \param damaged If non-NULL, it is only necessary to fill the region
     *                 which is the intersection of this and bounds. */
    virtual void Draw(view *vw, const rectangle *bounds, boolean sensitive, boolean selected, const rectangle *damaged=NULL)=0;
    AmenuGraphic();
    virtual ~AmenuGraphic()=0;    
};

/** The graphical representation of a text label menu item.
 *  This draws a plain text label. */
class AmenuLabel : public AmenuGraphic {
  public:
    AmenuLabel(const char *label, const char *font=NULL, const char *color=NULL, unsigned short pos=graphic::ATLEFT|graphic::BETWEENTOPANDBOTTOM, boolean atomize=TRUE);
    /**< Create a simply styled menu label.
     *   A label is just text (a font) in a color, so only those styling
     *   options are available in this constructor.  Note that while the
     *   full set of standard button look preferences are read, only these
     *   two really matter.
     *   \param label The label text.  If NULL, "none" is used.
     *   \param font The font to use.
     *   \param color The color to render the label text in.
     *   \param pos The position of the label.  An ored combination of
     *              graphic::position constants.
     *   \param atomize If TRUE, save the label in an atom.  Otherwise, the label
     *          text's lifetime must meet or exceed this object's lifetime.
     *   \sbuttonpref{AmenuLabel,label,menu item label,label}
     */
    AmenuLabel(const char *label, struct sbutton_prefs *prefs, unsigned short pos=graphic::ATLEFT|graphic::BETWEENTOPANDBOTTOM, boolean atomize=TRUE);
    /**< Create a menu label with full styling.
     *   See the other constructor for preferences which may affect appearance.
     *   \param label The label text.  If NULL, "none" is used.
     *   \param prefs the styling preferences for this label.
     *   \param pos The position of the label.  An ored combination of
     *              graphic::position constants.
     *   \param atomize If TRUE, save the label in an atom.  Otherwise, the label
     *          text's lifetime must meet or exceed this object's lifetime.
     */
    AmenuLabel();
    /**< Default constructor for menu label representation.
     *   Builds a demo menu label.  Do not call.  */
    virtual ~AmenuLabel();
    void ComputeSize(view *vw, long *w, long *h);
    void Draw(view *vw, const rectangle *bounds, boolean sensitive, boolean selected, const rectangle *damaged=NULL);
    AmenuLabel &SetPosition(unsigned short spos);
    /**< Modify the label's positioning.
     *   \param pos The position of the label.  An ored combination of
     *              graphic::position constants.
     *   \return the label, for chaining calls. */
    AmenuLabel &SetLabel(const char *label, boolean atomize=TRUE);
    /**< Modify the label's text.
     *   \param label The label text.  If NULL, "none" is used.
     *   \param atomize If TRUE, save the label in an atom.  Otherwise, the label
     *          text's lifetime must meet or exceed this object's lifetime.
     *   \return the label, for chaining calls. */
    AmenuLabel &SetPrefs(struct sbutton_prefs *prefs);
    /**< Modify the label's look.
     *   \param prefs the styling preferences for this label.
     *   \return the label, for chaining calls. */
    AmenuLabel &SetColor(const char *c);
    /**< Modify the label's foreground color.
     *   \param color The color to render the label in.
     *   \return the label, for chaining calls. */
    AmenuLabel &SetFont(const char *f);
    /**< Modify the label's font.
     *   \param font The font to use.
     *   \return the label, for chaining calls. */
  private:
    const atom *astr;
    const char *str;
    unsigned short pos;
    struct sbutton_prefs *prefs; // FIXME: static functions access this
    void EnsurePrefs();
    void GetModPrefs();
};

/** The graphical representation of a menu separator
 *  This draws horizontal line. */
class AmenuSep : public AmenuGraphic {
  public:
    AmenuSep(struct sbutton_prefs *prefs=NULL);
    /**< Create a menu separator with full styling.
     *   See the other constructor for preferences which may affect appearance.
     *   \param prefs the styling preferences for this separator. */
    AmenuSep(const char *topshadow, const char *bottomshadow);
    /**< Create a simply styled menu separator.
     *   The separator is a line with beveled edges, so only the edge
     *   colors need to be set.  Note that while the full set of standard
     *   button look preferences are read, only these two really matter.
     *   \param topshadow The top bevel color.
     *   \param bottomshadow The bottom bevel color.
     *   \sbuttonpref{AmenuSep,separator,menu separator,separator}
     */
    void SetShadows(const char *topshadow, const char *bottomshadow);
    /**< Set the color of the separator.
     *   \param topshadow The top bevel color.
     *   \param bottomshadow The bottom bevel color. */
    virtual ~AmenuSep();
    void ComputeSize(view *vw, long *w, long *h);
    void Draw(view *vw, const rectangle *bounds, boolean sensitive, boolean selected, const rectangle *damaged=NULL);
    void SetPrefs(struct sbutton_prefs *prefs);
    /**< Set the styling options of the separator.
     *   \param prefs the styling preferences for this separator. */
  private:
    struct sbutton_prefs *prefs; // FIXME: static functions access this
    void EnsurePrefs();
    void GetModPrefs();
};

class AmenuAuxlabels;

/** A single line (entry) in a menu. */
class AmenuEntry  {
    // An insensitive menu item won't be highlighted or activated.
    boolean sensitive;
    /*AmenuGraphic objects on the left list will be drawn to the left of the
     label. Objects on the right list will be drawn to the right.  A future enhancement
     might allow the entries to be rotated 90 degrees for use as a menubar. */
    /* note that these need to be pointers to keep AmenuAuxlabels type private */
    AmenuAuxlabels *left, *right;
    /* The primary label for this item. */
    AmenuGraphic *label;
    /* The arguments to be passed when this item is chosen. */
    avalueflex args;
    /* The action to be invoked. */
    aaction *act;
    /* The object on which this action will be invoked. */
    ATK *obj;
    /* The key to be used to identify this entry. */
    const atom *key;
    friend class AmenuCard; // for key
    friend class AmenuCardv; // for sensitive
    
  public:
    // The lack of a GetKey() makes me wonder if I should make key public
    inline AmenuEntry &SetKey(const atom *k) {
	key=k;
	return *this;
    }
    /**< Explicitly set the key for this menu entry.
     *   By default, the key of a labeled menu entry is the label text.
     *   \param k The new key.
     *   \return The menu entry for call chaining. */
    // Set a string label.
    AmenuEntry &SetLabel(const char *label, struct sbutton_prefs *prefs=NULL, unsigned short pos=graphic::ATLEFT|graphic::BETWEENTOPANDBOTTOM);
    /**< Set the menu entry type to text label, and set the label appearance.
     *   \param label The label text.  If NULL, "none" is used.
     *   \param prefs The full styling preferences for this menu item.
     *   \param pos The position of the label.  An ored combination of
     *              graphic::position constants.
     *   \return The menu entry for call chaining. */
    AmenuEntry &SetLabel(const char *label, const char *font=NULL, const char *color=NULL, unsigned short pos=graphic::ATLEFT|graphic::BETWEENTOPANDBOTTOM);
    /**< Set the menu entry type to text label, and set the label appearance.
     *   A label is just text (a font) in a color, so only those styling
     *   options are available in this constructor.  Note that while the
     *   full set of standard button look preferences are read, only these
     *   two really matter.
     *   \param label The label text.  If NULL, "none" is used.
     *   \param font The font to use.
     *   \param color The color to render the label text in.
     *   \param pos The position of the label.  An ored combination of
     *              graphic::position constants.
     *   \return The menu entry for call chaining. */
    
    // The lack of a GetLabel() makes me wonder if I should make label public
    /* Sets the label graphic for this entry. */
    inline AmenuEntry &SetLabel(AmenuGraphic *new_label) {
	label=new_label;
	return *this;
    }
    /**< Set the menu entry type to a given text label.
     *   \param new_label The label.
     *   \return The menu entry for call chaining. */
    // The lack of a GetSensitivity() makes me wonder if I should make sensitife publc
    inline AmenuEntry &SetSensitivity(boolean sense) {
	sensitive=sense;
	return *this;
    }
    /**< Set the sensitive flag for this menu entry.
     *   An insensitive entry does not get highlighted or activated.
     *   It is usually rendered as if grayed/ghosted out.
     *   \param sense The sensitivity flag.
     *   \return The menu entry for call chaining. */
    // The lack of a GetArgs() makes me wonder if I should make args public
    AmenuEntry &SetArgs(const avalueflex &a);
    /**< Set the input arguments to the activation function.
     *   The input arguments set here are passed to the
     *   \p in parameter of the activation function, with an
     *   additional named argument named "object" which is this.
     *   \param a The input parameter array.
     *   \return The menu entry for call chaining. */
    AmenuEntry &SetAction(ATK *o, const aaction *a);
    /**< Set the function to call on activation.
     *   This function is called with the configured input parameters,
     *   or just a named parameter named "object" which is this if no
     *   parameters have been set.
     *   The \p out return values are ignored.  The \p in */
    AmenuEntry &SetAction(ATK *o, const aaction *a, const avalueflex &av);
    /* Adds a graphic to be displayed to the left or right of the label. Lower priority values place the graphic closer to the label. */
    // the lack of a Remove function or a way to iterate through them makes
    // me think maybe I should've left left & right public.
    AmenuEntry &AddLeftGraphic(AmenuGraphic *g, int priority=10);
    /**< Add a graphic to be displayed to the left of the label.
     *   \param g The graphic item to display.
     *   \param priority Ordering of multiple graphics to the left;
     *                   lower numbers are closer to the main graphic. */
    AmenuEntry &AddRightGraphic(AmenuGraphic *g, int priority=10);
    /**< Add a graphic to be displayed to the right of the label.
     *   \param g The graphic item to display.
     *   \param priority Ordering of multiple graphics to the right;
     *                   lower numbers are closer to the main graphic. */

    static const short ICONPOS = graphic::BETWEENLEFTANDRIGHT | graphic::BETWEENTOPANDBOTTOM;
    AmenuEntry &AddLeftIcon(const char *icon, const char *color=NULL, int priority=10, unsigned short pos=ICONPOS);
    /**< Add an icon to the left of the main label.
     *   An icon is a text label in the "icon12" font.
     *   \param icon The text of the label.
     *   \param color The foreground color of the label.
     *   \param priority Ordering of multiple graphics to the left;
     *                   lower numbers are closer to the main graphic.
     *   \param pos The position of the label.  An ored combination of
     *              graphic::position constants.  */
    AmenuEntry &AddRightString(const char *str, const char *font=NULL, const char *color=NULL, int priority=10, unsigned short pos=graphic::ATLEFT|graphic::BETWEENTOPANDBOTTOM);
    /**< Add a label to the right of the main label.
     *   This is generally used for keyboard shortcuts.
     *   \param str The text of the label.
     *   \param font The font of the label.
     *   \param color The foreground color of the label.
     *   \param priority Ordering of multiple graphics to the left;
     *                   lower numbers are closer to the main graphic.
     *   \param pos The position of the label.  An ored combination of
     *              graphic::position constants.  */

    /* Returns the size of the label. */
    virtual void ComputeSize(view *vw, long *labelw, long *leftw, long *rightw, long *h);
    /** Compute the total size of the menu item.
     *  This computes the minimum size of the menu item within the given view.  It
     *  includes all left and right label additions.
     *  \param vw A view on the display on which this is to be rendered.
     *            This is usually the view into which the graphic will be drawn.
     *  \param labelw The returned main label width.
     *  \param leftw The returned left extension width.
     *  \param rightw The returned right extension width.
     *  \param h The returned height. */

    /* Draws the item.  The label  is drawn within labelbounds.  The left and right graphics are drawn within  the bounds, to the left and right of the labelbounds respectively. Drawing will be restricted to the damaged rectangle. */
    virtual void Draw(view *vw, const rectangle *bounds, const rectangle *labelbounds, boolean sensitive, boolean selected, const rectangle *damaged=NULL);
    /** Draw the menu item.
     *  \param vw The view into which to render the item.
     *  \param bounds The bounds of the area into which to render.
     *  \param labelbounds The bounds into which to render the main label.
     *         The left and right extension graphics are drawn within the
     *         bounds, but to the left and right of \p labelbounds,
     *         respectively.
     *  \param sensitive If FALSE, draw grayed/ghosted out.
     *  \param selected If TRUE, draw highlighted.
     *  \param damaged If non-NULL, restrict drawing to this rectangle.
     */
    AmenuEntry();
    virtual ~AmenuEntry();
  private:
    virtual void Dispatch();
};

class AmenuEntrylist;

/** Data object for storing a menu.
 *  An AmenuCard stores a menu card.  It does not have a serialized (on-disk)
 *  representation; it is intended to be created and filled in using its
 *  menu building methods.  These methods make building a menu using
 *  chained method calls relatively easy.  For example, here is the default
 *  demo menu:
 *
 *  \qcode
 *    DEFINE_AACTION_FUNC_CLASS(democlass,AmenuCard);
 *    static void demofunc(AmenuCard *obj, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
 *      printf("obj:%p\n", obj);
 *      printf("aux:%ld\n",aux[0].Integer());
 *      printf("in:%ld\n",in[0].Integer());
 *    }
 *    static democlass demo(demofunc,9L);
 *    AddEntry(10). SetAction(this,&demo,42L). AddLeftIcon("\100").
 *                  SetLabel("Foo", NULL, "pink");
 *    AddEntry(5).  SetAction(this,&demo,88L). AddRightString(NULL,"andy22i").
 *                  AddLeftIcon("\047"). SetLabel("foo", NULL, "pink");
 *    AddSeparator(7);
 *  \endqcode
 *
 *   In this demo, 9L 42L and 88L are all automatically converted to
 *   single-entry avalueflex arrays with one unnamed member of type integer.
 *   AddEntry() creates a menu entry with a given priority.  SetAction()
 *   sets its action.  The remaining calls set the style and contents of the
 *   menu entry.
 *
 *   This was probably developed as an easer-to-use alternative to
 *   the cmenu package used by default for menus, but it never got beyond
 *   the demo stage.
 *
 *   The default ViewName for this data object is AmenuCardv.
 */
class AmenuCard : public dataobject {
  public:
    TRACED_CLASS(AmenuCard);
    inline AmenuCard();
    /**< Default constructor.
     *   This reads the preferences and prepares for menu construction.
     *   \sbuttonpref{AmenuCard,menu card,card,menu item}
     */
    virtual AmenuEntry &AddEntry(AmenuEntry *e, int priority=10);
    /**< Adds an entry line to a menu card.
     *   \param e The entry.
     *   \param priority Where to place the entry.
     *   \returns the menu entry for chaining with other methods. */
    virtual AmenuEntry &AddEntry(int priority=10);
    /**< Creates and adds an entry line to a menu card.
     *   \param priority Where to place the entry.
     *   \returns the menu entry for chaining with other methods. */
    virtual AmenuEntry &AddSeparator(int piority, struct sbutton_prefs *prefs=NULL);
    /**< Creates and adds a separator line to a menu card.
     *   \param priority Where to place the entry.
     *   \param prefs Styling preferences.
     *   \returns the menu entry for chaining with other methods. */
    virtual AmenuEntry &AddSeparator(int priority, const char *topshadow, const char *bottomshadow);
    /**< Creates and adds a separator line to a menu card.
     *   \param priority Where to place the entry.
     *   \param topshadow The top shadow color, if non-NULL. Otherwise, use the default.
     *   \param bottomshadow The bottom shadow color, if non-NULL.  Otherwise, use the default.
     *   \returns the menu entry for chaining with other methods. */
    virtual void RemoveEntry(AmenuEntry *e);
    /**< Remove a specific entry line from the menu card.
     *   Does nothing if the entry is not in the menu card.
     *   \param e The entry to remove. */
    virtual void RemoveEntry(const atom *key); 
    /**< Remove a named entry line from the menu card.
     *   The default key assigned to a menu entry is its label, set with
     *   AmenuEntry::SetLabel, but it can also be set explicitly using
     *   AmenuEntry::SetKey.  Does nothing if the entry is not in the menu card.
     *   \param key The name of the entry to remove. */
    void Sort();
    /**< Ensure that menu items are correctly sorted by priority.
     *   Call this after changing menu items' priority. */
    AmenuEntry &operator[](size_t i);
    /**< Access menu items on this card.
     *   \param i The entry number.  Out-of-range indices produce
     *            indeterminate results.
     *   \return a reference to the requested menu item. */
    size_t GetN();
    /**< Find the number of menu items on this card.
     *   \return the number of items on this card. */
  private:
    struct sbutton_prefs *prefs;
    AmenuEntrylist *entries;
    friend class AmenuCardv;
};

class AmenuEntries;

/** A view to display AmenuCard menu cards */
class AmenuCardv : public view {
  public:
    TRACED_CLASS(AmenuCardv);

    AmenuCardv();
    view::DSattributes DesiredSize(long  width, long  height, enum view::DSpass  pass, long  *desired_width, long  *desired_height);
    void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
    void Update();
    class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
    AmenuEntry *highlight, *newhighlight; // tjm - FIXME: used by AmenuOptionv
  private:
    AmenuEntries *ed;
  protected:
    AmenuEntry *FindHit(long x, long y);
};


class AmenuOption : public AmenuCard {
    void EnsurePrefs();
    friend class AmenuOptionv;
  public:
    TRACED_CLASS(AmenuOption);
    AmenuEntry *selected;
    struct sbutton_prefs *prefs;
    AmenuOption();
    /**< Default constructor.
     *   \sbuttonpref{AmenuOption,option,option selector,option selector}
     */
};

class AmenuOptionCardv;
class AmenuOptionv : public view {
  public:
    TRACED_CLASS(AmenuOptionv);
    AmenuOptionv();
    view::DSattributes DesiredSize(long  width, long  height, enum view::DSpass  pass, long  *desired_width, long  *desired_height);
    void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
    void Update();
    class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;

    AmenuOptionCardv *card;
    popupwin *win;
};
/** @} */
#endif
