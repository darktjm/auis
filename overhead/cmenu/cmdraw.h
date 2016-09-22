/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

struct drawingState {
    int stackLeft;
    int stackTop;
    int stackHeight;
    int stackWidth;
    int paneWidth;
    int paneHeight;
    int maxSelections;
    int ySelectionOffset;
    int wormLeft;
    int wormRight;
    int wormTop;
    int wormBottom;
    int paneNum;                /* Index of current pane. */
    struct pane *panePtr;          /* Pointer to current pane. */
    int selectionNum;           /* Index of current selection. */
    struct selection *selectionPtr;/* Pointer to current Selection. */
#ifdef ATTEMPTSAVEUNDERS
    int doSaveUnder;            /* Boolean indicating whether or not to do save under. */
    Pixmap saveUnder;           /* Actual saved bits. */
    int saveUnderX;             /* rectangle of saveunder in parent window. */
    int saveUnderY;
    int saveUnderWidth;
    int saveUnderHeight;
#endif /* ATTEMPTSAVEUNDERS */
};

#define cmenu_Behind     1 /* Draw a card behind another card. */
#define cmenu_BeFront    2 /* Draw a card in fron of th top card. */
#define cmenu_OnTop      3 /* Draw this card as the one the user is selecting on. */
#define cmenu_Expose     4 /* Go from Behind to OnTop efficiently. */
#define cmenu_Hide       5 /* Go from OnTop to Behind efficiently. */

#define BOUNDINGBOXSLOPX 40
#define BOUNDINGBOXSLOPY 40

extern NO_DLL_EXPORT struct pane *SetPaneNum(struct cmenu *menu, struct drawingState *state, int paneNum);
extern NO_DLL_EXPORT int SetPanePtr(struct cmenu *menu, struct drawingState *state, struct pane *panePtr);
extern NO_DLL_EXPORT struct selection *SetSelectionNum(struct cmenu *menu, struct drawingState *state, int selectionNum);
extern NO_DLL_EXPORT int SetSelectionPtr(struct cmenu *menu, struct drawingState *state, struct selection *selectionPtr);
extern NO_DLL_EXPORT void CalculatePaneAndSelection(struct cmenu *menu, struct drawingState *state, int x, int y, int *returnPaneNum, int *returnSelNum, struct pane **returnPanePtr, struct selection **returnSelPtr);
extern NO_DLL_EXPORT void ShowAPane(struct cmenu *menu, struct drawingState *state, struct pane *pane, long x, long y, int position);
extern NO_DLL_EXPORT void CreateMenuStack(struct cmenu *menu, struct drawingState *state, long x, long y, Window parentWindow);
extern NO_DLL_EXPORT void DrawMenus(struct cmenu *menu, struct drawingState *state);
extern NO_DLL_EXPORT void FlipButton(struct cmenu *menu, struct drawingState *state, int paneNum, int selectionNum, struct selection *selectionPtr, int onOrOff);

#define GetPaneNum(state) ((state)->paneNum)
#define GetPanePtr(state) ((state)->panePtr)
#define GetSelectionNum(state) ((state)->selectionNum)
#define GetSelectionPtr(state) ((state)->selectionPtr)
#define SetPanePtrAndNum(state, ptr, num) ((state)->panePtr = (ptr), (state)->paneNum = (num))
#define SetSelectionPtrAndNum(state, ptr, num) ((state)->selectionPtr = (ptr), (state)->selectionNum = (num))
