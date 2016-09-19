/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *       Copyright Carnegie Mellon Univ. 1992,1993 - All Rights Reserved  *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("text.H")
#include <ctype.h>

#define CHECK_BE1

#include <attribs.h>
#ifdef CHECK_BE1
#include <be1be2.H>
#endif /* CHECK_BE1 */
#include <dataobject.H>
#include <dictionary.H>
#include <environ.H>	/* for datastream test only */
#include <environment.H>
#include <filetype.H>
#include <style.H>
#include <stylesheet.H>
#include <tabs.H>
#include <txtstvec.h>
#include <viewref.H>
#include <text.H>

#include <util.h>

#define MAXENVSTACK 100
#define TEXT_VIEWREFCHAR '\377'

#define DEFAULTDATASTREAMNUMBER 12
#define text_UNSET -100

#define MAX_QP_CHARS 76 /* The Quoted-Printable encoding REQUIRES that encoded lines be no more than 76 characters long */
#define LAST_QP_CHAR (MAX_QP_CHARS - 1)

static int stylesIncludeBeginning = text_UNSET;
static int stylesIncludeEnd = text_UNSET;

/* Place holder character for viewrefs */
/* All viewrefs contain this char, but the presence of this */
/* char does not necessarily denote a viewref */

struct environmentelement {
    class environment *environment;
    long pos;
};

/* When breaking up a line, trailing spaces (except the first one) */
/* are now written on the following line. When the new reader is */
/* generally distributed, the following define can be commented out */
/* and this redundant blank can be avoided */

#define WRITETRAILINGBLANK 1

/* various routines have environment stacks allocated dynamically
	when they are called.  Some functions (WrapStyles, 
	text_HandleKeyword, ...) assume these stacks exist and
	access them via envptr and envBegin.
	Any function using these variables must save and restore them.
*/
static struct environmentelement *envBegin = NULL;
static struct environmentelement *envptr = NULL;

static long HighBitStart = -1;


static int DataStreamVersion = 0;


ATKdefineRegistry(text, simpletext, NULL);
static void AddObj(class text  *self, class dataobject  *obj);
static void DelObj(class text  *self, class dataobject  *obj);
#if 0
static long text_ListObjects(class text  *self, class dataobject  **list, long  size);
#else /* !0 */
static long text_ListObjects(class text  *self, class dataobject  **list, long  size);
#endif /* !0 */
static void ClearStyles(class text  *self);
boolean DoReplaceCharacters(class text  *self, long  pos , long  len, const char  *repStr, long  repLen, boolean  alwaysp);
static int ParseInteger(FILE  *file,long  *id);
static boolean DiscardToEnddata(FILE  *file);
#ifdef CHECK_BE1
static boolean HasBinaryChars(class text  *self  /* (Other than viewrefs) */);
static void TryConversion(class text  *self);
#endif /* CHECK_BE1 */
static int StringMatch(class text  *self, long  pos, const char  *c);
static boolean TestForNoTemplate(class style  *style);
static void PutsRange(char  *p, FILE  *fp, char  *ep);
static char *WriteOutBuf(FILE  *file,char  *outbuf,char  *outp,char  *lastblank);
static void WrapStyle(class text  *self,class environment  *curenv,long  pos);
static void  CopySurroundingStyles(class text  *self, long  pos, class environment  *curenv);
static void PlayTabs(struct text_statevector  *sv , struct text_statevector  *oldsv, class style  * styleptr);
static void PlayStyle(struct text_statevector  *sv, class style  *styleptr);
char * WriteStyle(class environment  *env, char  *outp , int  IsOpen, char  *outbuf);
static void PushLevel(char  *s, int  pos , int  len , int  IsReal);
static char *PopLevel(int  *IsReal);
static void DeleteStyleNode(struct stk  *styleNode);
static char *WriteOutBufOther(FILE  *file, char  *outbuf, char  *outp);
static int ComingNext(class text  *self, int  pos);


static void AddObj(class text  *self, class dataobject  *obj)
{
    if(self->nobjs>=self->objssize) {
	long oldsize=self->objssize;
	self->objssize+=10;
	if(self->objs) self->objs=(class dataobject **)realloc(self->objs, sizeof(class dataobject *)*self->objssize);
	else self->objs=(class dataobject **)malloc(sizeof(class dataobject *)*self->objssize);
	if(self->objs==NULL) {
	    self->nobjs=0;
	    self->objssize=0;
	    return;
	}
	while(oldsize<self->objssize) {
	    self->objs[oldsize]=NULL;
	    oldsize++;
	}
    }
    (obj)->AddObserver( self);
    self->objs[self->nobjs]=obj;
    self->nobjs++;
}

static void DelObj(class text  *self, class dataobject  *obj)
{
    long i;
    for(i=0;i<self->nobjs;i++) {
	if(self->objs[i]==obj) break;
    }
    if(i>=self->nobjs) return;
    self->nobjs--;
    while(i<self->nobjs) {
	self->objs[i]=self->objs[i+1];
	i++;
    }
}

 class environment *text::AlwaysWrapViewChar(long  pos, const char  *viewtype, class dataobject  *dataobject)
{
    class viewref *newviewref;
    class environment *newenv;
    
    AddObj(this, dataobject);
    newviewref = viewref::Create(viewtype, dataobject);
    (newviewref)->AddObserver(this);
    newenv = (this->rootEnvironment)->InsertView( pos, newviewref, TRUE);
    (newenv)->SetLength( 1);
    (newenv)->SetStyle( FALSE, FALSE);
    return newenv;
}

text::text()
{
    this->objs=NULL;
    this->nobjs=0;
    this->objssize=0;
    this->executingGetModified = FALSE;
    this->rootEnvironment = environment::GetRootEnvironment();
    this->styleSheet = new stylesheet;
    styleSheet->AddObserver(this);
    this->currentViewreference = NULL;
    this->exportEnvs = TRUE;
    this->WriteAsText = FALSE;
    this->CopyAsText = FALSE;
    this->writeStyle = text_DefaultWrite;
    this->templateName = NULL;
#if 0
    if (DataStreamVersion == 0) {
        char *buf;
        if ((buf = environ::GetConfiguration("BE2TextFormat")) != NULL && *buf != '\0')
            DataStreamVersion = atoi(buf);
        if (DataStreamVersion < 10)
#endif /* 0 */
            DataStreamVersion = DEFAULTDATASTREAMNUMBER;
#if 0
    }
#endif /* 0 */
    if(stylesIncludeEnd == text_UNSET){
	stylesIncludeEnd = environ::GetProfileSwitch("stylesincludeend", TRUE);
	stylesIncludeBeginning = environ::GetProfileSwitch("stylesincludebeginning", FALSE);
    }
    THROWONFAILURE((TRUE));
}

text::~text()
{
    ClearStyles(this);
    delete this->rootEnvironment;
    if (this->styleSheet) {
	styleSheet->RemoveObserver(this);
	(this->styleSheet)->Destroy();
    }
    if (this->templateName != NULL)
        free(this->templateName);
    while(this->nobjs > 0)
	DelObj(this, this->objs[0]);
    free(this->objs);
}


void text::SetBaseTemplateName(const char  *name)
{
    if(this->templateName != NULL) free(this->templateName);
    if(name==NULL) this->templateName=NULL;
    else this->templateName=strdup(name);
}


void text::SetReadOnly(boolean ro) {
    struct attributes attrs;
    int i;
    if(this->GetReadOnly()==ro) return;
    this->simpletext::SetReadOnly(ro);
    attrs.key="readonly";
    attrs.value.integer=ro;
    attrs.next=NULL;
    for(i=0;i<nobjs;i++) {
	if(this->objs[i]) this->objs[i]->SetAttributes(&attrs);
    }	
}

/* 
 * Perhaps there should be some other condition for reading this template.
 * Otherwise maybe it should just store the template name away somewhere and
 * read it "when the time is right."
 */

void text::SetAttributes(struct attributes  *attributes)
{
    (this)->simpletext::SetAttributes( attributes);

    while (attributes) {
        if (strcmp(attributes->key, "template") == 0) {
            if (this->templateName != NULL)
                free(this->templateName);
            this->templateName = strdup(attributes->value.string);
            (this)->ReadTemplate( this->templateName, ((this)->GetLength() == 0)); 
	}
	else if (strcmp(attributes->key, "datastream") == 0) {
	    if (strcmp(attributes->value.string, "no") == 0) {
		(this)->SetWriteStyle( text_NoDataStream);
	    }
	    else if (strcmp(attributes->value.string, "yes") == 0) {
		(this)->SetWriteStyle( text_DataStream);
	    }
	}
    
	attributes = attributes->next;
    }
}

class viewref *text::InsertObject(long  pos, const char  *name, const char  *viewname)
{
    class dataobject *newobject;
    class environment *env;

    if((newobject = (class dataobject *) ATK::NewObject(name)))  {
        newobject->id = (newobject)->UniqueID(); 
        /* Register the object with the dictionary */
	dictionary::Insert(NULL, (char *) newobject->id, (char *) newobject);
        if (viewname == NULL || *viewname == '\0')
	    viewname = (newobject)->ViewName();
	/* the viewref will prevent the dataobject from being destroyed until the viewref is destroyed. */
	(newobject)->UnReference();
        if ((env = (this)->AddView( pos, viewname, newobject)))
            return env->data.viewref;
    }
    return NULL;
}


/* 
 * ListObjects is slightly sneaky.  If SIZE is 0, then the type of 
 * list should be (struct dataobject ***).  It is an output parameter 
 * that will be filled in with a pointer to a malloced array of 
 * pointers to children objects.  The number of children will be 
 * returned as usual.  In the unlikely event of a malloc or realloc 
 * failure, no array is returned, and the number of children reported 
 * is -1. 
 */
