/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <cmintern.h>
#include <cmdraw.h>

#define WHheight 11
#define WHwidth 11

extern struct pane *PaneNumToPtr();
extern int PanePtrToNum();
extern struct selection *SelectionNumToPtr();
extern int SelectionPtrToNum();

struct pane *SetPaneNum(struct cmenu *menu, struct drawingState *state, int paneNum)
{
    if (paneNum == -1) {
        state->paneNum = -1;
        state->panePtr = NULL;
    }
    else if ((state->panePtr = PaneNumToPtr(menu, paneNum)) == NULL)
        state->paneNum = -1;
    else
        state->paneNum = paneNum;

    return state->panePtr;
}

int SetPanePtr(struct cmenu *menu, struct drawingState *state, struct pane *panePtr)
{
    if (panePtr == NULL) {
        state->paneNum = -1;
        state->panePtr = NULL;
    }
    else if ((state->paneNum = PanePtrToNum(menu, panePtr)) == -1)
        state->panePtr = NULL;
    else
        state->panePtr = panePtr;

    return state->paneNum;
}

struct selection *SetSelectionNum(struct cmenu *menu, struct drawingState *state, int selectionNum)
{
    if (selectionNum == -1) {
        state->selectionNum = -1;
        state->selectionPtr = NULL;
    }
    else if ((state->selectionPtr = SelectionNumToPtr(menu, state->panePtr, selectionNum)) == NULL)
        state->selectionNum = -1;
    else
        state->selectionNum = selectionNum;

    return state->selectionPtr;
}

int SetSelectionPtr(struct cmenu *menu, struct drawingState *state, struct selection *selectionPtr)
{
    if (selectionPtr == NULL) {
        state->selectionNum = -1;
        state->selectionPtr = NULL;
    }
    else if ((state->selectionNum = SelectionPtrToNum(menu, state->panePtr, selectionPtr)) == -1)
        state->selectionPtr = NULL;
    else
        state->selectionPtr = selectionPtr;

    return state->selectionNum;
}

/* This routine calculates which pane should be on top and which selection
 * should be higlighted.
 */
