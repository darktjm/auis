#ifndef _dataobject_H_
#define _dataobject_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <atom.H>
#include <namespace.H>
#include <stdio.h>

struct property {
    const class atom * type;
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
    virtual const char * ViewName()  ;
    virtual void SetAttributes(struct attributes *attributes);
    virtual void Put( const class atom * property, const class atom * type, long rock );
    virtual short Get( const class atom * property, const class atom ** type, long * rock )
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
    class Namespace * properties;
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
/** @} */
#endif /* _dataobject_H_ */
