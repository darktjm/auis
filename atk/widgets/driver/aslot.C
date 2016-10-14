/* Copyright 1995, 1996 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
ATK_IMPL("aslot.H")
#include <util.h>

#include <atom.H>
#include <avalue.H>
#include <aaction.H>
#include <proctable.H>
#include <simpletext.H>
#include <path.H>
#include <im.H>
#include <fontdesc.H>

#include <astring.H>
#include <aslot.H>

#define CONST 

ATK_CLASS(ASlotFont);
ATK_CLASS(ASlotBool);
ATK_CLASS(ASlotColor);

ATKregistryEntry *ASlot::fontslotreg = NULL;
ATKregistryEntry *ASlot::boolslotreg = NULL;
ATKregistryEntry *ASlot::intslotreg = NULL;
ATKregistryEntry *ASlot::realslotreg = NULL;
ATKregistryEntry *ASlot::stringslotreg = NULL;
ATKregistryEntry *ASlot::fileslotreg = NULL;
ATKregistryEntry *ASlot::ptrslotreg = NULL;
ATKregistryEntry *ASlot::atkslotreg = NULL;
ATKregistryEntry *ASlot::functionslotreg = NULL;
ATKregistryEntry *ASlot::dataobjslotreg =  NULL;
ATKregistryEntry *ASlot::figureslotreg =  NULL;

ATKregistryEntry *ASlot::atkreg = NULL;
ATKregistryEntry *ASlot::dataobjreg =  NULL;
ATKregistryEntry *ASlot::textreg =  NULL;
ATKregistryEntry *ASlot::figurereg =  NULL;
static ATKregistryEntry *simpletextreg = NULL;
static ATKregistryEntry *aactionreg = NULL;

const char *ASlot::client = NULL;

const atom *ASlot::in0type;

const unsigned short ASlot::fromresources = 1<<0;
	// value came from resource file or inherited in tree
const unsigned short ASlot::todatastream = 1<<1;
	// will save in datastream
const unsigned short ASlot::isowner = 1<<2;
	// the ~ASlot must free val.obj
const unsigned short ASlot::isdefault = 1<<3;
	// most recent value was given by Default()
const unsigned short ASlot::validating = 1<<4;
	// value is presently being computed.
const unsigned short ASlot::valid = 1<<5;
	// value has been computed.
// (next value is 1<<6)


// 
// ASlot - the base class
//

	static boolean 
InitializeClass() {
	ASlot::in0type = atom::Intern("avalue_u");

	ASlot::fontslotreg = ATK::LoadClass("ASlotFont");
	ASlot::boolslotreg = ATK::LoadClass("ASlotBool");
	ASlot::intslotreg = ATK::LoadClass("ASlotInt");
	ASlot::realslotreg = ATK::LoadClass("ASlotReal");
	ASlot::stringslotreg = ATK::LoadClass("ASlotString");
	ASlot::fileslotreg = ATK::LoadClass("ASlotFile");
	ASlot::ptrslotreg = ATK::LoadClass("ASlotPtr");
	ASlot::atkslotreg = ATK::LoadClass("ASlotATK");
	ASlot::functionslotreg = ATK::LoadClass("ASlotFunction");
	ASlot::dataobjslotreg = ATK::LoadClass("ASlotDataObj");
	ASlot::figureslotreg = ATK::LoadClass("ASlotFigure");

	ASlot::atkreg = ATK::LoadClass("ATK");
	ASlot::dataobjreg = ATK::LoadClass("dataobject");
	ASlot::textreg = ATK::LoadClass("text");
	ASlot::figurereg = ATK::LoadClass("figure");

	simpletextreg = ATK::LoadClass("simpletext");
	aactionreg = ATK::LoadClass("aaction");
	return TRUE;
}

ATKdefineRegistry(ASlot, ATK, InitializeClass);

	void 
ASlot::Init() {
	ATKinit;  // calls InitializeClass if needed
}

static atom_def union_src("union_src");

	ASlot &
ASlot::Assign(const avalue_u *src) {
	    
	if(hook) {
	    hook->Assign(this, src);
	}
	else val = *src;
	if (source) FREESTRINGVAR(source);
	if (flags & fromresources)  Remember();
	DisableFlags(isdefault); 
	return *this;
}

	ASlot &
ASlot::Assign(CONST ASlot *src) {
	// checks type and calls version with avalue_u
	// then sets source, if src has one
            if(src->IsType((ATK *)this) || ( src->IsType(ASlot::intslotreg) && IsType(ASlot::boolslotreg))) {
		Assign(&src->val);
		if (src->source)
			source = strdup(src->source);
	}
	else
		CantTake(src);
	return *this;
}

	void 
ASlot::CantTake(const char *invalid) {
	if (ASlot::client)
		fprintf(stderr, "While executing %s, ", ASlot::client);
	fprintf(stderr, 
		"Incorrectly tried to assign %s to slot %s of type %s\n",
			 invalid, name->Name(), GetSlotType());
	Default();
}

	void 
ASlot::CantTake(CONST ASlot *got) {
	if (ASlot::client)
		fprintf(stderr, "While executing %s - ", ASlot::client);
	fprintf(stderr, 
		"%s of slot %s of type %s to slot %s of type %s.\n", 
		"Illegal assignment ",
		got->name->Name(), got->GetSlotType(),
		name->Name(), GetSlotType());
	Default();
}

// slot cannot be converted to desiredtype
	void 
ASlot::InvalidAs(const char *desiredtype) const {
	if (ASlot::client)
		fprintf(stderr, "While executing %s - ", ASlot::client);
	fprintf(stderr, 
		"Cannot convert slot %s of type %s to type %s.\n",
		name->Name(), GetSlotType(), desiredtype);
}


ASlot_NAME(NO_NAME);

boolean ASlot::Equal(const avalue_u &, const avalue_u &) const {
    return FALSE;
}

ASlot::ASlot() {
	ATKinit;
	name = slot_NO_NAME;
	hook = NULL;
	flags = isdefault;	// most flags are zeroed
	source = NULL;
	val.real = 0.0;
	val.obj = NULL;
}

ASlot::~ASlot() {
	if (hook) { hook->Destroy(); hook = NULL; }
	if (source) FREESTRINGVAR(source);
	// val.obj must be freed or deleted or Unreferenced by
	// the derived class
}
    
	ASlot &
ASlot::Default()
{
	if (IsDefault()) return *this;
	avalue_u v;  
	v.obj = NULL;
	Assign(&v);
	EnableFlags(isdefault);
	return *this; 
}

/*
// Write()  -  write to file a line with the string value for the slot
//
//	process value:  
//		precede newlines with \
//		chop lines greater than 80 bytes by inserting \\\newline
//		make \ into \\
*/
	void 