void CalculatePaneAndSelection(struct cmenu *menu, struct drawingState *state, int x, int y, int *returnPaneNum, int *returnSelNum, struct pane **returnPanePtr, struct selection **returnSelPtr)
{

    int xShift = menu->gMenuData->xShift;
    int yShift = menu->gMenuData->yShift;
    int zone; /* Which zone from the left edge of the window we are in. A zone is the active area of a pane. */
    int paneNum = state->paneNum;
    int selectionNum;
    int paneTop;
    int paneTopSelection;
    int overlapWidth = menu->gMenuData->overlapWidth;
    int leftEdgeOfCard = (menu->numberOfPanes - 1 - state->paneNum) * xShift;
    struct pane *panePtr;
    struct selection *selPtr;
    int overlapPctWidth = (state->paneWidth * menu->gMenuData->overlapPct + 50) / 100;

    if (overlapPctWidth - xShift > overlapWidth) {
        overlapWidth = overlapPctWidth - xShift;
    }

    if (x < 0 || x > state->stackWidth) {
        zone = -1;
    }
    else if (x < leftEdgeOfCard) { /* Moving left. */
        zone = x / xShift;
    }
    else {
        zone = menu->numberOfPanes - 1 - paneNum;
        if (x > leftEdgeOfCard + xShift + overlapWidth) { /* Moving right */
            zone += (x - leftEdgeOfCard - overlapWidth) / xShift;
            if (zone >= menu->numberOfPanes) {
            zone = menu->numberOfPanes - 1; /* First card has a much larger zone. */
            }
        }
    }

    paneTop = zone * yShift;

    /* Handle moving into the grey portion of the cards */
    if (y > paneTop + state->paneHeight) {
        zone = x / xShift;
        paneTop = zone * yShift;
    }

    paneNum = menu->numberOfPanes - 1 - zone;

    paneTopSelection = paneTop + state->ySelectionOffset;
    if (y > paneTop + state->paneHeight) {
        paneNum = -1;
        panePtr = NULL;
        selectionNum = -1;
        selPtr = NULL;
    }
    else {

        panePtr = PaneNumToPtr(menu, paneNum);

        if (y > paneTopSelection) {

            struct selection *previous;

            if (panePtr != NULL) {

                int count;

                selectionNum = (y - paneTopSelection) / menu->gMenuData->selectionFontHeight;

                selPtr = panePtr->selections;
                previous = selPtr;
                for (count = selectionNum; count > 0 && selPtr != NULL; count--) {
                    if (selPtr->groupPriority == previous->groupPriority) {
                        previous = selPtr;
                        selPtr = selPtr->next;
                    }
                    else                    
                        previous = selPtr;
                }
                if (selPtr != NULL) {
                    /* Check for landing in the middle of a space. */
                    if (previous->groupPriority != selPtr->groupPriority) {
                        if (y < paneTopSelection + selectionNum * menu->gMenuData->selectionFontHeight + menu->gMenuData->selectionFontHeight / 2) {
                            selPtr = previous;
                            --selectionNum;
                        }
                        else
                            ++selectionNum;
		    }

                    if (!selPtr->active) {
                        selectionNum = -1;
                        selPtr = NULL;
                    }
                }
                else
                    selectionNum = -1;
            }
            else {
                paneNum = -1;
                selPtr = NULL;
                selectionNum = -1;
            }
        }
        else {
            if (y < zone * yShift) {
                zone = y / yShift;
                if (zone >= menu->numberOfPanes)
                    zone = menu->numberOfPanes - 1;
                if (x >= zone * xShift + state->paneWidth) {
                    paneNum = -1;
                    panePtr = NULL;
                }
                else {
                    paneNum = menu->numberOfPanes - 1 - zone;
                    panePtr = PaneNumToPtr(menu, paneNum);
                }
            }
            selectionNum = -1;
            selPtr = NULL;
        }
    }

    if (menu->wormPane != -1 && paneNum == 0 &&
        x >= state->wormLeft && x <= state->wormRight &&
        y >= state->wormTop && y <= state->wormBottom)  {

        struct pane *wormPanePtr = PaneNumToPtr(menu, menu->wormPane);
        
        if (wormPanePtr != NULL) {

            struct selection *thisSelection;
            int y = 0;
            int x;
            int lastPriority = 1000;
            int wormSelNum = menu->wormSelection;

            for (thisSelection = wormPanePtr->selections; thisSelection != NULL; thisSelection = thisSelection->next) {
                if (thisSelection->groupPriority > lastPriority)
                    y += menu->gMenuData->selectionFontHeight;
                lastPriority = thisSelection->groupPriority;
                if (--wormSelNum < 0)
                    break;
                y += menu->gMenuData->selectionFontHeight;
            }
            if (thisSelection != NULL) {
                y += menu->gMenuData->selectionFontHeight / 2 + (menu->numberOfPanes - 1 - menu->wormPane) * yShift + state->ySelectionOffset;
                x = (menu->numberOfPanes - 1 - menu->wormPane) * xShift + xShift / 2;
                XWarpPointer(menu->gMenuData->dpy,
                              None,
                              menu->gMenuData->menuWindow,
                              0, 0, 0, 0, x, y);
                paneNum = -1;
                panePtr = NULL;
                selectionNum = -1;
                selPtr = NULL;
            }
        }
    }


    *returnPaneNum = paneNum;
    *returnPanePtr = panePtr;
    *returnSelNum = selectionNum;
    *returnSelPtr = selPtr;
}

static void ShowASelection(struct cmenudata *globalData, struct pane *pane, struct selection *selection, int x, int y)
{
    Display *display = globalData->dpy;
    Window window = globalData->menuWindow;
    XGCValues gcv;

    XDrawString(display, window, selection->active ? globalData->blackGC : globalData->grayGC, x, y, selection->label, selection->labelLength);
    if(selection->keys) {
	GC tgc=selection->active? globalData->blackGC : globalData->grayGC;

	gcv.foreground=globalData->keysPixel;
	gcv.font=globalData->keysFont->fid;
	XChangeGC(display, tgc, GCForeground|GCFont, &gcv);
	XDrawString(display, window, tgc, x + pane->maxSelectionWidth + globalData->keyspace, y, selection->keys, strlen(selection->keys));
	gcv.foreground=(selection->active?globalData->foregroundPixel:globalData->grayforegroundPixel);
	gcv.font= globalData->selectionFont->fid;
	XChangeGC(display, tgc, GCForeground|GCFont, &gcv);
    }
}

