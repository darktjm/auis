/*                                                                     List object for libwww
                                       LIST OBJECT
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   The list object is a generic container for storing collections of things in order.  In
   principle it could be implemented in many ways, but in practice knowing that it is a
   linked list is important for speed.  See also the traverse macro for example.
   
   This module is implemented by HTList.c, and it is a part of the Library of Common Code.
   
 */
#ifndef HTLIST_H
#define HTLIST_H


typedef struct _HTList HTList;

struct _HTList {
  void * object;
  HTList * next;
};

#ifdef SHORT_NAMES
#define HTList_new                      HTLiNew
#define HTList_delete                   HTLiDele
#define HTList_addObject                HTLiAdOb
#define HTList_removeObject             HTLiReOb
#define HTList_removeLastObject         HTLiReLa
#define HTList_removeFirstObject        HTLiReFi
#define HTList_count                    HTLiCoun
#define HTList_indexOf                  HTLiInOf
#define HTList_objectAt                 HTLiObAt
#endif

extern HTList * HTList_new (void);
extern void     HTList_delete (HTList *me);
/*

  ADD OBJECT TO START OF LIST
  
 */
extern void     HTList_addObject (HTList *me, void *newObject);


extern BOOL     HTList_removeObject (HTList *me, void *oldObject);
extern void *   HTList_removeLastObject (HTList *me);
extern void *   HTList_removeFirstObject (HTList *me);
#define         HTList_isEmpty(me) (me ? me->next == NULL : YES)
extern int      HTList_count (HTList *me);
extern int      HTList_indexOf (HTList *me, void *object);
#define         HTList_lastObject(me) \
  (me && me->next ? me->next->object : NULL)
extern void *   HTList_objectAt (HTList *me, int position);
/*

  TRAVERSE LIST
  
   Fast macro to traverse the list. Call it first with copy of list header :  it returns
   the first object and increments the passed list pointer. Call it with the same variable
   until it returns NULL.
   
 */
#define HTList_nextObject(me) \
  (me && (me = me->next) ? me->object : NULL)
/*

  FREE LIST
  
 */
#define HTList_free(x)  free(x)

#endif /* HTLIST_H */
/*

   end */