ASlot::Write(FILE *fp) const {
        Check();
	fprintf(fp, "%s:(%s)", name->Name(), GetSlotType());
	if (source)
		fprintf(fp, "%s\n", source);
	else {
		AString dest;
		WriteValue(&dest);
		// xxx process value
		fprintf(fp, "%s\n", (char *)dest);
	}
}

	ASlot *
ASlot::Read(FILE *fp) {
	AString dest, slotname, slottype;
	int c = EOF;

	// read slotname
	while(!feof(fp) 
			&& (c=fgetc(fp))!=EOF 
			&& c!='\n' && c!=':') {
		if(!isspace(c)) slotname << (char)c;
	}
	if (feof(fp) || c != ':') return NULL;

	// read type
	while(!feof(fp) && (c=fgetc(fp))!=EOF && isspace(c)) {}
	if(c == '(') {
		while(!feof(fp) && 
				(c=fgetc(fp))!=EOF && c!='\n' && c!=')') {
			if(!isspace(c)) slottype << (char)c;
		}
		if(c!=')') return NULL;
	}
	else ungetc(c, fp);
	if(slottype.Length() == 0) slottype = "String";
	if(feof(fp) || c == EOF) return NULL;

	// create slot
	ASlot *res = NewASlot(slottype);
	if(res == NULL) return NULL;

	// Read the naive format...
	while(!feof(fp) && (c=fgetc(fp))!=EOF && c!='\n')
		dest << (char)c;
	if(feof(fp) || c == EOF) return NULL;

	/*
	// xxx unprocess the value:  
	//	\newline->newline
	//	\\\newline->empty
	//	\\->\
        */
	// process value and put in new slot
	if(res->EatString((char *)dest)) {
		if(slotname.Length())
		    res->SetName(atom_def((char *)slotname));
		// XXX ::Write must be updated to reprocess the value, or we must ensure
		// that the unprocessed value is written here.
		// (But the source must be set HERE, since Assign done by EatString
		// will clear any source previously set.)
		res->SetSource((char *)dest);
		return res;
	}
	delete res;
	return NULL;
}

	void
ASlot::WriteValue(AString *dest) const {
	dest->Sprintf(
		"{ %s::WriteValue missing.  Hex was 0x%lx }",
		((ATK *)this)->GetTypeName(), val.integer);
}

	boolean 
ASlot::ReadValue(const char *) {
	Default();
	fprintf(stderr, 
		"{ %s::ReadValue missing.  Used Default value. }\n",
		((ATK *)this)->GetTypeName());
	return FALSE;
}


static const char * const tagtbl[] = {
						"STRING", 
#define STRINGTAG     	0
						"PROC",
#define PROCTAG     	1
						"NESS",
#define NESSTAG    	2
						"NESSEXP",
#define NESSEXPTAG    	3
						"FILE",
#define FILETAG 		4
						NULL
#define NOTAG 		5
};


// EatString - convert string src into a value 
//		and call ReadValue with that value
//	check for TAG
//		none or STRING:  use value in the ReadValue call
//		PROC, NESS, or NESSEXP:  
//			convert to aaction
//			use ASlot::EatFunction(ASlotFunction) 
//		FILE:  use ASlotFile::EatString(char *) -> file name
//			use ASlotFile::ReadFile()
//			use ASlot::EatString(char *) to call ReadValue
//
	boolean 
