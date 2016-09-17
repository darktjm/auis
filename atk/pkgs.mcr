/* ********************************************************************** *\
 *	Copyright Carnegie Mellon, 1996
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
# Permission to use, copy, modify, and distribute this software and its 
# documentation for any purpose and without fee is hereby granted, provided 
# that the above copyright notice appear in all copies and that both that 
# copyright notice and this permission notice appear in supporting 
# documentation, and that the name of IBM not be used in advertising or 
# publicity pertaining to distribution of the software without specific, 
# written prior permission. 
#                         
# THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
# TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
# HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
# 
#  $
*/

/*
	This file causes to be defined these make varaibles:

	ATKSUBDIRS    -  which atk/ edirectories to buiild
	ATKREGISTERLIBS -  which libraries to register
	ATKRUNAPPLIBS - which libraries to include in runapp

*/

#undef DIRLIBS
#define DIRLIBS(name, directory, libraries)  @@\
Concat(DIR_,name) = directory @@\
Concat(LIB_,name) = libraries


/* ================================= */
/* build the fundamental directories */
/* ================================= */

DIRLIBS(DOC,doc,)
DIRLIBS(ATKVERS,atkvers,)
DIRLIBS(BASICS,basics,basics)
DIRLIBS(EXTENSIONS,extensions,compile \
	deskey dynsearch ezdiff filter gsearch \
	incsearch metax spell tags recsearch)
DIRLIBS(FRAME,frame,frame)
DIRLIBS(SUPPORT,support,support)
DIRLIBS(SUPPORTVIEWS,supportviews,supviews)
DIRLIBS(SYNTAX,syntax,tlex sym parser)
DIRLIBS(UTILS,utils,atkutils)

/* ================================= */
/* build the fundamental insets */
/* ================================= */


DIRLIBS(ADEW,adew,arbiter)
DIRLIBS(EZ,ez,ez)
DIRLIBS(HYPLINK,hyplink,link)
DIRLIBS(RASTER,raster,raster)
DIRLIBS(TEXT,text,text)
DIRLIBS(TEXTAUX,textaux,textaux)
DIRLIBS(TEXTOBJECTS,textobjects,textobjs)
DIRLIBS(VALUE,value,value)

DIRLIBS(APPS,apps,)

/* ================================= */
/* turn directories on in groups */
/* ================================= */

#ifdef 				MK_BLD_BLKS
#ifndef MK_APT
#define MK_APT
#endif
#ifndef MK_ORG
#define MK_ORG
#endif
#ifndef MK_BUSH
#define MK_BUSH
#endif
#ifndef MK_FIGURE
#define MK_FIGURE
#endif
#ifndef MK_CHART
#define MK_CHART
#endif
#endif				 /* MK_BLD_BLKS */

#ifdef 				MK_BASIC_UTILS
#ifndef MK_CONSOLE
#define MK_CONSOLE
#endif
#ifndef MK_EZPRINT
#define MK_EZPRINT
#endif
#ifndef MK_LAUNCHAPP
#define MK_LAUNCHAPP
#endif
#ifndef MK_TYPESCRIPT
#define MK_TYPESCRIPT
#endif
#endif 				/* MK_BASIC_UTILS */

#ifdef				 MK_BASIC_INSETS
#ifndef MK_EQ
#define MK_EQ
#endif
#ifndef MK_FAD
#define MK_FAD
#endif
#ifndef MK_TABLE
#define MK_TABLE
#endif
#ifndef MK_IMAGE
#define MK_IMAGE
#endif
#ifndef MK_WIDGETS
#define MK_WIDGETS
#endif
#endif				/* MK_BASIC_INSETS */

#ifdef 				MK_HELP_APP
#ifndef MK_HELP
#define MK_HELP
#endif
#ifndef MK_ROFFTEXT
#define MK_ROFFTEXT
#endif
#endif 				/* MK_HELP */

#ifdef 				MK_TEXT_EXT
#ifndef MK_LOOKZ
#define MK_LOOKZ
#endif
#ifndef MK_MIT_TOOLS
#define MK_MIT_TOOLS
#endif
#ifndef MK_SRCTEXT
#define MK_SRCTEXT
#endif
#endif 				/* MK_TEXT_EXT */

