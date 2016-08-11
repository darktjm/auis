/*                                                  Atoms - an easy way of organizing strings
                                          ATOMS
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   Atoms are strings which are given representative pointer values so that they can be
   stored more efficiently, and compaisons for equality done more efficiently. The list of
   atoms are stored in a has table, so when asking for a new atom you might infact get
   back an existing one.
   
   There are a whole bunch of MIME-types defined as atoms, so please use them!
   
   This module is implemented by HTAtom.c, and it is a part of the Library of Common Code.
   
 */
#ifndef HTATOM_H
#define HTATOM_H

#include "HTList.h"

#ifdef SHORT_NAMES
#define HTAt_for        HTAtom_for
#define HTAt_tMa        HTAtom_templateMatches
#endif

typedef struct _HTAtom HTAtom;
struct _HTAtom {
        HTAtom *        next;
        char *          name;
}; /* struct _HTAtom *//*

Public Functions

   The following methods are available for this class:
   
  GET AN ATOM
  
   This function returns a representative value (an atom) such that it will always (within
   one run of the program) return the same value for the same given string.
   
 */
extern HTAtom * HTAtom_for PARAMS((CONST char * string));/*

  GET CONTENT OF AN ATOM
  
 */
#define HTAtom_name(a) ((a)->name)/*

   This macro returns the string pointed to by the atom.
   
  SEARCH FOR ATOMS
  
   Returns a list of atoms which matches the template given. It is especially made for
   MIME-types so that for example a template like text<slash><star> returns a list of all
   MIME-types of type text.
   
 */
extern HTList * HTAtom_templateMatches PARAMS((CONST char * templ));/*

  CLEANUP MEMORY
  
   In order to cleanup memory, call this function. This is done automaticly from the
   HTLibTerminate function.
   
 */
extern void HTAtom_deleteAll NOPARAMS;

#endif/*

   End of HTAtom definition.  */