ASlot::EatString(const char *src) {

	// get tag, if any
	char tag[20], *tx = tag;
	const char *sx = src;
	while (isspace(*sx)) sx++;
	while (isupper(*sx) && tx < tag+sizeof(tag)-1) 
		*tx++ = *sx++;
	*tx = '\0';
	if (*sx == ':') sx++;  
	else tx = tag, sx = src;
	// sx points to body of string

	// check table
	int tagindex = 0;
	if (tx > tag) 
		for (; tagtbl[tagindex]; tagindex++)
			if (strcmp(tag, tagtbl[tagindex]) == 0)
				break;

	// process depending on tag type
	AString temp;
	ASlotFunction fun;
	ASlotFile fl;

	switch (tagindex) {
	case NOTAG:
	case STRINGTAG:
		return ReadValue(sx);
		break;
	case PROCTAG:  {
		// convert proc name in sx to aaction
		while (isspace(*sx)) sx++;
		temp = "";
                while (*sx && ! isspace(*sx)) temp << *sx++;
                if(temp.FoldedEq("none")) {
                    fun=NULLACT;
                } else {
                    proctable_Entry *pe = 
                      proctable::Lookup((char *)temp);
                    // if proc is name of function loaded by a ness script, 
                    // it needs to be loaded, either here or in proctbl::
                    if(pe == NULL) {
                        fprintf(stderr, "awidget: unknown proctable function %s.\n", (char *)temp);
                        return FALSE;
                    }
                    fun = pe;
                }
		return EatFunction(fun);
	       } break;
	case NESSTAG:
		//  xxx  compile Ness code to aaction in 'func'
		// should just call ASlotFunction::ReadValue
		return FALSE;		// interim xxx
		// return EatAaction(func);
		break;
	case NESSEXPTAG:
		//  xxx  compile Ness code to aaction in 'func'
		// should just call ASlotFunction::ReadValue
		return FALSE;		// interim xxx
		// return EatAaction(func);
		break;
	case FILETAG: {
		while (isspace(*sx)) sx++;
		if ( ! fl.EatString(sx)) return FALSE;
			// using EatString(sx) allows, e.g.,
			//	 "FILE:FILE:foo"
			//  which takes the file name from file foo
		if ( ! fl.ReadFile(&temp)) return FALSE;
		return EatString((char *)temp);		// recur
	      }  break;
	}
	return FALSE;		// not reached 
}


// EatFunction - execute the function act and EatAvalue resulting avalue
//
//	XXX need current widget to pass as argument to act
//
	boolean 
ASlot::EatFunction(const ASlotFunction &act) {
	// call the function
	avalueflex in, out;
	(*(aaction *)act)(NULL, in, out);
	if( ! out.GetN()) return FALSE;
	return EatAvalue(&out[0]);
}


// EatAvalue - analyze type of avalue object and analyze result
//		call EatXyz for string, atom, and ATK * values
//			(the latter include functions)
//		otherwise fail
//		(ASlotInt and ASlotReal override EatAvalue)
//
	boolean 
ASlot::EatAvalue(const avalue *v) {
	// branch depending on type of value
	if (v->Type() == avalue::cstring) 
		return EatString((char *)v);
	else if (v->Type() == avalue::atomatom) 
		return EatString(((const atom *)v)->Name());
	else if (v->Type() == avalue::atkatom) 
		// I assume this means the value is an ATK object
		// which includes aaction values xxx ???
		return EatATK((ATK *)(void *)v);
// avalue should provide conversion to ATK *
	else return FALSE;
}


// EatATK - analyze type of ATK object and 
//		call ReadValue appropriately
//		deals with text, simpletext, and aaction
//
	boolean 
ASlot::EatATK(const ATK *obj) {
	if (((ATK *)obj)->IsType(simpletextreg)) {
		AString body;
		simpletext *t = (simpletext *)val.obj;
		char *buf;
		long wantlen = t->GetLength(), pos = 0;
		long gotlen;
		while (wantlen) {
			buf = t->GetBuf(pos, wantlen, &gotlen);
			if ( ! gotlen) return FALSE;
			body.AppendN(buf, (int)gotlen);
			wantlen -= gotlen;
			pos += gotlen;
		}
		return EatString((char *)body);
	}
	else if (((ATK *)obj)->IsType(aactionreg)) {
		ASlotFunction f((aaction *)obj);
		return EatFunction(f);
	}
	else return FALSE;
	return TRUE;		// for cases without a 'return'
}

	const char *
ASlot::ASlotPrefix(const char *name) {
	static AString newname	// initially space for 30 chars
			 ("012345678901234567890123456789");
	newname = "";
	newname << "ASlot" << name;
	return newname;
}
 
	const char *
ASlot::GetSlotType() const {
	const char *t = ((ATK *)this)->GetTypeName();
	if (strncmp(t, "ASlot", 5) == 0) t+=5;
	return t;
}

	ATKregistryEntry *
ASlot::LoadType(const char *typenm) {
	ATKregistryEntry *n =
			ATK::LoadClass(ASlotPrefix(typenm));
	if(n == NULL) n = ATK::LoadClass(typenm);
	return n;
}

	ASlot *
ASlot::NewASlot(const char *typenm) {
	ASlot *n =
		(ASlot *)ATK::NewObject(ASlotPrefix(typenm));
	if (n == NULL) n = (ASlot *)ATK::NewObject(typenm);
	return n;
}

	void
ASlot::SetSource(const char *src) {
	if (source) free(source); 
	if (src && *src)
		source = strdup((char *)src); 
	else source = NULL;
}
 
	void
ASlot::SetAssign(aaction *newset) {
	    if(hook==NULL) hook=new ASHook;
	    hook->SetAssign(newset);
}

	void