#ifdef 				MK_AUTHORING
#ifndef MK_LAYOUT
#define MK_LAYOUT
#endif
#ifndef MK_CONTROLLERS
#define MK_CONTROLLERS
#endif
#ifndef MK_NESS
#define MK_NESS
#endif
#ifndef MK_CREATEINSET
#define MK_CREATEINSET
#endif
#endif 				/* MK_AUTHORING */

#ifdef 				MK_AUX_UTILS
#ifndef MK_DATACAT
#define MK_DATACAT
#endif
#ifndef MK_PREFS
#define MK_PREFS
#endif
#ifndef MK_TOEZ
#define MK_TOEZ
#endif
#endif 				/* MK_AUX_UTILS */


/* not listed above, but definable:
	MK_EXAMPLES
	MK_HTML
*/

/* ================================= */
/*  enforce required builds			*/
/* ================================= */

#if !defined(MK_APT) && (defined(MK_BUSH) || defined(MK_APT) || defined(MK_ZIP))
#define MK_APT
#endif
#if defined(MK_HELP) && !defined(MK_ROFFTEXT)
#define MK_ROFFTEXT
#endif
#if defined(MK_PREFS) && !defined(MK_DATACAT)
#define MK_DATACAT
#endif
#if SY_OS2 && defined(MK_CONSOLE)
#undef MK_CONSOLE
#endif


/* ================================= */
/*  define vars for dirs and libs			*/
/* ================================= */

#ifdef MK_APT
DIRLIBS(APT,apt,apt suite tree)
#endif
#ifdef MK_BUSH
DIRLIBS(BUSH,bush,bush)
#endif
#ifdef MK_CHART
DIRLIBS(CHART,chart,chart)
#endif
#ifdef MK_CONTROLLERS
DIRLIBS(CONTROLLERS,controllers,pcontrol)
#endif
#ifdef MK_CONSOLE
DIRLIBS(CONSOLE,console,consoleapp)
#endif
#ifdef MK_CREATEINSET
DIRLIBS(CREATEINSET,createinset,)
#endif
#ifdef MK_DATACAT
DIRLIBS(DATACAT,datacat,datacata)
#endif
#ifdef MK_EQ
DIRLIBS(EQ,eq,eq)
#endif
#ifdef MK_EXAMPLES
DIRLIBS(EXAMPLES,examples,)
#endif
#ifdef MK_EZPRINT
DIRLIBS(EZPRINT,ezprint,ezprintapp)
#endif
#ifdef MK_FAD
DIRLIBS(FAD,fad,fad)
#endif
#ifdef MK_FIGURE
DIRLIBS(FIGURE,figure,figure)
#endif
#ifdef MK_HELP
DIRLIBS(HELP,help,help)
#endif
#ifdef MK_HTML
DIRLIBS(WEB,web,htmltext)
#endif
#ifdef MK_IMAGE
DIRLIBS(IMAGE,image,imagev)
#endif
#ifdef MK_LAUNCHAPP
DIRLIBS(LAUNCHAPP,launchapp,launchapp)
#endif
#ifdef MK_LAYOUT
DIRLIBS(LAYOUT,layout,layout)
#endif
#ifdef MK_LOOKZ
DIRLIBS(LOOKZ,lookz,lookz)
#endif
#ifdef MK_MIT_TOOLS
DIRLIBS(MIT_TOOLS,mit,note)
#endif
#ifdef MK_NESS
DIRLIBS(NESS,ness,ness)
#endif
#ifdef MK_ORG
DIRLIBS(ORG,org,org)
#endif
#ifdef MK_PREFS
DIRLIBS(PREFED,prefed,prefs)
#endif
#ifdef MK_ROFFTEXT
DIRLIBS(ROFFTEXT,rofftext,rofftext)
#endif
#ifdef MK_SRCTEXT
DIRLIBS(SRCTEXT,srctext,srctext)
#endif
#ifdef MK_TABLE
DIRLIBS(TABLE,table,table)
#endif
#ifdef MK_TOEZ
DIRLIBS(TOEZ,toez,toezapp)
#endif
#ifdef MK_TYPESCRIPT
DIRLIBS(TYPESCRIPT,typescript,typescript)
#endif
#ifdef MK_WIDGETS
DIRLIBS(WIDGETS,widgets,awidget)
#endif


/* ================================= */
/*  define the lists	
/* order is impostant in ATKSUBDIRS
/* ================================= */

