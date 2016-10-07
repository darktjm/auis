/* Copyright 1995 Carnegie Mellon University All rights reserved. */
#include <andrewos.h>
ATK_IMPL("avarset.H")

#include <ctype.h>

#include <avarset.H>
#include <environ.H>
#include <atom.H>
#include <aslot.H>
#include <astring.H>
#include <mflex.H>

ATKdefineRegistryNoInit(AVarSet, dataobject);

class NO_DLL_EXPORT AVarSetPathSegs;
DEFINE_MFLEX_CLASS(AVarSetPathSegs,char *,5);

AVarSet::AVarSet() {
	ASlot::Init();	// so we can use xxxslotreg values
	resourceclass = NULL;
	fetchedresources = FALSE;
	// addedslots and localslots are inited by their constructors
}

AVarSet::~AVarSet() {
	size_t i;
	for(i = 0; i < addedslots.GetN(); i++)
			// removed test for NULL  -wjh
		delete addedslots[i];
}


void AVarSet::CopySlots(AVarSet &dest, boolean fromresources) {
    size_t i;
    for(i = 0; i < addedslots.GetN(); i++) if(addedslots[i]) {
	ASlot *s=addedslots[i];
	ASlot *d=dest.Get(s->GetName());
	if(d==NULL) d=dest.AddSlot(s->GetName(), s->GetTypeName());
	if(d) {
	    *d=addedslots[i];
	    if(fromresources) {
		d->DisableFlags(ASlot::todatastream);
		d->EnableFlags(ASlot::fromresources);
	    }
	}
    }
    for(i = 0; i < localslots.GetN(); i++) if(localslots[i]) {
	ASlot *s=addedslots[i];
	ASlot *d=dest.Get(s->GetName());
	if(d==NULL) d=dest.AddSlot(s->GetName(), s->GetTypeName());
	if(d) {
	    *d=localslots[i];
	    if(fromresources) {
		d->DisableFlags(ASlot::todatastream);
		d->EnableFlags(ASlot::fromresources);
	    }
	}
    }
}

/* make this a member function of dataobject! someday.
 It takes a FILE * and reads everything up through and including
 the \n at the end of the enddata{name,id}\n.  
 (other begin/enddatas can be ignored since the id must be unique.) 
*/
	static long 
ConsumeEverything(FILE *fp, const char *name, long id) {
	int c;
	while(!feof(fp) && (c=fgetc(fp)) != EOF) {
		if(c != '\\') continue;
		static const char enddata[] = "enddata{";
		const char *p = enddata;
		while(*p && !feof(fp) && (c=fgetc(fp)) != EOF && *p == c) p++;
		if(*p == '\0') {
			while(!feof(fp) && (c=fgetc(fp)) != EOF && isspace(c));
			const char *p = name;
			while(c != EOF && *p && *p == c && !feof(fp)) 
				if(*++p) c = fgetc(fp);
			while(!feof(fp) && (c=fgetc(fp))!=EOF && isspace(c)) {}
			if(c != ',') continue;
			long oid;
			fscanf(fp, "%ld}", &oid);
			if(oid != id) continue;
			if((c=fgetc(fp)) == '\n') 
				return dataobject::NOREADERROR;
			else {
				ungetc(c, fp);
				return dataobject::NOREADERROR;
			}
		}
		if(c == '\\') ungetc(c, fp);
	}
	return dataobject::PREMATUREEOF;
}

	ASlot *
AVarSet::CheckAndReport(ASlot_NameParam name, 
		ATKregistryEntry *ent, 
		const char *readableclassname) {
	ASlot *res = LocateSlot(name);
	if (res != NULL && ent && res->ATKregistry() != ent) {
		fprintf(stderr, "AVarSet: Resource %s of type %s\n", 
			res->GetName()->Name(),
			res->GetTypeName());
		fprintf(stderr,"\tis being overridden with type %s.\n",
			readableclassname);
		RemoveSlot(name);
		res = LocateSlot(name);		// it might be local
		if (res) 
			fprintf(stderr, "Slot is local, type unchanged.\n");
	}

// xxx NewASlot should check for initial "ASlot" and should accept
// xxx names without initial "ASlot" if there is such a class.

	if(res == NULL) {
		const char *tnm = ent->GetTypeName();
		if (strncmp(tnm, "ASlot", 5) == 0) tnm += 5;
		res = ASlot::NewASlot(tnm);
		res->SetName(name);
		*addedslots.Append() = res;
		size_t i;
		for(i=0;i<refedslots.GetN();i++) {
		    if(refedslots[i].Name()==name) {
			refedslots[i].Invalidate();
			refedslots.Remove(i);
			break;
		    }
		}
	}
	return res;
}

