/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/* 
 * Control inclusion of debugging code in object code
 */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/vui/RCS/vuidebug.h,v 1.1 1995/08/02 18:46:45 susan Stab74 $ */

#ifndef DEBUG
#define debug(xxx_foo)
#endif /* DEBUG */
#ifdef DEBUG
#define debug(xxx_foo) debugrtn xxx_foo
#endif /* DEBUG */