static void
DrawWormHole(struct cmenu *,struct drawingState *state);

void ShowAPane(struct cmenu *menu, struct drawingState *state, struct pane *pane, long x, long y, int position)
{
    struct cmenudata *globalData = menu->gMenuData;
    Display *display = globalData->dpy;
    Window window = globalData->menuWindow;
    int x2, y2;
    int paneWidth = state->paneWidth;
    int paneHeight = state->paneHeight;
    int stackXShift = globalData->xShift;
    int stackYShift = globalData->yShift;
    struct selection *selection;
    XSegment seg[4];

    x2 = x + paneWidth - 1;
    y2 = y + paneHeight - 1;

    if (position == cmenu_BeFront) {
	if (globalData->motifMenus) {
	    XFillRectangle(display, window, globalData->grayGC, x + paneWidth - stackXShift, y+2, stackXShift-2, paneHeight - 4);
	    XFillRectangle(display, window, globalData->grayGC, x + 2, y + paneHeight - stackYShift, paneWidth - stackXShift - 2, stackYShift-2);
	    XFillRectangle(display, window, globalData->topshadowGC, x + paneWidth - stackXShift, y, stackXShift, 2);
	    XFillRectangle(display, window, globalData->topshadowGC, x, y + paneHeight - stackYShift, 2, stackYShift);
	    XFillRectangle(display, window, globalData->bottomshadowGC, x + paneWidth - 2, y + 2, 2, paneHeight - 2);
	    XFillRectangle(display, window, globalData->bottomshadowGC, x + 2, y + paneHeight - 2, paneWidth - 4, 2);
	} else {
	    XFillRectangle(display, window, globalData->grayGC, x + paneWidth - stackXShift, y + 1, stackXShift - 1, paneHeight - 2);
	    XFillRectangle(display, window, globalData->grayGC, x + 1, y + paneHeight - stackYShift, paneWidth - stackXShift - 1, stackYShift - 1);
	}
    } 
    else if (position == cmenu_Behind) {
	XFillRectangle(display, window, globalData->whiteGC, x, y, paneWidth, stackYShift);
	XFillRectangle(display, window, globalData->whiteGC, x, y + stackYShift, stackXShift, paneHeight - stackYShift);
	if (globalData->motifMenus) {
	    XFillRectangle(display, window, globalData->topshadowGC, x, y, paneWidth, 2);
	    XFillRectangle(display, window, globalData->topshadowGC, x, y+2, 2, paneHeight-2);
	    XFillRectangle(display, window, globalData->bottomshadowGC, x+paneWidth-2, y+2, 2, stackYShift-2);
	    XFillRectangle(display, window, globalData->bottomshadowGC, x+2, y+paneHeight-2, stackXShift-2, 2);
	}
    }
    else if (position==cmenu_OnTop) {
	XFillRectangle(display, window, globalData->whiteGC,x + 1, y + 1, paneWidth - 2, paneHeight - 2);
	if (globalData->motifMenus) {
	    seg[0].x1 = x; seg[0].y1 = y; seg[0].x2 = x+paneWidth-1; seg[0].y2 = y;
	    seg[1].x1 = x; seg[1].y1 = y+1; seg[1].x2 = x+paneWidth-2; seg[1].y2 = y+1;
            seg[2].x1 = x; seg[2].y1 = y+2; seg[2].x2 = x; seg[2].y2 = y+paneHeight-1;
            seg[3].x1 = x+1; seg[3].y1 = y+2; seg[3].x2 = x+1; seg[3].y2 = y+paneHeight-2;
	    XDrawSegments(display, window, globalData->topshadowGC, seg, 4);
	    seg[0].x1 = x+paneWidth-1; seg[0].y1 = y+1; seg[0].x2 = x+paneWidth-1; seg[0].y2 = y+paneHeight-1;
	    seg[1].x1 = x+paneWidth-2; seg[1].y1 = y+2; seg[1].x2 = x+paneWidth-2; seg[1].y2 = y+paneHeight-1;
	    seg[2].x1 = x+1; seg[2].y1= y+paneHeight-1; seg[2].x2 = x+paneWidth-3; seg[2].y2 = y+paneHeight-1;
	    seg[3].x1 = x+2; seg[3].y1= y+paneHeight-2; seg[3].x2 = x+paneWidth-3; seg[3].y2 = y+paneHeight-2;
	    XDrawSegments(display, window, globalData->bottomshadowGC, seg, 4);
	}
    } else if (position==cmenu_Expose) {
	XFillRectangle(display, window, globalData->whiteGC, x + stackXShift, y + stackYShift, 
		paneWidth - stackXShift - 1, paneHeight - stackYShift - 1);
	if (globalData->motifMenus) {
	    seg[0].x1 = x+paneWidth-1; seg[0].y1 = y+1; seg[0].x2 = x+paneWidth-1; seg[0].y2 = y+paneHeight-1;
	    seg[1].x1 = x+paneWidth-2; seg[1].y1 = y+2; seg[1].x2 = x+paneWidth-2; seg[1].y2 = y+paneHeight-1;
	    seg[2].x1 = x+1; seg[2].y1= y+paneHeight-1; seg[2].x2 = x+paneWidth-3; seg[2].y2 = y+paneHeight-1;
	    seg[3].x1 = x+2; seg[3].y1= y+paneHeight-2; seg[3].x2 = x+paneWidth-3; seg[3].y2 = y+paneHeight-2;
	    XDrawSegments(display, window, globalData->bottomshadowGC, seg, 4);
	}
    }

    /* draw lines */
    if (!globalData->motifMenus) {
    if (position == cmenu_Expose) {
        XFillRectangle(display, window, globalData->blackGC, x + stackXShift + 1, y2, paneWidth - stackXShift - 2, 1);
        XFillRectangle(display, window, globalData->blackGC, x2, y + stackYShift + 1, 1, paneHeight - stackYShift - 1);
    }
    else if (position != cmenu_Hide)  {

        int temp;

    /* left */
        temp = (position == cmenu_BeFront) ? y + paneHeight - stackYShift : y;
	XFillRectangle(display, window, globalData->blackGC, x, temp, 1, y2 - temp);
    /* bottom */
        temp = (position == cmenu_Behind) ? x + stackXShift : x2;
	XFillRectangle(display, window, globalData->blackGC, x, y2, temp - x, 1);
    /* right */
        temp = (position == cmenu_Behind) ? y + stackYShift : y2;
	XFillRectangle(display, window, globalData->blackGC, x2, y, 1, temp - y + 1);
    /* top */
        temp = (position == cmenu_BeFront) ? x + paneWidth - stackXShift : x;
	XFillRectangle(display, window, globalData->blackGC, temp, y, x2 - temp, 1);
    }
    }

    if (position == cmenu_BeFront)
	XDrawString(display, window, globalData->titleBlackGC, x + globalData->xTitleOffset, y + paneHeight - globalData->titleFontDescent - globalData->yTitleOffset, pane->label, strlen(pane->label));
    else {
	if (position != cmenu_Expose)
	    XDrawString(display, window, globalData->titleBlackGC, x + globalData->xTitleOffset, y + globalData->yTitleOffset + globalData->titleFontAscent+1, pane->label, strlen(pane->label));
	if (position != cmenu_Behind && strlen(pane->label) != 0) {
	    GC gc;

	    if (position == cmenu_Hide)
		gc = globalData->whiteGC;
	    else
		gc = globalData->blackGC;
	    if (globalData->motifMenus) {
        	XFillRectangle(display, window, gc, x + globalData->xTitleOffset, y + globalData->yTitleOffset + globalData->titleFontHeight, paneWidth - globalData->xTitleOffset*2, 1);
        	XFillRectangle(display, window, gc, x + globalData->xTitleOffset, y + globalData->yTitleOffset + globalData->titleFontHeight+2, paneWidth - globalData->xTitleOffset*2, 1);
	    } else {
        	XFillRectangle(display, window, gc, x + globalData->xTitleOffset, y + globalData->yTitleOffset + globalData->titleFontHeight, pane->labelWidth, 1);
	    }
	}
    }

    if (position == cmenu_OnTop || position == cmenu_Expose) {

        int lastPriority = 1000;

	y2 = y + state->ySelectionOffset + globalData->selectionFontAscent;

	x2 = x + globalData->xSelectionOffset;

	for (selection = pane->selections; selection != NULL; selection = selection->next)  {
            if (selection->groupPriority > lastPriority)
		y2 += globalData->selectionFontHeight;
	    ShowASelection(globalData, pane, selection, x2, y2);
	    y2 += globalData->selectionFontHeight;
            lastPriority = selection->groupPriority;
	}
	if (pane == menu->panes)
	    DrawWormHole(menu, state);
    }

}

