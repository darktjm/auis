/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <X11/Xlib.h>
#include <cmintern.h>
#include <cmdraw.h>


#if !defined(PRE_X11R4_ENV) && defined(__STDC__)
static Bool SuitableEvent(Display *, XEvent *, char *);
#endif /* !defined(PRE_X11R4_ENV) && defined(__STDC__) */

#ifdef ATTEMPTSAVEUNDERS
#if !defined(PRE_X11R4_ENV) && defined(__STDC__)
static Bool DiscardableEvents(Display *, XEvent *, char *);
#endif /* !defined(PRE_X11R4_ENV) && defined(__STDC__) */
#endif /* ATTEMPTSAVEUNDERS */

struct activationState {
    long startTime;
    int up;                     /* TRUE while the menus should remain up. */
    int doublePress;            /* TRUE iff a double press menu selection is being made. */
    int buttonName;
    struct cmenu *menu;         /* Backpointer to menu for SuitableEvent function... */
    struct drawingState drawingState;
    Window parentWindow;       /* parentWindow of menu window used for save under support. */
    int lastPane;              /* Used to determine if we should warp the mouse or not. */
};

/* This function defines all events which are meaningful to the cmenuActivate
 * procedure's event loop.
 */
static Bool SuitableEvent(Display *display, XEvent *event, char *args /* should be void * */)
{

    struct activationState *state = (struct activationState *) args;
    struct cmenu *menu = state->menu;

    switch (event->type) {
        case Expose:
            return (((XExposeEvent *) event)->window == menu->gMenuData->menuWindow);
        case ButtonPress:
        case ButtonRelease:
            return TRUE;
        case MotionNotify:
            return (((XMotionEvent *) event)->window == menu->gMenuData->menuWindow);
        default:
            return FALSE;
    }
}

#ifdef ATTEMPTSAVEUNDERS
/* This function defines all events which should be cleared from the queue
 * when we are done.
 */
static Bool DiscardableEvents(Display *display, XEvent *event, char *args /* should be void * */)
{

    struct activationState *state = (struct activationState *) args;
    struct cmenu *menu = state->menu;

    switch (event->type) {
        case Expose:
            return (((XExposeEvent *) event)->window == menu->gMenuData->menuWindow) ||
                    (state->drawingState.doSaveUnder && (((XExposeEvent *) event)->window == state->parentWindow));
        case ButtonPress:
        case ButtonRelease:
            return ((XButtonEvent *)event)->window == menu->gMenuData->menuWindow;
        case MotionNotify:
            return (((XMotionEvent *) event)->window == menu->gMenuData->menuWindow);
        default:
            return FALSE;
    }
}
#endif /* ATTEMPTSAVEUNDERS */

static int HandlePress(struct cmenu *menu, XButtonEvent *buttonEvent, struct activationState *state)
{

    if ((int)buttonEvent->button == state->buttonName) {
        if (!state->doublePress && (int)(buttonEvent->time - state->startTime) <= menu->gMenuData->clickInterval) {
            state->doublePress = TRUE;
        }	    
        else { /* Take down the menus and return status. */
            if (buttonEvent->type == ButtonRelease)
                state->up = FALSE;
            else
                state->doublePress = FALSE;
        }
    }
    return(0);
}

