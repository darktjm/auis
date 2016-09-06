/*                                                     Style Management for Line Mode Browser
                              STYLE DEFINITION FOR HYPERTEXT
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module defines the HText functions referenced from the HTML module in the Library
   of Common Code.
   
 */
#include "WWWLib.h"

#ifdef SHORT_NAMES
#define HText_childNumber               HTGTChNu
#define HText_canScrollUp               HTGTCaUp
#define HText_canScrollDown             HTGTCaDo
#define HText_scrollUp                  HTGTScUp
#define HText_scrollDown                HTGTScDo
#define HText_scrollTop                 HTGTScTo
#define HText_scrollBottom              HTGTScBo
#define HText_sourceAnchors             HTGTSoAn
#define HText_setStale                  HTGTStal
#define HText_refresh                   HTGTRefr
#endif


extern HTChildAnchor * HText_childNumber (HText * text, int n);

/* Is there any file left? */
extern BOOL HText_canScrollUp (HText * text);
extern BOOL HText_canScrollDown (HText * text);

/* Move display within window */
extern void HText_scrollUp (HText * text);      /* One page */
extern void HText_scrollDown (HText * text);    /* One page */
extern void HText_scrollTop (HText * text);
extern void HText_scrollBottom (HText * text);

extern int HText_sourceAnchors (HText * text);
extern void HText_setStale (HText * text);
extern void HText_refresh (HText * text);

#ifdef CURSES
extern int HText_getTopOfScreen (HText * text);
extern int HText_getLines (HText * text);
#endif
/*

   End of definition  */