ASlot::DumpSlot(FILE *f) {
	const atom *nm = GetName();
	fprintf(f, "Slot %s    (%s)",  (nm) ? nm->Name() : "(unnamed)",
			GetSlotType());
	
	const char *oldclient = ASlot::Client(NULL);
	if (oldclient) printf("      Client is %s.\n", oldclient);
	ASlot::Client(oldclient);
	if (GetAssign())
		fprintf(f, "\tHas an assignment function.\n");
	if (GetFlags()) {
		fprintf(f, "\tFlags  (0x%x):", GetFlags());
		if (GetFlags(ASlot::fromresources))
			fprintf(f, "   fromresources");
		if (GetFlags(ASlot::todatastream))
			fprintf(f, "   todatastream");
		if (GetFlags(ASlot::isowner))
			fprintf(f, "   isowner");
		if (GetFlags(ASlot::isdefault))
			fprintf(f, "   isdefault");
		int morf = GetFlags( ~ 
				(ASlot::fromresources 
				| ASlot::todatastream 
				| ASlot::isowner 
				| ASlot::isdefault)  );
		if (morf)
			fprintf(f, "  0x%x", morf);
		fprintf(f, "\n");
	}
	if (GetSource())
		fprintf(f, "\tSource: %s\n", GetSource());
	fprintf(f, "\tValue is ");
	if (IsType(ASlot::intslotreg))
		fprintf(f, "int: %ld\n", (long)*(ASlotInt *)this);
	else if (IsType(ASlot::realslotreg))
		fprintf(f, "real: %g\n", (double)*(ASlotReal *)this);
	else if (IsType(ASlot::stringslotreg))
		fprintf(f, "string: %s\n", 
				(const char *)*(ASlotString *)this);
	else if (IsType(class_ASlotFont))
		fprintf(f, "font: %s\n", 
				(const char *)*(ASlotFont *)this);
	else if (IsType(class_ASlotBool)) {
		int foo = *(ASlotBool*)this;
		fprintf(f, "boolean: %s  (%d)\n",
			foo ? "TRUE" : "FALSE", foo);
	}
	else if (IsType(class_ASlotColor))
		fprintf(f, "color: %s\n", 
				(const char *)*(ASlotColor *)this);
	else if ( ! IsType(ASlot::ptrslotreg))   // neither ptr norATK
		fprintf(f, "of unknown type\n");
	else if ( ! IsType(ASlot::atkslotreg))   // is a ptr
		fprintf(f, "a ptr: 0x%p\n", (void *)*(ASlotPtr *)this);
	else if ( ! (ATK *)*(ASlotATK *)this)
		fprintf(f, "ATK object: NULL\n");
	else 
		fprintf(f, "ATK object of type %s\n",
				((ATK *)*(ASlotATK *)this)
					->GetTypeName());
	AString dest;
	WriteValue(&dest);
	fprintf(f, "\tWriteValue() returns %s\n", (char *)dest);
}

class ASlot;
	
void ASlot::Invalidate(ADependerKey) {
    DisableFlags(valid);
    if(hook) hook->Invalidate();
}

void ASlot::Validate(ADependerKey) {
    if(flags&valid) return;
    if(flags&validating) {
	fprintf(stderr, "warning: ASlot: recursive slot evaluation detected.\n");
	return;
    }
    if(hook) {
        flags|=validating;
	Push();
	hook->Validate(this);
        Pop();
        flags&=~validating;
    } else flags|=valid;
}
    
// 
// ASlotInt
//
ATKdefineRegistryNoInit(ASlotInt, ASlot);

boolean ASlotInt::Equal(const avalue_u &dest, const avalue_u &src) const {
    return dest.integer==src.integer;
}

	ASlot &
ASlotInt::Default() { 
	if (IsDefault()) return *this;
	avalue_u v;  
	v.integer = -999;  
	Assign(&v); 
	EnableFlags(isdefault);  
	return *this; 
}

	void 
ASlotInt::WriteValue(AString *dest) const {
	*dest << (long)*this;
}

	boolean 
ASlotInt::ReadValue(const char *src) {
	*this = atol(src); 
	return TRUE;
}

int ASlotInt::EatAvalue(const avalue *v) {
	if (v->Type() == avalue::integer)  *this = (long)v;
	else if (v->Type() == avalue::real) *this = (int)v->Real();
	else return ASlot::EatAvalue(v);
	return TRUE;
}


// 
// ASlotReal
//
ATKdefineRegistryNoInit(ASlotReal, ASlot);

boolean ASlotReal::Equal(const avalue_u &dest, const avalue_u &src) const {
    return dest.real==src.real;
}

	ASlot &
ASlotReal::Default() {
	avalue_u v; 
	v.real = -999.0;
	Assign(&v); 
	EnableFlags(isdefault);  
	return *this; 
}

	void 
ASlotReal::WriteValue(AString *dest) const {
	*dest << (double)*this;
}

	boolean 
ASlotReal::ReadValue(const char *src) {
	*this = atof(src); 
	return TRUE;
}

int ASlotReal::EatAvalue(const avalue *v) {
	if (v->Type() == avalue::integer)  *this = (long)v+0.0;
	else if (v->Type() == avalue::real) *this = v->Real();
	else return ASlot::EatAvalue(v);
	return TRUE;
}


// 
// ASlotString
//
ATKdefineRegistryNoInit(ASlotString, ASlot);
const char *ASlotString::empty = "";
boolean ASlotString::Equal(const avalue_u &dest, const avalue_u &src) const {
    return strcmp(dest.cstr,src.cstr)==0;
}

	ASlot &
ASlotString::Default() {
	if (IsDefault()) return *this;
	if (val.cstr != empty)
		free((void *)val.cstr);
	avalue_u v; 
	v.cstr = empty;
	Assign(&v); 
	EnableFlags(isdefault);
	return *this;
}

	ASlot &
ASlotString::operator=(const char *s) {
	avalue_u v;
	const char *oldcstr = val.cstr;
	v.cstr = (s && *s) ? strdup((char *)s) : empty;
	Assign(&v);  
	if (oldcstr != empty && oldcstr!=val.cstr) free((void *)oldcstr);
	if(v.cstr!=empty && v.cstr!=val.cstr) free((void *)v.cstr);
	return *this;
}

	ASlot &
ASlotString::operator=(CONST ASlot *s) {
	const char *oldcstr = val.cstr;
	Assign(s);
	if (val.cstr != oldcstr) {
		if (!val.cstr || !*val.cstr)
		    val.cstr = empty;
		else val.cstr = strdup((char *)val.cstr);
		if (oldcstr != empty) free((void *)oldcstr);
	}
	return *this;
}

	void 