#define AVarSet_MAXLINE 256

//when reading, use classname to create object 
// and resourceclassname to FetchInitialResources
//
// lines expected:
//	!Class: <name>		zero or once
//	<slotname> : (<type>) <value>	zero or more
//	+local				one line (constant)
//	<lines of local data>		multiple
//
	long 
AVarSet::Read(FILE *fp, long id) {
	char line[AVarSet_MAXLINE+1];
	int c, errors = 0;

	// check for !Class and FetchInitialResources
	c = getc(fp);
	if (c == '!') {	
		fgets(line, AVarSet_MAXLINE, fp);
		if(strncmp(line, "Class:", 6) == 0) {
			char *nulch = line + strlen(line);
			while (isspace(nulch[-1])) *--nulch = '\0';
                        FetchInitialResources(atom_def(line+7));
		}
		// else ignore spurious ! line
	}
	else if (c != EOF) ungetc(c, fp);
        if ( ! fetchedresources) {
            const char *p=GetTypeName();
            if(*p=='A') p++;
            FetchInitialResources(atom_def(p));
        }

	// read resource definition lines (stop for + or \)
	while ((c = getc(fp)) != EOF && c != '\\' && c != '+') {
		ungetc(c, fp);
		ASlot *s = ASlot::Read(fp);
		if(s) {
			ASlot *t = CheckAndReport(s->GetName(), 
				s->ATKregistry(), s->GetTypeName());
			if (t) 	{
			    *t = s;
			    t->Remember();
			}
			else errors++;
			delete s;
		} else errors++;
	}

	// read local data, if any
	if (c == '+') {
		fgets(line, AVarSet_MAXLINE, fp);
		if (strcmp(line, "local") == 0) {
			void *state = NULL;
			while ((c=getc(fp)) != EOF && c != '\\') {
				// local data lines
				ungetc(c, fp);
				fgets(line, AVarSet_MAXLINE, fp);
				state = ParseLocalDataLine(line, state);
			}
			ParseLocalDataLine(NULL, state);
		}
		else c = getc(fp);   // ignore the spurious + line
	}
	if (c != EOF) ungetc(c, fp);

	// read end of data stream
	if (errors == 0)
		return ConsumeEverything(fp, GetTypeName(), id);
	else {
		ConsumeEverything(fp, GetTypeName(), id);
		return dataobject::BADFORMAT;
	}
}

	long 
AVarSet::Write(FILE *fp, long writeID, int ) {
	long objid = GetID();
	if (GetWriteID() == writeID)  
		return objid;
	SetWriteID(writeID);
	const char *classname = GetTypeName();
	fprintf(fp, "\\begindata{%s,%ld}\n", classname, objid);
	if (resourceclass)
		fprintf(fp, "!Class:%s\n", resourceclass->Name());
	size_t i;
	for(i = 0; i < localslots.GetN(); i++) 
		if(localslots[i]->GetFlags(ASlot::todatastream)) 
			localslots[i]->Write(fp);
	for(i = 0; i < addedslots.GetN(); i++) 
		if(addedslots[i]->GetFlags(ASlot::todatastream)) 
			addedslots[i]->Write(fp);
	fprintf(fp, "+local\n");
	WriteLocalData(fp);
	fprintf(fp, "\\enddata{%s,%ld}\n", classname, objid);
	return objid;
}

	void *
AVarSet::ParseLocalDataLine(char * /* line */, void * /* state */) {
	return NULL;	// ignores local data lines
}

	void
AVarSet::WriteLocalData(FILE */* file */) { }


// compare tail of AString p vs s
static boolean taileq(AString *p, const char *s) {
    size_t len=strlen(s);
    if(p->Length()>=(int)len) return 0 ==
	  strcmp((*p)+p->Length()-len, s);
    else return FALSE;
}

// FetchFrom(path, dir, filenm) 
//   ensure path ends in /
//   remove 'atk/ from end of path (if any)
//   construct path/dir/filenm.wgt
//   FetchResources from named file