#if 0
static long text_ListObjects(class text  *self, class dataobject  **list, long  size)
{
    class dataobject *ob,**ptr,**cptr;
    class environment *rootenv;
    class environment *startenv;
    class environment *endenv;
    class environment *curenv;
    class environment *newenv;
    class environment *parentenv;
    long end;
    long i;
    long elen;
    int levels;
    long count = 0;
    long len =  (self)->GetLength();
    long pos = 0;
    boolean growYourOwn = (size == 0); /* grow an array ourselves? */
    class dataobject ***stash = (class dataobject ***)list;

    startenv = (self->rootEnvironment)->GetInnerMost( pos);
    endenv = (self->rootEnvironment)->GetInnerMost( pos + len - 1);
    rootenv = (startenv)->GetCommonParent( endenv);
    curenv = rootenv;

    if (growYourOwn) {
	size = 64;
	list = (class dataobject **)malloc(size *
					    sizeof(class dataobject **));
	if (list == NULL)
	    return -1;
	else
	    *stash = list;
    }

    while (curenv != startenv)  {
        curenv = (curenv)->GetChild( pos);
    }
    i = 0;
    end = len;
    cptr = list; 
    while (i < end)  {
        newenv = (self->rootEnvironment)->GetInnerMost( i);
        elen = (self->rootEnvironment)->GetNextChange( i);
        if (elen + i > end)
            elen = end - i;
        if (curenv != newenv)  {
            parentenv = (curenv)->GetCommonParent( newenv);
            levels = (curenv)->Distance( parentenv);
            while (levels > 0)  {
                levels--;
            }
            curenv = parentenv;
            if (curenv != newenv)  {
                class environment *stack[100];
                class environment *te;
                int level = 0;
                for (te = newenv; te != parentenv; te = (class environment *) te->parent)
                    stack[level++] = te;
                while (level > 0)  {
                    curenv = stack[--level];
                    if (curenv->type == environment_View)  
                        break;
                }
            }
        }
        if (curenv->type == environment_View)  {
            if((ob = curenv->data.viewref->dataObject) != NULL){
                for(ptr = list ; ptr < cptr; ptr++)
                    if (*ptr == ob) break;
                if(ptr == cptr) {
                    if(count == size)
			if (!growYourOwn)
			    return count;
			else {
			    size *= 2;
			    list = (class dataobject **)
			      realloc(list,size * sizeof(class dataobject **));
			    if (list == NULL)
				return -1;
			    else{
				*stash = list;
				cptr = list + count;
			    }
			}
		    *cptr++	= ob;
		    count++;
		}
            }
            i += curenv->length;
            elen = 0;
        }
        elen += i;
        i = elen;
    }
    if (count == 0 && growYourOwn)
	free(list);
    return count;
}
#else /* !0 */
static long text_ListObjects(class text  *self, class dataobject  **list, long  size)
{
    class dataobject ***stash=(class dataobject ***)list;
    boolean growYourOwn = (size == 0); /* grow an array ourselves? */

    if (growYourOwn) {
	if(self->nobjs==0) return 0;
	size = self->nobjs;
	list = (class dataobject **)malloc(size *
					    sizeof(class dataobject **));
	if (list == NULL)
	    return -1;
	else
	    *stash = list;
    } else if(size>self->nobjs) size=self->nobjs;

    while(--size>=0) {
	list[size]=self->objs[size];
    }
    return self->nobjs;
}
#endif /* !0 */

/* setStyleFlags sets the "includeBeginning/Ending" boolean flags of the passed environment.  If the style definition specifies them, those values are used.  If not, the stylesincludebeginning/ending preference settings are used.  (This function should ONLY be called with STYLE environments!) */
static void setStyleFlags(environment *env)
{
    style *sty= env->data.style;
    env->SetStyle( sty->IsIncludeBeginningAdded() ? TRUE : (sty->IsIncludeBeginningRemoved() ? FALSE : stylesIncludeBeginning),
		   sty->IsIncludeEndAdded() ? TRUE : (sty->IsIncludeEndRemoved() ? FALSE : stylesIncludeEnd));
}

static void ClearStyles(class text  *self)
{
    class environment *rt;
    
    class dataobject **dbuf, **d;
    long num;
    
    num = text_ListObjects(self, (class dataobject **)&dbuf, 0);
    
    if (num > 0) {
	for(d = dbuf; num--; d++) {
	    (*d)->RemoveObserver( self);
	}
	free(dbuf);
    }

    rt = self->rootEnvironment;
    self->rootEnvironment = NULL;
    (rt)->FreeTree();
    if(self->styleSheet) self->styleSheet->FreeStyles();
    self->rootEnvironment = environment::GetRootEnvironment();
    self->nobjs=0;
}

void text::Clear()
{
    ClearStyles(this);
    (this)->simpletext::Clear();
}

long text::GetModified()
{
    class dataobject **dbuf, **d;
    long maxSoFar;
    long num;

    if (this->executingGetModified)
	return 0;

    maxSoFar = (this)->simpletext::GetModified();
    num = text_ListObjects(this, (class dataobject **)&dbuf, 0);

    if (num > 0) {
	this->executingGetModified = TRUE;
	for(d = dbuf; num--; d++) {
	    int x;
	    x = (*d)->GetModified();
	    if (x > maxSoFar)
		maxSoFar = x;
	}
	free(dbuf);
	this->executingGetModified = FALSE;
    }

    return maxSoFar;
}

void text::ClearCompletely()
{
    ClearStyles(this);
    (this)->simpletext::Clear();
}

void text::LengthChanged(long  pos, long  len)
{
    (this)->simpletext::LengthChanged( pos, len);
    (this->rootEnvironment)->Update( pos, len);
}

boolean DoReplaceCharacters(class text  *self, long  pos , long  len, const char  *repStr, long  repLen, boolean  alwaysp)
{
    class environment *environment;

    if (alwaysp)
        (self)->simpletext::AlwaysReplaceCharacters( pos, len, repStr, repLen);
    else
        if (! (self)->simpletext::ReplaceCharacters( pos, len, repStr, repLen))
            return FALSE;

    environment = (class environment *)(self->rootEnvironment)->GetInnerMost( pos);

    if (((environment)->Eval() == pos) && ((environment)->GetNextChange( pos) == len))
        (environment)->SetLength( environment->length + repLen - len);

    return TRUE;
}

boolean text::ReplaceCharacters(long  pos , long  len, const char  *repStr, long  repLen)
{
    return DoReplaceCharacters(this, pos, len, repStr, repLen, FALSE);
}

void text::AlwaysReplaceCharacters(long  pos , long  len, const char  *repStr, long  repLen)
{
    DoReplaceCharacters(this, pos, len, repStr, repLen, TRUE);
}

	static environment *
DoAddView(text *self, long pos, 
			const char *viewtype, class dataobject *dataobject) {
	class viewref *newviewref;
	environment *newenv;

	AddObj(self, dataobject);
	newviewref = viewref::Create(viewtype, dataobject);
	(newviewref)->AddObserver(self);
	newenv = (self->rootEnvironment)->WrapView( pos, 
			1, newviewref);
	(newenv)->SetStyle(FALSE, FALSE);
	(self)->RegionModified(pos, 1);
	(self)->SetModified();
	return newenv;
}

	environment * 
text::ReplaceWithView(long pos, long len, 
			const char *viewtype, class dataobject *dataobject) {
	char foo = TEXT_VIEWREFCHAR;
	if ( ! DoReplaceCharacters(this, pos, len, 
			&foo, 1, FALSE))
		return NULL;
	return DoAddView(this, pos, viewtype, dataobject);
}

	environment *
text::AlwaysReplaceWithView(long pos, long len, 
			const char *viewtype, class dataobject *dataobject) {
	char foo = TEXT_VIEWREFCHAR;
	DoReplaceCharacters(this, pos, len, &foo, 1, TRUE);
	return DoAddView(this, pos, viewtype, dataobject);
}



void text::AlwaysDeleteCharacters(long  pos, long  len)
{
    class environment *te;
    class environment *le;
    class environment *re;
    class environment *ne;
    long stylePos;
    long lPos;

    (this)->simpletext::AlwaysDeleteCharacters( pos, len);

    te = this->rootEnvironment;
    stylePos = pos;
    while (stylePos > 0) {
	re = (class environment *)(te)->GetChild( pos);
	if (re != NULL
	    && re->type == environment_Style) {
	    if ((lPos = (re)->Eval()) == pos) {
		le = (class environment *)(te)->GetChild( pos - 1);
		if (le != NULL
		    && le->type == environment_Style
		    && re->data.style == le->data.style
		    && ((lPos = (le)->Eval()) + (le)->GetLength()) == pos) {
		    /* merge together these two styles */

		    ne = (class environment *)(te)->Add( lPos, pos - lPos + (re)->GetLength());
		    if (ne != NULL) {
			ne->type = environment_Style;
			ne->data.style = le->data.style;
			setStyleFlags(ne);
			(le)->Delete();
			(re)->Delete();
			te = ne;
			stylePos = pos - lPos;
			continue;
		    }
		}
	    }
	    else {
		stylePos = pos - lPos;
		te = re;
		continue;
	    }
	}
	return;
    }
}

static int ParseInteger(FILE  *file,long  *id)
{
    int c;
    while ((c = getc(file)) != EOF && c != ',' && c != '}')
        if (c >= '0' && c <= '9')
            *id = *id * 10 + c - '0';
    return c;
}

static long viewpadding=(-1);
static long ViewPadding() {
    if(viewpadding<0) viewpadding=environ::GetProfileInt("ViewScrollHack", 1);
    return viewpadding;
}
    
