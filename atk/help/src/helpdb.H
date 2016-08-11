ATK_INTER
#ifndef _helpdb_H_
#define _helpdb_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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

/* $ACIS$ */

 

/*---------------------------------------------------------------------------*/
/*	MODULE: helpdb.c						     */
/*		Package of routines to do the index-related lookups and	     */
/*		matching algorithms.					     */
/*---------------------------------------------------------------------------*/

#include <index.h>
#include <ATK.H>
typedef void (*helpdb_efptr)(char *alias, char *orig, char *ptr);
class helpdb : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        
	static boolean InitializeClass();

	static int CheckIndex(class view *v)  ;
	static int SetIndex(char *anindex)  ;
	static int SetupHelp(struct cache *c, char *aname, int strip)  ;
	static struct helpDir * GetHelpDirs()  ;
	static void AddSearchDir(char *dirname);
	static void ReadAliasesFile(char *aname);
	static char * MapAlias(char *name)  ;
	static void Enumerate(index_efptr proc, char *self);
	static void EnumerateAll(helpdb_efptr proc, char *rock);
	static void PrintSearchDirs();
};

#endif /* _helpdb_H_ */