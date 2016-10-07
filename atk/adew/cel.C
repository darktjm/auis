/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("cel.H")

#include <ctype.h>

#include <dataobject.H>
#include <dictionary.H>
#include <proctable.H>
#include <environment.H>
#include <text.H>
#include <filetype.H>
#include <observable.H>
#include <arbiter.H>
#include <cel.H>

#define VALUE 10
static long viewID = 0;



ATKdefineRegistry(cel, dataobject, cel::InitializeClass);
static void SetVisible(class cel  *self);
static void SetInvisible(class cel  *self);
static const class atom *getline(char  **place);
static long                      searchatt(class cel  *self,char  *attname,long  *len);
void cel__FinializeObject(struct classheader  *classID, class cel  *self);
#if 0
void cel__ClearChain(class cel  *self);
#endif


void cel::ObservedChanged(class observable  *changed, long  value)
{
    if(changed==(class	observable *)this->dataObject) {
	
	if(value==observable::OBJECTDESTROYED) {
	    this->dataObject=NULL;
	    (this)->Destroy();
	} else (this)->NotifyObservers(value);
    }
    (this)->dataobject::ObservedChanged( changed, value);
}

short cel::Get(const class atom  *property, const class atom  **type, long  *rock)
{
    short result=(this)->dataobject::Get(property,type,rock);
    if(this->dataObject && !result)
	return (this->dataObject)->Get( property, type, rock);
    else return result;
}

cel::~cel()
{
	ATKinit;

    if(this->dataObject) { (this->dataObject)->RemoveObserver(this);
       (this->dataObject)->Destroy();
       this->dataObject=NULL;
    }
    
    /* cel_NotifyObservers(self,observable::OBJECTDESTROYED);
      this doesn't appear to be needed and this should never be
      done since observable will take care of notifying observers
      of the destruction */
    if(this->chain != this){
	class cel *nlink;
	if(this->ab == NULL || this->ab->first == (class cel *) this->ab) return;
	nlink = this->ab->first;
	if(nlink == this){
	    this->ab->first = this->chain;
	}
	else for(; nlink != NULL && nlink != nlink->chain ; nlink = nlink->chain){
	    if(nlink->chain == this){
		nlink->chain = this->chain;
		break;
	    }
	}
    }
}

static void SetVisible(class cel  *self)
{
    (self)->SetVisible();
}
static void SetInvisible(class cel  *self)
{
    (self)->SetInvisible();
}
cel::cel()
{
	ATKinit;

	NoSave=FALSE;
    this->refname = NULL;
    this->viewType = NULL;
    this->dataType = NULL;
    this->dataatm = NULL;
    this->viewatm = NULL;
    this->refatm = NULL;
    this->linkatm = NULL;
    this->linkname = NULL;
    this->viewID = ::viewID++;
    this->dataObject = NULL;
    this->desw = this->desh = 0;
    this->script = NULL;
    this->application = FALSE;
    this->script = NULL;
    this->readfromfile = 0;
    this->usedefaultview = 0;
    this->mode = cel_VISIBLE;
    (this)->SetDefaultStream(NULL);
    (this)->SetInitFile(NULL);
    (this)->SetWriteChild(TRUE);
    this->count = 0;
    this->chain = this;
    this->ab = NULL;
    THROWONFAILURE( TRUE);
}