long text::HandleKeyWord(long  pos, char  *keyword, FILE  *file)
{
    class environment *newenv;
    class style *stylep;

    static const char EOFerror[] =
      "EOF encountered while reading in a view marker or template name - ignoring\n";

    if (strcmp(keyword, "textdsversion") == 0)  {
        long versionnumber = 0;
        int c;

        while ((c = getc(file)) != EOF && c != '}')
            if (c >= '0' && c <= '9')
                versionnumber = versionnumber * 10 + (c - '0');

        while (c != EOF && (c = getc(file)) != '\n')
            ;

        ((class simpletext *) this)->Version = versionnumber;

        /* Handle outdated data stream versions here */

        return 0;
    }

    if (strcmp(keyword, "view") == 0) {
        /* Parse the view name, the dataobject name, and inset id. */

        class dataobject *mydataobject;
        char viewname[200];
        long viewid;
        long objectid,desw,desh;
        unsigned int i;
        class viewref *newviewref;
        int c;

        i = 0;
        while ((c = getc(file)) != EOF && c != ',')
            if (i < sizeof (viewname) - 1)
                viewname[i++] = c;
        viewname[i] = '\0';
        if (c == EOF) {
            fprintf(stderr, EOFerror);
            return -1;
        }

        objectid = 0;
        c = ParseInteger(file, &objectid);
        if (c == EOF || c == '}') {
            fprintf(stderr, EOFerror);
            return -1;
        }

        viewid = 0;
        c = ParseInteger(file, &viewid);
        if (c == EOF) {
            fprintf(stderr, EOFerror);
            return -1;
        }

        desw = desh = 0;

        if (c == ',') {
            /* New format with desired view size saved */

            if ((c = ParseInteger(file, &desw)) == EOF) {
                fprintf(stderr, EOFerror);
                return -1;
            }	

            if (c == ',' && ((c = ParseInteger(file, &desh)) == EOF)) {
                fprintf(stderr, EOFerror);
                return -1;
            }

            if (c == ',') {
                long junk;  /* Eat up any future args that may be added */

                while ((c = ParseInteger(file, &junk)) != EOF) {
                    if (c == '}')
                        break;
                    junk = 0;
                }

                if (c == EOF) {
                    fprintf(stderr, EOFerror);
                    return -1;
                }	
            }
        }
	if((this)->GetObjectInsertionFlag() == FALSE){
	    char bb[512];
	    long ll;
	    sprintf(bb,"[A %s VIEW WAS HERE]",viewname);
	    ll = strlen(bb);
	    (this)->InsertCharacters( pos, bb, ll);
	    return ll;
	}

        mydataobject = (class dataobject *) dictionary::LookUp(NULL, (char *) objectid);
        /* No dataobject for this view; it may have never existed or */
        /* maybe the dataobject could not be found. */
        if (mydataobject == NULL)
	    return 0;
	AddObj(this, mydataobject);
        newviewref = viewref::Create(viewname, mydataobject);
        (newviewref)->AddObserver( this);
        newviewref->desw = desw;
        newviewref->desh = desh;
	newenv = (envptr->environment)->InsertView( pos - envptr->pos, newviewref, TRUE);
	for(i=0;i<ViewPadding();i++) {
	    (this)->AddInCharacter(pos+i, TEXT_VIEWREFCHAR);
	}
        (newenv)->SetLength(viewpadding);
        (newenv)->SetStyle( FALSE, FALSE);

        return viewpadding;   /* Added n chars for viewref... */
    }

    if (strcmp(keyword, "define") == 0) {
        (this->styleSheet)->Read( file, 0);
        return 0;
    }

    if (strcmp(keyword, "template") == 0) {
        char templatename[200];
        int i;
        int c;

        i = 0;
        while ((c = getc(file)) != EOF && c != '}')
            templatename[i++] = c;
        templatename[i] = '\0';
        if (c == EOF)  {
            fprintf(stderr, EOFerror);
            return -1;
        }

        if ((c = getc(file)) != EOF && c == '\n')
	    {}

        (this)->ReadTemplate( templatename, FALSE);

        return 0;
    }
    if( *keyword == '^' && keyword[1] == '\0'){
	/* Note that the high order bit needs set for the following characters
	    This works because these are always the inner-most brackets */
	HighBitStart = pos;
	return(0);
    }
    else HighBitStart = -1; /* just to be safe */

    /* Assume Style keyword: */
    /* Insert an environment with a yet undetermined length, */
    /* and push it on the stack pending an end brace. */

    stylep = (this->styleSheet)->Find( keyword);
    if (stylep == NULL)  {
        stylep = new style;
        (stylep)->SetName( keyword);
        (this->styleSheet)->Add( stylep); 
    }

    newenv = (envptr->environment)->InsertStyle( pos - envptr->pos, stylep, TRUE);
    envptr++;
    envptr->environment = newenv;
    envptr->pos = pos;

    return 0;
}

long text::HandleCloseBrace(long  pos, FILE  *file)
{
    if(HighBitStart != -1){
	unsigned char *foo;
	long rl;
	foo =(unsigned char *) (this)->GetBuf(HighBitStart,pos - HighBitStart,&rl);
	while(rl-- > 0)
	    *foo++ |= '\200';
	HighBitStart = -1;
	return(0);
    }
	
    if (envptr != envBegin) {
        long len = pos - envptr->pos;
	if (len > 0){
	    (envptr->environment)->SetLength( len);
	    if (envptr->environment->type == environment_Style)
		setStyleFlags(envptr->environment);
	}
        else
            (envptr->environment)->Delete();
        envptr--;
        return 0;
    } else {
        /* Extra close-braces */
        (this)->AddInCharacter( pos, '}');
        return 1;
    }
}

class environment *text::AlwaysAddStyle(long  pos , long  len, class style  *stylep)
{
    class environment *newenv;

    if ((newenv = (this->rootEnvironment)->WrapStyle( pos, len, stylep)) != NULL) {
	setStyleFlags(newenv);
        (this)->RegionModified( pos, len);
        (this)->SetModified();
    }

    return newenv;
}

class environment *text::AddStyle(long  pos, long  len, class style  *stylep)
{
    if ((this)->GetReadOnly() || pos < (this)->GetFence())
        return NULL;
    else
        return (this)->AlwaysAddStyle( pos, len, stylep);
}

class environment *text::AlwaysAddView(long  pos, const char  *viewtype, class dataobject  *dataobject)
{
    char c = TEXT_VIEWREFCHAR;
    (this)->AlwaysInsertCharacters(pos, &c, 1);
    return DoAddView(this, pos, viewtype, dataobject);
}

class environment *text::AddView(long  pos, const char  *viewtype, class dataobject  *dataobject)
{
    if ((this)->GetReadOnly() || pos < (this)->GetFence())
        return NULL;
    else
        return (this)->AlwaysAddView( pos, viewtype, dataobject);
}

/*
 * Assuming a \begindata has been read, discards up to and including
 * a matching \enddata (discards internal levels of \begindata ... \enddata).
 * Returns FALSE if EOF is reached before the \enddata.
 * Something better needs to be done about this.
 */

static boolean DiscardToEnddata(FILE  *file)
{
    unsigned int i;
    int c;
    char buf[20];
trymore:
    do {
        if ((c = getc(file)) == EOF)
            return FALSE;
    } while (c != '\\');

    i = 0;
    while (1) {     /* Read possible keyword */
        if ((c = getc(file)) == EOF)
            return FALSE;
        if (i == 0 && (c == '\\' || c == '}' || c == '}'))
            goto trymore;   /* Just a quoted char */
        if (c == '{')       /* End of keyword */
            break;
        if (i < sizeof (buf) - 1)
            buf[i++] = c;
    }
    buf[i] = '\0';
    do {
        if ((c = getc(file)) == EOF)
            return FALSE;
    } while (c != '}');
    /* If it's a begindata, recurse to discard subobject */
    if (strcmp(buf, "begindata") == 0) {
        if (DiscardToEnddata(file) == FALSE)
            return FALSE;
        goto trymore;
    }
    if (strcmp(buf, "enddata") != 0)
        goto trymore;
    return TRUE;
}


long text::AlwaysInsertFile(FILE  *file, const char  *filename, long  position)
{
    char *objectName;
    long objectID;
    int myfile = 0;
    int length = 0;
    if (file == NULL) {
        if (filename != NULL && ((file = fopen(filename,"r")) != NULL))
            myfile++;
        else
            return 0;
    }
    objectName = filetype::Lookup(file, filename, &objectID, NULL);
    if (objectName)
	/* load the class, to make sure ATK::IsTypeByName doesn't lie due to ignorance */
	ATK::LoadClass(objectName);
    if (objectName != NULL && !ATK::IsTypeByName(objectName, "text"))  {
        class dataobject *dat;
	if((this)->GetObjectInsertionFlag() == FALSE){
	    /* ignore non-text object */
	    char bb[512];
	    long ll;
	    fprintf(stderr,
		    "Insertion of objects not allowed, ignoring %s!\n",objectName);
	    DiscardToEnddata(file);
	    sprintf(bb,"[A %s OBJECT WAS INSERTED HERE]",objectName);
	    ll = strlen(bb);
	    (this)->AlwaysInsertCharacters( position, bb, ll);
	    length = ll;
	}
	else {
	    dat = (class dataobject *) ATK::NewObject(objectName);
	    if (dat == NULL) {
		fprintf(stderr,
			"Text: Can't find routines for object '%s'; ignoring!\n", objectName);
		DiscardToEnddata(file);
		length = 0;
	    } else {
		struct attributes attr;
		attr.key="readonly";
		attr.next=NULL;
		if(this->GetReadOnly()) {
		    attr.value.integer=1;
		} else {
		    attr.value.integer=0;
		}
		(dat)->Read( file, objectID);
		(dat)->SetAttributes(&attr);
		dictionary::Insert(NULL, (char *) objectID, (char *) (dat)->UniqueID());
		/* the viewref will prevent the dataobject from being destroyed until the viewref is destroyed. */
		(dat)->UnReference();
		(this)->AlwaysAddView( position, (dat)->ViewName(), dat);
		length = 1;
	    }
	}
    } else {
        boolean wasReadOnly;
	long oldfence = (this)->GetFence();
	wasReadOnly = (this)->GetReadOnly();
        /* ReadSubString checks read-only, making this ugliness necessary. */
	if(wasReadOnly){
	    (this)->SetReadOnly( FALSE);
	    length = (this)->ReadSubString( position, file, objectID > 0);
	    (this)->SetReadOnly( wasReadOnly);
	}
	else if( position < oldfence){
	    /* reset the fence properly */
	    (this)->SetFence(0);
	    length = (this)->ReadSubString( position, file, objectID > 0);
	    (this)->SetFence(oldfence + length);
	}
	else 
	    length = (this)->ReadSubString( position, file, objectID > 0);
    }

    if (myfile)
        fclose(file);
    return length;
}


long text::InsertFile(FILE  *file, const char  *filename, long  position)
{
    if ((this)->GetReadOnly() || position < (this)->GetFence())
        return 0;
    else
        return (this)->AlwaysInsertFile( file, filename, position);
}

#ifdef CHECK_BE1

static boolean HasBinaryChars(class text  *self  /* (Other than viewrefs) */)
{
    long pos = 0;
    while (pos < (self)->GetLength()) {
        long gotlen;
        unsigned char *p =
          (unsigned char *) (self)->GetBuf( pos, 1024, &gotlen);
        while (gotlen--) {
            if (*p == (unsigned char) TEXT_VIEWREFCHAR) {
                class environment *env =(class environment *)
                  (self->rootEnvironment)->GetInnerMost( pos);
                if (env == NULL || env->type != environment_View)
                    return TRUE;
            } else if (*p & 0x80)
                return TRUE;
            pos++;
            p++;
        }
    }
    return FALSE;
}

static void TryConversion(class text  *self)
{
/*    fprintf(stderr, "File contains nonascii characters\n"); */
    if (ATK::LoadClass("be1be2") == NULL) {
        fprintf(stderr, "Be1be2 not found; skipping BE1 check\n");
        return;
    }
    if (be1be2::CheckBE1(self) == FALSE)
        return;
    fprintf(stderr, "Converting BE1 file\n");
    be1be2::Convert(self);
}

#endif /* CHECK_BE1 */

long text::Read(FILE  *file, long  id)
{
    int retval;
    ClearStyles(this);
    if (this->templateName != NULL)
        (this)->ReadTemplate( this->templateName, FALSE); 
    retval = (this)->simpletext::Read( file, id);

#ifdef CHECK_BE1
    if (retval == dataobject_NOREADERROR)
        if (HasBinaryChars(this))
            TryConversion(this);
#endif /* CHECK_BE1 */

    if(this->styleSheet) {
	class style *global=this->styleSheet->Find("global");
	if(global) this->SetGlobalStyle(global);
    }
    return retval;
}