static void HandleMovement(struct cmenu *menu, XMotionEvent *motionEvent, struct activationState *state)
{

    struct drawingState *drawingState = &state->drawingState;
    int currentPaneNum = GetPaneNum(drawingState);
    struct pane *currentPanePtr = GetPanePtr(drawingState);
    int currentSelectionNum = GetSelectionNum(drawingState);
    struct selection *currentSelectionPtr = GetSelectionPtr(drawingState);
    int paneNum;
    int selectionNum;
    struct pane *panePtr;
    struct selection *selectionPtr;
/* These shift parameters don't belong here. */
    int xShift = menu->gMenuData->xShift;
    int yShift = menu->gMenuData->yShift;
    int x = motionEvent->x;
    int y = motionEvent->y;

    if (state->doublePress && (x < -BOUNDINGBOXSLOPX || x > drawingState->stackWidth + BOUNDINGBOXSLOPX || y < -BOUNDINGBOXSLOPY || y > drawingState->stackHeight + BOUNDINGBOXSLOPY))
         state->up = FALSE;

/* SuitableEvent guarantees that MotionNotify events are only delivered on the menu window. */
    CalculatePaneAndSelection(menu, drawingState, x, y,
                               &paneNum, &selectionNum, &panePtr, &selectionPtr);

    state->lastPane = paneNum;

    if (currentSelectionNum != -1 &&
         (currentPaneNum != paneNum || currentSelectionNum != selectionNum))  {
        FlipButton(menu, drawingState, currentPaneNum, currentSelectionNum, currentSelectionPtr, FALSE);
    }

    if (panePtr != NULL) {
        if (paneNum < currentPaneNum)  {

            struct pane *tempPanePtr = panePtr->next;
            int x;
            int y;

            x = (menu->numberOfPanes - 1 - paneNum) * xShift;
            y = (menu->numberOfPanes - 1 - paneNum) * yShift;
            ShowAPane(menu, drawingState, panePtr, x, y, cmenu_OnTop);
            while (tempPanePtr != currentPanePtr)  {
                x -= xShift;
                y -= yShift;
                ShowAPane(menu, drawingState, tempPanePtr, x, y, cmenu_Behind);
                tempPanePtr = tempPanePtr->next;
            }
            x -= xShift;
            y -= yShift;
            ShowAPane(menu, drawingState, currentPanePtr, x, y, cmenu_Hide);
        }
        else if (paneNum > currentPaneNum)  {

            int x;
            int y;

            x = (menu->numberOfPanes - 1 - currentPaneNum) * xShift;
            y = (menu->numberOfPanes - 1 - currentPaneNum) * yShift;
            while (panePtr != currentPanePtr)  {
                ShowAPane(menu, &state->drawingState, currentPanePtr, x, y, cmenu_BeFront);
                currentPanePtr = currentPanePtr->next;
                x -= xShift;
                y -= yShift;
            }
            ShowAPane(menu, &state->drawingState, panePtr, x, y, cmenu_Expose);
        }

        if (selectionNum != -1 &&
             (currentPaneNum != paneNum || currentSelectionNum != selectionNum))  {
            FlipButton(menu, &state->drawingState, paneNum, selectionNum, selectionPtr, TRUE);
        }
 
        SetPanePtrAndNum(drawingState, panePtr, paneNum);
    }
    SetSelectionPtrAndNum(drawingState, selectionPtr, selectionNum);
}

static void EventLoop(struct cmenu *menu, Display *display, struct activationState *state)
{

    XEvent events[2];
    int nextEventIndex = 0;
    XEvent *thisEvent;
    XEvent *nextEvent = NULL;

    while (state->up) {

        if (nextEvent == NULL) { /* If we don't have an event from look ahead. */
            XIfEvent(display, &events[nextEventIndex], SuitableEvent, (char *) state);
            thisEvent = &events[nextEventIndex];
        }
        else {
            thisEvent = nextEvent;
            nextEvent = NULL;
        }
        nextEventIndex = 1 - nextEventIndex;

	/*
	 * Dispatch on the event type.
	 */
	switch (thisEvent->type) {
	    case Expose:
 /* SuitableEvent guarantees expose events only come through on the menu window. */
                DrawMenus(menu, &state->drawingState);
		break;
	    case ButtonPress:
	    case ButtonRelease:
                HandlePress(menu, (XButtonEvent *) thisEvent, state);
                break;
            case MotionNotify:
/* This code here does look ahead for mouse motion events. */
                if (XCheckIfEvent(display, &events[nextEventIndex], SuitableEvent,
                                  (char *) state)) {
                    nextEvent = &events[nextEventIndex];
                    if (nextEvent->type == MotionNotify)
                        break;
                }
                HandleMovement(menu, (XMotionEvent *) thisEvent, state);
                break;
        }
    }
}

