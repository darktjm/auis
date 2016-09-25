/* 
 *	Copyright Carnegie Mellon University, 1996 - All Rights Reserved
 *	For full copyright information see:'andrew/doc/COPYRITE' 
*/
/** \addtogroup config
 * @{ */
/* template for site.h

	To install Andrew, copy this file to 
		ANDREWSRC/config/site.h 
	and modify it appropriately for your site.

*/


/* Andrew execution directory   =============== 
	The directory path by which users will execute Andrew.
	Some Andrew software uses this value to find pieces of the
	installed software.  Usually this is the tree into which
	Andrew is installed;  if not, set BASEDIR in site.mcr.

	To use Andrew, users should have xxx/bin in their path, 
	where xxx is the value of DEFAULT_ANDREWDIR_ENV
*/
#define DEFAULT_ANDREWDIR_ENV /usr/local/auis


/* AndrewSetup   =============== 
	If files cannot be installed in /etc/AndrewSetup, 
	the path to AndrewSetup can be specified here.  
	In any case, Andrew will use the first AndrewSetup file 
	it finds in looking through
		/AndrewSetup
		/etc/AndrewSetup
		$(LOCAL_ANDREW_SETUP_ENV)
		/usr/vice/etc/AndrewSetup
		$(DEFAULT_ANDREWDIR_ENV)/etc/AndrewSetup
		/usr/andrew/etc/AndrewSetup
*/
/*
#define LOCAL_ANDREW_SETUP_ENV xxx-pathname-xxx
*/


/* print_PSCPRINTCOMMAND   =============== 
	specifies the command to use 
	to print a file.  The file will be piped into this command.  
	In addition, any instances of %s in the command will be 
	replaced with the name of the file that is to be printed.
*/
#define print_PSCPRINTCOMMAND "lpr -T%s"



/* Define sections of AUIS to build =============== */

/* here you could #undef any of the MK_ variables that control
	the builds in $/atk/Imakefile.  See INSTALL. */

/* If you picked up contrib.tar.Z and you wish to build and install
	almost all of it, then uncomment the following: */
#define MK_CONTRIB 1  
#define CONTRIB_ENV 1
#define MK_CALC 1
#define MK_CHAMP 1
#define MK_SCRIBETEXT 1
#define MK_RTF 1
#define MK_VUI 1
#define MK_ZIP 1
#define MK_PREVIEW 1
#define MK_HELP 1
#define MK_CONTRIB_HTML 1
#define MK_STROFFET 1
#define MK_GTEXT 1
#define MK_DOXYDOC 1

/* If you are programming in Andrew, you may want:
	Examples: a set of program examples.
	Source indices: doc/atk/FilesList and doc/atk/FunctionIndex 
*/
#define MK_EXAMPLES 1
#define FUNCTION_INDEX_ENV 1 

#define MEGARUNAPP_ENV 1


/* Site specific sources   =============== */
/* If your site wishes to include sources as part of the build
	you can add them as subdirectories to $/site
	To build the site directory, uncomment the lines below. */
/*
#ifndef SITE_ENV
#define SITE_ENV 1
#endif
*/
/** @} */