static int StringMatch(class text  *self, long  pos, const char  *c)
{
    /* Tests if the text begins with the given string */
    while (*c != '\0') {
        if ((self)->GetChar( pos) != *c)
            return FALSE;
        pos++; c++;
    }
    return TRUE;
}

static boolean TestForNoTemplate(class style  *stylep)
{
    return ! stylep->template_c;
}

long text::Write(FILE  *file, long  writeID, int  level)
{
    boolean quoteCharacters = FALSE;

    /* Determine when to use datastream format (quoteCharacters TRUE) */

    if (level != 0) /* Text object is a child */
        quoteCharacters = TRUE;

    if (this->exportEnvs) {
        if ((this->rootEnvironment)->NumberOfChildren() > 0)
	    quoteCharacters = TRUE; /* There's at least one style */
	else if ((this->styleSheet)->EnumerateStyles( (stylesheet_efptr) TestForNoTemplate, 0) != NULL) {
	    quoteCharacters = TRUE;
	}
    }

    if (StringMatch(this, 0, "\\begindata"))
        quoteCharacters = TRUE;

    switch ((this)->GetWriteStyle()) {
	case text_NoDataStream:
	    quoteCharacters = FALSE;
	    break;
	case text_DataStream:
	    quoteCharacters = TRUE;
	    break;
	case text_DefaultWrite: /* tjm: FIXME: what should this do? */
	    break;
    }

    if (this->writeID != writeID)  {
        if (quoteCharacters) {
            this->writeID = writeID;
	    fprintf(file, "\\begindata{%s,%ld}\n", 		
		    (this->WriteAsText)?"text": (this)->GetTypeName(),
		    this->UniqueID());
            fprintf(file, "\\textdsversion{%d}\n", DataStreamVersion);
            if (this->styleSheet->templateName)
                fprintf(file, "\\template{%s}\n", this->styleSheet->templateName);
            (this->styleSheet)->Write( file);
            (this)->WriteSubString( 0, (this)->GetLength(), file, quoteCharacters);
	    fprintf(file, "\\enddata{%s,%ld}\n",
		    (this->WriteAsText)?"text": (this)->GetTypeName(),
		    this->id);
            fflush(file);
        }
        else
            (this)->simpletext::Write( file, writeID, level);
    }
    return this->id;
}

long text::ReadSubString(long  pos, FILE  *file, boolean  quoteCharacters)
{
    struct environmentelement environmentStack[MAXENVSTACK];
    struct environmentelement *lastEnvBegin = envBegin;
    struct environmentelement *lastEnvptr = envptr;
    class environment *rootenv;
    long len;

    envptr = environmentStack;
    envBegin = environmentStack;

    rootenv = (class environment *)(this->rootEnvironment)->GetEnclosing( pos);
    envptr->environment = rootenv;
    envptr->pos = (rootenv)->Eval();
    HighBitStart = -1 ; 

    len = (this)->simpletext::ReadSubString( pos, file, quoteCharacters);
    if (envptr != environmentStack)  {
        fprintf(stderr, "All environments not closed. - Closing them by default\n");

        while (envptr != environmentStack)  {
            (envptr->environment)->SetLength( pos + len - envptr->pos);
            envptr--;
        }
    }

    envBegin = lastEnvBegin;
    envptr = lastEnvptr;

    return len;
}

static void PutsRange(char  *p, FILE  *fp, char  *ep)
{
    while (p < ep)
        putc(*p++, fp);
}

static char *WriteOutBuf(FILE  *file,char  *outbuf,char  *outp,char  *lastblank)
{
    char blankchar,*temp;
    if(lastblank == NULL || lastblank == outbuf) {
        lastblank = outp;
	blankchar = '\\';
    }
    else if (lastblank>outp) { /* we're right at the end of the paragraph anyway, so don't output a backslash, don't output a space, don't output ANYTHING -RSK*/
	lastblank= outp;
	blankchar= 0;
    }
    else blankchar= *lastblank;
    PutsRange(outbuf, file, lastblank);
    lastblank++;
#ifdef WRITETRAILINGBLANK
    if (blankchar) putc(blankchar,file);
#else /* WRITETRAILINGBLANK */
    if (blankchar && blankchar != ' ')  putc(blankchar,file);
#endif /* WRITETRAILINGBLANK */
    putc('\n',file);
    for(temp = outbuf; lastblank < outp; lastblank++)
	*temp++ = *lastblank;
    return temp;
}

/*
 * This is now always writing version 12.  Version 11 code has been
 * removed to save space.  It could be retrieved if necessary.
 */

 void text::WriteSubString(long  pos, long  len, FILE  *file, boolean quoteCharacters)
{
    class environment *rootenv;
    class environment *startenv;
    class environment *endenv;
    class environment *curenv;
    class environment *newenv;
    class environment *parentenv;
    int lastblankset = FALSE;
    long end;
    long i;
    long elen;
    int levels;
    char c;
    long envpos;
    int  realcount;
    char outbuf[120],*outp,*endp,*temp,*lastblank;
    char *buf = NULL;
    long bufLen;
    boolean writinghighbit;
    int prevblankset=FALSE;
    endp = outbuf + LAST_QP_CHAR;
    outp = outbuf;lastblank = NULL; 

    if (len <= 0 ) return;

    if(!quoteCharacters){
        (this)->simpletext::WriteSubString(pos,len,file,FALSE);
        return;
    }

    startenv = (class environment *)(this->rootEnvironment)->GetInnerMost( pos);
    endenv = (class environment *)(this->rootEnvironment)->GetInnerMost( pos + len - 1);
    rootenv = (class environment *)(startenv)->GetCommonParent( endenv);

    for (envpos = (rootenv)->Eval(); pos == envpos && pos + len == envpos + rootenv->length; rootenv = (class environment *) rootenv->parent, envpos = (rootenv)->Eval());

    curenv = rootenv;
    realcount = 1;
    while (curenv != startenv)  {
	curenv = (class environment *)(curenv)->GetChild( pos);
	if ((curenv->type == environment_Style && 
	     (outp + 2 + strlen(curenv->data.style->name) > endp))
	    || (curenv->type == environment_View && outp != outbuf)){
	    outp = WriteOutBuf(file,outbuf,outp,lastblank);
	}
	if (curenv->type == environment_Style){
	    *outp++ = '\\';
	    for(temp = curenv->data.style->name; *temp; temp++){
		*outp++ = *temp;
	    }
	    *outp++ = '{';
	}
    }

    i = pos;

    end = pos + len;
    while (i < end)  {
        newenv = (class environment *)(this->rootEnvironment)->GetInnerMost( i);
        elen = (this->rootEnvironment)->GetNextChange( i);
        if (elen + i > end)
            elen = end - i;
        if (curenv != newenv)  {
            parentenv = (class environment *)(curenv)->GetCommonParent( newenv);
            levels = (curenv)->Distance( parentenv);
            while (levels > 0)  {
                *outp++ = '}';
                levels--;
            }  
            curenv = parentenv;
            if (curenv != newenv)  {
                class environment *stack[100];
                class environment *te;
                int level = 0;

                for (te = newenv; te != parentenv; te = (class environment *) te->parent)
                    stack[level++] = te;
                while (level > 0)  {
                    curenv = stack[--level];
                    if ((curenv->type == environment_Style && 
                          (outp + 2 + strlen(curenv->data.style->name) > endp))
                         || (curenv->type == environment_View && outp != outbuf)){
                        outp = WriteOutBuf(file,outbuf,outp,lastblank);
                        lastblank = NULL;
                        lastblankset = FALSE;
                    }
                    if (curenv->type == environment_Style){
                        *outp++ = '\\';
                        for(temp = curenv->data.style->name; *temp; temp++){
			    *outp++ = *temp;
			    if(outp>=endp) {
				PutsRange(outbuf, file, outp);
				outp=outbuf;
			    }
                        }
                        *outp++ = '{';
                    }
                    else if (curenv->type == environment_View)  
                        break;
                }
            }
        }
        if (curenv->type == environment_View)  {
            if(outp != outbuf){
                /* flush out remaining cached characters */
                outp = WriteOutBuf(file,outbuf,outp,lastblank);
            }
            /*  code to write out view information */
            (curenv->data.viewref->dataObject )->Write(file,this->writeID,2);
            sprintf(outbuf,"\\view{%s,%ld,%ld,%ld,%ld", curenv->data.viewref->viewType, (curenv->data.viewref->dataObject)->UniqueID( ), curenv->data.viewref->viewID,curenv->data.viewref->desw, curenv->data.viewref->desh);
            while(*outp) outp++;
            i += curenv->length;
            elen = 0;
            /*	    realcount = outp - outbuf; */
        }
        elen += i;

        bufLen = 0;
	writinghighbit = FALSE;
        while (i < elen)  {
            /* 	    Code for writing out actual text
                */
            if (bufLen == 0)
                buf = (this)->GetBuf( i, 1024, &bufLen);
            bufLen--, c = *buf++;

	    if(((int)c & 128 )!= 0){
		if(!writinghighbit){
		    *outp++ = '\\';
		    *outp++ = '^';
		    *outp++ = '{';
		    writinghighbit = TRUE;
		}
		c = (char) (((int)c) & 127);
	    }
	    else{
		if(writinghighbit){
		    *outp++ = '}';
		    writinghighbit = FALSE;
		}
	    }

            if (c == '\\' || c == '{' || c == '}')
                *outp++ = '\\';

	    *outp = c;
	    prevblankset=lastblankset;
            if(c == ' ' || c == '\t'){
                if(lastblankset == FALSE){
                    lastblank = outp;
                    lastblankset = TRUE;
                }
            }
            else 
                lastblankset = FALSE;
            outp++;
	    if(c == '\n'){
		PutsRange(outbuf,file,outp-1);
		if(prevblankset) {
		    putc('\\', file);
		    putc('\n', file);
		}
		putc('\n', file);
                if(realcount ) putc('\n',file);
                outp = outbuf;
                lastblank = NULL;
                realcount = 0;
            }
            else if(outp > endp){
                char *cp;
		if ((bufLen ? (*buf) : GetChar(i+1))=='\n')
		    lastblank = outp+1; /* secret message for WriteOutBuf(). Tell it we're at the end of the paragraph and don't want a stupid backslash -RSK*/
                outp = WriteOutBuf(file,outbuf,outp,lastblank);
                lastblank = NULL;
                lastblankset = FALSE;
                for(cp = outbuf; cp < outp; cp++){
                    if(*cp == ' '){
                        if (cp == outbuf || *(cp - 1) != ' '){
                            lastblank = cp;
                            lastblankset = TRUE;
                        }
                    }
                    else lastblankset = FALSE;
                }
		if (outbuf == outp) realcount = 0;
            }
            else realcount++;
            i++;
        }
	if(writinghighbit){
	    *outp++ = '}';
	    writinghighbit = FALSE;
	}

    }

    /* flush out cached characters */
    if(outp != outbuf){
        *outp++ = '\\';
        *outp++ = '\n';
        PutsRange(outbuf,file,outp);
    }
    levels = (curenv)->Distance( rootenv);
    while (levels-- > 0)
        putc('}', file);
}

