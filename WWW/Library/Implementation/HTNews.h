/*                                  Network News Transfer protocol module for the WWW library
                                       NEWS ACCESS
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module is implemented by HTNews.c, and it is a part of the Library of Common Code.
   
 */
#ifndef HTNEWS_H
#define HTNEWS_H

#include "HTAccess.h"

GLOBALREF HTProtocol HTNews;

extern void             HTSetNewsHost PARAMS((CONST char *value));
extern CONST char *     HTGetNewsHost NOPARAMS;
extern char *           HTNewsHost;

#endif /* HTNEWS_H */
/*

    */
