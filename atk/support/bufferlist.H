ATK_INTER
#ifndef _bufferlist_H_
#define _bufferlist_H_
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


 

#include <observable.H>

typedef boolean (*bufferlist_efptr)(class buffer *b, long d);
typedef boolean (*bufferlist_sfptr)(class buffer *a, class buffer *b);

class bufferlist : public  observable {
public:
	virtual ATKregistryEntry *ATKregistry();
	 void ObservedChanged (class observable *changed, long value);
    
        virtual void AddBuffer(class buffer *buffer);
        virtual void RemoveBuffer(class buffer *buffer);
        virtual class buffer * Enumerate(bufferlist_efptr mapFunction, long functionData)  ;
        virtual class buffer * CreateBuffer(char *bufferName, char *fileName, char *objetName, class dataobject *data)  ;
	
/* Lookup functions */
        virtual class buffer * FindBufferByName(char *bufferName)  ;
        virtual class buffer * FindBufferByFile(char *filename)  ;
        virtual class buffer * FindBufferByData(class dataobject *data)  ;

/* File functions. */
        virtual class buffer * GetBufferOnFile(char *filename, long flags)  ;
        virtual void GuessBufferName( char *filename, char *bufferName, int nameSize);
        virtual void GetUniqueBufferName( char *proposedBufferName, char *bufferName, int nameSize);
        virtual void SetDefaultObject(char *objectname);

/* Sorting the buffer list. */
	virtual void Sort(bufferlist_sfptr compareRoutine);
    
        
	virtual ~bufferlist();
        
	bufferlist();
    
	struct listentry *head;
};


#endif /* _bufferlist_H_ */
