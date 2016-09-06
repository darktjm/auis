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
ATK_IMPL("palette.H")
#include <fontdesc.H>
#include <graphic.H>
#include <view.H>
#include <palette.H>

static struct palette_item *FreeList;


ATKdefineRegistry(palette, view, NULL);
static struct palette_item *palette_AddItem(class palette  *self, union palette_iteminfo  info, int  pos, palette_hitfptr fn, long  rock, enum palette_autoselect  autoselect);


palette::palette()
{
    this->loc = palette_LEFT;
    this->items = NULL;
    this->hit_item = NULL;
    this->child = NULL;
    this->border = -1;
    this->needs_full = 0;

    THROWONFAILURE( TRUE);
}

class palette *palette::Create(class view  *child, enum palette_location  loc)
{
    class palette *new_c = new palette;

    (new_c)->SetLocation( loc);
    (new_c)->SetChild( child);

    return new_c;
}

palette::~palette()
{
    struct palette_item *p;

    if ((p = this->items) != NULL)
        while (1)
            if (p->next != NULL) {
                p->next = FreeList;
                FreeList = this->items;
                break;
            }
            else
                p = p->next;

    (this)->SetChild( NULL);
}


void palette::DestroyItem(struct palette_item  *item)
{
    class palette *self = item->palette;
    struct palette_item *ptr, **prev;

    if (self == NULL)
        /* Already been deleted. */
        return;

    for (prev = &self->items; (ptr = *prev) != NULL; prev = &ptr->next)
        if (ptr == item) {
            *prev = ptr->next;

            ptr->next = FreeList;
            FreeList = ptr;
            ptr->palette = NULL;

            if (self->hit_item == item)
                self->hit_item = NULL;

            self->needs_full = 1;
            (self)->WantUpdate( self);

            return;
        }

    /* Something is very wrong. The item said it was in the palette, but the palette didn't know about the item. */
    abort();
}

void palette::SelectItem(struct palette_item  *item)
{
    if (item->new_selected)
        return;
    
    item->new_selected = TRUE;
    (item->palette)->WantUpdate( item->palette);
}

void palette::DeselectItem(struct palette_item  *item)
{
    if (!item->new_selected)
        return;
    
    item->new_selected = FALSE;
    (item->palette)->WantUpdate( item->palette);
}