static char *CheckDir(const char *path, const char *dir, const char *filenm) {

    static AString pathname;
    pathname=path;
    if ( ! taileq(&pathname, "/"))
	pathname.Append("/");
    if (taileq(&pathname, "atk/"))
	pathname.Delete(pathname.Length() - 4, 4);
    pathname.Append(dir);
    if ( ! taileq(&pathname, "/"))
	pathname.Append("/");
    pathname.Append(filenm);
    if ( ! taileq(&pathname, ".wgt"))
	pathname.Append(".wgt");
    if(access((char *)pathname, R_OK) == 0) return pathname;
    return NULL;
}

static int FetchFrom(AVarSet *self, const char *path, const char *dir, const char *filenm) {
    int errors = 0;
    char *pathname=CheckDir(path, dir, filenm);
    FILE *fp = fopen((char *)pathname, "r");
    if (fp) {
	errors += self->FetchResources(fp);
	fclose(fp);
    }
    else errors ++;
    return errors;
}

// XXX must go through the path once and at least cache 
// the existence of directories.  Also cache names of .wgt files
	int
	  AVarSet::FetchResourcesFromPath(const char *filenm) {
	int errors = 0;

	// get path
	AString path = getenv("CLASSPATH");
	if (path == "") path = environ::AndrewDir(ATKDLIBDIR);

	// divide path into segments
	AVarSetPathSegs seglist;
	char *px=path, *segend;
	while (*px) {
	    *seglist.Append()=px;
	    segend = strchr(px, ':');
	    if ( ! segend) break;
	    *segend = '\0';
	    px = segend+1;
	}

	// using segments right to left, look for files named 
	//	segment/filenm.wgt and segment/widget/filenm.wgt
	// FetchResources from each file found
		// FetchResources strips trailing "atk/" from path
	
	size_t i,n=seglist.GetN()-1;
	for(i=0;i<=n;i++) {
		errors += FetchFrom(this, seglist[n-i], "", filenm);
		errors += FetchFrom(this, seglist[n-i], "widgets/", filenm);
	}
	
	// check $HOME/widgets
	errors += FetchFrom(this, environ::GetHome(NULL), 
				"widgets/", filenm);

	return errors;
}

int AVarSet::FetchInitialResources(const atom *resclass) {
	int errors = 0;
        if (resclass) SetResourceClass(resclass);
	char buf[MAXPATHLEN];
	buf[0]='A';
        strcpy(buf+1,resclass->Name());
        ATK::LoadClass(buf);
        errors += FetchResourcesFromPath(resourceclass->Name());
        fetchedresources=TRUE;
	return errors;
}

int AVarSet::FetchResources(FILE *fp) {
	int c;
	int errors = 0;
	char line[AVarSet_MAXLINE+1];

	// check for !Class and FetchInitialResources
	c = getc(fp);
	if (c == '!') {	
		fgets(line, AVarSet_MAXLINE, fp);
		if(strncmp(line, "Class:", 6) == 0) {
			char *nulch = line + strlen(line);
			while (isspace(nulch[-1])) *--nulch = '\0';
			FetchInitialResources(atom_def(line+7));
		}
		// else ignore spurious ! line
	}
	else if (c != EOF) ungetc(c, fp);
	while(!feof(fp)) {
		c = fgetc(fp);
		if (isspace(c)) continue;
		switch(c) {

		case '!':	// comment, skip line
			while(!feof(fp) && (c=fgetc(fp)) != EOF && 
					c != '\n') {}
			break;

		case '#': {   // #include widget-class-name
			// skip space after the '#', before "include"
			while(!feof(fp) && (c=fgetc(fp)) != EOF 
						&& isspace(c)) {}
			// skip "include"
			while(!feof(fp) && (c=fgetc(fp)) != EOF 
						&& !isspace(c)) {}
			// skip space after "include"
			while(!feof(fp) && (c=fgetc(fp)) != EOF 
						&& isspace(c)) {}
			if(c != '<' && c != '"') ungetc(c, fp);
			char filename[MAXPATHLEN+1];
			char *p = filename;
			while(!feof(fp) && filename-p<MAXPATHLEN 
					&& (c=fgetc(fp))!=EOF && !isspace(c)
					&& c != '>' && c != '"') *p++ = c;
			*p = '\0';
			errors += 
				FetchResourcesFromPath(filename);
			// skip space after the filename
			while(!feof(fp) && (c=fgetc(fp)) != EOF && 
				c != '\n') {}
			} break;

		case '\\':
			ungetc(c,fp);
			// FALL THRU
		case EOF:
			return errors;

		default:	{	// process a resource line (at last:-)
		    ungetc(c, fp);
		    ASlot *s = ASlot::Read(fp);
		    if (s) {
			ASlot *t = CheckAndReport(
						  s->GetName(), s->ATKregistry(),
						  s->GetTypeName());
			if (t) {
			    *t = s; 
			    t->DisRemember();
			}
			delete s;
		    } else errors++;
		    }
			break;
		}
	}
	return errors;
}

	ASlot &
