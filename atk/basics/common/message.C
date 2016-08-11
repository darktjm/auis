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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/message.C,v 3.5 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

/* message.c
 * Provides application level interface to message handler facillities.
 */

#include <andrewos.h>
ATK_IMPL("message.H")

#include <msghandler.H>
#include <view.H>
#include <im.H>
#include <messitem.H>
#include <message.H>



ATKdefineRegistry(message, ATK, NULL);

int message::DisplayString(class view  *view, int  priority, char  *string)
                {
    char *str = messitem::Replace(string);
    class msghandler *handler;

    if (view == NULL) {
        view = (class view *) im::GetLastUsed();
        if (view == NULL)
            return -1;
    }

    handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler == NULL) {
        return -1;
    }
    return (handler)->DisplayString( priority, str);
}

int message::AskForString(class view  *view, int  priority, char  *prompt , char  *defaultString , char  *buffer, int  bufferSize)
                    {
    char *str = messitem::Replace(prompt);
    class msghandler *handler;

    if (view == NULL) {
        view = (class view *) im::GetLastUsed();
        if (view == NULL)
            return -1;
    }

    handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler == NULL) {
        return -1;
    }
    return (handler)->AskForString( priority, str, defaultString, buffer, bufferSize);
}

int message::AskForPasswd(class view  *view, int  priority, char  *prompt , char  *defaultString , char  *buffer, int  bufferSize)
                    {
    char *str = messitem::Replace(prompt);
    class msghandler *handler;

    if (view == NULL) {
        view = (class view *) im::GetLastUsed();
        if (view == NULL)
            return -1;
    }

    handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler == NULL) {
        return -1;
    }
    return (handler)->AskForPasswd( priority, str, defaultString, buffer, bufferSize);
}


int message::AskForStringCompleted(struct view  *view, int  priority, char  *prompt , char  *defaultString , char  *buffer, int  bufferSize, struct keystate  *keystate, message_completionfptr  completionProc , message_helpfptr  helpProc, long  functionData, int  flags)
                                    {
    char *str = messitem::Replace(prompt);
    class msghandler *handler;

    if (view == NULL) {
        view = (class view *) im::GetLastUsed();
        if (view == NULL)
            return -1;
    }

    handler = (class msghandler *) (view)->WantHandler( "message");
    if (handler == NULL) {
        return -1;
    }

    return (handler)->AskForStringCompleted( priority, str, defaultString, buffer, bufferSize, keystate, completionProc, helpProc, functionData, flags);
}

int message::MultipleChoiceQuestion(class view  *view, int  priority, char  *prompt, long  defaultChoice, long  *result, char  **choices, char  *abbrevKeys)
                                {
    class msghandler *handler;
    char *nchoices[128];
    int i = 0;
    char *str = messitem::Replace(prompt);
    while (choices && *choices) {
	nchoices[i] = messitem::Replace(*choices);
	i++;
	choices++;
    }
    nchoices[i] = NULL;

    if (view == NULL) {
        view = (class view *) im::GetLastUsed();
        if (view == NULL)
            return -1;
    }

    handler = (class msghandler *) (view)->WantHandler( "message");
    if (handler == NULL) {
        return -1;
    }

    return (handler)->MultipleChoiceQuestion( priority, str, defaultChoice, result, nchoices, abbrevKeys);
}

void message::CancelQuestion(class view  *view)
        {

    class msghandler *handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler != NULL) {
        (handler)->CancelQuestion();
    }
}
void message::Advice(class view  *view,enum message_Preference  pp)
            {

    class msghandler *handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler != NULL) {
        (handler)->Advice(pp);
    }
}

int message::GetCurrentString(class view  *view, char  *buffer, int  bufferSize)
                {

    class msghandler *handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler == NULL) {
        return -1;
    }

    return (handler)->GetCurrentString( buffer, bufferSize);
}

int message::InsertCharacters(class view  *view, int  pos, char  *string, int  len)
                    {

    class msghandler *handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler == NULL) {
        return -1;
    }

    return (handler)->InsertCharacters( pos, string, len);
}

int message::DeleteCharacters(class view  *view, int  pos , int  len)
            {

    class msghandler *handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler == NULL) {
        return -1;
    }

    return (handler)->DeleteCharacters( pos, len);
}

int message::GetCursorPos(class view  *view)
        {

    class msghandler *handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler == NULL) {
        return -1;
    }

    return (handler)->GetCursorPos();
}

int message::SetCursorPos(class view  *view, int  pos)
            {

    class msghandler *handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler == NULL) {
        return -1;
    }

    return (handler)->SetCursorPos( pos);
}

boolean message::Asking(class view  *view)
        {

    class msghandler *handler = (class msghandler *) (view)->WantHandler( "message");

    if (handler == NULL) {
        return -1;
    }

    return (handler)->Asking();
}
