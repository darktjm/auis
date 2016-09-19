ATK_INTER
#ifndef _framemessage_H_
#define _framemessage_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* framemsg.H
 * Class for the frame's message line abstraction.
 *
 */

/* All buffer size arguments in this module are as returned by sizeof(buffer).
 * That is, they include the NUL at the end of the string.
 */
#include <msghandler.H>

class framemessage: public  msghandler {
public:
	virtual ATKregistryEntry *ATKregistry();

    
         int DisplayString(int priority, const char *string)  ;
         int AskForString(int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize)  ;
         int AskForPasswd(int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize)  ;
         int AskForStringCompleted(int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize, class keystate *keystatep,
					  message_completionfptr completionProc, message_helpfptr helpProc, long functionData, int flags)  ;
         int MultipleChoiceQuestion(int priority, const char *prompt, long defaultChoice, long *result, const char * const *choices, const char *abbrevKeys)  ;
         void CancelQuestion();
/* These next calls are only applicable during an AskFor... operation. */
         int GetCurrentString(char *buffer, int bufferSize)  ;
         int InsertCharacters(int pos, const char *string, int len)  ;
         int DeleteCharacters(int pos, int len)  ;
         int GetCursorPos()  ;
         int SetCursorPos(int pos)  ;
         boolean Asking()  ;
	 void Advice(enum message_Preference);

	
	virtual void SetCompanion(class msghandler *companion);

    
        
	virtual ~framemessage();
        static class framemessage * Create(class frame *myFrame)  ;
        
	static boolean InitializeClass();

	
	framemessage();    

    
        class msghandler *companion;
        class frame *frame;
        class frameview *messageView;
        class text *messageText;
        class view *oldInputFocus;
        class buffer *realBuffer;
        long messageLen; /* Length of message displayed during an AskFor operation. */
	message_completionfptr completionProc;
	message_helpfptr helpProc;
        long completionData;
        char *textBuffer;
        int maxTextSize;
        class keystate *keystatep;
        struct im_InteractionEvent *erasureEvent;
        char flags;
        char asking;
        char punt;
        char hasDefault; /* TRUE if defaultString in nonNULL. */
};

#endif /* _framemessage_H_ */