ASlotString::WriteValue(AString *dest) const {
	const char *body = *this;
	if (isupper(*body) && isupper(body[1]))
		*dest << "STRING:";
	*dest << body;
}

	boolean 
ASlotString::ReadValue(const char *src) {
	*this = src; 
	return TRUE;
}


// 
// ASlotFile
//
ATKdefineRegistryNoInit(ASlotFile, ASlotString);
const char ASlotFile::devnull[] = "/dev/null";

	ASlot &
ASlotFile::Default() {
	if (IsDefault()) return *this;
	if (val.cstr != devnull) free((void *)val.cstr);
	avalue_u v; 
	v.cstr = devnull;
	Assign(&v); 
	EnableFlags(isdefault);
	return *this;
}

	ASlot &
ASlotFile::operator=(const char *s) {
	avalue_u v;
	const char *oldcstr = val.cstr;
	while (isspace(*s)) s++;
	v.cstr = (s && *s) ? strdup((char *)s) : devnull;
	Assign(&v);  
	if (oldcstr != devnull) free((void *)oldcstr);
	return *this;
}

	ASlot &
ASlotFile::operator=(CONST ASlot *s) {
	const char *oldcstr = val.cstr;
	Assign(s);
	if (val.cstr != oldcstr) {
		if (!val.cstr || !*val.cstr)
			val.cstr = devnull;
		else val.cstr = strdup((char *)val.cstr);
		if (oldcstr != devnull) free((void *)oldcstr);
	}
	return *this;
}

// read the entire file into dest
// return TRUE for success, else FALSE
	boolean
ASlotFile::ReadFile(AString *dest) const {
	*dest = "";
	int fd = OpenFile(O_RDONLY);
	if (fd < 0) return FALSE;
	FILE *f = fdopen(fd, "r");
	if ( ! f) {
	    close(fd);
	    return FALSE;
	}
	
	char buf[500];
	int zerocnt = 0;
	while ( ! feof(f)) {
		int n = fread(buf, sizeof(*buf), sizeof(buf), f);
		if (n <= 0)
			if (n < 0 || zerocnt++ > 2) {
				fclose(f); 
				return FALSE; 
			}
		dest->AppendN(buf, n);
	}
	fclose(f);
	return TRUE;
}


// The value written for an ASlotFile is a colon-separated sequence of
// pathnames.  The first is the absolute path.  Others are from among 
// these possibilies:
//	relative path (no more than three leading ../)
//	$(HOME)/...   (no ../)
//	$(ANDREWDIR)/...  (no ../)
//
// ReadValue only guarantees that the file can be read
//
	boolean
ASlotFile::ReadValue(const char *src) {
	const char *sx = src;
	const char *colon, *segend;
	while (*sx) {
		// get next segment
		colon = strchr(sx, ':');
		if ( ! colon) 
			colon = sx + strlen(sx);  // pt to \0
		segend = colon-1;
		while (isspace(*sx))  sx++;
		while (isspace(*segend))  segend--;
		if (segend >= sx) {
			// copy the segment to a string
			char path[MAXPATHLEN];
			strncpy(path, sx, segend-sx+1);
			path[segend-sx+1] = '\0';
		
			// unfold the name in the segment
			// this resolves $, ~, and other features
			char exppath[MAXPATHLEN];
			const char *expandedname =    // 'path' or 'exppath' 
				path::UnfoldFileName(path, exppath, 0);
				// xxx last arg should be name of resourcefile

			// check if file is readable
			if (access(expandedname, R_OK) == 0) {
				// SUCCESS
				*this = expandedname;
				return TRUE;
			}
		}	
		// advance to next segment
		if ( ! *colon)  break;
		sx = colon+1;
	}
	return FALSE;
}

	void 
ASlotFile::WriteValue(AString *dest) const {
	// This is the case with no source
	// Usually there should be a source giving alternate paths
	/// xxx or maybe we should generate alternate paths here???
	*dest << (const char *)*this;
}


// 
// ASlotPtr
//
ATKdefineRegistryNoInit(ASlotPtr, ASlot);
boolean ASlotPtr::Equal(const avalue_u &dest, const avalue_u &src) const {
    return dest.obj==src.obj;
}

	ASlot &
ASlotPtr::Default() { 
	if (IsDefault()) return *this;
	avalue_u v;  
	v.obj = NULL;  
	Assign(&v); 
	EnableFlags(isdefault);  
	return *this; 
}
ASlotPtr::operator ATK *() const {
    Check(); 
    ATK *src = (ATK *)(void *)*this;
    if ( ! src) return NULL;
    if(src->IsType(atkreg))  
	return (ATK *)src;
    InvalidAs("ATK *");
    return ASlotATK();		// return default value
}

	ASlot &
ASlotPtr::operator=(long s) {
	if (s == (long)NULL) {
		avalue_u v;  
		v.obj = NULL;  
		Assign(&v); 
	}
	else CantTake("numeric");
	return *this;
}

	void
ASlotPtr::WriteValue(AString *dest) const {
	if ( ! val.obj)
		*dest << "NULL";
	else
		dest->Sprintf(
		"{ %s::WriteValue missing.  Hex was 0x%lx }",
		((ATK *)this)->GetTypeName(), val.integer);
}

int ASlotPtr::EatAvalue(const avalue *v) {
	if (v->Type() == avalue::voidptr)  *this = v->VoidPtr();
	else if (v->Type() == avalue::atkatom) 
		return EatATK(v->ATKObject());
	else return ASlot::EatAvalue(v);
	return TRUE;
}

