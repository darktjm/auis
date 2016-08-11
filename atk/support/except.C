

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/support/RCS/except.C,v 3.2 1994/11/30 20:42:06 rr2b Stab74 $";
#endif
#include <andrewos.h>
ATK_IMPL("except.H")
#include <except.H>

except_HandlerContext_p except_CurrentContext = NULL;
except_Exception except_ExceptionID = NULL;
char *except_ExceptionValue = NULL;
except_UncaughtExceptionHandler except_UncaughtHandler = NULL;


ATKdefineRegistry(except, ATK, NULL);
#ifndef NORCSID
#endif
static void except_DefaultHandler();


except_Exception except::GetRaisedException()
    {
    return (except_ExceptionID);
} /* except__GetRaisedException */

void except::SetExceptionValue(char  *v)
        {
    except_ExceptionValue = v;
} /* except__SetExceptionValue */

char *except::GetExceptionValue()
    {
    return (except_ExceptionValue);
} /* except__GetExceptionValue */

void except::SetUncaughtExceptionHandler(except_UncaughtExceptionHandler  h)
        {
    except_UncaughtHandler = h;
} /* except__SetUncaughtExceptionHandler */

except_UncaughtExceptionHandler except::GetUncaughtExceptionHandler()
    {
    return (except_UncaughtHandler);
} /* except__GetUncaughtExceptionHandler */

void except::PushContext(except_HandlerContext_p  context)
        {
    context->nested = except_CurrentContext;
    except_CurrentContext = context;
} /* except__PushContext */

boolean except::CheckException(except_Exception  xid, except_HandlerContext_p  context, int  *flags)
                {
    if ((strcmp(xid, except_ExceptionID) == 0) ||
	(strcmp(xid, except_ANY) == 0))
    {
	except_CurrentContext = context->nested;
	*flags |= except_Handled;

	return (TRUE);
    }

    return (FALSE);
} /* except__CheckException */

void except::ResetContext(except_HandlerContext_p  context)
        {
    except_CurrentContext = context;
} /* except__ResetContext */

except_HandlerContext_p except::GetCurrentContext()
    {
    return (except_CurrentContext);
} /* except__GetCurrentContext */

static void except_DefaultHandler()
{
    static int i = 0;

    /* something that is guaranteed to cause an uncaught signal */
    i = 3 / i;
} /* except_DefaultHandler */

boolean except::Raise(except_Exception  xid, char  *value)
            {
    if (xid != NULL) {
	except_ExceptionID = xid;
	except_ExceptionValue = value;
    }

    if (except_CurrentContext != NULL) {
	longjmp(except_CurrentContext->env, except_Raised);
    }
    else if (except_UncaughtHandler != NULL) {
	(*except_UncaughtHandler)();
    }
    else {
	except_DefaultHandler();
    }

    return (FALSE);
} /* except__RAISE */