class cel *cel::Create(char  *viewType, class dataobject  *dataObject)
        {
	ATKinit;

    class cel *newvr;
    
    if ((newvr = new cel))  {
	if ((newvr->viewatm = atom::Intern(viewType))!= NULL) {
	    newvr->viewType = (newvr->viewatm)->Name();
	    newvr->dataObject = dataObject;
	    (dataObject)->Reference();
	    (dataObject)->AddObserver(newvr);
	    
	    return newvr;
	}
    }
    fprintf(stderr, "Could not allocate cel structure.\n");
    return NULL;
}
const char *cel::SetRefName(const char  *refname)
{
    if(refname){
	if((this->refatm = atom::Intern(refname)) != NULL)
	    this->refname = (this->refatm)->Name();
	}
    return this->refname;
}
void cel::UnsetRefName()
{
#if 0 /* refname isn't alloc'd by SetRefName, so this seems unsafe - tjm */
    if(this->refname && *this->refname){
	free(this->refname);
#endif
	this->refname = NULL;
#if 0
    }
#endif
}
class dataobject *cel::GetObject()
{
    return (this->dataObject);
}
boolean cel::SetChildObject(class dataobject  *newobject,const char  *viewName)
{
    if(viewName == NULL || *viewName == '\0')
	(this)->SetViewName(viewName,TRUE);
    else (this)->SetViewName(viewName,FALSE);
    (this)->SetObject(newobject);
    (this)->SetRefName(this->dataType);
    return TRUE;
}
long cel::GetModified()
{
    long mod = (this)->dataobject::GetModified();
    if(this->NoSave) return mod;
    if(this->dataObject)
	mod += (this->dataObject)->GetModified();
    if(this->script)
	mod += (this->script)->GetModified();
    return mod;
}
boolean cel::SetObject(class dataobject  *newobject)
{
    if(newobject){
	/* 	    Register the object with the dictionary */
	dictionary::Insert(NULL,(char *)newobject->GetID(),(char *) newobject);
	this->dataObject = newobject;
	
	(newobject)->Reference();
	(newobject)->AddObserver(this);
	
	
	(this)->SetObjectName((char *)(newobject)->GetTypeName());
	if(this->usedefaultview)
	    (this)->SetViewName(NULL,TRUE);
	if(strcmp(this->dataType,"value")== 0) 
	    this->application =  VALUE;
	return TRUE;
    }
    return FALSE;
}
boolean cel::SetObjectByName(const char  *dataname)
{
    class dataobject *newobject;
    if((dataname == NULL || *dataname == '\0')) return FALSE;
    if((newobject = (class dataobject *) ATK::NewObject(dataname)) != NULL) {
	(this)->SetObject(newobject);
	(newobject)->Destroy(); /* won't really destroy since SetObject got a ref. */
	return TRUE;
    }
    return FALSE;
}
void cel::SetObjectName(const char  *dataname)
{
    if(dataname && *dataname && 
	(this->dataatm = atom::Intern(dataname))!= NULL) {
	this->dataType = (this->dataatm)->Name();
	if(strcmp(this->dataType,"value")== 0) 
	    this->application =  VALUE;
    }
}
    
void cel::SetViewName(const char  *viewname,int  usedefaultview)
{
    if(viewname && *viewname){
	this->usedefaultview = FALSE;
    }
    else {
	this->usedefaultview = usedefaultview;
	if(usedefaultview && this->dataObject)
	   viewname = (this->dataObject)->ViewName();
    }
    if (viewname && *viewname && 
	 (this->viewatm = atom::Intern(viewname))!= NULL) {
	    this->viewType = (this->viewatm)->Name();
    }
}
void cel::SetLinkName(const char  *linkname)
{
    if (linkname && *linkname && 
	 (this->linkatm = atom::Intern(linkname))!= NULL) {
	    this->linkname = (this->linkatm)->Name();
    }
    else{
	this->linkatm = NULL;
	this->linkname = NULL;
    }
}
void cel::SetApplication(int  app)
{
    if(this->dataType != NULL){
	if(app != cel_VALUE && strcmp(this->dataType,"value") == 0) {
	this->application = cel_VALUE;
	return ;
	}
    }
    this->application = app;
}