int ASlotPtr::EatATK(const ATK *obj) {
	if (((ATK *)obj)->IsType(aactionreg)) {
		//
		// call function rather than saving ptr to it ???
		//
		ASlotFunction f((aaction *)obj);
		return EatFunction(f);
	}
	*this = (void *)obj;
	return TRUE;
}


// 
// ASlotATK
//
ATKdefineRegistryNoInit(ASlotATK, ASlotPtr);


ASlotATK::operator dataobject *() const { 
    Check();
    ATK *src = (ATK *)*this;
    if ( ! src) return NULL;
    if (src->IsType(dataobjreg))   return (dataobject *)src;
    InvalidAs("dataobject *");
    return ASlotDataObj();	// return default value
}

ASlotATK::operator text *() const {
    Check(); 
    ATK *src=(ATK *)*this;
    if ( ! src) return NULL;
    if(src->IsType(textreg)) return (text *)src;
    InvalidAs("text *");
    return NULL;
}

ASlotATK::operator figure *() const {
    Check(); 
    ATK *src = (ATK *)*this;
    if ( ! src) return NULL;
    if(src->IsType(figurereg))  
	return (figure *)src;
    InvalidAs("figure *");
    return ASlotFigure();		// return default value
}

int ASlotATK::EatAvalue(const avalue *v) {
	if (v->Type() == avalue::atkatom) 
		return EatATK(v->ATKObject());
	else return ASlot::EatAvalue(v);
	return TRUE;
}

	boolean 
ASlotATK::EatATK(const ATK *obj) {
	*this = (ATK *)obj;
	return TRUE;
}


// 
// ASlotFunction
//   
ATKdefineRegistryNoInit(ASlotFunction, ASlotATK);

DEFINE_AACTION_FUNC_CLASS(ASlotFunctionAct, ATK);

	static void 
DummyFunction(ATK *, const avalueflex &, 
		const avalueflex &, avalueflex &out) {
	fprintf(stderr, "ASlot::DummyFunction called.\n");
	out.add("ASlot::DummyFunction called.");
}

static traced_ptr<ASlotFunctionAct> dummy(new ASlotFunctionAct(DummyFunction, 0L));

// Dispatch is used as the aaction for 
//	old proctable values that are not aactions
//
static atom_def proc("proc");
//
	static void 
Dispatch(ATK *obj, const avalueflex &a, 
			const avalueflex &in, avalueflex &out) {
	proctable_Entry *pe =
		(proctable_Entry *)a[0].VoidPtr(proc);
	proctable::Call(pe, obj, in, &out);
}

ASlotFunction::ASlotFunction()  { 
	val.obj = (void *) dummy;
	pe = NULL; 
}
ASlotFunction::ASlotFunction(const atom *n)  { 
	SetName(n);  
	val.obj = (void *) dummy;
	pe = NULL; 
}
ASlotFunction::ASlotFunction(aaction *s) { 
	val.obj = (void *) dummy;
	pe = NULL; 
	*this=s; 
}
ASlotFunction::ASlotFunction(const atom *n, aaction *s) { 
	SetName(n);  
	val.obj = (void *) dummy;
	pe = NULL; 
	*this=s; 
}

	ASlot &
ASlotFunction::Default() {
	if (IsDefault()) return *this;
	if (GetFlags(isowner)) {
		free(val.obj); /* tjm - can't delete if not real object -- should it be ATK? */
		DisableFlags(isowner);
	}
	pe = NULL;
	avalue_u v;
	v.obj = (void *) dummy;
	Assign(&v);
	EnableFlags(isdefault);
	return *this;
}

	ASlot &
ASlotFunction::operator=(CONST ASlot *v) {
	if (v->IsType(ASlot::functionslotreg))  
		*this=(ASlotFunction *)v;
	else if (v->IsType(ASlot::stringslotreg))  
		*this=(const char *)*v;
	else if (v->IsType(ASlot::fileslotreg))  
		*this=(ASlotFile *)v;
	else 
		return Assign(v);	// gives error msg
	if (v->GetSource())
		SetSource(v->GetSource());
	return *this;
}

	ASlot &
ASlotFunction::operator=(aaction *s) {
	if (GetFlags(isowner)  &&  val.obj) {
		((aaction *)val.obj)->Destroy();
		DisableFlags(isowner);
	}
	pe=NULL;
	avalue_u v;
	v.obj = (void *)s;
	Assign(&v);
	return *this;
}

	ASlot &
ASlotFunction::operator=(proctable_Entry *p) {
	if(GetFlags(isowner) && val.obj) 
		((aaction *)val.obj)->Destroy();
	pe = p;
	if (pe) {
		avalue_u v;
		proctable::ForceLoaded(pe);
		if(pe->act) {
		    DisableFlags(isowner);
		    v.obj = pe->act;
		} 
		else {
		    EnableFlags(isowner);
		    v.obj = new ASlotFunctionAct(Dispatch,
				avalueflex(pe, proc, proc));
		}
		Assign(&v);
	} else Default();
	return *this;
}

	ASlot &
ASlotFunction::operator=(const char *func) {
	if ( ! EatString(func))	
		Default();
	return *this;
}

	ASlot &
ASlotFunction::operator=(const ASlotFunction *t) {
	if (t->pe) *this = t->pe;
	else *this = (aaction *)*t;
	return *this;
} 

	ASlot &
ASlotFunction::operator=(const ASlotFile *f) {
	// read file and process to get a function
	AString filebody;
	f->ReadFile(&filebody);
	if ( ! EatString((char *)filebody))  
		Default();
	return *this;
}