static void
DrawWormHole(struct cmenu *menu, struct drawingState *state)
{
    if (menu->wormPane != -1 && menu->wormSelection != -1)
        XCopyArea(menu->gMenuData->dpy, menu->gMenuData->wormIcon, menu->gMenuData->menuWindow, menu->gMenuData->blackGC, 0, 0, menu->gMenuData->wormWidth, menu->gMenuData->wormHeight, state->wormLeft, state->wormTop);
}

/* Initializes drawingState and maps the menu window. Perhaps this function should be renamed. */
void CreateMenuStack(struct cmenu *menu, struct drawingState *state, long x, long y, Window parentWindow)
{

    int	paneWidth = 0;
    int mouseOffsetX;       /* Offset of mouse starting location from menu window corner. */
    int mouseOffsetY;
    int	mouseStartX;
    int mouseStartY;        /* Position to move mouse to */
    struct cmenudata *globalData = menu->gMenuData;
    Display *display = globalData->dpy;
    long displayWidth = DisplayWidth(display, DefaultScreen(display));
    long displayHeight = DisplayHeight(display, DefaultScreen(display));
    struct pane *pane;
    int maxSelections = 0;
    int wormCenter;

    /* Find maximum width, height of all the menus */
    for (pane = menu->panes; pane != NULL; pane = pane->next)  {
	if (paneWidth < pane->labelWidth)
	    paneWidth = pane->labelWidth;
	if (paneWidth < pane->maxSelectionWidth + pane->maxKeysWidth + globalData->keyspace)
	    paneWidth = pane->maxSelectionWidth + pane->maxKeysWidth + globalData->keyspace;

/* Need to handle spaces correctly. */
	if (maxSelections < pane->numberOfSelections)
	    maxSelections = pane->numberOfSelections;
    }

    state->maxSelections = maxSelections;

    if (menu->panes->label == NULL || menu->panes->label[0] == '\0') {
        state->ySelectionOffset = max(globalData->titleFontHeight + 2, globalData->wormHeight) + globalData->yTitleOffset + 6;
        wormCenter = state->ySelectionOffset / 2;
    }
    else {
        state->ySelectionOffset = globalData->titleFontHeight + globalData->wormHeight + globalData->yTitleOffset + 8;
        wormCenter = state->ySelectionOffset - globalData->wormHeight / 2 - 3;
    }

/* xSelectionOffset includes the left margin. The HORIZONTALMARGIN is the rightMargin. */
    state->paneWidth = paneWidth + globalData->xSelectionOffset + HORIZONTALMARGIN;
/* ySelectionOffset includes the topMargin. */
    state->paneHeight = state->ySelectionOffset + maxSelections * globalData->selectionFontHeight + globalData->bottomMargin;

    state->stackWidth = state->paneWidth + globalData->xShift * (menu->numberOfPanes - 1);
    state->stackHeight = state->paneHeight + globalData->yShift * (menu->numberOfPanes - 1);

    /* Try to position upper left corner of containing rectangle so original
       cursor position (before popping menu) will be at the left top of the
	topmost menu */
    mouseOffsetX = globalData->xShift / 2 + (menu->numberOfPanes - 1) * globalData->xShift;
    mouseOffsetY = wormCenter + (menu->numberOfPanes - 1) * globalData->yShift;

    state->stackLeft = x - mouseOffsetX;
    state->stackTop = y - mouseOffsetY;

    /* Correct if off screen */
    if (state->stackLeft + state->stackWidth >= displayWidth)
	state->stackLeft = displayWidth - state->stackWidth;
    if (state->stackTop + state->stackHeight >= displayHeight)
	state->stackTop = displayHeight - state->stackHeight;
    if (state->stackLeft < 0)
	state->stackLeft = 0;
    if (state->stackTop < 0)
	state->stackTop = 0;

    /* Get X,Y for top left corner of front menu */

/* The constants in this next piece of code should be symbolic. */
    mouseStartX = state->stackLeft + mouseOffsetX;
    mouseStartY =  state->stackTop + mouseOffsetY;

/* Try to position the worm hole above the first letter of a selection. */
    state->wormLeft = globalData->xSelectionOffset + (menu->numberOfPanes - 1) * globalData->xShift;
    if (state->wormLeft < mouseOffsetX + 5)
        state->wormLeft = mouseOffsetX + 5;
    state->wormTop = mouseOffsetY - globalData->wormHeight / 2;
    state->wormRight = state->wormLeft + globalData->wormWidth;
    state->wormBottom = state->wormTop + globalData->wormHeight;

    /*
      Need to Map the menuWindow in this code.  That window should have no background so that previous bits show through.  Also have to modify to handle exposure events
      */

    XMoveResizeWindow(display, globalData->menuWindow, state->stackLeft, state->stackTop, state->stackWidth, state->stackHeight);

#ifdef ATTEMPTSAVEUNDERS

/* This code has been ifdef'ed out because of a problem that occurs when the parentWindow of the menuWindow is somehow not around to receive expose events after the menuWindow is unmapped from a screen whose owning X server does not have save-under enabled.  The result is that the server hangs, waiting for that expose event in the routine cmenu_Active (cmactiv.c). */

    if (state->doSaveUnder) { /* Hair city... */

        Window dummyWindow;
        unsigned int parentWidth;
        unsigned int parentHeight;
        unsigned int borderWidth;
        unsigned int depth;
        int menuX;
        int menuY;
        int menuWidth = state->stackWidth;
        int menuHeight = state->stackHeight;
        int temp;


        XGetGeometry(display, parentWindow, &dummyWindow, &temp, &temp, &parentWidth, &parentHeight, &borderWidth, &depth);
        XTranslateCoordinates(display, globalData->menuWindow, parentWindow, 0, 0, &menuX, &menuY, &dummyWindow);

        parentWidth += borderWidth;
        parentHeight += borderWidth;


        if (menuX < 0) {
            menuWidth += menuX;
            menuX = 0;
        }
        if (menuY < 0) {
            menuHeight += menuY;
            menuY = 0;
        }

        if ((temp = menuX + menuWidth - parentWidth) > 0)
            menuWidth -= temp;
        if ((temp = menuY + menuHeight - parentHeight) > 0)
            menuHeight -= temp;

        if (menuWidth <= 0 || menuHeight <= 0)
            menuWidth = menuHeight = 0;

        state->saveUnderX = menuX;
        state->saveUnderY = menuY;
        state->saveUnderWidth = menuWidth;
        state->saveUnderHeight = menuHeight;

        if ((state->saveUnder = XCreatePixmap(display, parentWindow, state->saveUnderWidth, state->saveUnderHeight, depth)) != 0)
            XCopyArea(display, parentWindow, state->saveUnder, globalData->saveUnderGC, state->saveUnderX, state->saveUnderY, state->saveUnderWidth, state->saveUnderHeight, 0, 0);
    }
#endif /* ATTEMPTSAVEUNDERS */

    if (menu->numberOfPanes > 0) {
	/* Position Cursor at left of topmost menu */
	XWarpPointer(display,
                      None,
                      RootWindow(display,
                                  DefaultScreen(display)),
                      0, 0, 0, 0, mouseStartX, mouseStartY);
    }

    XMapRaised(display, globalData->menuWindow);

    XGrabPointer(display, globalData->menuWindow, FALSE,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeSync, None, None, CurrentTime);

    XSync(display, 0);
}