int
cmenu_Activate(struct cmenu *menu, XButtonEvent *menuEvent, long *data, int backgroundType, long background)
{

    int ret_val;			/* Return value. */

    Display *display = menu->gMenuData->dpy;
    struct activationState state;       /* Packaged state for passing to subroutines. */

    /*
     * If there are no panes in the menu then return failure
     * beacuse the menu is not initialized.
     */
    if (menu->panes == NULL) {
	_cmErrorCode = cmE_NOT_INIT;
	return(cm_FAILURE);
    }

    /*
     * If the event type is not a valid choice return failure.
     */
    if ((menuEvent->type != ButtonPress) && (menuEvent->type != ButtonRelease)) {
	return(cm_FAILURE);
    }

#ifdef ATTEMPTSAVEUNDERS
/* Decide if we need/want to save the image under the menus. */
    state.drawingState.doSaveUnder = (backgroundType != cmenu_NoSaveUnder) && !DoesSaveUnders(DefaultScreenOfDisplay(display));

/* If saving the image, grab the server so the image cannot be modified while we are up. */
    if (state.drawingState.doSaveUnder)
        XGrabServer(display);
#endif /* ATTEMPTSAVEUNDERS */
    state.lastPane=(-1);
    state.startTime = menuEvent->time;
    state.up = TRUE;
    if (menuEvent->type == ButtonPress)
        state.doublePress = FALSE;
    else /* Assume buttons are up. */
        state.doublePress = TRUE;
    state.buttonName = menuEvent->button;
    state.menu = menu;
    state.parentWindow = menuEvent->window;

    SetPaneNum(menu, &state.drawingState, 0);
    SetSelectionNum(menu, &state.drawingState, -1);

    CreateMenuStack(menu, &state.drawingState, menuEvent->x_root, menuEvent->y_root, state.parentWindow);
   
    XSync(display, 0);

    EventLoop(menu, display, &state);

    if (GetSelectionNum(&state.drawingState) != -1 &&
         GetPaneNum(&state.drawingState) != -1 &&
         GetSelectionPtr(&state.drawingState)->active) {
        *data = GetSelectionPtr(&state.drawingState)->data;
        menu->wormPane = GetPaneNum(&state.drawingState);
        menu->wormSelection = SelectionPtrToNum(menu, GetPanePtr(&state.drawingState), GetSelectionPtr(&state.drawingState));
        ret_val = cm_SUCCESS;
    }
    else {
        ret_val = cm_NO_SELECT;
    }

#ifdef ATTEMPTSAVEUNDERS
/* Prevent server from covering area with background when menus go down. */

    if (state.drawingState.doSaveUnder)
        XSetWindowBackgroundPixmap(display, state.parentWindow, None);
#endif /* ATTEMPTSAVEUNDERS */

    XUnmapWindow(display, menu->gMenuData->menuWindow);

    /*
     * If we changed the original position of the cursor,
     * put it back. When one wonders out of the menus, it is visually displeasing to have the mouse jerked back so we don't do that...
     */
    if (/* !state.doublePress && */ state.lastPane != -1)
        XWarpPointer(display,
                      None,
                      RootWindow(display, DefaultScreen(display)),
                      0, 0, 0, 0,
                      menuEvent->x_root, menuEvent->y_root);

    /*
     * Synchronize the X buffers and the X event queue.
     */
    XSync(display, 0);
    
#ifdef ATTEMPTSAVEUNDERS
    /*
     * Dispatch any events remaining on the queue for menu or bounding box windows.
     */

    if (state.drawingState.doSaveUnder) {
        while(XCheckIfEvent(display, &event, DiscardableEvents, (char *) &state));

        XCopyArea(display, state.drawingState.saveUnder, state.parentWindow, menu->gMenuData->saveUnderGC, 0, 0, state.drawingState.saveUnderWidth, state.drawingState.saveUnderHeight, state.drawingState.saveUnderX, state.drawingState.saveUnderY);

        if (state.drawingState.saveUnder)
            XFreePixmap(display, state.drawingState.saveUnder);

        if (backgroundType == cmenu_BackgroundPixel)
            XSetWindowBackground(display, state.parentWindow, background);
        else if (backgroundType == cmenu_BackgroundPixmap)
            XSetWindowBackgroundPixmap(display, state.parentWindow, (Pixmap) background);

        XUngrabServer(display);
    }
#endif /* ATTEMPTSAVEUNDERS */

    /* Make sure everything is out to the server. */
    XFlush(display);

    /*
     * Return successfully.
     */
    _cmErrorCode = cmE_NO_ERROR;
    return ret_val;
}