// ReadValue - for functions, the only untagged string
//		value is a ness script
//
	boolean 
ASlotFunction::ReadValue(const char *) {
	// XXX  compile Ness code and make an aaction
	return FALSE;
}

	void 
ASlotFunction::WriteValue(AString *dest) const {
	if (pe) 
		*dest << "PROC:"  <<  proctable::GetName(pe);
	else if ( ! val.obj)
		*dest << "NULL";
	else 
		// no way to write out an arbitrary function
		// usually this will be handled by the 'source' field
		*dest << "ERROR: no character representation";
}

	boolean 
ASlotFunction::EatFunction(const ASlotFunction &act) {
	*this = &act;
	return TRUE;
}
	boolean 
ASlotFunction::EatATK(const ATK *obj) {
	if ( ! ((ATK *)obj)->IsType(aactionreg))  
		return ASlot::EatATK(obj);
	*this = (aaction *)obj;
	return TRUE;
}

// 
// ASlotDataObj
//
ATKdefineRegistryNoInit(ASlotDataObj, ASlotATK);

	ASlot &
ASlotDataObj::Default() {
	if (IsDefault()) return *this;
	avalue_u v;
	v.obj = NULL;
	if (val.obj) ((dataobject *)val.obj)->Destroy();
	Assign(&v);
	EnableFlags(isdefault);
	return *this;
}

ASlotDataObj::~ASlotDataObj() {
	if (val.obj) ((dataobject *)val.obj)->Destroy();
}

	ASlot &
ASlotDataObj::operator=(dataobject *s)  {
	dataobject *olddo = (dataobject *)val.obj;
	avalue_u v;
	v.obj = (void *)s;
	Assign(&v);
	if (s) s->Reference();
	if (olddo) olddo->Destroy();
	return *this;
} 

	ASlot &
ASlotDataObj::operator=(CONST ASlot *src) {
	void *oldptr = val.obj;
	Assign(src);
	if (val.obj) ((observable *)val.obj)->Reference();
	if (oldptr) ((observable *)oldptr)->Destroy();
	return *this;
}

	void 
ASlotDataObj::WriteValue(AString *dest) const {
	dataobject *dobj = *this;
	FILE *f = dest->VfileOpen("w");
	if ( ! f) return;	// FAILED
	if ( ! dobj)
		fprintf(f, "NULL");
	else
		dobj->Write(f, im::GetWriteID(), 0);
	dest->VfileClose(f);
}

	boolean
ASlotDataObj::ReadValue(const char *src) {
	int objid;
	struct expandstring es;
	const char begindata[] = "\\begindata{", *bex;
	char objname[100], *objx;

	// be sure we have an appropriate dataobject object
	dataobject *dobj = *this;

	es.string = (char *)src;
	es.size = es.length = 1 + strlen(src);
	es.pos = 0;
	FILE *f = im::vfileopen("r", &es);

	// check and skip over \begindata
	for (bex = begindata; *bex && getc(f) == *bex; bex++) {}
	if (*bex) goto fail;
	for (objx = objname; ; 
			objx += (objx-objname<(int)sizeof(objname)-2)?1:0)
		if (feof(f) || (*objx=getc(f)) == ',' || *objx == '\n')
			break;
	if (*objx != ',') goto fail;
	*objx = '\0';
	if (fscanf(f, "%d}\n", &objid) != 1) goto fail;

	if (dobj && strcmp(objname, dobj->GetTypeName()) != 0) {
		dobj->Destroy();
		dobj = NULL;
	}
	if ( ! dobj) {
		// sigh could test type before making object
		dobj = (dataobject *)ATK::NewObject(objname);
		if ( ! dobj->IsType(dataobjreg)) {
			dobj->Destroy();
			goto fail;
		}
	}
	if ( ! dobj) goto fail;

	// call dataobject::Read (f)
	if (dobj->Read(f, objid) != dataobject::NOREADERROR) {
		dobj->Destroy();
		goto fail;
	}

	im::vfileclose(f, &es);
	*this = dobj;
	return TRUE;

      fail:
	im::vfileclose(f, &es);
	Default();
	return FALSE;

}

	boolean 
ASlotDataObj::EatATK(const ATK *obj) {
		// ambiguity here:
		// if the dataobject is a simpletext, 
		// should we Eat it as a string ???
	if ( ! ((ATK *)obj)->IsType(dataobjreg))  
		return ASlot::EatATK(obj);
	*this = (dataobject *)obj;
	return TRUE;
}

// 
// ASlotFigure
//
ATKdefineRegistryNoInit(ASlotFigure, ASlotDataObj);

	ASlot &
ASlotFigure::operator=(figure *s)  {
	ASlotDataObj::operator=((dataobject *)s);
	return *this;
} 

	ASlot &
ASlotFigure::operator=(ATK *src) {
	if ( ! src->IsType(figurereg))
		CantTake(src->GetTypeName());
	else *this = (figure*)src;
	return *this;
}

	boolean 
ASlotFigure::EatATK(const ATK *obj) {
	if ( ! ((ATK *)obj)->IsType(figurereg))  
		return ASlot::EatATK(obj);
	*this = (figure *)obj;
	return TRUE;
}

ATKdefineRegistryNoInit(ASlotFont,ASlot);

boolean ASlotFont::Equal(const avalue_u &dest, const avalue_u &src) const {
    return dest.obj==src.obj;
}

ASlotFont::ASlotFont(const char *s) {
	if (!s) s = "andy12";
	fontname=strdup((char *)s);
	val.obj=fontdesc::Create("andy", fontdesc_Plain, 12);
}

