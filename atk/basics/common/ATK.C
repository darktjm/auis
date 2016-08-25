/* Copyright 1993 Carnegie Mellon University All rights reserved.
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
#include <andrewos.h>
ATK_IMPL("ATK.H")
#include <ATK.H>
#include <environ.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

#ifdef RCH_ENV
#include <ATKdebug.H>
#endif
/* NOTE:  ATK::DynamicLoad is implemented in a machine specific file. */

/* Registry entry for the "ATK" class. */
static ATK *ATK_new_() { return new ATK; };
ATKregistryEntry ATK_ATKregistry_ = {
    "ATK", ATK_new_, NULL, /* no parents */
};
ATKregistryEntry *ATK::ATKregistry() { return &ATK_ATKregistry_; }

/* This is the global ATK class registry. */
static ATKregistryEntry *ATKregistryHash[57];
static boolean RegNeedsInited = TRUE;
#define EndChain ((ATKregistryEntry *)1)

/* ATK_aliases: a mapping from old style name-keys to full or short names,
 and from full names to short names.

These are needed since file names are now restricted to 12 characters.
 (leaving space for a 1 character extension when/if necessary.)
 */
 
struct ATK_aliases {
    char *name, *alias;
    struct ATK_aliases *next;
    ATK_aliases(char *namep, char *aliasp);
    static char *FindAlias(const char *name);
};

static int readaliases=0;
static struct ATK_aliases *aliases=NULL;

ATK_aliases::ATK_aliases(char *namep, char *aliasp) {
    name=namep;
    alias=aliasp;
    next=aliases;
    aliases=this;
}
#define HASH(str,len)  \
	((( *(str)  \
		| ((str)[len>>2]<<24)  \
		| ((str)[(len)>>1]<<8) \
		| ((str)[(len)?(len)-1:0]<<16)    ) \
	^ (len)  ) \
	 % (sizeof(ATKregistryHash)/sizeof(*ATKregistryHash))  )

	static void
InitReg() {
	size_t i;
	for (i = sizeof(ATKregistryHash)/sizeof(*ATKregistryHash); i--; )
		ATKregistryHash[i] = EndChain;
	RegNeedsInited = FALSE;
	ATK::RegisterClass(&ATK_ATKregistry_);  // RECURSE
}

static char lowered[256];
char *ATK_aliases::FindAlias(const char *name) {
    ATK_aliases *f=aliases;
    if(name==NULL) return NULL;
    while(f) {
	if(f->name && strcmp(f->name, name)==0) return f->alias;
	f=f->next;
    }
    int len = strlen(name);
    if(len > sizeof(lowered) - 1)
	len = sizeof(lowered) - 1;
    memmove(lowered, name, len); /* sometimes overlaps */
    lowered[len] = 0;
    char *p=lowered;
    while(*p && *p!='.') {
	if(isupper(*p)) *p=tolower(*p);
	p++;
    }
    return lowered;
}

static void ReadAliasFile(const char *path) {
    FILE *fp;
    char mapping[512];
    if(path==NULL) return;
    fp=fopen(path, "r");
    if(fp==NULL) return;
    while(!feof(fp) && fgets(mapping, sizeof(mapping)-1, fp)) {
	char *p=strchr(mapping, '\n');
	if(p) *p='\0';
	p=mapping;
	if(*p=='#') continue;
	while(*p && *p!=' ' && *p!='\t') p++;
	if(*p=='\0') continue;
	*p++='\0';
	while(*p==' ' || *p=='\t') p++;
	new ATK_aliases(NewString(mapping), NewString(p));
    }
    fclose(fp);
}

static void ReadAliases() {
    /* Read aliases needed for ATK proper. */
    ReadAliasFile(environ::AndrewDir("/lib/atkaliases"));
    /* Read aliases for ATK client classes */
    ReadAliasFile(environ::GetConfiguration("ATKClassAliases"));
    ReadAliasFile(environ::GetProfile("ATKClassAliases"));
    ReadAliasFile(environ::Get("ATKCLASSALIASES"));
}

static int trace;

int &ATK::DynamicLoadTrace()
{
    return trace;
}


#ifdef HAVE_DYNAMIC_LOADING
#include <ATKDoLoad.H>

struct ATKregistryEntry *(*ATKDynamicLoader)(const char *name, boolean trace)=ATK::DynamicLoad;

