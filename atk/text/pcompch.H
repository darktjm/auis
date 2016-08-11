ATK_INTER
#ifndef _pcompch_H_
#define _pcompch_H_
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
    static boolean ReadCompositesFile(char *file)  ;
    static void ClearComposites();
    static void DeleteComposite(char key,char *exts);
    static long EnumerateComposites(char key, pcompch_ecfptr func,long rock)  ;
    static char * CharacterToTroff( char ch, class environment *env,  class textview *tv)  ;
    static  char * StringToTroff( char *str,char *buf, long bufsize, class environment *env,class textview *tv)  ;
};

#endif /* _pcompch_H_ */
