/*                                                                             HTML Generator
                                      HTML GENERATOR
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module converts structed stream into stream.  That is, given a stream to write to,
   it will give you a structured stream to.
   
   This module is implemented by HTMLGen.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTMLGEN_H
#define HTMLGEN_H

#include "HTML.h"
#include "HTStream.h"

/* Special Creation: */
extern HTStructured * HTMLGenerator (HTStream * output);

#ifndef pyramid
extern HTConverter HTPlainToHTML;
#endif

#endif/*

    */