static char dynpathopen_realpath[MAXPATHLEN+1];
static int dynpathopen(const char  *path, const char  *fpath)
{
    const char *p=path;
    int fd=(-1);
    while (p && *p && fd<0) {
        int len;
        const char *q=strchr(p, ':');
        if(q==NULL) len=strlen(p);
        else len = q-p;
        if(len+strlen(fpath)+2<MAXPATHLEN) {
            strncpy(dynpathopen_realpath, p, len);
            strcpy(dynpathopen_realpath+len, "/");
            strcat(dynpathopen_realpath+len, fpath);
	    fd=access(dynpathopen_realpath, R_OK);
	    if(fd==0) {
		return fd;
	    }
        }
        p+=len;
        if(*p) p++;
    }
    if(fd<0) {
        return -1;
    }
    return fd;
}


extern "C" void ATKLoadedObject(const char *name, const char *path)
{
}

static ATKregistryEntry *FindRegistryEntryByName(const char *classname);
ATKregistryEntry *ATK::DynamicLoad(const char *name, boolean atrace)
{
    ATKregistryEntry *ent=FindRegistryEntryByName(name);
    if(ent!=NULL) return ent;
    
    char *base=NULL;
    long len=0;
    int (*mainfunc)(int, char **);
    int retval=0;
    int slen=strlen(name);
    char *alias = NULL;
    char fullname[MAXPATHLEN];
    
#if !SY_OS2
    const char *path=getenv("CLASSPATH");
    int fd=(-1);
    
    ATKDynamicLoader=ATK::DynamicLoad;
#ifdef DYNAMIC_LOAD_DEBUG
    printf("name:%s\n",name);
#endif
    
    if(slen>MAXPATHLEN-1) {
	fprintf(stderr, "DynamicLoad: Class name %s is too long.\n", name);
	return NULL;
    }

#ifdef ATKDYNOBJS_ARE_LIBS
    strcpy(fullname, "lib");
#else
    fullname[0]='\0';
#endif
    strcat(fullname, name);
    strcat(fullname, ".");
    strcat(fullname, ATKDYNEXT);
    
    if(path==NULL) path=environ::AndrewDir(ATKDLIBDIR);
    
#ifdef DYNAMIC_LOAD_DEBUG
    printf("Attempting to open %s on path %s\n", fullname, path);
#endif
    
    fd=dynpathopen(path, fullname);
    if(fd<0) {
	alias=ATK_aliases::FindAlias(name);
	if(alias) {
	    if(strlen(alias)>MAXPATHLEN-1) {
		fprintf(stderr, "DynamicLoad: Class file name %s is too long.\n", alias);
		return NULL;
	    }
#ifdef ATKDYNOBJS_ARE_LIBS
	    strcpy(fullname, "lib");
#else
	    fullname[0]='\0';
#endif
	    strcat(fullname, alias);
	    strcat(fullname, ".");
	    strcat(fullname, ATKDYNEXT);

	    fd=dynpathopen(path, fullname);
	}
    }
    strcpy(fullname,dynpathopen_realpath);

    if(fd<0) {
	
#ifdef DYNAMIC_LOAD_DEBUG
	fprintf(stderr, "DynamicLoad: Couldn't find or open %s.\n", fullname);
#endif
	
	return NULL;
    }
    
#ifdef DYNAMIC_LOAD_DEBUG
    printf("found %s\n", fullname);
#endif
    
    mainfunc=ATKDoLoad(fullname, trace||atrace);
    
#else
   /* OS/2 has its own mechanism for finding dynamic objects.  We let it
    * do its thing, but because we don't have symlinks we must attempt an
    * alias if the initial load doesn't work.  It is assumed that the
    * initial load will usually work.
    */ 
    mainfunc = 0;
    if (mainfunc == NULL) {
      /* Lookup an alias and try again if mainfunc is still not defined. */
      alias = ATK_aliases::FindAlias(name);
      if (alias) {
	strcpy(fullname, alias);
        mainfunc=ATKDoLoad(fullname, trace||atrace);
	if(trace)
	    printf("Alias Load of \"%s\" for \"%s\" returns %x\n", fullname, name, mainfunc);
      }
    }
    if((mainfunc == NULL) && (slen < 9)) {
       /*short name ... might load as-is. */
       strcpy(fullname, name);
       mainfunc=ATKDoLoad(fullname, trace||atrace);
       if(trace)
	   printf("Load of \"%s\" returns %x\n", fullname, mainfunc);
    }
#endif /* OS/2 */

#ifdef DYNAMIC_LOAD_DEBUG
    printf("mainfunc:%x\n", mainfunc);
#endif
    
    if(mainfunc==NULL) return NULL;
    
#ifdef RCH_ENV
    ATKdebug::MallocInit();	// re-init memory checking to handle the new dynamic objects.
    // We rely on tracing in doload().
#else
    if (trace) {
	printf("Dynamic loaded %s\n", fullname);
	ATKLoadedObject(name, fullname);
    }
#endif /* RCH_ENV */

    retval=mainfunc(0, NULL);
    
#ifdef DYNAMIC_LOAD_DEBUG
    printf("retavl:%d\n", retval);
#endif
    
    if(retval<0) return NULL;
#ifdef DYNAMIC_LOAD_DEBUG
for (ent = global_ATKregistry; ent; ent = ent->next) printf("%s is registered\n", ent->GetTypeName())
      ;
#endif
    return FindRegistryEntryByName(name);
}
#else
ATKregistryEntry *ATK::DynamicLoad(const char *name, boolean trace)
{
    return NULL;
}
#endif /* HAVE_DYNAMIC_LOADING */


