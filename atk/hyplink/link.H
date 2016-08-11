ATK_INTER
#ifndef _link_H_
#define _link_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	Copyright Carnegie Mellon, 1996
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



#define link linkclass

enum link_type {
	NoLink,		// 'E' goes nowhere
	FileLink,		// 'F' link to a file  (default for old style)
	RelLink,		// 'R' link to a file in a relative directory
	URLLink,		// 'U' link to a URL (name may have  ${...})
	VarFileLink,	// 'V' link to a file (name contains ${...})
};

#include <pushbutton.H>
class link: public  pushbutton {
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();

	link();
	virtual ~link();
    
	long Read (FILE *file, long id)  ;
	long Write (FILE *file, long writeid, int level)  ;
    
	virtual void SetLink(char *link);  // FileLink
	virtual void SetLink(enum link_type code, char *link);
	virtual void SetPos(long pos);
	virtual void SetLen(long len);
	virtual char * GetResolvedLink(view *viewer = NULL);
    
	inline char  * GetRawLink() { return (this->rawlink); }
	inline char * GetLink() 	/* compatibility with old code */
		{ return ((this)->GetResolvedLink()) ; }
	inline long GetPos() { return (this->pos); }
	inline long GetLen() { return (this->len); }
	inline int LabelSetP() { return (this->label_set); }
	inline enum link_type GetTypeCode() { return typecode; }
	inline void SetTypeCode(enum link_type code) 
		{ typecode = code; }
	
    
	char *rawlink;
	enum link_type typecode;
	long pos;
	long len;
	int label_set;
#ifdef PL8
	int new_ds;
#endif
};


#endif /* _link_H_ */
