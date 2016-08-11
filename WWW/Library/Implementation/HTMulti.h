/*                                                             Multiformat handling in libwww
                                   MULTIFORMAT HANDLING
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module is implemented by HTMulti.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTMULTI_H
#define HTMULTI_H

#include "HTAccess.h"

/*
**      Set default file name for welcome page on each directory.
*/
extern void HTAddWelcome PARAMS((char * welcome_name));

/*
**      Perform multiformat handling
*/
extern char * HTMulti PARAMS((HTRequest *       req,
                              char *            path,
                              struct stat *     stat_info));

#endif /* HTMULTI_H */
/*

   End of HTMulti. */
