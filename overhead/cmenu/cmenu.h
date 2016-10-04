/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libcmenu libcmenu
 * Library to support display and control of X11 popup menus.
 * @{ */

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

#include <cmerror.h>
#include <X11/X.h>
#include <X11/Xlib.h>

/* Could have just as easily used TRUE and FALSE for these. */
#define cmenu_Inactive                  0
#define cmenu_Active                    1

#define cmenu_DisallowDuplicates        8
#define cmenu_CreatePane                2
#define cmenu_DeleteEmptyPanes          4

#define cmenu_NoBackground              0
#define cmenu_BackgroundPixel           1
#define cmenu_BackgroundPixmap          2
#define cmenu_NoSaveUnder               3

struct cmenu;
typedef void (*cmenu_FreeFunction)(void *f);
extern struct cmenu *cmenu_Create(Display *display, Window parent,
                         const char *defaultEnvironment, cmenu_FreeFunction freeFunction);
extern void cmenu_Destroy(struct cmenu *menu);
extern int cmenu_AddPane(struct cmenu *menu, const char *paneTitle,
                         int panePriority, int flags);
extern int cmenu_DeletePane(struct cmenu *menu, const char *paneTitle, int priority);
extern int cmenu_AddSelection(struct cmenu *menu, const char *paneTitle,
               int panePriority, const char *selectionLabel, int selectionPriority,
               long selectionData, int flags, const char *keys);
extern int cmenu_DeleteSelection(struct cmenu *menu, const char *paneTitle,
               int panePriority, const char *slectionLabel, int selectionPriority,
               int flags);
extern int cmenu_Activate(struct cmenu *menu, XButtonEvent *menuEvent,
               long *data, int backgroundType, long backgroundValue);
/* following 2 basically unused */
extern int cmenu_SetActive(struct cmenu *menu, const char *paneTitle, int panePriority, int priority, int active);
extern int cmenu_GetActive(struct cmenu *menu, const char *paneTitle, int panePriority, int priority);
 
ENDCPLUSPLUSPROTOS

/** @} */
