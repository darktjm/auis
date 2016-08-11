ATK_INTER
#ifndef _framemessage_H_
#define _framemessage_H_
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

    
         int DisplayString(int priority, char *string)  ;
         int AskForString(int priority, char *prompt, char *defaultString, char *buffer, int bufferSize)  ;
         int AskForPasswd(int priority, char *prompt, char *defaultString, char *buffer, int bufferSize)  ;
         int AskForStringCompleted(int priority, char *prompt, char *defaultString, char *buffer, int bufferSize, class keystate *keystatep,
					  message_completionfptr completionProc, message_helpfptr helpProc, long functionData, int flags)  ;
         int MultipleChoiceQuestion(int priority, char *prompt, long defaultChoice, long *result, char **choices, char *abbrevKeys)  ;
         void CancelQuestion();
/* These next calls are only applicable during an AskFor... operation. */
         int GetCurrentString(char *buffer, int bufferSize)  ;
         int InsertCharacters(int pos, char *string, int len)  ;
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
