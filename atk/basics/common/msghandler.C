/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/* Complete bogosity. */


#include <andrewos.h>
ATK_IMPL("msghandler.H")
#include <msghandler.H>


ATKdefineRegistry(msghandler, ATK, NULL);

int msghandler::DisplayString(int  priority, const char  *string)
            {
    return -1;
}

int msghandler::AskForString(int  priority, const char  *prompt , const char  *defaultString , char  *buffer, int  bufferSize)
                {
    return -1;
}

int msghandler::AskForPasswd(int  priority, const char  *prompt , const char  *defaultString , char  *buffer, int  bufferSize)
                {
    return (this)->AskForString(priority,prompt,defaultString,buffer,bufferSize);
}

int msghandler::AskForStringCompleted(int  priority, const char  *prompt , const char  *defaultString , char  *buffer, int  bufferSize, class keystate  *keystate, message_completionfptr  completionProc , message_helpfptr  helpProc, long  functionData, int  flags)
                                {
    return -1;
}

int msghandler::MultipleChoiceQuestion(int  priority, const char  *prompt, long  defaultChoice, long  *result, const char  * const *choices, const char  *abbrevKeys)
                            {
    return -1;
}

void msghandler::CancelQuestion()
    {
}

int msghandler::GetCurrentString(char  *buffer, int  bufferSize)
            {
    return -1;
}

int msghandler::InsertCharacters(int  pos, const char  *string, int  len)
                {
    return -1;
}

int msghandler::DeleteCharacters(int  pos , int  len)
        {
	return 0;
}

int msghandler::GetCursorPos()
    {
    return -1;
}

int msghandler::SetCursorPos(int  pos)
        {
    return -1;
}

boolean msghandler::Asking()
    {
    return FALSE;
}

void msghandler::Advice( enum message_Preference  pp)
{
}