static void WrapStyle(class text  *self,class environment  *curenv,long  pos)
{
    class environment *newenv;
    if (curenv->type == environment_Style){
        /* New Style */
        class style *stylep;
        char *keyword = curenv->data.style->name;
        stylep = (self->styleSheet)->Find( keyword);
        if (stylep == NULL)  {
            stylep = new style;
            /* style_SetName(style, keyword); 		    -wjh */
            (curenv->data.style)->Copy( stylep);		/* -wjh */
            stylep->template_c = FALSE;			/* -wjh */
            (self->styleSheet)->Add( stylep); 
        }
	newenv = (envptr->environment)->InsertStyle( pos - envptr->pos, stylep, TRUE);
	envptr++;
	envptr->environment = newenv;
	envptr->pos = pos;
    }
    else {
        class viewref *newviewref;
        class environment *newenv;
        newviewref = viewref::Create(curenv->data.viewref->viewType, curenv->data.viewref->dataObject);
        newviewref->desw = curenv->data.viewref->desw;
        newviewref->desh = curenv->data.viewref->desh;
        (newviewref)->AddObserver(self);
        newenv = (envptr->environment)->InsertView( pos - envptr->pos, newviewref, TRUE);
	envptr++;
	envptr->environment = newenv;
	envptr->pos = pos;
    }
}

/* recurse up the tree and WrapStyles while unwinding.  This way, the
	outermost style is deepest in the stack
*/
	static void 
CopySurroundingStyles(class text  *self, long  pos, class environment  *curenv)
			{
	class environment *parent 
		= (class environment *)curenv->parent;
	if (parent == NULL) return;
	CopySurroundingStyles(self, pos, parent);
	WrapStyle(self, curenv, pos);
}

	
boolean text::CopyTextExactly(long  pos,class simpletext  *srctext,long  srcpos,long  len)
                    {
    if (pos >= (this)->GetFence()) {
	(this)->AlwaysCopyTextExactly(pos,srctext,srcpos,len);
	return TRUE;
    }
    else
        return FALSE;
}

void text::AlwaysCopyTextExactly(long  pos,class simpletext  *ssrctext,long  srcpos,long  len)
{
    class environment *startenv;
    class environment *curenv;
    class environment *newenv;
    class environment *dstrootenv;
    class environment *parentenv;
    long end,ll,dend;
    long i,j;
    long elen;
    static struct ATKregistryEntry  *SimpleText = NULL;		/* -wjh */
    struct environmentelement environmentStack[MAXENVSTACK];
    struct environmentelement *lastEnvBegin = envBegin;
    struct environmentelement *lastEnvptr = envptr;
    class text *srctext=NULL;
    
    if (SimpleText == NULL) 			/* -wjh */
	SimpleText = ATK::LoadClass("simpletext");	/* -wjh */

    if (len <= 0 || srcpos < 0)
        return;
    if((ssrctext)->GetLength() < srcpos + len)
        len = (ssrctext)->GetLength() - srcpos;

    if(this == ssrctext && srcpos + len > pos){
        class text *newtext = new text;
        /* there is probably a better way to do this, but for now
            this avoids a lot of problems */
        (newtext)->AlwaysCopyTextExactly(0,ssrctext,srcpos,len);
        (this)->AlwaysCopyTextExactly(pos,newtext,0,len);
        (newtext)->Destroy();
        return;
    }
    (this)->simpletext::AlwaysCopyText(pos,ssrctext,srcpos,len);

    if ((ssrctext)->ATKregistry() == SimpleText  || !ssrctext->IsType(&text_ATKregistry_))		/* -wjh */
	return;					/* -wjh */

    srctext=(class text *)ssrctext;

    /* loop through the environment tree of the source around the selected 
	text and copy styles and viewrefs to destination */

    /* the environmentStack is a stack of open environments in 
	the destination
    */
    envptr = environmentStack;
    envBegin = environmentStack;

    /*  The first entry in the stack is for the environment surrounding
	the destination point.
    */
    dstrootenv = (class environment *)(this->rootEnvironment)->GetEnclosing( pos);
    envptr->environment = dstrootenv;
    envptr->pos = (dstrootenv)->Eval();

    startenv = (class environment *)(srctext->rootEnvironment)->GetInnerMost( srcpos);
    
    /* put onto the environmentStack (via WrapStyles) a style for
	each style surrounding the beginning of the selected source text 
	(Older versions of this code did not go all the way out to the
	root, but went out only as far as the smallest style that surrounded
	the source text and stuck out at one end or the other.  This 
	meant that styles were not copied exactly and that if the text
	did not cross a style boundary, styles might not be copied at all.)
    */
    CopySurroundingStyles(this, pos, startenv);

    /* The guts of copying styles is here.  At each style change point:
	to close a style: an element is popped off environmentStack
	to open a style: an element is pushed on the environmentStack
    */
    curenv = startenv;
    i = srcpos;j = pos;
    end = srcpos + len;
    dend = pos + len;
    while (i < end)  {
        newenv = (class environment *)(srctext->rootEnvironment)->GetInnerMost( i);
        elen = (srctext->rootEnvironment)->GetNextChange( i);
        if (elen + i > end)
            elen = end - i;
        if (curenv != newenv)  {
            parentenv = (class environment *)(curenv)->GetCommonParent( newenv);
	    while (curenv != parentenv)  {
		long envlen = j - envptr->pos;
		if (len > 0)  {
		    (envptr->environment)->SetLength( envlen);
		    if (envptr->environment->type == environment_Style)
			setStyleFlags(envptr->environment);
		}
		else  {
		    (envptr->environment)->Delete();
		}
		envptr--;
		curenv = (class environment *) (curenv)->GetParent();
	    }

            if (curenv != newenv)  {
                class environment *stack[100];
                class environment *te;
                int level = 0;

                for (te = newenv; te != parentenv; te = (class environment *) te->parent)
                    stack[level++] = te;
                while (level > 0)  {
                    curenv = stack[--level];
                    ll = (curenv)->GetLength();
                    if( j + ll > dend ) ll = dend - j;
                    WrapStyle(this,curenv,j);
                }
            }
        }
        i += elen;
        j += elen;
    }

    /* Set length on the remaining environments in the stack */

    if (envptr != environmentStack)  {
        while (envptr != environmentStack)  {
            (envptr->environment)->SetLength( pos + len - envptr->pos);
            envptr--;
        }
    }
    envBegin = lastEnvBegin;
    envptr = lastEnvptr;
}

void text::AlwaysCopyText(long  pos,class simpletext  *src,long  srcpos,long  len)
{
    class text *srctext=(class text *)src;
    class environment *rootenv;
    class environment *startenv;
    class environment *endenv;
    class environment *curenv;
    class environment *newenv;
    class environment *dstrootenv;
    class environment *parentenv;
    long end,ll,dend;
    long i,j;
    long elen;
    long envpos;
    static struct ATKregistryEntry  *SimpleText = NULL;		/* -wjh */
    struct environmentelement environmentStack[MAXENVSTACK];
    struct environmentelement *lastEnvBegin = envBegin;
    struct environmentelement *lastEnvptr = envptr;

    if (SimpleText == NULL) 			/* -wjh */
	SimpleText = ATK::LoadClass("simpletext");	/* -wjh */

    if (len <= 0 || srcpos < 0 ||
         srcpos + len > (srctext)->GetLength())
        return;
    if(this == srctext && srcpos + len > pos){
        class text *newtext = new text;
        /* there is probably a better way to do this, but for now
            this avoids a lot of problems */
        (newtext)->AlwaysCopyText(0,srctext,srcpos,len);
        (this)->AlwaysCopyText(pos,newtext,0,len);
        (newtext)->Destroy();
        return;
    }
    (this)->simpletext::AlwaysCopyText(pos,srctext,srcpos,len);

    if ((srctext)->ATKregistry() == SimpleText)		/* -wjh */
	return;					/* -wjh */

    envptr = environmentStack;
    envBegin = environmentStack;

    dstrootenv = (class environment *)(this->rootEnvironment)->GetEnclosing( pos);
    envptr->environment = dstrootenv;
    envptr->pos = (dstrootenv)->Eval();

    /* loop through the environment tree and copy styles and viewrefs */
    if((srctext)->GetLength() < srcpos + len)
        len = (srctext)->GetLength() - srcpos;
    startenv = (class environment *)(srctext->rootEnvironment)->GetInnerMost( srcpos);
    endenv = (class environment *)(srctext->rootEnvironment)->GetInnerMost( srcpos + len - 1);
    rootenv = (class environment *)(startenv)->GetCommonParent( endenv);

    for (envpos = (rootenv)->Eval(); srcpos == envpos && srcpos + len == envpos + rootenv->length; rootenv = (class environment *) rootenv->parent, envpos = (rootenv)->Eval());

    curenv = rootenv;
    while (curenv != startenv)  {
        curenv = (class environment *)(curenv)->GetChild( srcpos);
        WrapStyle(this,curenv,pos);
    }
    i = srcpos;j = pos;
    end = srcpos + len;
    dend = pos + len;
    while (i < end)  {
        newenv = (class environment *)(srctext->rootEnvironment)->GetInnerMost( i);
        elen = (srctext->rootEnvironment)->GetNextChange( i);
        if (elen + i > end)
            elen = end - i;
        if (curenv != newenv)  {
            parentenv = (class environment *)(curenv)->GetCommonParent( newenv);
	    while (curenv != parentenv)  {
		long envlen = j - envptr->pos;
		if (len > 0)  {
		    (envptr->environment)->SetLength( envlen);
		    if (envptr->environment->type == environment_Style)
			setStyleFlags(envptr->environment);
		}
		else  {
		    (envptr->environment)->Delete();
		}
		envptr--;
		curenv = (class environment *) (curenv)->GetParent();
	    }

            if (curenv != newenv)  {
                class environment *stack[100];
                class environment *te;
                int level = 0;

                for (te = newenv; te != parentenv; te = (class environment *) te->parent)
                    stack[level++] = te;
                while (level > 0)  {
                    curenv = stack[--level];
                    ll = (curenv)->GetLength();
                    if( j + ll > dend ) ll = dend - j;
                    WrapStyle(this,curenv,j);
                }
            }
        }
        i += elen;
        j += elen;
    }

    /* Set length on the remaining environments in the stack */

    if (envptr != environmentStack)  {
        while (envptr != environmentStack)  {
            (envptr->environment)->SetLength( pos + len - envptr->pos);
            envptr--;
        }
    }
    envBegin = lastEnvBegin;
    envptr = lastEnvptr;
}