void DrawMenus(struct cmenu *menu, struct drawingState *state)
{
    long X = state->stackWidth - state->paneWidth;
    long Y = state->stackHeight - state->paneHeight;
    long xIncr = menu->gMenuData->xShift;
    long yIncr = menu->gMenuData->yShift;
    long currentValue = cmenu_BeFront;
    struct pane *currentPane = GetPanePtr(state);
    struct pane *pane;

    for (pane = menu->panes; pane != NULL; pane = pane->next)  {
	if (pane == currentPane)
	    currentValue = cmenu_OnTop;
	ShowAPane(menu, state, pane, X, Y, currentValue);
	if (currentValue == cmenu_OnTop)
	    currentValue = cmenu_Behind;
	X -= xIncr;
	Y -= yIncr;
    }
}

void FlipButton(struct cmenu *menu, struct drawingState *state, int paneNum, int selectionNum, struct selection *selectionPtr, int onOrOff)
{

    struct cmenudata *globalData = menu->gMenuData;
    int zone = menu->numberOfPanes - 1 - paneNum;
    int x = zone * globalData->xShift + 1;
    int y = zone * globalData->yShift + selectionNum * globalData->selectionFontHeight + state->ySelectionOffset;
    int height = globalData->selectionFontHeight;
    int width = state->paneWidth;
    XSegment seg[4];

    if (globalData->motifMenus) {
        if (onOrOff) {
	    XFillRectangle(globalData->dpy, globalData->menuWindow, globalData->whiteGC, x+4, y+4, width-8, height-8);
            seg[0].x1 = x+2; seg[0].y1 = y-1; seg[0].x2 = x+width-5; seg[0].y2 = y-1;
            seg[1].x1 = x+2; seg[1].y1 = y; seg[1].x2 = x+width-6; seg[1].y2 = y;
            seg[2].x1 = x+2; seg[2].y1 = y+1; seg[2].x2 = x+2; seg[2].y2 = y+height;
            seg[3].x1 = x+3; seg[3].y1 = y+1; seg[3].x2 = x+3; seg[3].y2 = y+height-1;
            XDrawSegments(globalData->dpy, globalData->menuWindow, globalData->topshadowGC, seg, 4);
            seg[0].x1 = x+width-5; seg[0].y1 = y; seg[0].x2 = x+width-5; seg[0].y2 = y+height;
            seg[1].x1 = x+width-6; seg[1].y1 = y+1; seg[1].x2 = x+width-6; seg[1].y2 = y+height;
            seg[2].x1 = x+3; seg[2].y1 = y+height; seg[2].x2 = x+width-7; seg[2].y2 = y+height;
            seg[3].x1 = x+4; seg[3].y1 = y+height-1; seg[3].x2 = x+width-7; seg[3].y2 = y+height-1;
	    XDrawSegments(globalData->dpy, globalData->menuWindow, globalData->bottomshadowGC, seg, 4);
	} else {
            XFillRectangle(globalData->dpy, globalData->menuWindow, globalData->whiteGC, x+2, y-1, state->paneWidth - 6, globalData->selectionFontHeight+2);
	}

	ShowASelection(globalData, PaneNumToPtr(menu, paneNum), selectionPtr, x - 1 + globalData->xSelectionOffset, y + globalData->selectionFontAscent);

    } else if (globalData->highlightUsingGray) {
        if (onOrOff) {
            XFillRectangle(globalData->dpy, globalData->menuWindow, globalData->grayGC, x, y, state->paneWidth - 2, globalData->selectionFontHeight);
	} else {
            XFillRectangle(globalData->dpy, globalData->menuWindow, globalData->whiteGC, x, y, state->paneWidth - 2, globalData->selectionFontHeight);
	}

	ShowASelection(globalData, PaneNumToPtr(menu, paneNum), selectionPtr, x - 1 + globalData->xSelectionOffset, y + globalData->selectionFontAscent);
    }
    else
        XFillRectangle(globalData->dpy, globalData->menuWindow, globalData->invertGC, x, y, state->paneWidth - 2, globalData->selectionFontHeight);
}