ATKSUBDIRS = \
 $(DIR_BASICS)  $(DIR_SUPPORT)  $(DIR_SUPPORTVIEWS)  \
 $(DIR_TEXT)  $(DIR_UTILS)  $(DIR_FRAME)  \
 $(DIR_TEXTAUX)  $(DIR_ATKVERS)  $(DIR_EZ)  \
 $(DIR_EXTENSIONS)  $(DIR_TYPESCRIPT)  $(DIR_HYPLINK)  \
 $(DIR_VALUE)  $(DIR_TEXTOBJECTS)  $(DIR_ADEW)  \
 $(DIR_RASTER)  $(DIR_SYNTAX)  \
 \
 $(DIR_APT)   $(DIR_IMAGE)   $(DIR_ORG)   $(DIR_BUSH)  \
 $(DIR_CHART)   $(DIR_CONSOLE)   $(DIR_EZPRINT)  \
 $(DIR_EQ)   $(DIR_FAD)   $(DIR_TABLE)   $(DIR_ROFFTEXT)  \
 $(DIR_HELP)   $(DIR_SRCTEXT)   $(DIR_LOOKZ)  \
 $(DIR_LAYOUT)   $(DIR_CONTROLLERS)  \
 $(DIR_CREATEINSET)  \
 $(DIR_FIGURE)   $(DIR_NESS)   $(DIR_WIDGETS)  \
 $(DIR_DATACAT)   $(DIR_TOEZ)   $(DIR_WEB)  \
 $(DIR_EXAMPLES)  $(DIR_LAUNCHAPP)   $(DIR_PREFED)  \
 $(DIR_MIT_TOOLS) $(DIR_APPS)  $(DIR_DOC)


/* these are ATKLibrary-s that need to be in runapp */
ATKFOUNDATION  =  \
    $(LIB_BASICS)  \
    $(LIB_EZ)  \
    $(LIB_FIGURE)  \
    $(LIB_FRAME)  \
    $(LIB_SUPPORT)  \
    $(LIB_SUPPORTVIEWS)  \
    $(LIB_TEXT)  \
    $(LIB_TEXTAUX)  \
    $(LIB_TEXTOBJECTS)  \
    $(LIB_UTILS)  \
    $(LIB_WIDGETS)

/* These are created with Dynamic[Multi]ObjectWithExports
	and can be linked against.
	They need to be in runapp if the platform does not have 
	interlinking  (or does not have dynamic loading at all)  
*/
ATKLINKABLEINSETS  = \
    $(LIB_ADEW)  \
    $(LIB_APT)  \
    $(LIB_CONTROLLERS)  \
    $(LIB_DATACAT)  \
    $(LIB_HYPLINK)  \
    $(LIB_IMAGE)  \
    $(LIB_LAYOUT)  \
    $(LIB_LOOKZ)  \
    $(LIB_NESS)  \
    $(LIB_ORG)  \
    $(LIB_RASTER)  \
    $(LIB_ROFFTEXT)  \
    $(LIB_SRCTEXT)  \
    $(LIB_SYNTAX)  \
    $(LIB_VALUE)

/* These are Dynamic[Multi]Objects and only need to be
	in runapp if the platform does not have dynamic loading 
*/
ATKLOADABLEINSETS = \
    $(LIB_BUSH)  \
    $(LIB_CHART)  \
    $(LIB_CONSOLE)  \
    $(LIB_EQ)  \
    $(LIB_EXTENSIONS)  \
    $(LIB_EZPRINT)  \
    $(LIB_FAD)  \
    $(LIB_HELP)  \
    $(LIB_LAUNCHAPP)  \
    $(LIB_MIT_TOOLS) \
    $(LIB_MITUTIL) \
    $(LIB_PREFED)  \
    $(LIB_TABLE)  \
    $(LIB_TOEZ)  \
    $(LIB_TYPESCRIPT)  \
    $(LIB_WEB)

ATKREGISTERLIBS = $(ATKLOADABLEINSETS) $(ATKLINKABLEINSETS) $(ATKFOUNDATION)

#ifdef HAVE_DYNAMIC_INTERLINKING
ATKRUNAPPLIBS =  $(ATKFOUNDATION)
#else
#ifdef HAVE_DYNAMIC_LOADING
ATKRUNAPPLIBS = $(ATKLINKABLEINSETS) $(ATKFOUNDATION)
#else
ATKRUNAPPLIBS = $(ATKLOADABLEINSETS) $(ATKLINKABLEINSETS) $(ATKFOUNDATION)
#endif
#endif