/*
 * Search the registry for the given class.
 * Return NULL if the given class has not yet been registered.
 *
 * ATKregistryEntries are kept on a simple list.
 */
static ATKregistryEntry *FindRegistryEntryByName(const char *classname)
{
    if(!readaliases) {
	ReadAliases();
	readaliases=1;
    }
    ATKregistryEntry *ent;
    if (RegNeedsInited) InitReg();
    int len = strlen (classname);
    int i = HASH(classname, len);
    for (ent = ATKregistryHash[i];
		 ent != EndChain && ent->ClassName 
			&& strcmp(classname, ent->ClassName) != 0;
		 ent = ent->next)
	{}
    if (ent != EndChain && ent->ClassName)
	return ent;	/* found it! */
    else {
	char *alias=ATK_aliases::FindAlias(classname);
	static int recursed=0;
	if(alias && !recursed) {
	    recursed=1;
	    ATKregistryEntry *tmp=FindRegistryEntryByName(alias);
	    recursed=0;
	    return tmp;
	} else return NULL;
    }
}

/*
 * Return TRUE iff the named class is already loaded.
 */
boolean ATK::IsLoaded(const char *classname) {
    return FindRegistryEntryByName(classname)!=NULL;
}

ATKregistryEntry *ATK::LoadClass(const char *classname)
{
    ATKregistryEntry *reg=ATK::DynamicLoad(classname);
    if (reg && reg->initfunc) {
	    boolean (*initfunc)() = reg->initfunc;
	    reg->initfunc = NULL;
	    if(!initfunc()) 
		return NULL;
    }
    return reg;
}

void ATK::ATKConstructorFailure()
{
    fprintf(stderr, "Warning: a constructor failed and exceptions are not being used.\nNo error recovery is possible, a coredump is likely.\n");
}

/*
 * Register an entry with the runtime system.
 * We automatically register its superclasses.
 * If the classent's next field is non-null we know
 * that the class has already been registered.
 */
void ATK::RegisterClass(ATKregistryEntry *classent)
{
    if (classent->next)
	return;	// already registered.
    if (RegNeedsInited) InitReg();
    int len = strlen (classent->ClassName);
    int i = HASH(classent->ClassName, len);
    classent->next = ATKregistryHash[i];
    ATKregistryHash[i] = classent;

    // Now register this class's superclasses.
    for (i = 0; i < ATKmaxRegistryParents; i++)
	if (classent->parents[i])
  	    ATK::RegisterClass(classent->parents[i]);
}

/*
 * Create an instance of an object given its name.
 * Lookup the name in the class_entries table and
 * call the new function.
 */
ATK *ATK::NewObject(const char *classname)
{
    ATKregistryEntry *classent = LoadClass(classname);
    if (classent)
	return classent->New();
    else
	return NULL;
}

/*
 * Return true if sub is some subclass of
 * super.
 *
 * Start at this object's registry entry and walk
 * up the inheritence tree of registry entries.  If
 * we find classent, the answer is yes.
 */
static boolean CheckTypes(const ATKregistryEntry *sub, const ATKregistryEntry *super)
{
    const ATKregistryEntry *myent;
    if(super==NULL || sub==NULL) return FALSE;
    for (myent=sub;myent && myent != super; myent = myent->parents[0]);
    return myent != NULL;
}

/*
 * Return true if this object is some subclass of
 * type{name,registry,object}.
 *
 * Start at this object's registry entry and walk
 * up the inheritence tree of registry entries.  If
 * we find classent, the answer is yes.
 */
boolean ATKregistryEntry::IsType(const ATK *typeobject)  const {
    return CheckTypes(this, ((ATK *)typeobject)->ATKregistry());
}
boolean ATKregistryEntry::IsType(const ATKregistryEntry *typeregistry)  const {
    return CheckTypes(this, typeregistry);
}
boolean ATKregistryEntry::IsType(const char *t)  const {
    return CheckTypes(this, FindRegistryEntryByName(t));
}


boolean ATK::IsTypeByName(const char *sub, const char *super) {
    ATKregistryEntry *subentry=FindRegistryEntryByName(sub);
    ATKregistryEntry *superentry=FindRegistryEntryByName(super);
    return CheckTypes(subentry, superentry);
}