void text::SetEnvironmentStyle(class environment  *envptr, class style  *styleptr)
{
    if (envptr->type != environment_View) {
        envptr->data.style = styleptr;
        envptr->type = environment_Style;
        (this)->RegionModified( (envptr)->Eval(), envptr->length);
    }
    else fprintf(stderr, "Can't set environment style; wrong environment type.\n");
}

void text::SetGlobalStyle(class style  *styleptr)
{
    if (this->rootEnvironment->type != environment_View) {
        this->rootEnvironment->data.style = styleptr;
        this->rootEnvironment->type = environment_Style;
        (this)->RegionModified( (this->rootEnvironment)->Eval(), this->rootEnvironment->length);
    }
    else fprintf(stderr, "Can't set global style; wrong environment type.\n");
}

class style *text::GetGlobalStyle()
{
    return this->rootEnvironment->data.style;
}

/* New definition of environment state vector -- controls the formatting of text */

void text::InitStateVector(struct text_statevector  *sv)
{
    sv->CurLeftMargin = sv->CurRightMargin = sv->CurRightEdge =
      sv->CurLeftEdge = sv->CurTopMargin = sv->CurBottomMargin =
      sv->CurFontAttributes = sv->CurScriptMovement = 
      sv->CurIndentation = sv->CurSpacing = sv->CurSpread =
      sv->SpecialFlags = 0;
    sv->CurFontSize = 12;
    sv->CurJustification = style_LeftAndRightJustified;
    sv->CurFontFamily = "andysans";
    sv->CurCachedFont = NULL;
    sv->CurView = NULL;
    sv->CurColor = NULL;
    sv->CurBGColor = NULL;
    sv->tabs = tabs::Create();
}

void text::FinalizeStateVector(struct text_statevector  *sv)
{
    if(sv->tabs) {
	tabs::Death(sv->tabs);
	sv->tabs=NULL;
    } else {
	fprintf(stderr, "text: WARNING FinalizeStatevector called when sv->tabs==NULL.\n");
#ifdef DEBUG
	abort();
#endif
    }
}

static void PlayTabs(struct text_statevector  *sv , struct text_statevector  *oldsv, class style  * styleptr)
{
    /* Tab updating is defined as copying over all of the old tabs and then */
    /* applying the modifiers in the style to the new tabs. */

    long numTabChanges;
    struct tabentry **TabChangeArray;
    long i;

    if (oldsv->tabs)
	sv->tabs = oldsv->tabs;
    else
	sv->tabs = tabs::Create();

    (styleptr)->GetTabChangeList( &numTabChanges, &TabChangeArray);
     for (i = 0; i < numTabChanges; i++)
	sv->tabs = (sv->tabs)->ApplyChange( TabChangeArray[i]);
    if (TabChangeArray)
	free(TabChangeArray);
}

/* This routine takes a pointer to a state vector, a pointer and the */
/* style to use, and plays that style over the state vector. */
static void PlayStyle(struct text_statevector  *sv, class style  *styleptr)
{
    long delta;
    struct text_statevector oldvalues;
    char *color;

    /* Using new style system.  Store old values for use in calculations. */

    oldvalues = *sv;

    /* Font Faces */

    sv->CurFontAttributes |= styleptr->AddFontFaces;
    sv->CurFontAttributes &= styleptr->OutFontFaces;

    /* Font Sizes */

    switch (styleptr->FontSize.SizeBasis) {
        case style_PreviousFontSize:
            sv->CurFontSize += styleptr->FontSize.Operand;
            break;
        case style_ConstantFontSize:
            sv->CurFontSize = styleptr->FontSize.Operand;
            break;
        default:
            /* Illegal basis */
            break;
    }

    /* Font Family */

    if (styleptr->FontFamily)
        sv->CurFontFamily = styleptr->FontFamily;

    /* Left Margin */

    delta = styleptr->NewLeftMargin.DotCvtOperand;

    switch(styleptr->NewLeftMargin.MarginBasis) {
	case style_LeftMargin:
	case style_LeftEdge:
	    sv->CurLeftMargin = oldvalues.CurLeftMargin + delta;
	    break;
	case style_ConstantMargin:
	    sv->CurLeftMargin = delta;
	    break;
	case style_RightEdge:
	case style_RightMargin:
	    sv->CurLeftMargin = oldvalues.CurRightMargin + delta;
	    break;
	case style_TopMargin:
	case style_TopEdge:
        case style_BottomMargin:
        case style_BottomEdge:
            /* Top and bottom margins not yet implemented */
            break;
        case style_PreviousIndentation:
            sv->CurLeftMargin = oldvalues.CurIndentation + delta;
            break;
        default:
            /* Unknown margin op */
            break;
    }

    /* Right Margin */

    delta = styleptr->NewRightMargin.DotCvtOperand;

    switch(styleptr->NewRightMargin.MarginBasis) {
        case style_LeftMargin:
        case style_LeftEdge:
            sv->CurRightMargin = oldvalues.CurLeftMargin + delta;
            break;
        case style_ConstantMargin:
            sv->CurRightMargin = delta;
            break;
        case style_RightEdge:
        case style_RightMargin:
            sv->CurRightMargin = oldvalues.CurRightMargin + delta;
            break;
        case style_TopMargin:
        case style_TopEdge:
        case style_BottomMargin:
        case style_BottomEdge:
            /* Top and bottom margins not yet implemented */
            break;
        case style_PreviousIndentation:
            sv->CurRightMargin = oldvalues.CurIndentation + delta;
            break;
        default:
            /* Unknown margin op */
            break;
    }

    /* Indent */

    delta = styleptr->NewIndentation.DotCvtOperand;

    switch(styleptr->NewIndentation.MarginBasis) {
        case style_LeftMargin:
        case style_LeftEdge:
            sv->CurIndentation = delta;
            break;
        case style_ConstantMargin:
            sv->CurIndentation = delta - sv->CurLeftMargin;
            break;
        case style_RightEdge:
        case style_RightMargin:
            sv->CurIndentation = sv->CurRightMargin - sv->CurLeftMargin + delta;
            break;
        case style_TopMargin:
        case style_TopEdge:
        case style_BottomMargin:
        case style_BottomEdge:
            /* Top and bottom margins not yet implemented */
            break;
        case style_PreviousIndentation:
            sv->CurIndentation = oldvalues.CurIndentation + delta;
            break; 
        default:
            /* Unknown margin op */
            break;
    }

    /* Script movements */

    delta = styleptr->FontScript.DotCvtOperand;

    switch(styleptr->FontScript.ScriptBasis) {
        case style_PreviousScriptMovement:
            sv->CurScriptMovement = oldvalues.CurScriptMovement + delta;
            break;
        case style_ConstantScriptMovement:
            sv->CurScriptMovement = delta;
            break;
        default:
            /* Unknown script movement */
            break;
    }

    /* Justifications */

    switch(styleptr->NewJustification) {
        case style_PreviousJustification: break;
        case style_LeftJustified:
        case style_RightJustified:
        case style_Centered:
        case style_LeftAndRightJustified:
        case style_LeftThenRightJustified: 
            sv->CurJustification = styleptr->NewJustification;
            break;
        default:
            /* Unknown justification */
            break;
    }

    /* Interline spacing */

    delta = styleptr->NewInterlineSpacing.DotCvtOperand;

    switch(styleptr->NewInterlineSpacing.SpacingBasis) {
        case style_InterlineSpacing:
            sv->CurSpacing = oldvalues.CurSpacing + delta;
            break;
        case style_InterparagraphSpacing:
            sv->CurSpacing = oldvalues.CurSpread + delta;
            break;
        case style_ConstantSpacing:
            sv->CurSpacing = delta;
            break;
        case style_AboveSpacing:
        case style_BelowSpacing:
            /* Above and below spacing not yet implemented */
            break;
        default:
            /* Unknown interline spacing */
            break;
    }

    /* Interparagraph spacing (spread) */

    delta = styleptr->NewInterparagraphSpacing.DotCvtOperand;

    switch(styleptr->NewInterparagraphSpacing.SpacingBasis) {
        case style_InterlineSpacing:
            sv->CurSpread = oldvalues.CurSpacing + delta;
            break;
        case style_InterparagraphSpacing:
            sv->CurSpread = oldvalues.CurSpread + delta;
            break;
        case style_ConstantSpacing:
            sv->CurSpread = delta;
            break;
        case style_AboveSpacing:
        case style_BelowSpacing:
            /* Above and below spacing not yet supported */
            break;
        default:
            /* Unknown interparagraph spacing (spread) */
            break;
    }

    /* Miscellaneous flags */

    sv->SpecialFlags |= styleptr->AddMiscFlags;
    sv->SpecialFlags &= styleptr->OutMiscFlags;

    /* Color */

    if( ( color = (styleptr)->GetAttribute("color")) != NULL)
        sv->CurColor = color;
    
    if( ( color = (styleptr)->GetAttribute("bgcolor")) != NULL)
	sv->CurBGColor = color;

    /* Tabs */
    PlayTabs(sv, &oldvalues, styleptr);
}

/* Takes a pointer to a state vector, and a pointer to an */
/* environment, and applies the environment changes to the */
/* state vector, in the right order
  The state vector must be initialized.*/

void text::ApplyEnvironment(struct text_statevector  *sv, class style  *defaultStyle, class environment  *env)
{
    if (env == NULL) {
        if (defaultStyle != NULL)
            PlayStyle(sv, defaultStyle);
        return;
    }

    text::ApplyEnvironment(sv, defaultStyle,
      (class environment *) env->parent);

    if ((env->type == environment_Style) && (env->data.style != NULL))
        PlayStyle(sv, env->data.style);
}

void text::ApplyEnvironmentsToStyle(environment *env, style *dest) {
    if(env->GetParent()!=NULL) ApplyEnvironmentsToStyle((environment *)env->GetParent(), dest);
    if(env->type!=environment_Style || env->data.style==NULL) return;
    env->data.style->MergeInto(dest);
}

class viewref *text::FindViewreference(long  pos , long  len)
{
    while (len > 0) {
        long gotlen;
        char *s = (this)->GetBuf( pos, len, &gotlen);
	if(s == NULL) break;
        for (len -= gotlen; gotlen--; pos++)
            if (*s++ == TEXT_VIEWREFCHAR) {
                class environment *curenv =(class environment *) (this->rootEnvironment)->GetInnerMost( pos);
                if (curenv && curenv->type == environment_View)  
                    return curenv->data.viewref;
            }
    }
    return NULL;
}

