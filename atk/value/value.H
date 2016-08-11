ATK_INTER
#ifndef _value_H_
#define _value_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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


 


#define value_ROCK1 1
#define value_ROCK2 2 
#define value_STRING 3
#define value_STRINGARRAY 4

/* Values for Notify Observers */
#define value_NOCHANGE		0
#define value_BADVALUE		1
#define value_NEWVALUE		2
#define	value_NOTACTIVE		3
#define value_ACTIVE		4


#include <dataobject.H>
typedef void (*value_fptr)(ATK *observer, class value *v, long orock, long rock);
struct observer
{
  ATK  * observer;
  value_fptr callBack;
  long rock;
};

class value : public  dataobject  {
public:
	virtual ATKregistryEntry *ATKregistry();
 /* really it would be nice if these two were in the opposite relationship */
 
   void NotifyObservers( long rock );
   long Write(FILE *file, long writeID, int level)  ;
   long Read (FILE *file, long id)  ;
   char * ViewName()  ;

  virtual void AddCallBackObserver( ATK  * observer,
	      value_fptr callBack, long rock );
  virtual void RemoveCallBackObserver( ATK  * observer );
  virtual void RemoveCallBack( ATK  * observer,
		 value_fptr callBack );
  virtual void SetValueType(long rock,int type);
  virtual void SetStrArrayAndSize(char **rock,long size);

  inline void SetValue(long rock) { ((this)->SetValueType((long)rock,value_ROCK1)); }
  inline void SetArraySize(long rock) { ((this)->SetValueType((long)rock,value_ROCK2)); }
  inline void SetString(char *rock) { ((this)->SetValueType((long)rock,value_STRING)); }
  inline void SetStringArray(char **rock) { ((this)->SetValueType((long)rock,value_STRINGARRAY)); }
  inline long GetValue() { return (this->rock1); }
  inline long GetArraySize() { return (this->rock2); }
  inline char * GetString()  { return (this->string); }
  inline char ** GetStringArray()  { return (this->stringarray); }
  inline void SetNotify(boolean val) { ((this)->notify = val); }
  inline long GetUpdateCount() { return ((this)->updatecount); }


  
	static boolean InitializeClass();


  
	value();
  
	virtual ~value();

  short maxObservers;			/* number of entries in observers table */
  struct observer *observers;	/* table of observers */
  long rock1,rock2;
  char *string, **stringarray;
  boolean notify;
  long updatecount;
};


#define value_OBJECTDESTROYED -1


/* 
  the call back is invoked
  callBack( self, observed, observed rock, observers rock );
*/

#endif /* _value_H_ */