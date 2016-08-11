ATK_INTER
#ifndef _dataobject_H_
#define _dataobject_H_
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


 


#include <atom.H>
#include <namespace.H>

struct property {
    class atom * type;
    long data;
};

/* "Other Format" codes -- so far only mail */
#define dataobject_OTHERFORMAT_MAIL 1

/* Encoding codes for 7 bit transport */
#define dataobject_ENCODING_BASE64 1
#define dataobject_ENCODING_QUOTEDPRINTABLE 2

#include <observable.H>

class dataobject: public  observable {
public:
	virtual ATKregistryEntry *ATKregistry();
	dataobject();
	virtual ~dataobject();


    virtual long Read (FILE *file, long id)  ;
    virtual long Write (FILE *file, long writeid, int level)  ;
    virtual boolean ReadOtherFormat(FILE *file, char *fmt, char *encoding, char *description)  ;
    virtual long WriteOtherFormat(FILE *file, long writeid, int level, int usagetype, char *boundary)  ;
    virtual long GetModified()  ;
    virtual void SetModified();
    virtual char * ViewName()  ;
    virtual void SetAttributes(struct attributes *attributes);
    virtual void Put( class atom * property, class atom * type, long rock );
    virtual short Get( class atom * property, class atom ** type, long * rock )
       ;
    virtual int ListCurrentViews(class view **array,int size)  ;
    virtual void RestoreModified(long oldmodified);

    inline long UniqueID() { return ((long)(this)); }
    inline long GetWriteID() { return ( (this)->writeID ); }
    inline void SetWriteID(long newValue) { ( ((this)->writeID) = (newValue) ); }
    inline long GetID()      { return ( (this)->id ); }
    inline void SetID(long newValue)      { ( ((this)->id) = (newValue) ); }

    long id;
    long writeID;
    long modified;
    class namespace * properties;
};

#define dataobject_UNDEFINEDID -1

/* return values from Read */
#define	dataobject_NOREADERROR	0
#define	dataobject_PREMATUREEOF	1
#define	dataobject_NOTBE2DATASTREAM 2 /* backward compatibility */
#define	dataobject_NOTATKDATASTREAM 2 /* preferred version */
#define dataobject_MISSINGENDDATAMARKER 3
#define	dataobject_OBJECTCREATIONFAILED	4
#define dataobject_BADFORMAT 5

#endif /* _dataobject_H_ */