static boolean remap(long rock, text *self, long pos, environment *curenv) {
    if(self->styleSheet && curenv->type==environment_Style) {
	style *newstyle=NULL;
	boolean replace=self->styleSheet->FindReplacement(curenv->data.style, &newstyle);
	if(replace) {
	    if(newstyle) curenv->data.style=newstyle;
	    else {
		style *n=new style;
		curenv->data.style->Copy(n);
		curenv->data.style=n;
		self->styleSheet->Add(n);
	    }
	}
    }
    return FALSE;
}

void text::ObservedChanged (class observable  *changed, long  value)
{
    long pos, len;
    static struct ATKregistryEntry  *vci=NULL;
    
    if (this->rootEnvironment == NULL)
        return;

    if (changed == (class observable *) this->currentViewreference && value == observable_OBJECTDESTROYED)
        this->currentViewreference = NULL;

    pos = 0;
    len = (this)->GetLength();
    if(changed==styleSheet) {
	if(value==observable_OBJECTCHANGED) {
	    remap(0, this, 0, this->rootEnvironment);
	    EnumerateEnvironments(0, rootEnvironment->GetLength(), remap, 0);
	}
	return;
    }
    if(vci==NULL) vci=ATK::LoadClass("viewref");
    if(value==observable_OBJECTDESTROYED && (class observable *)this!=changed) {
	if((changed)->ATKregistry()!=vci) {
	    DelObj(this, (class dataobject *)changed);
	}
    }
    // if we get an object changed notification for something other than
    // the viewref just call NotifyObservers, we wouldn't do anything
    // different based on which dataobject it was...
    // (but we might have skipped the notify observers if it wasn't one
    // of the dataobjects... the NotifyObservers call in this case will
    // almost certainly be harmless.)
    if(value==observable_OBJECTCHANGED && changed->ATKregistry()!=vci) {
        NotifyObservers(0);
        return;
    }
    while (len > 0) {
        long gotlen;
        char *s = (this)->GetBuf( pos, len, &gotlen);
        for (len -= gotlen; gotlen--; pos++)
            if (*s++ == TEXT_VIEWREFCHAR) {
                class environment *curenv =(class environment *) (this->rootEnvironment)->GetInnerMost( pos);
		if (curenv && curenv->type == environment_View) {
		    if(changed == (class observable *) curenv->data.viewref) {
			if(value == observable_OBJECTDESTROYED) {
			    curenv->data.viewref = NULL;
			    (this)->AlwaysDeleteCharacters( pos, 1);
                        } else {
                            SetModified();
			    (this)->RegionModified( pos, 1);
			}
			(this)->NotifyObservers( 0);
			return;
                    }
#if 0
                    // this is now handled in the catchall above
                    // for all non-viewref 'changed' objects.
		    if(changed == (class observable *)curenv->data.viewref->dataObject && value!=observable_OBJECTDESTROYED) {

			/* text_RegionModified(self, pos, 1); */
			(this)->NotifyObservers( 0);
			return;
                    }
#endif
		}
	    }
    }
}
 class environment *text::EnumerateEnvironments(long  pos,long  len,text_eefptr callBack,long  rock)
{   /* calls callback(rock,self,current_pos,env) on each environment found
      starting at pos and going len characters
      if callback returns TRUE .text__EnumerateEnvironment
        returns the current environment  */
    class environment *rootenv;
    class environment *startenv;
    class environment *endenv;
    class environment *curenv;
    class environment *newenv;
    class environment *parentenv;
    long end;
    long i;
    long elen;
    long envpos;  

    startenv = (class environment *)(this->rootEnvironment)->GetInnerMost( pos);
    endenv = (class environment *)(this->rootEnvironment)->GetInnerMost( pos + len - 1);
    rootenv = (class environment *)(startenv)->GetCommonParent( endenv);

    for (envpos = (rootenv)->Eval(); pos == envpos && pos + len == envpos + rootenv->length && rootenv->parent; rootenv = (class environment *) rootenv->parent, envpos = (rootenv)->Eval());

    curenv = rootenv;
    while (curenv != startenv)  {
	curenv = (class environment *)(curenv)->GetChild( pos);
	    if((*(callBack))(rock,this,pos,curenv))
		return curenv;
    }
    i = pos;
    end = pos + len;
    while (i < end)  {
        newenv = (class environment *)(this->rootEnvironment)->GetInnerMost( i);
        elen = (this->rootEnvironment)->GetNextChange( i);
        if (elen + i > end)
            elen = end - i;
        if (curenv != newenv)  {
            parentenv = (class environment *)(curenv)->GetCommonParent( newenv);
/*            levels = environment_Distance(curenv, parentenv); */
            curenv = parentenv;
            if (curenv != newenv)  {
                class environment *stack[100];
                class environment *te;
                int level = 0;

                for (te = newenv; te != parentenv; te = (class environment *) te->parent)
                    stack[level++] = te;
                while (level > 0)  {
                    curenv = stack[--level];
		    if((*(callBack))(rock,this,i,curenv))
			return curenv;
                }
            }
	}
	i += elen;
    }
    return NULL;
}

static struct stk {
    char *item;
    int IsReal;
    struct stk *next, *prev;
    int pos, len;
} *Top = NULL;


const char * const TranslateStyleFrom[] = {
    "typewriter",
    "quotation",
    "excerptedcaption",
    "signature",
    "footnote",
    NULL
};

const char * const TranslateStyleTo[] = {
    "fixed",
    "excerpt",
    "bold,excerpt",
    "smaller",
    "",
    NULL
};

char *
WriteStyle(class environment  *env, char  *outp , int  IsOpen, char  *outbuf)
{
    const char *name = env->data.style->name;
    char *temp, *s, *p, *comma, *dum, negation[50];
    int i, IsReal=1, len;

    for (i=0; TranslateStyleFrom[i] != NULL; ++i) {
	if (!strcmp(name, TranslateStyleFrom[i])) {
	    name = TranslateStyleTo[i];
	    if(*name == (char)0) 
		return(outp);
	    break;
	}
    }
    if((s = p = strdup(name))) {
	while (s) {
	    comma = (char *) strchr(s, ',');
	    if (comma) *comma = (char)0;
	    sprintf(negation, "</%s>", s);
	    len = strlen(negation);
	    dum = outp-len + 1;
	    if ((outp > outbuf) && !strncmp(dum, negation, len)) {
	    /* This is just a case of </bold><bold>, which we optimize away in the output stream when it is really easy to do so, in order to make the richtext more readable */
		outp = dum;
	    } else {
		*outp++ = '<';
		for(temp = s; *temp; temp++){
		    *outp++ = *temp;
		}
		*outp++ = '>';
	    }
	    PushLevel(s, (env)->Eval(), env->length, IsReal);
	    IsReal = 0;
	    if (comma) {
		*comma = ',';
		s = ++comma;
	    } else s = NULL;
	}
	free(p);
    }
    return(outp);
}

static void PushLevel(char  *s, int  pos , int  len , int  IsReal)
{
    struct stk *tmp = (struct stk *) malloc(sizeof(struct stk));
    char *cp = strdup(s);
    tmp->item = cp;
    tmp->IsReal = IsReal;
    tmp->pos = pos;
    tmp->len = len;
    tmp->next = Top;
    tmp->prev = NULL;
    if (Top) Top->prev = tmp;
    Top = tmp;
}

static char *PopLevel(int  *IsReal)
{
    char *s;
    struct stk *tmp = Top;

    if (!Top) return(NULL);
    s = Top->item;
    *IsReal = Top->IsReal;
    Top = Top->next;
    if (Top) Top->prev = NULL;
    free(tmp);
    return(s);
}

/* This routine removes a styleNode from the list of embedded styles */
static void DeleteStyleNode(struct stk  *styleNode)
{
    if(styleNode->prev)
	styleNode->prev->next = styleNode->next;
    else Top = NULL;
    if(styleNode->next)
	styleNode->next->prev = styleNode->prev;
    if(styleNode->item)
	free(styleNode->item);
    free(styleNode);
}

static char *WriteOutBufOther(FILE  *file, char  *outbuf, char  *outp)
{
    char *savedp, *endp, *new_endp = NULL;
    boolean UglyChop = FALSE;

    /* set endp to point to the character which should begin the next line.  Need to be sure UglyChop is not in the middle of an =xx encoding. */

    savedp = endp = outp;
    if (outp - outbuf >= MAX_QP_CHARS) { /* if there are MAX_QP_CHARS or more, do special tests for =xx codes */

        /* Look for =xx codes at end of outbuf */
	if (*(outbuf + LAST_QP_CHAR) == '=')
	    endp = outbuf + LAST_QP_CHAR;
	else if (*(outbuf + (LAST_QP_CHAR - 1)) == '=')
	    endp = outbuf + (LAST_QP_CHAR - 1);
	else
	    endp = outbuf + MAX_QP_CHARS;
	savedp = endp;

	{ /* Look left for beginning of style code <xxxx.  If we find one, set endp to the LeftBracket; if not, restore endp to savedp. */
	     boolean FoundLeftBracket = FALSE;
	     boolean FoundRightBracket = FALSE;

	     endp--; /* back up one; you don't want to use the char at endp but the one before. */
	     while (!FoundLeftBracket && !FoundRightBracket && endp > outbuf) {
		 switch (*endp) {
		     case '<':
			 FoundLeftBracket = TRUE;
			 savedp = endp;
			 break;
		     case '>':
			 FoundRightBracket = TRUE;
			 endp = savedp;
			 break;
		     default:
			 endp--;
			 break;
		 }
		 if (FoundRightBracket || FoundLeftBracket)
		     break;
	     }

	     if (endp == outbuf) /* If we didn't find a style code, restore endp from savedp; hopefully we'll find a space. */
		 endp = savedp;
	 }
    }

    if (endp == savedp) /* back up one; you don't want to use the char at endp but the one before. */
	endp--;

    while (endp > outbuf) { /* Look for space so we can substitute a newline for prettier output. */
	if (isspace(*endp)) {
	    if (endp + 1 < outp)/* endp points to space; set new_endp to char after space. */
		new_endp = endp + 1; 
	    else
		new_endp = outp;
	    break;
	}
	endp--;
    }

    if (endp == outbuf) { /* Found no space; must use UglyChop; restore endp from savedp; set new_endp to NULL, signifying that there is no space. */
       endp = savedp;
       new_endp = NULL;
       UglyChop = TRUE;
    }

    PutsRange(outbuf, file, endp); /* Spit out outbuf up to, but not including, endp. */

    if (UglyChop) /* Handle UglyChop, if necessary. */
	putc('=', file);
    putc('\n', file);

    if (new_endp) /* If new_endp is non-NULL, this means we found a space and will substitute a newline for the space. */
	endp = new_endp;

    memmove(outbuf, endp, outp - endp);
    return((char*) (outbuf + (outp - endp)));
}

