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

#include <andrewos.h>
ATK_IMPL("panel.H")
#include <cursor.H>
#include <environment.H>
#include <fontdesc.H>
#include <graphic.H>
#include <bind.H>
#include <keymap.H>
#include <keystate.H>
#include <nestedmark.H>
#include <proctable.H>
#include <style.H>
#include <text.H>
#include <textview.H>
#include <im.H>
#include <view.H>
#include <message.H>

#include <panel.H>

static class keymap *classKeymap;
static class style *defaultHighlightStyle;
static class style *defaultOverallStyle;
static class fontdesc *defaultIconFont = NULL;
static char defaultIcon;

/*
 * Statics
 */


ATKdefineRegistry(panel, textview, panel::InitializeClass);
static void DestroyPanelList(register struct panel_Entry  *pe);
static void DestroyKeyList(register struct key_Entry  *ke);
static void ClearHighlight(register class panel  *self);
static void SetupHighlight(register class panel  *self, struct panel_Entry  *entry);
static void SelectAtPos(class panel  *self, long  pos);
static void KeyDispatch(class panel  *self, long  rock);
static void ProcNext(long  rock, class panel  *self, char  c);
static void ProcPrev(long  rock, class panel  *self, char  c);
static void AddLabels( class panel		 *self );


static void DestroyPanelList(register struct panel_Entry  *pe)
{
    while (pe != NULL) {
        register struct panel_Entry *ne;
        ne = pe->next;
        free(pe);
        pe = ne;
    }
}

static void DestroyKeyList(register struct key_Entry  *ke)
{
    while (ke != NULL) {
        register struct key_Entry *ne;
        ne = ke->next;
        free(ke);
        ke = ne;
    }
}

static void ClearHighlight(register class panel  *self)
{
    register long pos, len;

    if (self->highlightEnv == NULL)
        return;

    pos = (self->highlightEnv)->Eval();
    len = (self->highlightEnv)->GetLength();

    (self->highlightEnv)->Delete();
    (self->textp)->RegionModified( pos, len);

    self->highlightEnv = NULL;
    self->highlightEntry = NULL;
}

static void SetupHighlight(register class panel  *self, struct panel_Entry  *entry)
{
    self->highlightEnv =
      (self->textp)->AddStyle(
        entry->pos, entry->len,
          self->highlightStyle);

    self->highlightEntry = entry;
}

static void SelectAtPos(class panel  *self, long  pos)
{
    register struct panel_Entry *pe;

    for (pe = self->panelList; pe != NULL; pe = pe->next)
        if (pos >= pe->pos && pos <= pe->pos + pe->len)
            break;

    if (pe == NULL)
        return;

    ClearHighlight(self);
    SetupHighlight(self, pe);

    (self)->SetDotPosition( pe->pos);
    (self)->FrameDot( pe->pos);
    (self)->SetDotLength( 0);
    (self)->LoseInputFocus();
    im::ForceUpdate();

    if(self->handler)
	(*self->handler)(self->globalTag, pe->tag, self);
}

static void KeyDispatch(class panel  *self, long  rock)
{
    struct key_Entry *k = self->keyList;
    char c = (char) rock;

    while (k != NULL) {
        if (k->key == c)
            break;
        k = k->next;
    }

    if (k == NULL)
        return;

    (*k->proc)(k->rock, self, c);
}

static void ProcNext(long  rock, class panel  *self, char  c)
{
    (self)->SelectNext();
}

static void ProcPrev(long  rock, class panel  *self, char  c)
{
    (self)->SelectPrevious();
}

/*
 * Class Procedures
 */

boolean panel::InitializeClass()
{
    defaultHighlightStyle = new style;
    (defaultHighlightStyle)->AddNewFontFace( fontdesc_Bold);

    defaultOverallStyle = new style;
    (defaultOverallStyle)->SetJustification( style_LeftJustified);
    (defaultOverallStyle)->SetFontSize( style_ConstantFontSize, 10);
    (defaultOverallStyle)->SetNewLeftMargin( style_LeftMargin,             16384, style_Inches);
    (defaultOverallStyle)->SetNewIndentation( style_LeftEdge,             -16384, style_Inches);

    defaultIconFont = fontdesc::Create("icon", fontdesc_Plain, 12);
    defaultIcon = 'R';

    classKeymap = new keymap;

    {
        struct proctable_Entry *pte;
        unsigned char s[2];

        pte = proctable::DefineProc("key-dispatch",
          (proctable_fptr)KeyDispatch, &panel_ATKregistry_ , NULL, NULL);

        s[1] = '\0';
        for (s[0] = '\0'; s[0] < 128; s[0]++)
            (classKeymap)->BindToKey( (char *)s, pte, (char)s[0]);
    }

    return TRUE;
}

