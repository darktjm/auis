ATK_INTER
#ifndef _palette_H_
#define _palette_H_
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


 



enum palette_location {palette_LEFT = 0, palette_TOP = 1, palette_RIGHT = 2, palette_BOTTOM = 3};

enum palette_autoselect {palette_NONE = 0, palette_TOGGLE = 1, palette_FOLLOWMOUSE = 2, palette_EXCLUSIVE = 3};

enum palette_itemtype {palette_ICON = 0, palette_VIEW = 1, palette_STRING = 2};

#include <view.H>

	typedef void (*palette_hitfptr)(long rock, struct palette_item *info, enum view_MouseAction act, long numclicks);

class palette: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();
    
    static class palette * Create(class view *child, enum palette_location loc)  ;
    
	palette();
    
	virtual ~palette();

    static void DestroyItem(struct palette_item *item);
    static void SelectItem(struct palette_item *item);
    static void DeselectItem(struct palette_item *item);

    
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
     void Update();
     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;

     void WantUpdate(class view *requestor);

    
    virtual void SetLocation(enum palette_location loc);
    virtual void SetChild(class view *child);

    virtual struct palette_item * AddIcon(class fontdesc *font, int ch, int pos, palette_hitfptr fn, long rock, enum palette_autoselect autoselect)  ;
    virtual struct palette_item * AddView(class view *view, int pos, palette_hitfptr fn, long rock, enum palette_autoselect autoselect)  ;
    virtual struct palette_item * AddString(class fontdesc *font, char *str, int pos, palette_hitfptr fn, long rock, enum palette_autoselect autoselect)  ;

    
    static inline long GetRock(struct palette_item *item) ;
    static inline void SetRock(struct palette_item *item, long newrock) ;
    static inline boolean GetAutoSelect(struct palette_item *item) ;
    static inline void SetAutoSelect(struct palette_item *item, enum palette_autoselect sel) ;
    static inline boolean Selected(struct palette_item *item) ;

    
    inline enum palette_location  GetLocation() { return this->loc; }
    inline class view * GetChild() { return this->child; }

    
    enum palette_location loc;
    struct palette_item *items, *hit_item;
    class view *child;
    long border;
    boolean needs_full:1;
};
union palette_iteminfo {
    struct {                            /* For palette_ICON: */
     class fontdesc *font;          /*   The font this icon is in. */
      char ch;                        /*   The char in that font. */
       } icon;
       struct {                            /* For palette_VIEW: */
	class view *view;              /*   The view we should display. */
	 boolean wants_update:1;         /*   If the view wants an update. */
	  } view;
	  struct {                            /* For palette_STRING: */
	   class fontdesc *font;          /*   The font to use. */
	    char *str;                      /*   The string to display. */
	     } str;      
};

struct palette_item {
    class palette *palette;                /* The palette we are in. */
    enum palette_itemtype type;             /* The type of item. */
    union palette_iteminfo  u;
    int pos;                                /* Relative position parameter. */
    long x, y, w, h;                        /* Rect for icon. */
    palette_hitfptr fn;                           /* Function to call. */
    long rock;                              /* Rock to pass to the function */
    enum palette_autoselect autoselect;     /* Automatic selection style */
    boolean selected:1;                     /* True if the item is selected. */
    boolean new_selected:1;                 /* True if the item should be selected. */
    
    struct palette_item *next;
};


inline long palette::GetRock(struct palette_item *item) { return ((item)->rock); }
inline void palette::SetRock(struct palette_item *item, long newrock) { ((item)->rock = (newrock)); }
inline boolean palette::GetAutoSelect(struct palette_item *item) { return ((item)->autoselect); }
inline void palette::SetAutoSelect(struct palette_item *item, enum palette_autoselect sel) { ((item)->autoselect = (sel)); }
inline boolean palette::Selected(struct palette_item *item) { return ((item)->selected); }

#endif /* _palette_H_ */