void cel::InsertObject (class dataobject  *newobject, char  *dataname,const char  *viewname,int  usedefaultview)
{
    char buf[128];
    if(newobject != NULL){
	dataname = (char *)(newobject)->GetTypeName();
    }
    else if((dataname == NULL || *dataname == '\0')) /* no object */;
    else{
	if((newobject = (class dataobject *) ATK::NewObject(dataname)) == NULL) {
	    if(this->application == VALUE){
		if((newobject = (class dataobject *) ATK::NewObject("value")) == NULL) return;
		if(viewname == NULL || *viewname == '\0'){
		    viewname = buf;
		    sprintf(buf,"%sV",dataname);
		}
	    }
	    else return;
	}
    }
    if(newobject){
	/* 	    Register the object with the dictionary */
	dictionary::Insert(NULL,(char *)newobject->GetID(),(char *) newobject);
	this->dataObject = newobject;

	(newobject)->AddObserver(this);
	
    }
  
 
    if(dataname && *dataname){
	 if ((this->dataatm = atom::Intern(dataname))!= NULL) {
	    this->dataType = (this->dataatm)->Name();

	}
    }
    if(this->dataType && (strcmp(this->dataType,"value")== 0)) 
	this->application =  VALUE;
#ifdef DEBUG
printf("Initing v = %s, d = %s, r = %s\n",this->viewType,this->dataType,this->refname);
#endif /* DEBUG */
}
static const class atom *getline(char  **place)
{
    char tmpbuf[512];
    char *c = tmpbuf;
    char *buf;
    buf = *place;
#ifdef DEBUG
printf("GETLINE GOT ---- %s XXXXXXX\n",*place);
#endif /* DEBUG */
/* printf("Getting line from %s\n",buf); */
    if(buf == NULL || *buf == '\0'){
	*c = '\0';
	return NULL;
    }
    if(*buf == '>' && strchr(buf,'<') != NULL) {
	buf = strchr(buf,'<');
	buf++;
    }
    while(*buf == ' ') buf++;
    while((*c = *buf++) != '\n') {
/*	putchar(*c); */
	if(*c++ == '\0') return NULL;
    }
    *c = '\0';
    *place = buf;
    return atom::Intern(tmpbuf);
}
long cel::ReadSup(FILE  *file, long  id)
            {
	return dataobject::NOREADERROR;
    }
long cel::ReadFile(FILE  *thisFile)
{  
    long objectID;
    long result = 0;
    const char *objectName;
    objectName = filetype::Lookup(thisFile, NULL, &objectID, NULL); /* For now, ignore attributes. */
    if(objectName == NULL) objectName = "text";
    if(ATK::IsTypeByName("cel",objectName) || ATK::IsTypeByName(objectName,"cel")){ 
	result = (this)->Read(thisFile,objectID);
/* 	if(self->arb) arbiterview_InitCell(self->arb,self); */
    }
    else{

	if(/* objecttest(self,objectName,"dataobject") && */ (this)->SetObjectByName(objectName) && (this)->GetObject() != NULL){
	    const char *nm = NULL;
	    class cel *arb = NULL;
	    (this)->SetViewName(NULL,TRUE);
	    (this)->SetRefName("");
	    result = ((this)->GetObject())->Read(thisFile,objectID);
	    if(strcmp(objectName,"arbiter") == 0){
		arb = (class cel *) (this)->GetObject();
		if((nm = (arb)->GetRefName()) == NULL || *nm == '\0'){
		    (arb)->SetRefName(objectName);
		}
		(this)->SetRefName("arbcel");
	    }
	    else{
		(this)->SetRefName(objectName);
	    }
	}
    }
    (this)->NotifyObservers(0);
    return result;
}