panel::panel()
{
    class style *defstyle=new style;
	ATKinit;

    if(defstyle && defaultOverallStyle) defaultOverallStyle->Copy(defstyle);
    
    this->panelList = NULL;
    this->keyList = NULL;
    this->keystate = keystate::Create(this, classKeymap);
    this->textp = new text;
    (this)->textview::SetDataObject(this->textp);
    this->ourText = TRUE;
    this->handler = NULL;
    this->iconFont = defaultIconFont;
    this->icon = defaultIcon;
    this->cursor = cursor::Create(this);
    this->highlightStyle = defaultHighlightStyle;
    this->highlightEntry = NULL;
    this->highlightEnv = NULL;

    (this)->SetBorder( 5, 5);
    (this)->SetDefaultStyle(defstyle);

    (this)->AssignKey( 'P' - 64, (panel_keyfptr)ProcPrev, 0);
    (this)->AssignKey( 'P', (panel_keyfptr)ProcPrev, 0);
    (this)->AssignKey( 'p', (panel_keyfptr)ProcPrev, 0);
    (this)->AssignKey( 'B', (panel_keyfptr)ProcPrev, 0);
    (this)->AssignKey( 'b', (panel_keyfptr)ProcPrev, 0);
    (this)->AssignKey( 'N' - 64, (panel_keyfptr)ProcNext, 0);
    (this)->AssignKey( 'N', (panel_keyfptr)ProcNext, 0);
    (this)->AssignKey( 'n', (panel_keyfptr)ProcNext, 0);
    (this)->AssignKey( 'F', (panel_keyfptr)ProcNext, 0);
    (this)->AssignKey( 'f', (panel_keyfptr)ProcNext, 0);

    THROWONFAILURE( TRUE);
}

panel::~panel()
{
	ATKinit;

    ClearHighlight(this);   /* clears env */

    DestroyPanelList(this->panelList);
    DestroyKeyList(this->keyList);

    delete this->keystate;
    if(this->ourText) (this->textp)->Destroy();
    delete this->cursor;
}

/*
 * Methods
 */

struct panel_Entry *panel::Add(const char  *item, char  *tag, int  showNow			/* make new selection visible now? */)
{
    register struct panel_Entry *new_c;
    register long len;
    register long textlen;
    register class text *text = NULL;
    char c = '\n';

    text = this->textp;
    new_c = (struct panel_Entry*) malloc(sizeof (struct panel_Entry));
    if (!new_c)
	return (struct panel_Entry *)NULL;

    len = strlen(item);
    textlen = (text)->GetLength();

    new_c->pos = textlen;
    new_c->tag = tag;
    new_c->len = len;
    new_c->next = this->panelList;

    this->panelList = new_c;

    (text)->AlwaysInsertCharacters( textlen, item, len);
    (text)->AlwaysInsertCharacters( textlen + len, &c, 1);
    if (showNow)
	(this)->FrameDot( textlen);
    (text)->NotifyObservers( 0);

    return new_c;
}

void panel::Remove(register struct panel_Entry  *entry)
{
    register long len;
    register struct panel_Entry *pe, **le;

    /* Find and unlink from list */

    le = &this->panelList;
    for (pe = *le; pe; le = &pe->next, pe = *le)
        if (pe == entry)
            break;

    if (pe == NULL)
        return;     /* Invalid entry */

    *le = entry->next;

    /* Clear highlight if the entry to be deleted is highlighted */
    if (entry == this->highlightEntry)
	ClearHighlight(this);

    /* Remove from display and deallocate */

    len = entry->len + 1;
    (this->textp)->AlwaysDeleteCharacters( entry->pos, len);
    (this)->WantUpdate( this);

    for (pe = this->panelList; pe != NULL; pe = pe->next)
        if (pe->pos >= entry->pos)
            pe->pos -= len;

    free(entry);
}

