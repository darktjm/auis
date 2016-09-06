/*                                                                 String handling for libwww
                                    STRING MANAGEMENT
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   These functions provide functionality for case-independent string comparison and
   allocations with copies etc.
   
   This module is implemented by HTString.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTSTRING_H
#define HTSTRING_H


extern CONST char * HTLibraryVersion;   /* String for help screen etc *//*

Dynamic String Manipulation

   These two functions are dynamic versions of strcpy and strcat. They use malloc for
   allocating space for the string. If StrAllocCopy is called with a non-NULL dest, then
   this is freed before the new value is assigned so that only the last string created has
   to be freed by the user. If StrAllocCat is called with a NULL pointer as destination
   then it is equivalent to StrAllocCopy.
   
 */
#define StrAllocCopy(dest, src) HTSACopy (&(dest), src)
#define StrAllocCat(dest, src)  HTSACat  (&(dest), src)
extern char * HTSACopy (char **dest, CONST char *src);
extern char * HTSACat  (char **dest, CONST char *src);/*

Case-insensitive String Comparison

   The usual routines (comp instead of cmp) had some problem.
   
 */
extern int strcasecomp  (CONST char *a, CONST char *b);
extern int strncasecomp (CONST char *a, CONST char *b, int n);/*

Case-insensitive strstr

   This works like strstr() but is not case-sensitive.
   
 */
extern char * strcasestr (char * s1, char * s2);/*

Next word or quoted string

 */
extern char * HTNextField (char** pstr);
#endif/*

    */
