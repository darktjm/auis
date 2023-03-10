/*                                                                          Association Pairs
                      ASSOCIATION LIST FOR STORING NAME-VALUE PAIRS
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   Lookups from assosiation list are not case-sensitive.
   
   This module is implemented by HTAssoc.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTASSOC_H
#define HTASSOC_H

#include "HTList.h"


#ifdef SHORT_NAMES
#define HTAL_new        HTAssocList_new
#define HTAL_del        HTAssocList_delete
#define HTAL_add        HTAssocList_add
#define HTAL_lup        HTAssocList_lookup
#endif /*SHORT_NAMES*/

typedef HTList HTAssocList;

typedef struct {
    char * name;
    char * value;
} HTAssoc;


extern HTAssocList *HTAssocList_new (void);
extern void HTAssocList_delete (HTAssocList * alist);

extern void HTAssocList_add (HTAssocList *       alist,
                                    CONST char *        name,
                                    CONST char *        value);

extern char *HTAssocList_lookup (HTAssocList *   alist,
                                        CONST char *    name);

#endif /* not HTASSOC_H *//*

   End of file HTAssoc.h. */
