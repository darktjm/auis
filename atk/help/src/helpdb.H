ATK_INTER
#ifndef _helpdb_H_
#define _helpdb_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*---------------------------------------------------------------------------*/
/*	MODULE: helpdb.c						     */
/*		Package of routines to do the index-related lookups and	     */
/*		matching algorithms.					     */
/*---------------------------------------------------------------------------*/

/** \addtogroup libhelp
 * @{ */
#include <index.h>
#include <ATK.H>
typedef void (*helpdb_efptr)(char *alias, char *orig, char *ptr);
class helpdb : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        
	static boolean InitializeClass();

	static int CheckIndex(class view *v)  ;
	static int SetIndex(const char *anindex)  ;
	static int SetupHelp(struct cache *c, const char *aname, int strip)  ;
	static struct helpDir * GetHelpDirs()  ;
	static void AddSearchDir(const char *dirname);
	static void ReadAliasesFile(const char *aname);
	static const char * MapAlias(const char *name)  ;
	static void Enumerate(index_efptr proc, char *self);
	static void EnumerateAll(helpdb_efptr proc, char *rock);
	static void PrintSearchDirs();
};
/** @} */
#endif /* _helpdb_H_ */