AVarSet::Baptize(ASlot &v, ASlot_NameParam name, boolean fr) {
	if (name) v.SetName(name);
	*localslots.Append() = &v;
	if(fr) v.EnableFlags(ASlot::fromresources);
	else v.DisableFlags(ASlot::fromresources);
	return v;
}

	ASlot *
AVarSet::LocateSlot(ASlot_NameParam name) {
	size_t i;
	for(i = 0; i < localslots.GetN(); i++)
		if(localslots[i]->GetName() == name) 
			return localslots[i];
	for(i = 0; i < addedslots.GetN(); i++)
		if(addedslots[i]->GetName() == name) 
		    return addedslots[i];
	ADepender *depender;
	if((depender=ADepender::Top())) {
	    for(i=0;i<refedslots.GetN();i++) if(refedslots[i].Name()==name) {
		refedslots[i].AddDepender(depender, ADepender::TopPart());
		return NULL;
	    }
	    ADependerPlaceHolder *adph=refedslots.Append();
	    adph->SetName(name);
	    adph->AddDepender(depender, ADepender::TopPart());
	}
	return NULL;
}

	ASlot *
AVarSet::AddSlot(ASlot_NameParam name, const char *typenm) {
	AString tnm = typenm;
	if (strncmp(tnm, "ASlot", 5) != 0) tnm.Insert(0, "ASlot");
	ASlot *res = CheckAndReport(name, ATK::LoadClass(tnm), typenm);
	return res;
}

void AVarSet::RemoveSlot(ASlot_NameParam name) {
	size_t i;
// localslots cannot be removed because they may be 
// reference directly from the view.   -wjh
//	for(i = 0; i < localslots.GetN(); i++) {
//		if(localslots[i]->GetName() == name) {
//			localslots.Remove(i,1);
//			return;
//		}
//	}
	for(i = 0; i < addedslots.GetN(); i++) {
		if(addedslots[i]->GetName() == name) {
			delete addedslots[i];
			addedslots.Remove(i,1);
			return;
		}
	}
}

// calls f once for each slot in the varset
// stops iteration if a call returns FALSE
// call is f(this, slot, rock)
//
	void 
AVarSet::EnumerateSlots(AVarSet_esfptr *f, void *rock) {
	size_t i;
	for(i = 0; i < localslots.GetN(); i++) {
		if ( ! f(this, localslots[i], rock))
			return;
	}
	for(i = 0; i < addedslots.GetN(); i++) {
		if ( ! f(this, addedslots[i], rock))
			return;
	}
}

#define AVSSET(argtype, reg, typenm)  \
	AVarSet& \
AVarSet::Set(ASlot_NameParam name, argtype v) {  \
	ASlot *res = CheckAndReport(name, ASlot::reg, Stringize(typenm));  \
	if (res)  *(Concat(ASlot,typenm) *)res = v;   \
	return *this;   \
}
// NO SPACES AFTER COMMAS HERE
AVSSET(fontdesc *,fontslotreg,Font);
AVSSET(abool,boolslotreg,Bool);        
AVSSET(long,intslotreg,Int);
AVSSET(int,intslotreg,Int);
AVSSET(double,realslotreg,Real);
AVSSET(const char *,stringslotreg,String);
AVSSET(void *,ptrslotreg,Ptr);
AVSSET(ATK *,atkslotreg,ATK);
AVSSET(aaction *,functionslotreg,Function);
AVSSET(proctable_Entry *,functionslotreg,Function);
AVSSET(dataobject *,dataobjslotreg,DataObj);
AVSSET(figure *,figureslotreg,Figure);
#undef AVSSET

	AVarSet&
AVarSet::Set(ASlot_NameParam name, ASlot * v) { 
	ASlot *res = CheckAndReport(name, v->ATKregistry(),
				v->GetTypeName());
	if (res) *res = v;
	return *this;
}

	ASlot *