long cel::Read(FILE  *file, long  id)
            {
    long endcount = 1;
    boolean begindata;
    char *s;
    long c;
    long status;
    char objectname[200];
    const char *ccp;
    char *cp;
    long objectid;
    class dataobject *newobject = NULL;
    char cbuf[2048];
    char *buf;
    long textpending;
    long did,version = 0l;
    did = 0l;
    textpending = 0;
    buf = cbuf;
/* printf("In Cel Read\n"); */
    this->count++;
    while (endcount != 0)  {
        while ((c = getc(file)) != EOF && c != '\\')  {
	    if(endcount == 1){
		/* Place actual read code here */
	    *buf++ = c;
	    }
        }
        if (c == EOF) return dataobject::NOREADERROR;
        if ((c = getc(file)) == EOF)
            return dataobject::PREMATUREEOF;
	const char *be;
        if (c == 'b')  {
            begindata = TRUE;
            be = "egindata";
        }
        else if (c == 'e')  {
            begindata = FALSE;
            be = "nddata";
        }
        else  {
	    if(endcount == 1){
		/* Place handling of \x characters here */
		if(c == 'V') {
		    version = 0;
		    while ((c = getc(file)) != EOF && c != '\n')
			if(isdigit(c)) version = (version * 10) + (c - '0');
		    if (c == EOF) return dataobject::NOREADERROR;
		    if((status = (this)->ReadSup( file, id)) != dataobject::NOREADERROR){
			return status;
		    }
		}
	    }
            continue;
        }
        while ((c = getc(file)) != EOF && c == *be) be++;
        if (c == '{' && *be == '\0')  {
            if (begindata) {
                s = objectname;
                while ((c = getc(file)) != EOF && c != ',')
                    *s++ = c;
                if (c == EOF) return dataobject::PREMATUREEOF;
                *s = '\0';
                objectid = 0;
                while ((c = getc(file)) != EOF && c != '}')
                    if(c >= '0' && c <= '9')objectid = objectid * 10 + c - '0';
                if (c == EOF) return dataobject::PREMATUREEOF;
		if((c = getc(file))!= '\n' || (strcmp(objectname,"zip") == 0)) ungetc(c,file);
                /* Call the New routine for the object */
		if( buf == cbuf && endcount == 1 && version == 0 && id == 0 &&
		   (strcmp(objectname,(this)->GetTypeName()) == 0) ){
		    /* reading the begindata for this object */
		    id = 1;
		    continue;
		}
		ccp = objectname;
		if(!ATK::LoadClass(ccp)) {
		    ccp="unknown";
		}
                if ((newobject = (class dataobject *) ATK::NewObject(ccp)))  {
                    /* Register the object with the dictionary */
		    dictionary::Insert(NULL,(char *)objectid, (char *)newobject);
		    /* Call the read routine for the object */
		    (newobject)->UnReference();
                    status = (newobject)->Read( file, objectid);
		    if (status != dataobject::NOREADERROR) {
			printf("ERROR reading %s, %ld\n",objectname,status);
			return status;
		    }
		}
                else {
                    endcount += 1;
		    /* return dataobject::OBJECTCREATIONFAILED; */
		}

	    }
            else  {
                endcount -= 1;
                while ((c = getc(file)) != EOF && c != '}');
		if((c = getc(file))!= '\n') ungetc(c,file);
            }
        }
        else if(endcount == 1){
	    
        /* 	    Place Handling of characters following \  
           */	
	    *buf++ = c;
	}
    }
/*    cp = strchr(cbuf,'\n'); cp++; */
    cp = cbuf;
    *buf = '\0';
    if (*cp == '\n') cp++;
    switch(version){
	case 2:
	    sscanf(cp,"%d %ld %ld %ld %ld %d\n" ,&(this->application),
		   &did,&textpending,&(this->desw), &(this->desh),&(this->mode));
	    break;
	default:
	    sscanf(cp,"%d %ld %ld %ld %ld\n" ,&(this->application),
		   &did,&textpending,&(this->desw), &(this->desh));
    }
    while (*cp != '\n') cp++;
    cp++;

    if((this->dataatm = getline(&cp)) != NULL)this->dataType = (this->dataatm)->Name();
    else this->dataType = NULL;
    if((this->viewatm = getline(&cp)) != NULL) this->viewType =(this->viewatm)->Name();
    else this->viewType = NULL;
    if((this->refatm = getline(&cp)) != NULL) this->refname =(this->refatm)->Name();
    else this->refname = NULL;
    if((this->linkatm = getline(&cp)) != NULL) this->linkname =(this->linkatm)->Name();
    else this->linkname = NULL;
    *buf = '\0';
    if(textpending){
	this->script = (class text *) newobject;
	textpending = FALSE;
    }
    this->readfromfile = TRUE;
#ifdef DEBUG
   printf("<%s>\n",cbuf);
   printf("['%s' '%s' '%s' %ld]\n",this->dataType,this->viewType,this->refname,did); fflush(stdout); 
#endif /* DEBUG */
   
    if(did) {
	this->dataObject = (class dataobject *) dictionary::LookUp(NULL,(char *) did);
	(this->dataObject)->Reference();
	(this->dataObject)->AddObserver(this);
    }
    else if((this->linkname == NULL  || *(this->linkname) == '\0') && this->dataType != NULL){
	if(this->viewType == NULL || *(this->viewType) == '\0')
	    this->usedefaultview = TRUE;
	(this)->SetObjectByName(this->dataType);
    }
/*    registerobject(self); Assume arbiter will do this when view is linked.
      Unfortunately, this meens it will be unavailable until then .
      Maybe the arbiter should save a list of the objects and names */
#ifdef DEBUG
    printf("dobj = %d\n",this->dataObject);
#endif /* DEBUG */
    if(this->dataObject == NULL) this->count = 0;
    if((this->ab = arbiter::GetMaster()) != NULL)
	(this->ab)->DeclareRead(this);
    return dataobject::NOREADERROR;
}

