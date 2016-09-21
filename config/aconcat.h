/** \addtogroup libatkos
 * @{ */
/* Sigh..., unfortunately if your cpp lies all you can do is add -DUnixCPP or -DANSI_CPP to the imake command, or edit this file... -rr2b */

/*
 * Concat - concatenates two strings.
 */
#ifndef Concat
#if defined(ANSI_CPP) || (__STDC__ && !defined(UnixCpp))
#define Concat(a,b)a##b
#else
#define Concat(a,b)a/**/b
#endif
#endif

/*
 * Concat3 - concatenates three strings.
 */
#ifndef Concat3
#if defined(ANSI_CPP) || (__STDC__ && !defined(UnixCpp))
#define Concat3(a,b,c)a##b##c
#else
#define Concat3(a,b,c)a/**/b/**/c
#endif
#endif

/*
  * Stringize, return a double quoted string containing the first argument.
 * the second argument is ignored and can always be empty.  The second
 * argument is only needed to allow an empty first argument to be specified.
 */
#ifndef Stringize
#if defined(ANSI_CPP) || (__STDC__ && !defined(UnixCpp))
#define Stringize(a) #a
#else
#define Stringize(a) "a"
#endif
#endif
/** @} */
