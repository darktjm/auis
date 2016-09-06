/*                                                     HTML to rich text converter for libwww
                         THE HTML TO STYLED TEXT OBJECT CONVERTER
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This interprets the HTML semantics and some HTMLPlus.
   
   This module is implemented by HTML.c, and it is a part of the Library of Common Code.
   
 */
#ifndef HTML_H
#define HTML_H

#include "HTFormat.h"
#include "HTAnchor.h"
#include "HTMLPDTD.h"

#define DTD HTMLP_dtd

#ifdef SHORT_NAMES
#define HTMLPresentation        HTMLPren
#define HTMLPresent             HTMLPres
#endif

extern CONST HTStructuredClass HTMLPresentation;/*

HTML_new: A structured stream to parse HTML

   When this routine is called, the request structure may contain a childAnchor value.  In
   that case it is the responsability of this module to select the anchor.
   
   
   
 */
extern HTStructured* HTML_new (HTRequest * request,
                                        void *   param,
                                        HTFormat input_format,
                                        HTFormat output_format,
                                        HTStream * output_stream);
/*

  REOPEN
  
   Reopening an existing HTML object allows it to be retained (for example by the styled
   text object) after the structured stream has been closed.  To be actually deleted, the
   HTML object must be closed once more times than it has been reopened.
   
 */
extern void HTML_reopen (HTStructured * me);/*

Converters

   These are the converters implemented in this module:
   
 */
#ifndef pyramid
extern HTConverter HTMLToPlain, HTMLToC, HTMLPresent, HTMLToTeX;
#endif/*

Selecting internal character set representations

 */
typedef enum _HTMLCharacterSet {
        HTML_ISO_LATIN1,
        HTML_NEXT_CHARS,
        HTML_PC_CP950
} HTMLCharacterSet;

extern void HTMLUseCharacterSet (HTMLCharacterSet i);
/*

Record error message as a hypertext object

   The error message should be marked as an error so that it can be reloaded later. This
   implementation just throws up an error message and leaves the document unloaded.
   
  ON ENTRY,
  
  sink                    is a stream to the output device if any
                         
  number                  is the HTTP error number
                         
  message                 is the human readable message.
                         
  ON EXIT,
  
   a return code like HT_LOADED if object exists else < 0
   
 */
extern int HTLoadError (
        HTRequest *     req,
        int             number,
        CONST char *    message);

/*

White Space Treatment

   There is a small number of different ways of treating white space in SGML, in mapping
   from a text object to HTML.  These have to be programmed it seems.
   
 */
/*
In text object  \n\n            \n      tab     \n\n\t
--------------  -------------   -----   -----   -------
in Address,
Blockquote,
Normal,         <P>             <BR>    -               NORMAL
H1-6:           close+open      <BR>    -               HEADING
Glossary        <DT>            <DT>    <DD>    <P>     GLOSSARY
List,
Menu            <LI>            <LI>    -       <P>     LIST
Dir             <LI>            <LI>    <LI>            DIR
Pre etc         \n\n            \n      \t              PRE

*/

typedef enum _white_space_treatment {
        WS_NORMAL,
        WS_HEADING,
        WS_GLOSSARY,
        WS_LIST,
        WS_DIR,
        WS_PRE
} white_space_treatment;
/*

Nesting State

   These elements form tree with an item for each nesting state: that is, each unique
   combination of nested elements which has a specific style.
   
 */
typedef struct _HTNesting {
    void *                      style;  /* HTStyle *: Platform dependent */
    white_space_treatment       wst;
    struct _HTNesting *         parent;
    int                         element_number;
    int                         item_number;    /* only for ordered lists */
    int                         list_level;     /* how deep nested */
    HTList *                    children;
    BOOL                        paragraph_break;
    int                         magic;
    BOOL                        object_gens_HTML; /* we don't generate HTML */
} HTNesting;

/*

Nesting functions

   These functions were new with HTML2.c.  They allow the tree of SGML nesting states to
   be manipulated, and SGML regenerated from the style sequence.
   
 */
extern void HTRegenInit (void);

extern void HTRegenCharacter (
        char                    c,
        HTNesting *             nesting,
        HTStructured *          target);

extern  void HTNestingChange (
        HTStructured*   s,
        HTNesting*              old,
        HTNesting *             newnest,
        HTChildAnchor *         info,
        CONST char *            aName);

extern HTNesting * HTMLCommonality (
        HTNesting *     s1,
        HTNesting *     s2);

extern HTNesting * HTNestElement (HTNesting * p, int ele);
extern /* HTStyle * */ void * HTStyleForNesting (HTNesting * n);

extern HTNesting* HTMLAncestor (HTNesting * old, int depth);

extern HTNesting* CopyBranch (HTNesting * old, HTNesting * newnest,
                                     int depth);

extern HTNesting * HTInsertLevel (HTNesting * old,
                int     element_number,
                int     level);
extern HTNesting * HTDeleteLevel (HTNesting * old,
                int     level);
extern int HTMLElementNumber (HTNesting * s);
extern int HTMLLevel ( HTNesting * s);
extern HTNesting* HTMLAncestor (HTNesting * old, int depth);

#endif          /* end HTML_H */
/*

   end */
