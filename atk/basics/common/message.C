/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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



ATKdefineRegistryNoInit(message, ATK);

int message::DisplayString(class view  *view, int  priority, const char  *string)
                {
    const char *str = messitem::Replace(string);
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

int message::AskForString(class view  *view, int  priority, const char  *prompt , const char  *defaultString , char  *buffer, int  bufferSize)
                    {
    const char *str = messitem::Replace(prompt);
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

int message::AskForPasswd(class view  *view, int  priority, const char  *prompt , const char  *defaultString , char  *buffer, int  bufferSize)
                    {
    const char *str = messitem::Replace(prompt);
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


int message::AskForStringCompleted(struct view  *view, int  priority, const char  *prompt , const char  *defaultString , char  *buffer, int  bufferSize, struct keystate  *keystate, message_completionfptr  completionProc , message_helpfptr  helpProc, long  functionData, int  flags)
                                    {
    const char *str = messitem::Replace(prompt);
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

int message::MultipleChoiceQuestion(class view  *view, int  priority, const char  *prompt, long  defaultChoice, long  *result, const char  * const *choices, const char  *abbrevKeys)
                                {
    class msghandler *handler;
    const char *nchoices[128];
    int i = 0;
    const char *str = messitem::Replace(prompt);
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

int message::InsertCharacters(class view  *view, int  pos, const char  *string, int  len)
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
