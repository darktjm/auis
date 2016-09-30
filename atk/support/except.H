#ifndef _except_H_
#define _except_H_

#include <andrewos.h>
#include <setjmp.h>

/** \addtogroup libsupport
 * @{ */
typedef char *except_Exception;

typedef boolean (*except_UncaughtExceptionHandler)();

#define except_ANY "except_ANY"

/* Simpler interface to ANY */
#define ANY except_ANY

/* Simpler interfaces to RAISE, ASSERT, and PROPAGATE */
#define RAISE(x, v) except::Raise(x, v)
#define ASSERT(condition, xid, value) except_ASSERT(condition, xid, value)
#define PROPAGATE() except_PROPAGATE()

/*
   PRIVATE STRUCTURE
	Used to deal with a stack of current handler contexts.
	The head of the stack is held by except_CurrentContext.
 */
typedef struct except_HandlerContext {
    struct except_HandlerContext *nested;
    jmp_buf env;
} except_HandlerContext_t, *except_HandlerContext_p;

/*
   PRIVATE FLAGS
	except_Raised indicates an exception was raised.
	except_Handled indicates the exception was handled
		in the current handler context.
  
   INVARIANT:	Flags may have except_Handled set only if
		except_Raised is also set.
 */
#define except_Raised	0x1
#define except_Handled	0x2

#define TRY \
	{ int _except_flags; \
	  except_HandlerContext_t _except_context; \
	  except::PushContext(&_except_context); \
	  if ((_except_flags = setjmp(_except_context.env)) == 0) {

#define EXCEPT(xid) \
          } else if (except::CheckException(xid, &_except_context, &_except_flags)) {

#define FINALLY \
          } \
	  except::ResetContext(_except_context.nested); \
	  {

#define ENDTRY \
	  } \
	  except::ResetContext(_except_context.nested); \
	  if (_except_flags == except_Raised) { \
	      except::Raise(NULL, NULL); \
          } \
        }

#define except_ExceptionRaised() (_except_flags & except_Raised)
#define except_ExceptionHandled() (_except_flags & except_Handled)

#define except_ASSERT(condition, xid, value) \
	((condition) || except::Raise(xid, value))
#define except_PROPAGATE() \
	{ if (except_ExceptionRaised()) except::Raise(NULL, NULL); }

#include <ATK.H>

class except : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	static except_Exception GetRaisedException()  ;
	static void SetExceptionValue(char *value);
	static char * GetExceptionValue()  ;
	static void SetUncaughtExceptionHandler(except_UncaughtExceptionHandler h);
	static except_UncaughtExceptionHandler GetUncaughtExceptionHandler()  ;
	static boolean Raise(except_Exception x, char *value)  ;

/*
   LOCAL NON-EXPORTED ROUTINES USED BY THE MACROS ABOVE -- USE MACROS INSTEAD!
 */
	static void PushContext(except_HandlerContext_p context);
	static boolean CheckException(except_Exception xid, except_HandlerContext_p context, int *flags)  ;
	static void ResetContext(except_HandlerContext_p context);
	static except_HandlerContext_p GetCurrentContext()  ;
};

/***************************************************************

	|-| Name:	except_begin, except_end, except_for, except_while, except_do

	|-| Abstract:	Bracketing constructs for routines containing TRY
			statements with embedded returns or loop statements
			containing TRY statements with embedded continues or
			breaks.

			TRY statements may be nested.  Embedded return or
			continue or break statements must be replaced by
			except_return, except_continue, or except_break
			(see above).  This does NOT apply to break statements
			for switch statements.

	|-| Parameters:	None

	|-| Results:	None

	|-| Side-effects:	Sets up appropriate state

	|-| Exceptions:	None

	|-| Log:
	    19 Jun 90	mlh	Created.

***************************************************************/

/*
   Use this to replace the open bracket { at the start of a routine
   containing a TRY statement with an embedded except_return statement.
 */
#define except_begin \
	{ except_HandlerContext_p _except_routinecontext = except::GetCurrentContext();

/*
   Use this to replace the close bracket } at the end of a routine to
   match the corresponding except_begin.

   Similarly, use this to match any of except_for, except_while, or except_do
   when used under the circumstances described below.  Note that in this case
   the except_end does not replace a close bracket if an open bracket is used.
 */
#define except_end }

/*
   Use these to replace for, while, or do when such statements contain a
   TRY statement with an embedded except_break or except_continue.  Such use
   must be terminated with a matching except_end.  If the loop requires
   { } bracketing, the except_end does NOT replace the closing } bracket!
 */
#define except_for \
	{ except_HandlerContext_p _except_loopcontext = except::GetCurrentContext(); for
#define except_while \
	{ except_HandlerContext_p _except_loopcontext = except::GetCurrentContext(); while
#define except_do \
	{ except_HandlerContext_p _except_loopcontext = except::GetCurrentContext(); do

/***************************************************************

	|-| Name:	except_break, except_continue, except_return, except_returnvalue

	|-| Abstract:	These are non-local exits from a TRY clause.
			They should NOT be used for loops totally
			embedded within TRY clauses.

			except_break and except_continue for loops that
			contain a TRY statement.

			except_return(value) for returning from a routine
			from within a TRY statement.

	|-| Parameters:
		t	type for return statement value.

		e	value for return statement;
			if value expression may raise an exception
			during its evaluation, better to store
			the result in a local and return the local,
			otherwise the exception will not be handled
			by the handler context established by the
			containing TRY statement.

	|-| Results:	None

	|-| Side-effects:
		As one might expect.

	|-| Exceptions:	See note for parameter e above.

	|-| Log:
	    27 Mar 90	mlh	Created.

***************************************************************/

#define except_break \
	{ except::ResetContext(_except_loopcontext); break; }
#define except_continue \
	{ except::ResetContext(_except_loopcontext); continue; }
#define except_returnvalue(t, e) \
	{ t _rval = e; except::ResetContext(_except_routinecontext); return _rval; }
#define except_return \
	{ except::ResetContext(_except_routinecontext); return; }
/** @} */
#endif /* _except_H_ */
