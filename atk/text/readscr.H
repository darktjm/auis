ATK_INTER
#ifndef _readscr_H_
#define _readscr_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <ATK.H>

class readscr : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();



    readscr();
	static class text * Begin(class text *t, int pos, int len, int purge, const char *version, int GetTemplate)  ;
    static int PrintFile(char *filename, class textview *tv, class text *t, const char *version, int TrashWhenDone)  ;

};

#endif /* _readscr_H_ */
