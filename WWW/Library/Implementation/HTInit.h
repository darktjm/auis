/*                                                          Initialization routines in libwww
                                  INITIALIZATION MODULE
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module resisters all the plug & play software modules which will be used in the
   program on client side (The server has it's own initialization module).
   
   The initialization consists of two phases: Setting up the MIME type conversions and
   defining the relation between a basic set of file suffixes and MIME types.  To override
   this, just copy it and link in your version before you link with the library.
   
   This module is implemented by HTInit.c, and it is a part of the Library of Common Code.
   
 */
#ifndef HTINIT_H
#define HTINIT_H

#include "HTList.h"/*

MIME Type Conversions

   Two default functions can be used to do the MIME type initialization. The first
   contains all MIME types the client can handle plus the types that can be handled using
   save and execute calls, e.g. to start a PostScript viewer. The second function only
   contains MIME type conversions that can be handled internally in the client. As an
   example, the latter is used when Line Mode Browser is started in Non-Interactive mode.
   
 */
extern void HTFormatInit PARAMS((HTList * conversions));
extern void HTFormatInitNIM PARAMS((HTList * conversions));/*

File Suffix Setup

   This functions defines a basic set of file suffixes and the corresponding MIME types.
   
 */
extern void HTFileInit NOPARAMS;
#endif/*

   End of HTInit Module. */
