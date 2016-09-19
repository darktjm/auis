ATK_INTER
#ifndef _msghandler_H_
#define _msghandler_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* msghndlr.H
 * Definition of a msghandler object for use by message handlers.
 *
 */

#define msghandler_VERSION 1


#include <ATK.H>
#include <message.H>

class msghandler : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        virtual int DisplayString(int priority, const char *string)  ;
        virtual int AskForString(int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize)  ;
        virtual int AskForPasswd(int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize)  ;
        virtual int AskForStringCompleted(int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize, class keystate *keystate, message_completionfptr completionProc, message_helpfptr helpProc, long functionData, int flags)  ;
        virtual int MultipleChoiceQuestion(int priority, const char *prompt, long defaultChoice, long *result, const char * const *choices, const char *abbrevKeys)  ;
        virtual void CancelQuestion();
/* These next calls are only applicable during an AskFor... operation. */
        virtual int GetCurrentString(char *buffer, int bufferSize)  ;
        virtual int InsertCharacters(int pos, const char *string, int len)  ;
        virtual int DeleteCharacters(int pos, int len)  ;
        virtual int GetCursorPos()  ;
        virtual int SetCursorPos(int pos)  ;
        virtual boolean Asking()  ;
	virtual void Advice(enum message_Preference pref) ;
};

#endif /* _msghandler_H_ */
