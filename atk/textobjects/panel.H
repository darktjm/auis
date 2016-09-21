ATK_INTER
#ifndef _panel_H_
#define _panel_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtextobjs libtextobjs.so
 *  A statically loadable ATK class file.  Contains chlist, chlistview,
 *  dired, diredview, panel, unknown, unknownv, printopts.
 * @{ */
#include <style.H>
#include <environment.H>
#include <text.H>
#include <cursor.H>

/*
 * A call to panel_SetHandler(panel, HandlerProc, globalTag) is required.
 * The handler procedure receives three arguments
 * and must be defined like this:
 *
 * void HandlerProc(globalTag, tag, self)
 * char *globalTag, *tag;
 * struct panel *self;
 */

typedef void (*panel_hfptr)(char *gtag, char *tag, class panel *self);
typedef void (*panel_keyfptr)(long rock, struct panel *self, char c);

struct panel_Entry {
    struct panel_Entry *next;
    long pos;
    long len;
    char *tag;              /* general mem address */
};

struct key_Entry {
    struct key_Entry *next;
    char key;
    panel_keyfptr proc;
    long rock;
};

#define panel_VERSION   1

#include <textview.H>
class panel: public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();



    
	static boolean InitializeClass();

    
	panel();
    
	virtual ~panel();


    virtual struct panel_Entry * Add(const char *item, char *tag, int showNow)  ;
    virtual void Remove(struct panel_Entry *entry);
    virtual void RemoveAll();
    virtual void FreeAllTags(); /* destroys the client tag(rock) associated w/ a panel_Entry */

    /* MakeSelection highlights but does not call handler. */
    /* User clicking on item highlights and calls handler. */

    virtual void SelectNext();
    virtual void SelectPrevious();
    virtual void ClearSelection();
    virtual void MakeSelection(struct panel_Entry *entry);

    /* Each panel has its own key handlers for simple single-keystrokes. */
    /* Default keys may be reassigned or cancelled using NULL;  they are: */
    /* ^P, P, p, B, and b are bound to do a SelectPrevious. */
    /* ^N, N, n, F, and f are bound to do a SelectNext. */
    /* The proc must be defined like: */
    /*   void Proc(whatever rock, struct panel *receivedBy, char c) */
    /* Things bound to ESC or ^X will not work! */

    virtual void AssignKey(char c, panel_keyfptr proc, long rock);


    inline void SetHandler(panel_hfptr proc, char *gt) 
        { ((this)->handler = (proc), (this)->globalTag = (gt)); }

    /* New highlight style takes effect upon next selection */

    inline void SetHighlightStyle(class style *s) { ((this)->highlightStyle = (s)); }
    inline void SetCursorFont(class fontdesc *cf) { ((this)->iconFont = (cf)); }
    inline void SetCursorIcon(char c) { ((this)->icon = (c)); }

    /* Returns currently selected panel entry, NULL if */
    /* nothing is currently selected. */

    inline struct panel_Entry * CurrentHighlight() { return ((this)->highlightEntry); }

    /* Note: SetBorder, SetCursorFont, SetCursorIcon, and */
    /* SetDefaultStyle require a FullUpdate and should usually */
    /* be used only during initialization. */

    /* Useful inheritance:  SetDefaultStyle(pv, style) */
    /* Useful inheritance: SetBorder(pv, hpix, vpix); */

    /* panel_Entry access macros */

    inline struct panel_Entry * EntryRoot() { return ((this)->panelList); }
    inline struct panel_Entry * EntryNext(struct panel_Entry *pe) { return ((pe)->next); }
    inline char *EntryTag(struct panel_Entry *pe) { return ((pe)->tag); }


     class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void FullUpdate(enum view_UpdateType type, long x, long y, long w, long h);
     void PostMenus(class menulist *ml);
     void PostKeyState(class keystate *ks);
     void SetDataObject(class dataobject *dataObj);


    struct panel_Entry *panelList;
    struct key_Entry *keyList;

    panel_hfptr handler;
    char *globalTag;

    class keystate *keystate;

    class text *textp;
    boolean ourText;    /* This is TRUE if text is (de)allocated by panel.  If the client sets his own text dataobj. we do nothing with self->text and self->ourText is FALSE.  This is for backward compatibility with existing applications that rely on panel being a text/textview pair-- which is, of course, a no-no in an object-oriented environ that supports multiple views on a single dataobject. */

    class fontdesc *iconFont;
    char icon;
    class cursor *cursor;

    class style *highlightStyle;
    struct panel_Entry *highlightEntry;
    class environment *highlightEnv;
};
/** @} */
#endif /* _panel_H_ */