void palette::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  w , long  h)
{
    struct fontdesc_charInfo info;
    int maxwidth, maxheight, items, columns, col, x, y, tmp;
    long width, height, strwidth, strheight;
    struct palette_item *item;
    struct rectangle child;
    class graphic *black;

    if (type != view_FullRedraw && type != view_LastPartialRedraw)
        return;

    this->needs_full = 0;

    width = (this)->GetLogicalWidth();
    height = (this)->GetLogicalHeight();

    maxwidth = maxheight = 20;
    items = 0;
    for (item = this->items; item != NULL; item = item->next) {
        switch (item->type) {
            case palette_ICON:
                (item->u.icon.font)->CharSummary( (this)->GetDrawable(), item->u.icon.ch, &info);
                if (info.width > maxwidth)
                    maxwidth = info.width;
                if (info.height > maxheight)
                    maxheight = info.height;
                break;
            case palette_VIEW:
                break;
            case palette_STRING:
                (item->u.str.font)->StringSize( (this)->GetDrawable(), item->u.str.str, &strwidth, &strheight);
                maxwidth += strwidth;
                break;
        }
        items++;
    }
    maxwidth += 2;
    maxheight += 2;

    if (height > 0 && width > 0 && items > 0) {
        (this)->SetTransferMode( graphic_COPY);
        switch (this->loc) {
            case palette_LEFT:
                columns = (items * maxheight + height - 1) / height;
                this->border = maxwidth*columns;
                (this)->MoveTo( this->border, 0);
                (this)->DrawLineTo( this->border, height);
                rectangle_SetRectSize(&child, this->border + 1, 0, width - this->border - 1, height);
                col = 1;
                x = 0;
                y = 0;
                for (item = this->items; item != NULL; item = item->next) {
                    item->x = x;
                    item->y = y;
                    item->w = maxwidth;
                    item->h = maxheight;
                    if (++col > columns) {
                        x = 0;
                        y += maxheight;
                        col = 1;
                    }
                    else
                        x += maxwidth;
                }
                break;
            case palette_TOP:
                tmp = width/maxwidth;
                columns = (items + tmp - 1) / tmp;
                this->border = maxheight*columns;
                (this)->MoveTo( 0, this->border);
                (this)->DrawLineTo( width, this->border);
                rectangle_SetRectSize(&child, 0, this->border + 1, width, height - this->border - 1);
                x = 0;
                y = 0;
                for (item = this->items; item != NULL; item = item->next) {
                    item->x = x;
                    item->y = y;
                    item->w = maxwidth;
                    item->h = maxheight;
                    x += maxwidth;
                    if (x > width - maxwidth) {
                        y += maxheight;
                        x = 0;
                    }
                }
                break;
            case palette_RIGHT:
                columns = (items * maxheight + height - 1) / height;
                this->border = width - maxwidth*columns;
                (this)->MoveTo( this->border, 0);
                (this)->DrawLineTo( this->border, height);
                rectangle_SetRectSize(&child, 0, 0, this->border, height);
                col = 1;
                x = this->border + 1;
                y = 0;
                for (item = this->items; item != NULL; item = item->next) {
                    item->x = x;
                    item->y = y;
                    item->w = maxwidth;
                    item->h = maxheight;
                    if (++col > columns) {
                        x = this->border + 1;
                        y += maxheight;
                        col = 1;
                    }
                    else
                        x += maxwidth;
                }
                break;
            case palette_BOTTOM:
                tmp = width/maxwidth;
                columns = (items + tmp - 1) / tmp;
                this->border = height - maxheight*columns;
                (this)->MoveTo( 0, this->border);
                (this)->DrawLineTo( width, this->border);
                rectangle_SetRectSize(&child, 0, 0, width, this->border);
                x = 0;
                y = this->border + 1;
                for (item = this->items; item != NULL; item = item->next) {
                    item->x = x;
                    item->y = y;
                    item->w = maxwidth;
                    item->h = maxheight;
                    x += maxwidth;
                    if (x > width - maxwidth) {
                        y += maxheight;
                        x = 0;
                    }
                }
                break;
        }
        black = (this)->BlackPattern();
        for (item = this->items; item != NULL; item = item->next) {
            switch (item->type) {
                case palette_ICON:
                    (this)->MoveTo( item->x + item->w/2, item->y + item->h/2);
                    (this)->SetFont( item->u.icon.font);
                    (this)->DrawText( &item->u.icon.ch, 1, graphic_NOMOVEMENT);
                    break;
                case palette_VIEW:
                    item->u.view.wants_update = FALSE;
                    (item->u.view.view)->InsertViewSize( this, item->x, item->y, item->w, item->h);
                    (item->u.view.view)->FullUpdate( view_FullRedraw, 0, 0, item->w, item->h);
                    break;
                case palette_STRING:
                    (this)->MoveTo( item->x + item->w/2, item->y + item->h/2);
                    (this)->SetFont( item->u.str.font);
                    (this)->DrawString( item->u.str.str, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBASELINE);
                    break;
            }
            if ((item->selected = item->new_selected)) {
                (this)->SetTransferMode( graphic_INVERT);
                (this)->FillRectSize( item->x, item->y, item->w,   item->h, black);
                (this)->SetTransferMode( graphic_COPY);
            }
        }
    }
    else {
        this->border = -1;
        rectangle_SetRectSize(&child, 0, 0, width, height);
    }

    (this->child)->InsertView( this, &child);
    (this->child)->FullUpdate( view_FullRedraw, 0, 0, rectangle_Width(&child), rectangle_Height(&child));
}

void palette::Update()
{
    struct palette_item *item;
    class graphic *black;

    if (this->needs_full) {
        (this)->SetTransferMode( graphic_SOURCE);
        (this)->EraseVisualRect();
        (this)->FullUpdate( view_FullRedraw, 0, 0, (this)->GetLogicalWidth(), (this)->GetLogicalHeight());
    }
    else {
        (this)->SetTransferMode( graphic_INVERT);
        black = (this)->BlackPattern();
        for (item = this->items; item != NULL; item = item->next) {
            if (item->type == palette_VIEW && item->u.view.wants_update) {
                /* If they want an update, we need to re-invert them. */
                item->u.view.wants_update = FALSE;
                if (item->selected) {
                    (this)->FillRectSize( item->x, item->y, item->w, item->h, black);
                    item->selected = FALSE;
                }
                (item->u.view.view)->Update();
            }
            if (item->selected != item->new_selected) {
                (this)->FillRectSize( item->x, item->y, item->w, item->h, black);
                item->selected = item->new_selected;
            }
        }
    }
}

