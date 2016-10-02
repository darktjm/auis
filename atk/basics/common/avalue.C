/* Copyright 1995 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
ATK_IMPL("avalue.H")
#include <avalue.H>
#include <util.h>

const atom *avalue::integer=NULL;
const atom *avalue::real=NULL;
const atom *avalue::cstring=NULL;
const atom *avalue::voidptr=NULL;
const atom *avalue::valueflex=NULL;
const atom *avalue::none=NULL;
const atom *avalue::voidatom=NULL;
const atom *avalue::atomatom=NULL;
const atom *avalue::atkatom=NULL;

avalue_init::avalue_init() {
    static boolean t=0;
    if(!t) {
	t=TRUE;
	avalue::integer= atom::Intern("integer");
	avalue::real= atom::Intern("real");
	avalue::cstring= atom::Intern("cstring");
	avalue::voidptr= atom::Intern("voidptr");
	avalue::valueflex= atom::Intern("valueflex");
	avalue::none= atom::Intern("none");
	avalue::voidatom= atom::Intern("void");
	avalue::atomatom= atom::Intern("atom");
	avalue::atkatom= atom::Intern("ATK");
    }
}

avalue::avalue() {
    type=name=NULL;
}

avalue::avalue(const avalue &src) {
    type=src.type;
    name=src.name;
    val=src.val;
}

avalue &avalue::operator=(const avalue &src) {
    type=src.type;
    name=src.name;
    val=src.val;
    return *this;
}
    
avalue::~avalue() {
}

long avalue::Integer(const atom *t) const {
    if(t==NULL) t=avalue::integer;
    if(t!=type) return 0;
    return val.integer;
}

double avalue::Real(const atom *t) const {
    if(t==NULL) t=avalue::real;
    if(t!=type) return 0.0;
    return val.real;
}
const char *avalue::CString(const atom *t)  const {
    if(t==NULL) t=avalue::cstring;
    if(t!=type) return NULL;
    return val.cstr;
}

const atom *avalue::Atom(const atom *t)  const {
    if(t==NULL) t=avalue::atomatom;
    if(t!=type) return NULL;
    return (const atom *)val.obj;
}

ATK *avalue::ATKObject(const atom *t)  const {
    if(t==NULL) t=avalue::atkatom;
    if(type==NULL) return NULL;
    if(ATK::IsTypeByName(type->Name(), t->Name())) return (ATK *)val.obj;
    return NULL;
}

void *avalue::VoidPtr(const atom *t) const {
    if(t==NULL) t=avalue::voidptr;
    if(t!=type) return NULL;
    return val.obj;
}

avalueflex *avalue::ValueFlex(const atom *t) const {
    if(t==NULL) t=avalue::valueflex;
    if(t!=type) return NULL;
    return (avalueflex *)val.obj;
}

avalueflex *avalue::List(const atom *t) const {
    if(t==NULL) t=avalue::valueflex;
    if(t!=type) return NULL;
    return (avalueflex *)val.obj;
}

void avalue::SetName(const atom *n) {
    name=n;
}

void avalue::Set(const atom *a, const atom *n, const atom *t) {
    if(n==NULL) n=avalue::none;
    if(t==NULL) t=avalue::atomatom;
    val.obj=(atom *)a;
    name=n;
    type=t;
}
void avalue::Set(long i, const atom *n, const atom *t) {
    if(n==NULL) n=avalue::none;
    if(t==NULL) t=avalue::integer;
    val.integer=i;
    name=n;
    type=t;
}

void avalue::Set(double r, const atom *n, const atom *t) {
    if(n==NULL) n=avalue::none;
    if(t==NULL) t=avalue::real;
    val.real=r;
    name=n;
    type=t;
}

void avalue::Set(const char *str, const atom *n, const atom *t) {
    if(n==NULL) n=avalue::none;
    if(t==NULL) t=avalue::cstring;
    val.cstr=str;
    name=n;
    type=t;
}

void avalue::Set(void *ptr, const atom *n, const atom *t) {
    if(n==NULL) n=avalue::none;
    if(t==NULL) t=avalue::voidptr;
    val.obj=ptr;
    name=n;
    type=t;
}


void avalue::Set(avalueflex *l, const atom *n, const atom *t) {
    if(n==NULL) n=avalue::none;
    if(t==NULL) t=avalue::valueflex;
    val.obj=l;
    name=n;
    type=t;
}

void avalue::Set(ATK *a, const atom *n) {
    if(n==NULL) n=avalue::none;
    val.obj=a;
    name=n;
    if(a==NULL) type=avalue::voidatom;
    else type=atom::Intern(a->GetTypeName());
}

// Here we keep memory around for use with avalueflexes with fewer than 16 elements.
// Just a little speed hack.
static struct avflexmem {
    struct avflexmem *next;
    size_t len;
    char space[1];
} *space[16];


char *avalueflex::Allocate(size_t len) {
    size_t ind=len/typesize;
    if(ind<16 && space[ind]) {
	char *res=space[ind]->space;
	space[ind]=space[ind]->next;
	return res;
    }
    avflexmem *avflm=(struct avflexmem *)malloc(sizeof(struct avflexmem)+len-1);
    if(avflm==NULL) return NULL;
    avflm->next=NULL;
    avflm->len=len;
    return avflm->space;
}

static void fdeallocate(char *&ptr) {
    if(ptr) {
	const int pos=((avflexmem *)0)->space-(char *)0;
	avflexmem *res=(avflexmem *)(ptr-pos);
	size_t ind=res->len/sizeof(avalue);
	if(ind>15) free(res);
	else {
	    res->next=space[ind];
	    space[ind]=res;
	}
	ptr=NULL;
    }
}

avalue *avalueflex::Find(const atom *n, const atom *t) {
    for(unsigned int i=0;i<GetN();i++) {
	if((*this)[i].Name()==n && (t==NULL || (*this)[i].Name()==t)) return &((*this)[i]);
	if((*this)[i].Type()==avalue::valueflex) {
	    avalue *res=(*this)[i].ValueFlex()->Find(n);
	    if(res) return res;
	}
    }
    return NULL;
}

static int parseBackslashed(const char  **fromChars)
    {

    int returnChar;
    const char *from = *fromChars;
    static const char *bsSource = "ebrnt";
    static const char *bsDest = "\033\b\r\n\t";

    if (*from == '\\') {
        ++from;
        if (*from == 0)
            return -1;
        if (isdigit(*from)) {

            int sum = 0, i;

            for (i = 0; i < 3; ++i) {
                if (!isdigit(*from))
                    break;
                if (*from == '8' || *from == '9')
                    return -1;
                sum = sum * 8 + *from - '0';
                ++from;
            }
            returnChar = sum;
        }
        else {

            const char *p;

            p = strchr(bsSource, *from);
            if (p != NULL)
                returnChar = bsDest[p-bsSource];
            else
                returnChar = *from;
            ++from;
        }
    }
    else
        return -1;
    *fromChars = from;
    return (unsigned char)returnChar;
}

static const atom *ParseType(const char **str) {
    const char *p=(*str);
    if(*p==':') {
	p++;
	size_t dummy;
	flex type;
	while(*p && !isspace(*p) && *p!=')') {
	    *type.Append()=*p;
	    p++;
	}
	*type.Append()='\0';
	const char *t=type.GetBuf(0,type.GetN(),&dummy);
	if(t==NULL) return NULL;
	*str=p;
	return atom::Intern(t);
    }
    return NULL;
}

int avalueflex::InterpretString(const char **str) {
    const char *p=(*str);
    if(*p!='(') return -1;
    p++;
    while(*p) {
	flex name;
	const atom *aname=NULL;
	if(isspace(*p)) {
	    p++;
	    continue;
	} else if(isalpha(*p)) {
	    name.Remove(0,name.GetN());
	    while(*p && !isspace(*p) && *p!='=' && *p!=':' && *p!=')') {
		*name.Append()=*p;
		p++;
	    }
	    *name.Append()='\0';
	    size_t dummy;
	    const char *n=name.GetBuf(0,name.GetN(),&dummy);
	    if(n==NULL) continue;
	    if(*p=='=') {
		p++;
		aname=atom::Intern(n);
	    } else {
		add(atom::Intern(n),aname,ParseType(&p));
		aname=NULL;
	    }
	} else if(*p=='(') {
	    avalueflex *newaa=new avalueflex;
	    if(newaa==NULL) {
		int level=1;
		p++;
		while(*p && level>0) {
		    if(*p=='(') level++;
		    else if (*p==')') level--;
		    p++;
		}
		*str=p;
		return -1;
	    }
	    newaa->InterpretString(&p);
	    add(newaa,aname,ParseType(&p));
	    aname=NULL;
	    continue;
	} else if(*p==')') {
	    p++;
	    *str=p;
	    return 0;
	} else if(*p=='"') {
	    flex str;
	    p++;
	    while(*p) {
		if(*p=='\\') *str.Append()=(char)parseBackslashed(&p);
		else if(*p=='"') {
		    p++;
		    break;
		} else {
		    *str.Append()=*p;
		    p++;
		}
	    }
	    *str.Append()='\0';
	    size_t dummy;
	    const char *res=str.GetBuf(0,str.GetN(),&dummy);
	    res=strdup((char *)res);
	    if(res==NULL) continue;
	    add(res,aname,ParseType(&p));
	    aname=NULL;
	} else if(isdigit(*p)) {
	    const char *d=p;
	    const char *dot=p;
	    while(*dot && !isspace(*dot) && *dot!='.' && *dot!=')') dot++;
	    while(*p && !isspace(*p) && *p!=':' && *p!=')') p++;
	    if(*dot=='.') add(atof(d),aname,ParseType(&p));
	    else add(atol(d),aname,ParseType(&p));
	    aname=NULL;
	} else p++;	    
    }
    *str=p;
    return -1;
}

#ifdef USE_OBSOLETE_AVALUEARGS_CLASS
avalueargs::avalueargs() {
}

avalueargs::avalueargs(long integer, const atom *name, const atom *type) {
    Append()->Set(integer, name, type);
}
avalueargs::avalueargs(double real, const atom *name, const atom *type) {
    Append()->Set(real, name, type);
}

avalueargs::avalueargs(const char *str, const atom *name, const atom *type) {
    Append()->Set(str, name, type);
}

avalueargs::avalueargs(void *ptr, const atom *name, const atom *type) {
    Append()->Set(ptr, name, type);
}

avalueargs::avalueargs(avalueflex *avf, const atom *name, const atom *type) {
    Append()->Set(avf, name, type);
}

avalueargs::avalueargs(const atom *a, const atom *name, const atom *type) {
    Append()->Set(a, name, type);
}

avalueargs::avalueargs(ATK *a, const atom *name) {
    Append()->Set(a, name);
}
#endif

avalueflex::avalueflex()  {
    deallocate=fdeallocate;
}

avalueflex::~avalueflex() {
}

avalueflex::avalueflex(ATK *a, const atom *name)  {
    deallocate=fdeallocate;
    Append()->Set(a, name);
}

avalueflex::avalueflex(long integer, const atom *name, const atom *type)  {
    deallocate=fdeallocate;
    Append()->Set(integer, name, type);
}
avalueflex::avalueflex(double real, const atom *name, const atom *type)  {
    deallocate=fdeallocate;
    Append()->Set(real, name, type);
}

avalueflex::avalueflex(const char *str, const atom *name, const atom *type)  {
    deallocate=fdeallocate;
    Append()->Set(str, name, type);
}

avalueflex::avalueflex(void *ptr, const atom *name, const atom *type)  {
    deallocate=fdeallocate;
    Append()->Set(ptr, name, type);
}

avalueflex::avalueflex(avalueflex *avf, const atom *name, const atom *type)  {
    deallocate=fdeallocate;
    Append()->Set(avf, name, type);
}

avalueflex::avalueflex(const atom *a, const atom *name, const atom *type)  {
    deallocate=fdeallocate;Append()->Set(a, name, type);
}

boolean avalueflex::TypeCheck(const avalueflex &argtypes) const {
    avalueflex_citer iter(argtypes);  // iterate over the argument types to be checked
    avalueflex_citer aiter(this);  // iterate over the arguments.
    while(!iter.done() && !aiter.done() && iter->Name()==avalue::none && aiter->Name()==avalue::none) {
	if(iter->Atom()!=aiter->Type()) {
	    if(!ATK::IsTypeByName(aiter->Type()->Name(), iter->Atom()->Name())) {
		fprintf(stderr, "Type mismatch between %s and %s.\n", aiter->Type()->Name(), iter->Atom()->Name());
		return FALSE;
	    }
	}
	iter++;
	aiter++;
    }
    while(!iter.done()) {
	if(iter->Name()==avalue::none) return FALSE;
	const avalue *val=Find(iter->Name());
	if(val && iter->Atom()!=val->Type()) {
	    if(!ATK::IsTypeByName(val->Type()->Name(), iter->Atom()->Name())) return FALSE;
	}
	iter++;
	
    }
    // If there are any more unnamed arguments after the ones mentioned in the argtypes list this isn't a match.
    while(!aiter.done()) {
	if(aiter->Name()==avalue::none) return FALSE;
	aiter++;
    }
    return TRUE;	
}
