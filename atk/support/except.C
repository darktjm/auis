#include <andrewos.h>
ATK_IMPL("except.H")
#include <except.H>

static except_HandlerContext_p except_CurrentContext = NULL;
static except_Exception except_ExceptionID = NULL;
static char *except_ExceptionValue = NULL;
static except_UncaughtExceptionHandler except_UncaughtHandler = NULL;


ATKdefineRegistry(except, ATK, NULL);
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