void panel::RemoveAll()
{
    ClearHighlight(this);
    DestroyPanelList(this->panelList);
    this->panelList = NULL;
    (this->textp)->Clear();
    (this)->WantUpdate( this);
}

void panel::SelectNext()
{
    long pos;

    if (this->highlightEnv == NULL)
        return;

    pos = (this->highlightEnv)->Eval() +
      (this->highlightEnv)->GetLength() + 1;

    SelectAtPos(this, pos);     /* Handles end of doc okay. */
}

void panel::SelectPrevious()
{
    long pos;

    if (this->highlightEnv == NULL)
        return;

    pos = (this->highlightEnv)->Eval() - 2;

    SelectAtPos(this, pos);     /* Handles beg. of doc okay. */
}

void panel::ClearSelection()
{
    ClearHighlight(this);
    (this)->WantUpdate( this);
}

void panel::MakeSelection(register struct panel_Entry  *entry)
{
    ClearHighlight(this);

    if (entry == (struct panel_Entry *)NULL)
	return;

    SetupHighlight(this, entry);
    (this)->SetDotPosition( entry->pos);
    (this)->SetDotLength( 0);
    (this)->LoseInputFocus();
    (this)->WantUpdate( this);
}

void panel::AssignKey(char  c, panel_keyfptr proc, long  rock)
{
    struct key_Entry *k;

    k = this->keyList;

    while (k != NULL) {
        if (k->key == c)
            break;
        k = k->next;
    }

    if (k == NULL) {
        k = (struct key_Entry *) malloc(sizeof (struct key_Entry));
        k->next = this->keyList;
        this->keyList = k;
    }

    k->key = c;
    k->proc = proc;
    k->rock = rock;
}

/*
 * Overrides
 */

void panel::FullUpdate(enum view_UpdateType  type, long  x , long  y , long  w , long  h)
{
    register class graphic *graphic;

    graphic = (this)->GetDrawable();
    (this->cursor)->SetGlyph( this->iconFont, this->icon);
    (this)->PostCursor( &graphic->visualBounds, this->cursor);

    (this)->textview::FullUpdate( type, x, y, w, h);
}

void panel::PostMenus(class menulist  *ml)
{
    /* Discard child menu postings */
    (this)->textview::PostMenus(NULL);
}

void panel::PostKeyState(class keystate  *ks)
{
    /* Post our own keystate, discarding keystate from child */

    this->keystate->next = NULL;
    (this->parent)->PostKeyState( this->keystate);
}

class view *panel::Hit(enum view_MouseAction  action, long  x , long  y , long  numberOfClicks)
{
    view *res=(this)->textview::Hit( action, x, y, numberOfClicks);
    if(action==view_LeftDown || action==view_RightDown) {
	SelectAtPos(this, (this)->GetDotPosition());
	(this)->SetDotLength( 0);
	(this)->LoseInputFocus();
    }
    return res;
}

void panel::FreeAllTags()
{
    register struct panel_Entry *e;
    register char *tag;
    
    if((e = (this)->EntryRoot()) != NULL) 
        for(;e != NULL;e = (this)->EntryNext(e))
            if((tag = (this)->EntryTag(e)) != NULL)
                free(tag);
}

static void
AddLabels( class panel		 *self )
  {
  static char		 answer[100];

  while(1) {
      *answer = '\0';
    if(message::AskForString(self,0,"Labels : ",NULL,answer,
			     sizeof(answer)) == -1)
	break;
    if((*answer == '\0') || !strcmp(answer,""))
	break;
    (self)->Add(answer,0,FALSE);
  }
}

void
panel::SetDataObject( class dataobject  *dataObj )
    {
  class text	    *text = NULL;

  if(dataObj) {
    if((text = (class text*)(this)->GetDataObject()) != NULL && 
       (this->ourText == TRUE)) {
      this->ourText = FALSE;
      (this)->FreeAllTags();
      (this)->RemoveAll();
      (text)->Destroy();
    }
    (this)->textview::SetDataObject( dataObj);
    this->textp = (class text *)dataObj;
  }
}
