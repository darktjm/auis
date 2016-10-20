/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("lset.H")

#include <lset.H>
#include <dictionary.H>
#include <dataobject.H>
#include <rm.H>
#include <atom.H>
#include <atomlist.H>
#include <text.H>
#include <ctype.h>
/* #define DEBUG 1 */

#define VALUE 10
#define CEL 5
static const class atom *a_vp,*a_name,*a_atomlist;


ATKdefineRegistry(lset, dataobject, lset::InitializeClass);
static void registerobject(class lset  *self);
static class dataobject *getregisteredobject(class lset  *self);
static char *getline(char  *buf,char  *c);


static void registerobject(class lset  *self)
{
    class atomlist *al;
    char buf[256];
    if(self->refname == NULL || *self->refname == '\0') return;
    sprintf(buf,"DOBJ-%s",self->refname);
    al = atomlist::StringToAtomlist(buf);
    if(self->dobj == NULL) {
	printf("BOGUS Posting of NULL for %s\n",buf);
    }
    else{ 
#ifdef DEBUG
	printf("Posting %d for %s (%s)\n",(long)self->dobj,buf,(self->dobj)->GetTypeName()); 
#endif /* DEBUG */
	rm::PostResource(al,(long)self->dobj,a_vp);
	(self)->Put(a_name,a_atomlist,(long)al);
    }
}
static class dataobject *getregisteredobject(class lset  *self)
{
    class atomlist *al;
    char buf[256];
    long val;
    if(self->refname == NULL || *self->refname == '\0') return NULL;
    sprintf(buf,"DataObject-%s",self->refname);
    al = atomlist::StringToAtomlist(buf);
    if(rm::GetResource(al,al,a_vp,&val))
	return (class dataobject *)val;
    return NULL;
}
char *lset::registername(const char  *name)
{
    strcpy(this->refname,name);
    if(getregisteredobject(this) != NULL){
	*this->refname = '\0';
	return NULL;
    }
    return this->refname;
}

boolean lset::InitializeClass()
{
    
    a_vp = atom::Intern("struct dataobject *");
    a_name = atom::Intern("name");
    a_atomlist = atom::Intern("atomlist");
    return TRUE;
}
void lset::InsertObject (const char  *name,const char  *viewname)
{
    class dataobject *newobject;
    char buf[128];
    const char *val = "value";
    if(name == NULL || *name == '\0') newobject = NULL;
    else if((newobject = getregisteredobject(this)) == NULL){
	if((this->application == VALUE) && !ATK::IsTypeByName(name,"value")) {
	    sprintf(buf,"%sV",name);
	    name = val;
	}
	if(name == val || (newobject = (class dataobject *) ATK::NewObject(name)) == NULL) {
	    if(this->application == VALUE){
		if((newobject = (class dataobject *) ATK::NewObject("value")) == NULL) return;
		if(viewname == NULL || *viewname == '\0'){
		    viewname = buf;
		}
	    }
	    else return;
	}
    }
    if(newobject){
	/* 	    Register the object with the dictionary */
	dictionary::Insert(NULL,(char *)newobject->GetID(),(char *) newobject);
    }
    if(viewname == NULL || *viewname == '\0'){
	if(newobject == NULL) return;
	viewname = (newobject)->ViewName();
    }
    this->dobj = newobject;
    if(viewname)strcpy(this->viewname,viewname);
    if(name)strcpy(this->dataname,name);
    registerobject(this);
}

long lset::GetModified()
{
    long mod = (this)->dataobject::GetModified();
    if(this->dobj)
	mod += (this->dobj)->GetModified();
    if(this->left)
	mod += (this->left)->GetModified();
    if(this->right)
	mod += (this->right)->GetModified();
    return mod;
}
	
static char *getline(char  *buf,char  *c)
{
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
    return (buf);
}