/* Codes to figure out how to write out the next chunk of stuff */
#define COMING_NOTHING 0
#define COMING_PLAIN 1
#define COMING_STYLE 2
#define COMING_INSET 3

long text::WriteOtherFormat(FILE  *file, long  writeID, int  level, int  usagetype, char  *boundary)
{
    long pos, len;
    class environment *rootenv;
    class environment *startenv;
    class environment *endenv;
    class environment *curenv;
    class environment *newenv;
    class environment *parentenv;
    long end;
    long i;
    long elen;
    int levels;
    unsigned char c;
    long envpos;
    int  retcode;
    char outbuf[120],*outp,*endp;
    char *buf = NULL;
    long bufLen;
    int nextcode;
    const char *charset;
    boolean terminateNewline = FALSE;

    if (strcmp((this)->GetTypeName(), "text")) {
        /* subclasses should act the same as superclasses */
        return((this)->simpletext::WriteOtherFormat( file, writeID, level, usagetype, boundary));
    }
    pos = 0;
    len = (this)->GetLength();
    Top = NULL;
    if (this->writeID == writeID)  return(this->id);
    this->writeID = writeID;

    if ((this)->CheckHighBit()) {
	charset = environ::Get("MM_CHARSET");
	if (!charset) charset = "ISO-8859-1";
    } else charset = "US-ASCII";

    nextcode = ComingNext(this, pos);
    /* if null boundary, this is NOT multipart at all */
    if (boundary && (nextcode == COMING_STYLE || nextcode == COMING_PLAIN)) {
	fprintf(file, "\n--%s\nContent-type: text/%s; charset=%s\nContent-Transfer-Encoding: quoted-printable\n\n", boundary, (nextcode == COMING_STYLE) ? "richtext" : "plain", charset);
    }
    endp = outbuf + LAST_QP_CHAR;
    outp = outbuf;

    if (len <= 0 ) return -1;

    startenv = (class environment *)(this->rootEnvironment)->GetInnerMost( pos);
    endenv = (class environment *)(this->rootEnvironment)->GetInnerMost( pos + len - 1);
    rootenv = (class environment *)(startenv)->GetCommonParent( endenv);

    for (envpos = (rootenv)->Eval(); pos == envpos && pos + len == envpos + rootenv->length; rootenv = (class environment *) rootenv->parent, envpos = (rootenv)->Eval());
    curenv = rootenv;
    while (curenv != startenv) {
	curenv = (class environment *)(curenv)->GetChild( pos);
	if (curenv->type == environment_Style && 
	     (outp + 2 + strlen(curenv->data.style->name) > endp)) {
	    outp = WriteOutBufOther(file,outbuf,outp);
	}
	if (curenv->type == environment_Style) {
	    if (nextcode == COMING_STYLE) {
		outp = WriteStyle(curenv, outp, 1, outbuf);
	    }
	}
    }

    i = pos;

    end = pos + len;
    while (i < end)  {
	newenv = (class environment *)(this->rootEnvironment)->GetInnerMost( i);
	elen = (this->rootEnvironment)->GetNextChange( i);
	if (elen + i > end)
	    elen = end - i;
	if(Top && curenv->type == environment_View) /* we've escaped all styles before the inset and opened them back up after writing the inset; we can now assume all style info in stack pointed to by Top is valid for next elen */
	    curenv = newenv;
	else if (curenv != newenv)  {
	    char *s, *envnm;
	    int IsReal;

	    parentenv = (class environment *)(curenv)->GetCommonParent( newenv);
	    levels = (curenv)->Distance( parentenv);
	    if (nextcode == COMING_STYLE) {
		while (levels > 0)  {
		    envnm = (char *) PopLevel(&IsReal);
		    if (envnm) {
			*outp++ = '<';
			*outp++ = '/';
			for (s = envnm; s && *s; ++s) {
			    *outp++ = *s;
			}
			*outp++ = '>';
			free(envnm);
		    }
		    else break;
		    if (IsReal) levels--;
		}
	    }
	    curenv = parentenv;
	    if (curenv != newenv) {
		class environment *stack[100];
		class environment *te;
		int level = 0;

		for (te = newenv; te != parentenv; te = (class environment *) te->parent)
		    stack[level++] = te;
		while (level > 0) {
		    curenv = stack[--level];
		    if (curenv->type == environment_Style && 
			 (outp + 2 + strlen(curenv->data.style->name) > endp)) {
			outp = WriteOutBufOther(file,outbuf,outp);
		    }
		    if (curenv->type == environment_Style){
			if (nextcode == COMING_STYLE) {
			    outp = WriteStyle(curenv, outp, 1, outbuf);
			}
		    }
		    else if (curenv->type == environment_View)  
			break;
		}
	    }
	}
	if (curenv->type == environment_View)  {
	    struct stk *tmp = Top;
	    if(outp != outbuf){
		/* We get here when there is less than 76 characters in the cache and it doesn't end in a hard newline */
		/* We need to add a soft-newline because this will be the last line in a multi-part part and there must be atleast one newline aside from the newline associated with the next boundary */
		outp = WriteOutBufOther(file,outbuf,outp);
		fprintf(file, "=\n"); /* soft newline */
	    }
	    if (tmp && (nextcode == COMING_STYLE)) { /* Close open environments */
		fprintf(file, "</%s>", tmp->item);
		while (tmp->next) {
		    fprintf(file, "</%s>", tmp->next->item);    
		    tmp = tmp->next;
		}
		fprintf(file, "=\n"); /* soft newline */
	    }	    
	    retcode = (curenv->data.viewref->dataObject )->WriteOtherFormat( file, this->writeID, 2, usagetype, boundary);
	    if (retcode) {
		if (!tmp || nextcode != COMING_STYLE) {
		    nextcode = ComingNext(this, i+curenv->length);
		}
		if (nextcode == COMING_STYLE || nextcode == COMING_PLAIN) {
		    fprintf(file, "\n--%s\nContent-type: text/%s; charset=%s\nContent-Transfer-Encoding: quoted-printable\n\n", boundary, (nextcode == COMING_STYLE) ? "richtext" : "plain", charset);
		}
	    }
	    if (tmp && (nextcode == COMING_STYLE)) { /* Reopen enviornments */
		struct stk *styleNode;
		for(styleNode = tmp; styleNode;) {
		    if(styleNode->pos + styleNode->len - 1 > i) {
			fprintf(file, "<%s>", styleNode->item);
			styleNode = styleNode->prev;
		    }
		    else { /* Don't reopen, delete, any style that ends on the inset */
			struct stk *gone = styleNode;
			styleNode = styleNode->prev;
			DeleteStyleNode(gone);
		    }
		}
		fprintf(file, "=\n");
	    }	    
	    i += curenv->length;
	    elen = 0;
	}
	elen += i;

	bufLen = 0;
	while (i < elen)  {
	    /* 	    Code for writing out actual text
		*/
	    if (bufLen == 0)
		buf = (this)->GetBuf( i, 1024, &bufLen);
	    bufLen--, c = *buf++;

	    if ((c < 32 && (c != '\n' && c != '\t'))
		|| (c == '=')
		|| ((c >= 127))) {
		static const char basis_hex[] = "0123456789ABCDEF";
		*outp++ = '=';
		*outp++ = basis_hex[c>>4];
		*outp++ = basis_hex[c&0xF];
	    } else if (c == '<' &&
		       (ULstrncmp(buf, "comment>", 8) && ULstrncmp(buf, "/comment>", 9)) &&
		       (nextcode == COMING_STYLE)) {
		*outp++ = '<';
		*outp++ = 'l';
		*outp++ = 't';
		*outp++ = '>';
	    } else if(c == '\n' && (nextcode == COMING_STYLE)) {
		*outp++ = '<';
		*outp++ = 'n';
		*outp++ = 'l';
		*outp++ = '>';
		/* Flush the line now, for richtext aesthetics */
		*outp++ = '\n';
		*outp = (char)0;
		outp = WriteOutBufOther(file,outbuf,outp);
	    } else  {
		*outp++ = c;
	    }
	    if(outp > endp){
		*outp = (char)0;
		outp = WriteOutBufOther(file, outbuf, outp);
	    }
	    i++;
	}
    }

    /* flush out cached characters */
    if(outp != outbuf){ /* We get here if there are some chars in outbuf (there must be less than MAX_QP_CHARS); the remaining contents may not be newline terminated. */
	terminateNewline = TRUE;
#if 0
	*outp = (char)0;
	outp = WriteOutBufOther(file, outbuf, outp);
#else
        PutsRange(outbuf,file,outp);
#endif
    }
    if((levels = (curenv)->Distance( rootenv)) > 0) {
	char *s;
	int IsReal;
	while (levels-- > 0) {
	    if ((s = PopLevel(&IsReal))) {
		terminateNewline = TRUE;
		fputs("</", file);
		fputs(s, file);
		fputs(">", file);
		free(s);
	    }
	    else {
		break;
	    }
	    if (!IsReal) ++levels;
	}
    }
    if(terminateNewline) {
	putc('\n', file);
    }
    return(this->id);
}

static int ComingNext(class text  *self, int  pos)
{
    class environment *e2, *e3;
    int elen;

    if(!self->rootEnvironment ||
       !(e2 = (class environment *)(self->rootEnvironment)->GetInnerMost( pos)) ||
        (pos >= (self)->GetLength()))
	return(COMING_PLAIN);
    else {
	elen = (self->rootEnvironment)->GetNextChange( pos);
	e3 = (class environment *)(self->rootEnvironment)->GetInnerMost( pos + elen + 1);
    }
    if(e2->type == environment_View)
	return(COMING_INSET);
    else if(e2->type == environment_Style)
	return(COMING_STYLE);
    else if(e3) {
	if(e3->type == environment_Style)
		return(COMING_STYLE);
    	else if(e3->type == environment_View)
		return(COMING_PLAIN);
    }
    else return(COMING_PLAIN);
    return COMING_PLAIN;
}

boolean text::CheckHighBit()
{
    long i, len=(this)->GetLength();
    class simpletext *st=(class simpletext *)this;
    
    if(st->highbitflag>=0) return st->highbitflag;
    
    st->highbitflag=0;
    for(i=0;i<len;i++) {
	if((this)->GetChar( i)&0x80) {
	    if((this)->GetChar( i)==TEXT_VIEWREFCHAR) {
		class environment *e=(class environment *) (this->rootEnvironment)->GetInnerMost( i);
		if(e->type==environment_View) continue;
	    }
	    st->highbitflag=1;
	    return st->highbitflag;
	}
    }
    return st->highbitflag;
}