long cel::WriteLink(FILE  *file ,long  writeid,int  level)
{
    long val;
    class dataobject *dob;
    dob = this->dataObject;
    this->dataObject = NULL;
    val = (this)->Write(file ,writeid,level);
    this->dataObject = dob;
    return val;
}
long cel::WriteSup(FILE  *file ,long  writeid,int  level)
{
return TRUE;
}
long cel::Write(FILE  *file ,long  writeid,int  level)
{
    long did;
    did = 0l;
    if (this->GetWriteID() == writeid)  return (this)->GetID();
    if(level == 0 && (this->refname==NULL || *(this->refname) == '\0') && this->script == NULL && 
	this->application == cel_APPLICATION && 
	this->dataObject != NULL && 
	strcmp((this)->GetTypeName(),"arbiter") == 0 &&
	ATK::IsTypeByName((this->dataObject)->GetTypeName(),"text") &&
	( ((class text *) this->dataObject)->GetExportEnvironments() == FALSE ||  
	 (((class text *) this->dataObject)->rootEnvironment)->NumberOfChildren() <= 0))
	/* don't write out self over plain text */
	level--;
    if(level != -1){
	this->SetWriteID(writeid);
	fprintf(file,"\\begindata{%s,%ld}\n",(this)->GetTypeName(),(this)->GetID());
	fprintf(file,"\\V 2\n"); /* Version Number */
	(this)->WriteSup(file ,writeid,level);
	if(this->WriteChild == FALSE){
	    fprintf(file,"1 0 0 0 0 0 \n>OBJ< \n>VIEW< \n>REF< \n");	    fprintf(file,"\\enddata{%s,%ld}\n",(this)->GetTypeName(),(this)->GetID());
	    return (this)->GetID();
	}
    }
#ifdef WOULDWORKBUT
    if(this->dataObject) did = (this->dataObject)->Write(file,writeid,level+1);
#else /* WOULDWORKBUT */
    if(this->dataObject){(this->dataObject)->Write(file,writeid,level+1); did = (this->dataObject)->GetID();}
#endif /* WOULDWORKBUT */
    if(level != -1){
	if(this->linkname)
	    fprintf(file,"%d %ld %d %ld %ld %d \n>OBJ< %s\n>VIEW< %s\n>REF< %s\n>LINK< %s\n" ,this->application,
		    did,(this->script != NULL),this->desw, this->desh,this->mode,(this->dataType)?this->dataType:"", (this->viewType)?this->viewType:"",(this->refname)?this->refname:"",(this->linkname)?this->linkname:"");
	else 
	    fprintf(file,"%d %ld %d %ld %ld %d \n>OBJ< %s\n>VIEW< %s\n>REF< %s\n" ,this->application,
		    did,(this->script != NULL),this->desw, this->desh,this->mode,(this->dataType)?this->dataType:"", (this->viewType)?this->viewType:"",(this->refname)?this->refname:"");
	if(this->script){
	    (this->script)->Write(file,writeid,level+1);
	}
	fprintf(file,"\\enddata{%s,%ld}\n",(this)->GetTypeName(),(this)->GetID());
    }
    return (this)->GetID();
}
void cel::SetVisibilityBit(int  mode)
{
    if(mode != this->mode){
	this->mode = mode;
	(this)->NotifyObservers(0);
    }
}
boolean cel::InitializeClass()
{
    proctable::DefineProc("cel-set-visible", (proctable_fptr)::SetVisible,&cel_ATKregistry_ ,NULL, "Make cel visible");
    proctable::DefineProc("cel-set-invisible", (proctable_fptr)::SetInvisible,&cel_ATKregistry_ ,NULL, "Make cel invisible");
    return TRUE;
}