AVarSet::Get(ASlot_NameParam name) {
	ASlot *val = LocateSlot(name);
	if (val) return val;

// xxx need to scan up view tree for resource 'name'

	return NULL;
}

long AVarSet::Get(ASlot_NameParam name, long def) {
    ASlot *val=LocateSlot(name);
    if (val) return *val;
    return def;
}


double AVarSet::Get(ASlot_NameParam name, double def) {
    ASlot *val=LocateSlot(name);
    if(val) return *val;
    return def;
}


const char * AVarSet::Get(ASlot_NameParam name, const char * def) {
    ASlot *val=LocateSlot(name);
    if(val) return *val;
    return def;
}

const void * AVarSet::Get(ASlot_NameParam name, const void * def) {
    ASlot *val=LocateSlot(name);
    if(val) return (void *)*val;
    return def;
}

aaction * AVarSet::Get(ASlot_NameParam name, aaction * def) {
    ASlot *val=LocateSlot(name);
    if(val) return *val;
    return def;
}

ATK *AVarSet::GetATK(ASlot_NameParam name, ATKregistryEntry *type) {
    ASlot *val=LocateSlot(name);
    if(!val) return NULL;
    ATK *result=*val;
    if(!result) return NULL;
    if(!type) return result;
    if(result->IsType(type)) return result;
    return NULL;
}

dataobject *AVarSet::GetDataObj(ASlot_NameParam name, ATKregistryEntry *type) {
    ASlot *val=LocateSlot(name);
    if(!val) return NULL;
    dataobject *result=*val;
    if(!result) return NULL;
    if(!type) return result;
    if(result->IsType(type)) return result;
    return NULL;
}

figure *AVarSet::GetFigure(ASlot_NameParam name, ATKregistryEntry *type) {
    ASlot *val=LocateSlot(name);
    if(!val) return NULL;
    figure *result=*val;
    if(!result) return NULL;
    if(!type) return result;
    if(((ATK *)result)->IsType(type)) return result;
    return NULL;
}

const color &AVarSet::Get(ASlot_NameParam name, const color &def) {
    ASlot *val=LocateSlot(name);
    if(!val) return def;
   return *val;
}

fontdesc *AVarSet::GetFont(ASlot_NameParam name, fontdesc *def) {
    ASlot *val=LocateSlot(name);
    if(!val) return def;
    return *val;
}
#if 0 /* unused */
	static boolean
SlotDumpFunc(AVarSet */* vars */, ASlot *slot, void *rock) {
	slot->DumpSlot((FILE *)rock);	
	return TRUE;
}

static void DumpVarSet(AVarSet *v) {
	const atom *rescl = v->GetResourceClass();
	printf("\n");
	if (rescl)
		printf("=== ResourceClass : %s ===\n", 
				rescl->Name());
	printf("=== Slots ===\n");
	v->EnumerateSlots(SlotDumpFunc, stdout);
}
#endif

/* $Log: avarset.C,v $
 * Revision 1.33  1996/11/06  20:18:58  robr
 * Support of an AVarSet::Set(name, font) method.
 * FEATURE
 *
 * Revision 1.32  1996/10/24  16:35:41  robr
 * added Get method for void *s
 * BUG
 *
 * Revision 1.31  1996/10/17  21:38:43  robr
 * Fixed so that fetchedresources is set to TRUE once the initial
 * resources have been fetched.
 * Fixed so that FecthInitialResources is passed the widget name
 * with the leading 'A' stripped off.
 * BUG
 *
 * Revision 1.30  1996/10/15  00:29:34  robr
 * Added automatic loading of the class referred to in a !Class: entry.
 * Added Set method taking an abool.
 * BUG
 *
 * Revision 1.29  1996/06/21  16:28:21  robr
 * Added initial support for partial formula evaluation.
 *
 * Revision 1.28  1996/06/19  17:57:49  robr
 * Added support for recording references to non-existent slots so formulas
 * can depend on such references.
 *
 * Revision 1.27  1996/05/22  19:34:15  robr
 * Fixed uninitialized memory reference and added Get methods for colors
 * and fonts.
 * BUG
 *
 * Revision 1.26  1996/05/20  17:04:47  robr
 * Fixed warnings.
 *
 * Revision 1.25  1996/05/20  15:25:07  robr
 * /tmp/msg
 *
 * Revision 1.24  1996/05/13  17:14:04  wjh
 *  add log at end of file; add int as a data type to which a slot can be Set directly
 *
 */
