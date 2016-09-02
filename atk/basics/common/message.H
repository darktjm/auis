ATK_INTER
#ifndef _message_H_
#define _message_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 

/* message.H
 * Classprocedures to interface to message handlers.
 *
 */

/* Return values for completion functions. */
enum message_CompletionCode {
    message_Complete, /* The string returned is a unique completion of the user input. */
    message_CompleteValid, /* The string returned is a completion and a valid substring of another complete. */
    message_Valid, /* The string returned is a valid substring of a complete entry. Must be at least as long a user's input. */
    message_Invalid /* The string returned is the longest viable prefix of the user's input. */
};

/* Values for help procedure calls. */
enum message_HelpItem {
    message_HelpListItem, /* This call is an item to be added to the list of possible completions. */
    message_HelpGenericItem /* This call contains text with no notable features. Presumably for the user to read. */
};

enum message_Preference {
    message_NoBlock = 3,
    message_OnTop = 2, 
    message_OnBottom = 1, 
    message_Center = 0
};


#define message_MustMatch 1 /* Can only return valid completions if this_c flag is set. */
#define message_InitialHelp 2 /* Display help at beggining if this_c flag is set. */
#define message_NoInitialString 4 /* Do not use default as initial string. */
#define message_Mandatory 8 /* Indicates that question must be answered. It cannot be punted. */

/* All buffer size arguments in this module are as returned by sizeof(buffer).
 * That is, they include the NUL at the end of the string.
 */
#include <ATK.H>

	typedef void (*message_workfptr)(long rock, enum message_HelpItem item, const char *str, const char *val);
        typedef enum message_CompletionCode (*message_completionfptr) (char *partial, long dummyData, char *buffer, int bufferSize);
        typedef void (*message_helpfptr)(char *partial,long myrock,message_workfptr HelpWork,long rock);

class message : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

        static int DisplayString(class view *view, int priority, const char *string)  ;
        static int AskForString(class view *view, int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize)  ;
        static int AskForPasswd(class view *view, int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize)  ;
        static int AskForStringCompleted(class view *view, int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize,
                              class keystate *keystate, message_completionfptr completionProc, message_helpfptr helpProc, long functionData, int flags)  ;
        static int MultipleChoiceQuestion(class view *view, int priority, const char *prompt, long defaultChoice, long *result, const char * const *choices, const char *abbrevKeys)  ;
        static void CancelQuestion(class view *view);
	static void Advice(class view *view, enum message_Preference pref);
/* These next calls are only applicable during an AskFor... operation. */
        static int GetCurrentString(class view *view, char *buffer, int bufferSize)  ;
        static int InsertCharacters(class view *view, int pos, const char *string, int len)  ;
        static int DeleteCharacters(class view *view, int pos, int len)  ;
        static int GetCursorPos(class view *view)  ;
        static int SetCursorPos(class view *view, int pos)  ;
        static boolean Asking(class view *view)  ;
};

#endif /* _message_H_ */
