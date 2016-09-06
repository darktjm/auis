/*                                                           Handling user messages in libwww
                          DISPLAYING MESSAGES AND GETTING INPUT
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This modue may be overridden for GUI clients. It allows progress indications and
   warning messages to be communicated to the user in a portable way.
   
      May 92 Created By C.T. Barker
      
      Feb 93 Portablized etc TBL
      
   This module is implemented by HTAlert.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTALERT_H
#define HTALERT_H
#include "HTUtils.h"/*

Flags for This Module

 */
extern BOOL HTInteractive;                  /* Any prompts from the Library? *//*

HTPrompt and HTPromptPassword: Display a message and get the input

   HTPromptPassword() doesn't echo reply on the screen.
   
  ON ENTRY,
  
  Msg                     String to be displayed.
                         
  deflt                   If NULL the default value (only for HTPrompt())
                         
  ON EXIT,
  
  Return value            is malloc'd string which must be freed.
                         
 */
                
extern char * HTPrompt (CONST char * Msg, CONST char * deflt);
extern char * HTPromptPassword (CONST char * Msg);
/*

HTPromptUsernameAndPassword: Get both username and password

  ON ENTRY,
  
  Msg                    String to be displayed.
                         
  username                Pointer to char pointer, i.e. *usernamepoints to a string.  If
                         non-NULL it is taken to be a default value.
                         
  password                Pointer to char pointer, i.e. *passwordpoints to a string.
                         Initial value discarded.
                         
  ON EXIT,
  
  *username               and
                         
  *password               point to newly allocated strings representing the typed-in
                         username and password.  Initial strings pointed to by them are
                         NOT freed!
                         
 */
extern void HTPromptUsernameAndPassword (CONST char *    Msg,
                                                char **         username,
                                                char **         password);
/*

Display a message, don't wait for input

  ON ENTRY,
  
  Msg                     String to be displayed.
                         
 */
extern void HTAlert (CONST char * Msg);

/*

Display a progress message for information (and diagnostics) only

  ON ENTRY,
  
   The input is a list of parameters for printf.
   
 */
extern void HTProgress (CONST char * Msg);

/*

Display a message, then wait for 'yes' or 'no'.

  ON ENTRY,
  
  Msg                     String to be displayed
                         
  ON EXIT,
  
  Returns                 If the user reacts in the affirmative, returns TRUE, returns
                         FALSE otherwise.
                         
 */
extern BOOL HTConfirm (CONST char * Msg);

#endif/*

    */
