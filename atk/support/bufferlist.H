#ifndef _bufferlist_H_
#define _bufferlist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupport
 * @{ */
#include <observable.H>

typedef boolean (*bufferlist_efptr)(class buffer *b, long d);
typedef boolean (*bufferlist_sfptr)(class buffer *a, class buffer *b);

class bufferlist : public  observable {
public:
	TRACED_CLASS(bufferlist);
	 void ObservedChanged (class observable *changed, long value);
    
        virtual void AddBuffer(class buffer *buffer);
        virtual void RemoveBuffer(class buffer *buffer);
        virtual class buffer * Enumerate(bufferlist_efptr mapFunction, long functionData)  ;
        virtual class buffer * CreateBuffer(const char *bufferName, const char *fileName, const char *objetName, class dataobject *data)  ;
	
/* Lookup functions */
        virtual class buffer * FindBufferByName(const char *bufferName)  ;
        virtual class buffer * FindBufferByFile(const char *filename)  ;
        virtual class buffer * FindBufferByData(class dataobject *data)  ;

/* File functions. */
        virtual class buffer * GetBufferOnFile(const char *filename, long flags)  ;
        virtual void GuessBufferName( const char *filename, char *bufferName, int nameSize);
        virtual void GetUniqueBufferName( const char *proposedBufferName, char *bufferName, int nameSize);
        virtual void SetDefaultObject(const char *objectname);

/* Sorting the buffer list. */
	virtual void Sort(bufferlist_sfptr compareRoutine);
    
        
        
	bufferlist();
    
	struct listentry *head;
};

/** @} */
#endif /* _bufferlist_H_ */