class view *palette::Hit(enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    struct palette_item *item, *ptr;

    switch (action) {
        case view_LeftDown:
        case view_RightDown:
            switch (this->loc) {
                case palette_LEFT:
                    if (x > this->border) {
                        if (this->child != NULL)
                            return (this->child)->Hit( action, (x - this->border - 1), y, numclicks);
                        else
                            return (class view *)this;
		    }
                    break;
                case palette_TOP:
                    if (y > this->border) {
                        if (this->child != NULL)
                            return (this->child)->Hit( action, x, (y - this->border - 1), numclicks);
                        else
                            return (class view *)this;
		    }
                    break;
                case palette_RIGHT:
                    if (x <= this->border) {
                        if (this->child != NULL)
                            return (this->child)->Hit( action, x, y, numclicks);
                        else
                            return (class view *)this;
                    break;
		    }
                case palette_BOTTOM:
                    if (y <= this->border) {
                        if (this->child != NULL)
                            return (this->child)->Hit( action, x, y, numclicks);
                        else
                            return (class view *)this;
		    }
                    break;
                default:
                    if (this->child != NULL)
                        return (this->child)->Hit( action, x, y, numclicks);
                    else
                        return (class view *)this;
                    /* break; */
            }

            for (item = this->items; item != NULL; item = item->next)
                if (x >= item->x && y >= item->y && x < (item->x + item->w) && y < (item->y + item->h))
                    break;

            this->hit_item = item;

            if (item != NULL) {
                switch (item->autoselect) {
                    case palette_FOLLOWMOUSE:
                        palette::SelectItem(item);
                        break;
                    case palette_TOGGLE:
                        if (palette::Selected(item))
                            palette::DeselectItem(item);
                        else
                            palette::SelectItem(item);
                        break;
                    case palette_EXCLUSIVE:
                        for (ptr = this->items; ptr != NULL; ptr = ptr->next)
                            if (palette::Selected(ptr))
                                palette::DeselectItem(ptr);
                        palette::SelectItem(item);
                        break;
                    default:
                        break;
                }
                (*item->fn)(item->rock, item, action, numclicks);
            }

            return (class view *)this;
            /* break; */

        case view_LeftUp:
        case view_RightUp:
            if ((item = this->hit_item) != NULL) {
                if (item->autoselect == palette_FOLLOWMOUSE)
                    palette::DeselectItem(item);
                (*item->fn)(item->rock, item, action, numclicks);
            }
            return NULL;
            /* break; */

        default:
            return (class view *)this;
    }
}

void palette::WantUpdate(class view  *requestor)
{
    struct palette_item *item;

    for (item = this->items; item != NULL; item = item->next)
        if (item->type == palette_VIEW && item->u.view.view == requestor) {
            item->u.view.wants_update = TRUE;
            (this)->view::WantUpdate( this);
            return;
        }
    (this)->view::WantUpdate( requestor);
}

void palette::SetLocation(enum palette_location  loc)
{
    if (this->loc != loc) {
        this->loc = loc;
        this->needs_full = 1;
        (this)->WantUpdate( this);
    }
}

void palette::SetChild(class view  *child)
{
    if (this->child != NULL)
        this->child->parent = NULL;

    this->child = child;

    if (child != NULL)
        child->parent = (class view *)this;
}

static struct palette_item *palette_AddItem(class palette  *self, union palette_iteminfo  info, int  pos, palette_hitfptr fn, long  rock, enum palette_autoselect  autoselect)
{
    struct palette_item *item, *ptr, **prev;

    if (FreeList == NULL)
        item = (struct palette_item *)malloc(sizeof(struct palette_item));
    else {
        item = FreeList;
        FreeList = FreeList->next;
    }

    item->palette = self;
    item->u = info;
    item->pos = pos;
    item->x = item->y = item->w = item->h = 0;
    item->fn = fn;
    item->rock = rock;
    item->autoselect = autoselect;
    item->selected = item->new_selected = FALSE;

    for (prev = &self->items; (ptr = *prev) != NULL; prev = &ptr->next)
        if (pos < ptr->pos)
            break;

    item->next = ptr;
    *prev = item;

    self->needs_full = TRUE;
    (self)->WantUpdate( self);

    return item;
}

struct palette_item *palette::AddIcon(class fontdesc  *font, int  ch , int  pos, palette_hitfptr fn, long  rock, enum palette_autoselect  autoselect)
{
    union palette_iteminfo info;

    info.icon.font = font;
    info.icon.ch = ch;

    return palette_AddItem(this, info, pos, fn, rock, autoselect);
}

struct palette_item *palette::AddView(class view  *view, int  pos, palette_hitfptr fn, long  rock, enum palette_autoselect  autoselect)
{
    union palette_iteminfo info;

    info.view.view = view;
    info.view.wants_update = FALSE;

    return palette_AddItem(this, info, pos, fn, rock, autoselect);
}

struct palette_item *palette::AddString(class fontdesc  *font, char  *str, int  pos, palette_hitfptr fn, long  rock, enum palette_autoselect  autoselect)
{
    union palette_iteminfo info;

    info.str.font = font;
    info.str.str = str;

    return palette_AddItem(this, info, pos, fn, rock, autoselect);
}
