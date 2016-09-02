/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/* 
 * Control inclusion of debugging code in object code
 */

#ifndef DEBUG
#define debug(xxx_foo)
#endif /* DEBUG */
#ifdef DEBUG
#define debug(xxx_foo) debugrtn xxx_foo
#endif /* DEBUG */
