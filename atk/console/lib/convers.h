/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* IMPORTANT  -- IF YOU CHANGE THE VERSION NUMBER BELOW, YOU NOW ALSO HAVE TO CHANGE IT IN ../cmd/consolea.c, in the InitializeObject procedure.  */
#define CONSOLE_VERSION " [7.6]" 
/* Don't forget leading space in version number*/
/* version information is contained in the next two defines, 
 * bump the minor version when we get significant bug fixes,
 * major version changes require negotiation- whoever is in charge of version control
 * should choose this in accordance with the version chosen for the system. as a whole
 * NOTE: The way this is written, you also need to make lib/convers.h conform to 
 * these numbers explicitly.
 */
#define MINORVERSION 6
#define MAJORVERSION 7