ASlotFont::ASlotFont(const atom *name, const char *s)  {
	if (name) SetName(name);
	if (!s) s = "andy12";
	fontname=strdup((char *)s);
	val.obj=fontdesc::Create("andy", fontdesc_Plain, 12);
}

ASlot &ASlotFont::Default() {
	if (IsDefault()) return *this;
	if (fontname) free((void *)fontname);
	fontname = strdup("andy12");
	*this = (const char *)NULL;
	EnableFlags(isdefault);
	return *this;
}

ASlot &ASlotFont::operator=(const char *s) {
	char family[256];
	long style = fontdesc_Plain;
	long size = 12;
	while (s && isspace(*s)) s++;
	if ( ! fontdesc::ExplodeFontName((char *)s, family, 
				sizeof(family), &style, &size)) {
		strcpy(family, "andy");
		s = "andy12";
	}
	avalue_u tval;
	tval.obj=fontdesc::Create(family, style, size);
	Assign(&tval);
	if(fontname) free((void *)fontname);
	fontname=strdup((char *)s);
	return *this;
}

ASlot &ASlotFont::operator=(fontdesc *f) {
	avalue_u tval;
	tval.obj=f;
	Assign(&tval);
	if (fontname) free((void *)fontname);
	if ( ! f) {
		fontname = NULL;
		return *this;
	}
	// XXX need to have a standard fontdesc -> name function.
	AString ftnm = f->GetFontFamily();
	ftnm.Append(AString::itoa(f->GetFontSize()));
	long style = f->GetFontStyle();
	if (style & fontdesc_Bold) ftnm.Append('b');
	if (style & fontdesc_Italic) ftnm.Append('i');
	if (style & fontdesc_Fixed) ftnm.Append('f');
	fontname = strdup((char *)ftnm);
	return *this;
}

ASlot &ASlotFont::operator=(CONST ASlot *s) {
	if (s->IsType(this)) {
		if (fontname) free((void *)fontname);
		fontname = strdup(
				(char *)((ASlotFont *)s)->fontname);
	}
	/* else;	// (here Assign does nothing and gives an error message) */
	Assign(s);
	return *this;
}

void ASlotFont::WriteValue(AString *dest) const {
	const char *body = *this;
	if (isupper(*body) && isupper(body[1]))
		*dest << "STRING:";
	*dest << body;
}

boolean ASlotFont::ReadValue(const char *src) {
	*this = src; 
	return TRUE;
}

ATKdefineRegistryNoInit(ASlotBool, ASlotInt);

boolean ASlotBool::Equal(const avalue_u &dest, const avalue_u &src) const {
    return dest.integer==src.integer;
}

ASlot &
ASlotBool::Default() { 
	if (IsDefault()) return *this;
	avalue_u v;  
	v.integer = FALSE;  
	Assign(&v); 
	EnableFlags(isdefault);  
	return *this; 
}

void ASlotBool::WriteValue(AString *dest) const {
	boolean flag = *this;
	if(flag) *dest << "yes";
	else *dest << "no";
}

boolean ASlotBool::ReadValue(const char *src) {
	switch(*src) {
	case 'y':  case 'Y':  case 'T':  case 't': case '1':
		*this=TRUE;
		break;
	case 'o': case 'O': {
		char c = src[1];
		*this = (c == 'n'  || c == 'N');
		break;
	}
	default:
		*this=FALSE;
		break;
	}
	return TRUE;
}

boolean ASlotBool::EatAvalue(const avalue *v) {
	if (v->Type() == avalue::integer)  *this = (int)(long)v;
	else if (v->Type() == avalue::real) *this = (int)v->Real();
	else return ASlot::EatAvalue(v);
	return TRUE;
}

ATKdefineRegistryNoInit(ASlotColor, ASlot);
boolean ASlotColor::Equal(const avalue_u &dest, const avalue_u &src) const {
    return dest.obj==src.obj;
}

ASlot &ASlotColor::Default() { 
	if (IsDefault()) return *this;
	avalue_u v;
	icolor *old=(icolor *)val.obj;
	v.obj = icolor::Create("NONE");
	Assign(&v);
	if(old) old->Destroy();
	EnableFlags(isdefault);
	return *this; 
}


void ASlotColor::WriteValue(AString *dest) const {
	color &lcol=(color &)*this;
	if(lcol.Name()) *dest << lcol.Name();
	else {
		unsigned short r,g,b;
		lcol.RGB(r,g,b);
		dest->Sprintf("#%0.2x%0.2x%0.2x", r, g, b);
	}
}

boolean ASlotColor::ReadValue(const char *src) {
	*this=src;
	return TRUE;
}

ASlot &ASlotColor::operator=(const char *s) {
    icolor *old=(icolor *)val.obj;
    avalue_u tval;
    tval.obj=icolor::Create(s);
    Assign(&tval);
    if(old) old->Destroy();
    return *this;
}

ASlot &ASlotColor::operator=(const color &c) {
    icolor *old=(icolor *)val.obj;
    color &lcol=(color &)c;
    avalue_u tval;
    if(lcol.Name()) tval.obj = icolor::Create(lcol.Name());
    else {
	unsigned short r, g, b;
	lcol.RGB(r, g, b);
	tval.obj = icolor::Create(r, g, b);
    }
    Assign(&tval);
    if(old) old->Destroy();
    return *this;
}

ASlot &ASlotColor::operator=(CONST ASlot *s) {
    if(s->IsType(this)) {
	icolor *src=(icolor *)&((color &)*s);
	if(src) src->Reference();
    } /* else;	// (here Assign does nothing and gives an error message) */
    Assign(s);
    return *this;
}
