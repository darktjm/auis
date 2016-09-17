/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/*
	conftest.c
	Test the current Andrew configuration for reasonableness.
*/

#include <andrewos.h>
#include <stdio.h>

/* ./conftest "$(RESOLVLIB)" */

int main (int argc, const char **argv)
{
    int Problem, OneProb;

    Problem = 0;
    printf("conftest: Checking configuration for sys type ``%s'', full name ``%s''\n", SYS_NAME, OPSYSNAME);
    if ((SY_B4x + SY_U5x + SY_AIXx + SY_OS2) == 0) {
	printf("conftest: This system type, %s, does not define any SY_xxx value\n", SYS_NAME);
	printf("          in its system.h file.\n");
	Problem = 1;
    } else {
	printf("Sys type %s is most like %s.\n", SYS_NAME,
	    (SY_B4x ? "4.3 BSD" :
	     (SY_U5x ? "System V" :
	      (SY_AIXx ? "AIX" :
	       (SY_OS2 ? "OS/2" : "NO SYSTEM!")))) );
    }

    printf("conftest: Checking for problems in the allsys.h/system.h/site.h files.\n");
    OneProb = 0;

#ifndef	SY_B42
    printf("SY_B42 is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_B42 0
#endif /* SY_B42 */
#ifndef	SY_B43
    printf("SY_B43 is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_B43 0
#endif /* SY_B43 */
#ifndef	SY_U51
    printf("SY_U51 is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_U51 0
#endif /* SY_U51 */
#ifndef	SY_U52
    printf("SY_U52 is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_U52 0
#endif /* SY_U52 */
#ifndef	SY_U53
    printf("SY_U53 is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_U53 0
#endif /* SY_U53 */
#ifndef	SY_AIX11
    printf("SY_AIX11 is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_AIX11 0
#endif /* SY_AIX11 */
#ifndef	SY_AIX12
    printf("SY_AIX12 is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_AIX12 0
#endif /* SY_AIX12 */
#ifndef	SY_AIX221
    printf("SY_AIX221 is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_AIX221 0
#endif /* SY_AIX221 */
#ifndef	SY_B4x
    printf("SY_B4x is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_B4x 0
#endif /* SY_B4x */
#ifndef	SY_U5x
    printf("SY_U5x is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_U5x 0
#endif /* SY_U5x */
#ifndef	SY_AIXx
    printf("SY_AIXx is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_AIXx 0
#endif /* SY_AIXx */
#ifndef SY_OS2
    printf("SY_OS2 is not defined.  It is turned off by defining it to have\n");
    printf("a value of zero, not undefining it.  It must be defined.\n");
    OneProb = 1;
#define SY_OS2 0
#endif /* SY_OS2 */
#if ((SY_B4x + SY_U5x + SY_AIXx + SY_OS2) > 1)
    printf("Your system.h file overspecifies the system kind.\n");
    printf("That is, it claims that this system is like more than one of\n");
    printf("the set 4.3BSD (SY_B4x), System V (SY_U5x), OS/2 and AIX (SY_AIXx).\n");
    OneProb = 1;
#else /* (SY_B4x + SY_U5x + SY_AIXx) > 1 */
#if ((SY_B4x + SY_U5x + SY_AIXx + SY_OS2) != 1)
    printf("Your system.h file underspecifies the system kind.\n");
    printf("That is, it does not claim that it is like any of\n");
    printf("the set 4.3BSD (SY_B4x), System V (SY_U5x), OS/2 and AIX (SY_AIXx).\n");
    OneProb = 1;
#endif /* (SY_B4x + SY_U5x + SY_AIXx) != 1 */
#endif /* (SY_B4x + SY_U5x + SY_AIXx) > 1 */

    if (OneProb != 0) {
	printf("You should correct the defined status of the SY_xxx variables\n");
	printf("in your new system.h file.\n");
	Problem = 1;
	OneProb = 0;
    }

/* DEBUG_MALLOC_ENV => ANDREW_MALLOC_ENV */
#ifdef DEBUG_MALLOC_ENV
#ifndef ANDREW_MALLOC_ENV
    printf("DEBUG_MALLOC_ENV is defined, but ANDREW_MALLOC_ENV is not.\n");
    printf("Either define ANDREW_MALLOC_ENV or undefine DEBUG_MALLOC_ENV .\n");
    OneProb = 1;
#endif /* ANDREW_MALLOC_ENV */
#endif /* DEBUG_MALLOC_ENV */

/* FONTS_TO_BDF_ENV => FONTS_TO_PCF_ENV */
#ifdef FONTS_TO_BDF_ENV
#ifdef FONTS_TO_PCF_ENV
    printf("Both FONTS_TO_BDF_ENV and FONTS_TO_PCF_ENV are defined.\n");
    printf("You must undef FONTS_TO_PCF_ENV or remove \n");
    printf("the define of FONTS_TO_BDF_ENV in site.h.\n");
    OneProb = 1;
#endif /* FONTS_TO_PCF_ENV */
#endif /* FONTS_TO_BDF_ENV */

#ifndef X11_ENV
    printf("X11_ENV is not defined.\n");
    printf("ATK won't build for you.\n");
    OneProb = 1;
#endif /* X11_ENV */

    if (OneProb != 0) {
	printf("conftest: You should correct the defined status of the _ENV variables\n");
	printf("in your config/site.h file.\n");
	Problem = 1;
	OneProb = 0;
    } else {
	printf("conftest: No inconsistencies detected in the allsys.h/system.h/site.h files.\n");
    }

    if (Problem == 0) {
	printf("conftest: No inconsistencies found.  Continuing.\n");
	exit(0);
    } else {
	printf("conftest: Inconsistent options selected for system build.  ABORTING!\n");
	exit(1);
    }
}