long lset::Read(FILE  *file, long  id)
            {
    long endcount = 1;
    boolean begindata;
    char *s;
    long c,version;
    long status;
    char objectname[200];
    const char *ccp;
    char *cp;
    long objectid;
    class dataobject *newobject = NULL;
    char cbuf[2048];
    char *buf;
    long textpending;
    long did,lid,rid;
    did = lid = rid = 0l;
    textpending = 0;
    buf = cbuf;

    while (endcount != 0)  {
        while ((c = getc(file)) != EOF && c != '\\')  {
	    if(endcount == 1){
#ifdef DEBUG
putchar(c);
#endif /* DEBUG */
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
		if(((c = getc(file))!= '\n') || (strcmp(objectname,"zip") == 0)) ungetc(c,file);
		/* Call the New routine for the object */
		ccp=objectname;
		if(!ATK::LoadClass(ccp)) {
		    ccp="unknown";
		}
                if ((newobject = (class dataobject *) ATK::NewObject(ccp)))  {
                    /* Register the object with the dictionary */
		    dictionary::Insert(NULL,(char *)objectid, (char *)newobject);
                    /* Call the read routine for the object */
                    status = (newobject)->Read( file, objectid);
		    if (status != dataobject::NOREADERROR){
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
    *buf = 0; /* valgrind says libc is scanning for a terminator, for some reason */
    sscanf(cbuf,"%d %d %d %ld %ld %ld %ld\n" ,&(this->type),&(this->pct),&(this->application),
	 &did,&lid,&rid,&textpending);
    cp = strchr(cbuf,'\n'); cp++;
    cp = getline(cp,this->dataname);
    cp = getline(cp,this->viewname);
    cp = getline(cp,this->refname);
    *buf = '\0';
    if(textpending){
	this->pdoc = (class text *) newobject;
	textpending = FALSE;
    }
#ifdef DEBUG
   printf("<%s>\n",cbuf);
   printf("[%d %d '%s' '%s' %ld %ld %ld]\n",this->type,this->pct,this->dataname,this->viewname,
	 did,lid,rid); fflush(stdout); 
#endif /* DEBUG */
    if(did) this->dobj = (class dataobject *)dictionary::LookUp(NULL,(char *) did);
    if(lid) this->left = (class dataobject *)dictionary::LookUp(NULL,(char *) lid);
    if(rid) this->right = (class dataobject *)dictionary::LookUp(NULL,(char *) rid);
    registerobject(this);
#ifdef DEBUG
    printf("dobj = %d, left = %d, right = %d\n",this->dobj, this->left, this->right);
#endif /* DEBUG */
    return dataobject::NOREADERROR;
}

long lset::Write(FILE  *file ,long  writeid,int  level)
{
    long did,lid,rid;
    did = lid = rid = 0l;
    if (this->GetWriteID() == writeid)  return (this)->GetID();
    this->SetWriteID(writeid);

    fprintf(file,"\\begindata{lset,%ld}\n",(this)->GetID());
    fprintf(file,"\\V 1\n"); /* Version Number */
    if(this->dobj){(this->dobj)->Write(file,writeid,level+1); did = (this->dobj)->GetID();}
    if(this->left){(this->left)->Write(file,writeid,level+1);lid = (this->left)->GetID();}
    if(this->right){ (this->right)->Write(file,writeid,level+1);rid = (this->right)->GetID();}
    fprintf(file,"%d %d %d %ld %ld %ld %d\n>OBJ< %s\n>VIEW< %s\n>REF< %s\n" ,this->type,this->pct,this->application,
	 did,lid,rid,(this->pdoc != NULL),this->dataname,this->viewname,this->refname);
    if(this->pdoc){
	(this->pdoc)->Write(file,writeid,level+1);
    }
    fprintf(file,"\\enddata{lset,%ld}\n",(this)->GetID());
    return (this)->GetID();
}

lset::lset()
{
	ATKinit;

*this->dataname = '\0';
*this->viewname = '\0';
*this->refname = '\0';
this->type = 0;
this->pct = 0;
this->revision = 0;
this->dobj = NULL;
this->left = NULL;
this->right = NULL;
this->application = FALSE;
this->pdoc = NULL;
THROWONFAILURE( TRUE);
}
