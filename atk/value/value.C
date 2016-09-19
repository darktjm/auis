/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("value.H")
#include <value.H>
#include <atom.H>
#include <dataobject.H>
#include <observable.H>
#include <view.H>
#define INITIALNUMOBSERVERS 4

static class atom *rock1atom, *rock2atom, *stringatom, *stringarrayatom, *valueatom;

/****************************************************************/
/*		static support					*/
/****************************************************************/

ATKdefineRegistry(value, dataobject, value::InitializeClass);
static short FindObserverCallBack(class value  * self, ATK   *observer, value_fptr  callBack);
static short FindObserver( class value  * self, ATK   *observer );
static short FreeSlot( class value  * self );


static short FindObserverCallBack(class value  * self, ATK   *observer, value_fptr  callBack)
               {
  short i = 0;

  for (i = 0; i < self->maxObservers; ++i)
    if (self->observers[i].observer == observer
	&& self->observers[i].callBack == callBack)
      return i;
  return -1;
}


static short FindObserver( class value  * self, ATK   *observer )
          {
  short i = 0;

  for (i = 0; i < self->maxObservers; ++i)
    if (self->observers[i].observer == observer)
      return i;
  return -1;
}


static short FreeSlot( class value  * self )
     {
  short i,j;
  
  for (i = 0; i < self->maxObservers; ++i)
    if (self->observers[i].observer == NULL)
      return i;

  if (self->maxObservers == 0)
    {
      self->maxObservers = INITIALNUMOBSERVERS;
      self->observers = (struct observer *) malloc (INITIALNUMOBSERVERS * sizeof (struct observer));
    }
  else
    {
      self->maxObservers += self->maxObservers / 2;
      self->observers = (struct observer *)
	realloc(self->observers, self->maxObservers * sizeof (struct observer));
    }
  j = i;
  while (i < self->maxObservers)
    self->observers[i++].observer = NULL;
  return j;
}


/****************************************************************/
/*		class procedures 				*/
/****************************************************************/
value::value()
     {
	ATKinit;

  this->maxObservers = 0;
  this->updatecount = 0L;
  this->observers = NULL;
  this->rock1 = this->rock2 = 0l;
  (this)->Put(valueatom,rock1atom,this->rock1);
  this->string = NULL;
  this->stringarray = NULL;
  this->notify = TRUE;
  THROWONFAILURE( TRUE);
}

value::~value()
{
	ATKinit;

  (this)->NotifyObservers( value_OBJECTDESTROYED);
  if (this->observers){
    free(this->observers);
    this->observers = NULL;
  }
}

/****************************************************************/
/*		instance methods				*/
/****************************************************************/


void value::AddCallBackObserver( ATK   * observer, value_fptr  callBack, long  rock )
                    {
  short free;

  if (FindObserverCallBack(this, observer, callBack) == -1)
    {
      free = FreeSlot(this);
      this->observers[free].observer = observer;
      this->observers[free].callBack = callBack;
      this->observers[free].rock = rock;
    }
}


void value::RemoveCallBack( ATK   * observer, value_fptr  callBack )
               {
  short i;

  if ((i = FindObserverCallBack(this, observer, callBack)) != -1)
    {
      this->observers[i].observer = NULL;
    }
}
     

void value::RemoveCallBackObserver( ATK   * observer )
          {
  short i;

  while ((i = FindObserver( this, observer )) != -1)
    this->observers[i].observer = NULL;
}


void value::NotifyObservers( long  rock )
          {
  short i;
  (this)->dataobject::NotifyObservers(  rock );
  if(this->observers){
      for (i = 0; i < this->maxObservers; ++i)
	  if (this->observers[i].observer != NULL)
	      (*(this->observers[i].callBack))(this->observers[i].observer, this,
					       this->observers[i].rock, rock);
  }
}




long value::Write(FILE  *file, long  writeID, int  level)
                {   /* NOTE : only the value (rock1)is saved */
    if ((this)->GetWriteID() != writeID)  {
	(this)->SetWriteID( writeID);
        fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
	fprintf(file,">%ld\n",(this)->GetValue());
        fprintf(file, "\\enddata{%s,%ld}\n",  (this)->GetTypeName(),(this)->GetID());
    }

    return (this)->GetID();
}
long value::Read(FILE  *file, long  id)
{
    char buf[256];
    while(1){
	if(fgets(buf,256,file) == NULL) return dataobject_PREMATUREEOF;
	if(*buf == '>') {
	    this->rock1 = atoi(buf + 1);
	    (this)->Put(valueatom,rock1atom,this->rock1);
	}
	else if(strncmp(buf,"\\enddata",8) == 0)return dataobject_NOREADERROR;
    }
    
}
	
void value::SetValueType(long  rock,int  type)
{
    
    switch(type){
	case  value_ROCK1:
	    this->rock1 = rock;
	    
	    (this)->Put(valueatom,rock1atom,rock);
	    
	    break;
	case  value_ROCK2:
	    this->rock2 = rock;
	    
	    (this)->Put(valueatom,rock2atom,rock);
	    
	    break;
	case  value_STRING:
	    this->string = (const char *) rock;
	    
	    (this)->Put(valueatom,stringatom,rock);
	    
	    break;
	case  value_STRINGARRAY:
	    this->stringarray = (const char * const *) rock;
	    
	    (this)->Put(valueatom,stringarrayatom,rock);
	    
	    break;
    }
    (this->updatecount)++;
    if(this->notify) (this)->NotifyObservers(value_NEWVALUE);
}
const char *value::ViewName()
    {
    return "buttonV";
}
void value::SetStrArrayAndSize(char  **rock,long  size)
{
    this->stringarray = (char **) rock;
    this->rock2 = size;
    if(this->notify) (this)->NotifyObservers(value_NEWVALUE);
}

boolean value::InitializeClass()
{
    valueatom=atom::Intern("value");
    rock1atom=atom::Intern("rock1");
    rock2atom=atom::Intern("rock2");
    stringatom=atom::Intern("string");
    stringarrayatom=atom::Intern("stringarray");
    if(rock1atom &&  rock2atom && stringatom && stringarrayatom) return TRUE;
    else return FALSE;
}
