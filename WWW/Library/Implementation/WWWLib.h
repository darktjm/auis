/*                                             Global Include File for Library of Common Code
                      GLOBAL INCLUDE FILE FOR LIBRARY OF COMMON CODE
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This is the main include file necessary in order to use the Library of Common Code.
   Currently the file only consists of well-known Library include files, but the goal is
   to define a nice API in this file, so that all external functionality can be reached
   through this one.
   
 */
#ifndef WWWLIB_H
#define WWWLIB_H

/* General Utilities */
#include "tcp.h"
#include "HTUtils.h"                    /* Macros and other stuff */
#include "HTString.h"
#include "HTList.h"                     /* Memory management for lists  */
#include "HTChunk.h"                    /* Memory management for chunks */

/* Format Management, and Access functions for loading a URL etc. */
#include "HTFormat.h"                   /* Stream Stack etc. */
#include "HTAccess.h"                   /* Document access network code */

/* URLs and Anchors - Internal representation of URLs */
#include "HTAnchor.h"                   /* Anchor class Definition */
#include "HTParse.h"                    /* Parse URLs */

/* History Management */
#include "HTHist.h"                     /* Navigational aids */

/* Presentation of graphical objects and other streams */
#include "HTML.h"                       /* For HTML parser */
#include "HTMLGen.h"                    /* For reformatting HTML */
#include "HTFWrite.h"                   /* For non-interactive output */

/* Graphic Object and Styles */
#include "HText.h"
#include "HTStyle.h"

/* Local file access */
#include "HTFile.h"                     /* For Dir access flags */

/* For TCP relevant information (mail address etc.) */
#include "HTTCP.h"

/* Management of Rule Files for configuring the application */
#include "HTRules.h"

/* Event loop for multithreaded functionality */
#include "HTEvent.h"

/* Messages and User prompts */
#include "HTAlert.h"

/* TEMPORARY STUFF */
#include "HTTP.h"

#endif/*

   End of WWWLib API definition  */
