ATK_INTER
#ifndef _pcompch_H_
#define _pcompch_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define MAXCHAR 127
#define BADCHAR MAXCHAR+1

struct composites_s {
     char exts[16],style[32];
     short code;
    struct composites_s *next;
};

#include <ATK.H>
	typedef long (*pcompch_ecfptr)(char key, struct composites_s *comps, long rock);
	typedef long (*pcompch_asciifptr)(class text *t,long pos,long j,char *magic,long rock);
	typedef long (*pcompch_atkfptr)(class text *t,long pos,long j, char *magic,long rock);

class pcompch : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();
	
    static void ATKToASCII(class text *text,long pos, long len, pcompch_asciifptr func, long rock);
    static void ASCIIToATK(class text *text,long pos, long len, pcompch_atkfptr func, long rock);
    static boolean ReadCompositesFile(const char *file)  ;
    static void ClearComposites();
    static void DeleteComposite(char key,char *exts);
    static long EnumerateComposites(char key, pcompch_ecfptr func,long rock)  ;
    static char * CharacterToTroff( char ch, class environment *env,  class textview *tv)  ;
    static  char * StringToTroff( char *str,char *buf, long bufsize, class environment *env,class textview *tv)  ;
};

#endif /* _pcompch_H_ */
