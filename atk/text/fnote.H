ATK_INTER
#ifndef _fnote_H_
#define _fnote_H_
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




#include <text.H>

class fnote: public  text {
public:
	virtual ATKregistryEntry *ATKregistry();


	static void CloseAll(class text *text);
	static int UpdateAll(class text *text,long count)  ;
	static int CopyAll(class text *text,class text *desttext,long count,boolean number)  ;
	static void OpenAll(class text *text);
	
	fnote();
	
	static boolean InitializeClass();
 

	virtual void Close(class text *text);
	virtual long CopyNote(class text *text,class text * desttext,long count,boolean number)  ;
	virtual void Open(class text *text);
	virtual boolean IsOpen()  ;
	virtual void addenv(class text *txt,long pos);
	virtual long GetLocLength()  ;

	  long Read (FILE *file, long id)  ;
	 const char * ViewName()  ;

    inline long GetLoc() { return ((this)->loc); }
    inline void SetLoc(long newValue) { ( ((this)->loc) = (newValue) ); }
    inline long GetOwnLoc() { return ((this)->ownloc); }
    inline void SetOwnLoc(long newValue) { ( ((this)->ownloc) = (newValue) ); }
    inline class text * GetParenttxt() { return ((this)->parenttext); }
    inline class environment * GetEnv() { return ((this)->env); }
    inline void SetEnv(class environment *newValue) { ( ((this)->env) = (newValue) ); }

   long loc,ownloc,notecount;
   class text *parenttext;
   class environment *env;
   class style *vstyle , *hstyle;
   boolean open;
};


#endif /* _fnote_H_ */