static long searchatt(class cel  *self,char  *attname,long  *len)
{
    long tlen,i,attlen,j;
    attlen = strlen(attname);
    tlen = (self->script)->GetLength();
    for(i = 0; ((i = (self->script)->Index(i,'<',tlen - i)) != EOF);){
	i++;
	if((i + attlen < tlen) && ((self->script)->GetChar(i + attlen) == '>') && (self->script)->Strncmp(i,attname,strlen(attname)) == 0){
	    i += attlen;
	    j = (self->script)->Index(i,'(', tlen - i);
	    if(j != EOF) {
		j++;
		i = (self->script)->Index(j,')', tlen - j);
		if(i == EOF) return -1;
		*len = i - j;
		return j;
	    }
	}
    }
    return -1;
}
void cel::SetStringAtt(char  *attname,char  *attval)
{
    char buf[256];
    long i,len;
    if(this->script == NULL) this->script = new text;
    if( (i = searchatt(this,attname,&len)) >= 0){
	if(attval == NULL) (this->script)->DeleteCharacters(i,len);
	else (this->script)->ReplaceCharacters(i,len,attval,strlen(attval));
    }
    else if(attval != NULL){
	sprintf(buf,"[string] <%s> (%s)\n",attname,attval);
	(this->script)->InsertCharacters(0,buf,strlen(buf));
    }
    (this)->NotifyObservers(cel_NeedsRepost);
}
void cel::SetLongAtt(char  *attname,long  val)
{
    char buf[256],attval[64];
    long i,len;
    if(val == cel_UNDEFINEDVALUE) *attval = '\0';
    else sprintf(attval,"%ld",val);
    if(this->script == NULL) this->script = new text;
    if( (i = searchatt(this,attname,&len)) >= 0){
	(this->script)->ReplaceCharacters(i,len,attval,strlen(attval));
    }
    else if(val != cel_UNDEFINEDVALUE){
	sprintf(buf,"[long] <%s> (%s)\n",attname,attval);
	(this->script)->InsertCharacters(0,buf,strlen(buf));
    }
    (this)->NotifyObservers(cel_NeedsRepost);
}
long cel::GetLongAtt(char  *attname)
{
    long i,len;
    char buf[256],*c;
    if(this->script == NULL) return 0;
    if( (i = searchatt(this,attname,&len)) >= 0){
	if(len > 255) len = 255;
	if(len == 0) return cel_UNDEFINEDVALUE;
	for(c = buf; len; len--,i++) *c++ = (this->script)->GetChar(i);
	*c = '\0';
	return(atoi(buf));
    }
    return cel_UNDEFINEDVALUE;
}
char * cel::GetStringAtt(char  *attname,char  *buf,long  buflen)
{
    char *c;
    long i,len;
    if(this->script == NULL) return NULL;
    if( (i = searchatt(this,attname,&len)) >= 0){
	if(len >= buflen) len = buflen - 1;
	for(c = buf; len; len--,i++) *c++ = (this->script)->GetChar(i);
	*c = '\0';
	return(buf);
    }
    return NULL;
}

long cel::InitDefault()
{
    
    FILE *f;
    long ret;
    char fnm[20];
    this->count++;
    if(this->initfile && *this->initfile &&
	(f = fopen(this->initfile,"r")) != NULL){
	ret = (this)->ReadFile(f);
	fclose(f);
	return ret;
    }
    if(this->defaultStream){
	strcpy(fnm, "/tmp/celXXXXXX");
	int fd = mkstemp(fnm);
	/* need to generate tmpfile name */
	if(fd < 0 || (f = fdopen(fd,"w")) == NULL){
	    fputs("Can't write init file for cel in /tmp\n",stderr);
	    fflush(stderr);
	    this->count--;
	    return dataobject::OBJECTCREATIONFAILED;
	}
	fwrite(this->defaultStream,strlen(this->defaultStream),1,f);
	fclose(f);
	f = fopen(fnm,"r");
	ret = (this)->ReadFile(f);
	fclose(f);
	unlink(fnm);
	return ret;
    }
    return dataobject::OBJECTCREATIONFAILED;

}
#if 0
void cel__ClearChain(class cel  *self)
{
    class cel *nlink;
    
    nlink = self->chain;
    while(self != nlink){
	self->chain = self;
	self = nlink;
 	nlink = self->chain;
   }
}
#endif
