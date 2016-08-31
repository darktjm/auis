/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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

/* rofftext
 *
 * Commands
 */

#include <andrewos.h> /* strings.h */
#include <link.H>
#include <hash.H>
#include <glist.H>
#include <text.H>
#include <mmtext.H>
#include <ctype.h>
#include <roffstyl.h>
#include <rofftext.H>
#include <rofftext.h>
#include <roffutil.h>


void ds_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void as_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void rm_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void nr_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void af_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void rr_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void so_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void br_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void ex_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void DoMacro(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void de_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void am_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void di_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void da_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void c0_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void c1_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void ie_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void if_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void el_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void Ct_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
static char gettblopts(int  argc, char  argv[80][80] );
static void gettblfmt(int  argc, char  argv[80][80] );
static char *parsetbl(char  *line	/* the input */, const char  *sep	/* separators */, int  multi	/* if 1, skip multiple separators */, char  *out );
void InsertTbl(class rofftext  *self,Trickle  t);
void Et_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void Hd_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void bp_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void Fn_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void Gc_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void Ps_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void Bu_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
#ifdef TROFF_TAGS_ENV
void Tag_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void Tag_ref(class rofftext  *self,Trickle  t, char  *sym);
void Tag_fixup(class rofftext  *self );
#endif /* TROFF_TAGS_ENV */	
void PM_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void PA_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void tl_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
int SortTraps(struct trap  *trap1,struct trap  *trap2);
void wh_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void sp_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void sv_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void it_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void ft_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void in_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void ti_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void nf_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void fi_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void ns_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void rs_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void ce_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void Ce_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void ig_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void tr_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);
void ev_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[]);

/* define or re-define string
  * must parse its own arguments
 */
void ds_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    register int c;
    static BUF b = NULL;
    char name[3];
    boolean tmp = self->v_CopyMode;

    if (b == NULL)
        b = NewBuf();

    getarg(self,t,name,2,0);
    if (*name == '\n')
        return;

    ClearBuf(b);

    self->v_CopyMode = TRUE;
    /* read whitespace */
    while((c = get(self,t)) == ' ' || c == '\t');

    if (c == '\"') /* strip leading quote */
        c = get(self,t);

    /* read string */
    while((c != '\n') && (c != EOF)) {
        Add2Buf(b,c);
        c = get(self,t);
    }
    Add2Buf(b,'\0');
    self->v_CopyMode = tmp;

    DEBUG(1, (stderr,"Defining string (%s) as (%s)\n",name,Buf2Str(b)));
    putstring(self,name,Buf2Str(b));
}

/*  append to string */
void as_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    register int c;
    static BUF b = NULL;
    char name[3];
    const char *init;
    boolean tmp = self->v_CopyMode;

    b = NewBuf();

    getarg(self,t,name,2,0);
    if (*name == '\n')
        return;

    ClearBuf(b);
    init = getstring(self, name);
    while(*init) Add2Buf(b, *init++);

    self->v_CopyMode = TRUE;
    /* read whitespace */
    while((c = get(self,t)) == ' ' || c == '\t');

    if (c == '\"') /* strip leading quote */
        c = get(self,t);

    /* read string */
    while((c != '\n') && (c != EOF)) {
        Add2Buf(b,c);
        c = get(self,t);
    }
    Add2Buf(b,'\0');
    self->v_CopyMode = tmp;

    DEBUG(1, (stderr,"Appending to string (%s) becoming (%s)\n",name,Buf2Str(b)));
    putstring(self,name,Buf2Str(b));
    FreeBuf(b);
}

/* rename or remove request, string, macro */
	void 
rm_cmd(class rofftext  *self, Trickle  t, boolean  br, 
			int  argc, char  *argv[]) {
	const char *value, *v, *arg;

	if (argv[0][1] == 'n') { /* this is rn, not rm */
		arg = argv[1];
		value = (self->Macros)->Lookup(arg);
		if (value) {
			DEBUG(1, (stderr, "renaming string (%s)  ", arg));
			v = (self->Macros)->Rename(arg, argv[2]);
			v += (int)(long)(self->Commands)->Rename(arg, argv[2])
			DEBUG(1,  (stderr, "%s\n", 
					(v ? "succeeded" : "FAILED ***")));
			return;
		}
		value = (self->Commands)->Lookup(arg);
		if (value) {
			DEBUG(1, (stderr, "renaming cmd (%s)  ", arg));
			v = (self->Commands)->Rename(arg, argv[2]);
			DEBUG(1, (stderr, "%s\n", 
					(v ? "succeeded" : "FAILED ***")));
			return;
		}
		DEBUG(1, (stderr, "RENAME: (%s) NOT FOUND ***\n",
					arg));
		return;
	}

	// rm, not rn
	int i;
	for (i = 1; i < argc; i++) {
		arg = argv[i];
		value = (self->Macros)->Lookup(arg);
		if (value) {
			DEBUG(1, (stderr, "removing string (%s)", arg));
			v = (self->Macros)->Delete(arg);
			v += (int)(long)(self->Commands)->Delete(arg);
			free((char *)value);
			DEBUG(1, (stderr, "%s\n",
					(v ? "succeeded" : "FAILED ***")));
			continue;
		}
		value = (self->Commands)->Lookup(arg);
		if (value) {
			DEBUG(1, (stderr, "removing command (%s)", arg));
			v = (self->Commands)->Delete(arg);
			free((char *)value);
			DEBUG(1, (stderr, "%s\n",
					(v ? "succeeded" : "FAILED ***")));
			continue;
		}
		DEBUG(1, (stderr, "REMOVE: (%s) NOT FOUND\n", 
					arg));
	}
}


/* define register */
void nr_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int value,inc;
    boolean relative;

    EvalString(self,argv[2],&value,scale_u,NULL,&relative);
    if (argc >= 4) {
        EvalString(self,argv[3],&inc,scale_u,NULL,NULL);
    }
    else
        inc = 0;

    DEBUG(1, (stderr,"Defining register (%s) as (%d) inc (%d) rel (%d)\n",argv[1],value, inc, relative));
    putregister(self,argv[1],value,reg_Single,inc,relative);
}

/* assign format */
void af_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    struct reg *r = (Reg)(self->Registers)->Lookup(argv[1]);
    enum RegFmt fmt;

    switch(*argv[2]) {
        case '0':
            fmt = reg_Triple;
            break;
        case 'i':
            fmt = reg_LCRoman;
            break;
        case 'I':
            fmt = reg_UCRoman;
            break;
        case 'a':
            fmt = reg_LCaz;
            break;
        case 'A':
            fmt = reg_UCaz;
            break;
        default:
            fmt = reg_Single;
    }
    if (r==NULL) {
        r = NewObj(struct reg);
        r->value = 0;
        r->format = fmt;
        r->autoinc = 0;
        (self->Registers)->Store(argv[1],(char *)r);
    }
    else {
        r->format = fmt;
    }

}

/* remove register */
void rr_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{    
    (self->Registers)->Delete( argv[1]);
}


/* read from a file */
void so_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    tpush(self,t,argv[1],NULL,NULL,FALSE,0,NULL);
}



/* line break */
void br_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    DoBreak(self);
}

/* exit be2roff ??? */
void ex_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
}


/* Do a macro */
void DoMacro(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{

    const char *macro = (self->Macros)->Lookup(argv[0]);
    DEBUG(1, (stderr,"v-----Calling Macro named (%s)-------v\n",argv[0]));
    tpush(self,t,NULL,NULL,macro,TRUE,argc,argv);
}



/* define or re-define macro */
void de_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    char *macro,*existing,*name = strdup(argv[1]);
    char *end = strdup(argv[2]);
    static BUF b = NULL;
    char *oldValue;
    boolean svCopyMode = self->v_CopyMode;
    BUF svSnarfOutput = self->CurrentDiversion->SnarfOutput;

    b = NewBuf();
    ClearBuf(b);

    self->v_CopyMode = TRUE;
    self->CurrentDiversion->SnarfOutput = b;

    Scan(self,t,((argc<3)?".":end));

    self->CurrentDiversion->SnarfOutput = NULL;
    self->v_CopyMode = FALSE;
    Add2Buf(b,'\0');

    macro = strdup(Buf2Str(b));

    (self->Commands)->Delete(name);
    existing = (char *)(self->Macros)->Delete(name);
    if (existing)
        free(existing);

    DEBUG(1, (stderr,"--Defining Macro (%s)--\n",name));

    (self->Macros)->Store(name,macro);
    (self->Commands)->Store(name,(char *)DoMacro);
    free(name);
    free(end);
    FreeBuf(b);

    self->v_CopyMode = svCopyMode;
    self->CurrentDiversion->SnarfOutput = svSnarfOutput;

/*    DEBUG(1, (stderr,"%s\n<----------------\n",macro));*/
}

/* append to macro */
void am_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    char *macro,*existing,*name = strdup(argv[1]);
    char *end = strdup(argv[2]);
    const char *init;
    BUF b;

    b =	NewBuf(); 

    ClearBuf(b);
    init = getstring(self, name);
    while(*init) Add2Buf(b, *init++);

    self->v_CopyMode = TRUE;
    self->CurrentDiversion->SnarfOutput = b;

    Scan(self,t,((argc<3)?".":end));

    self->CurrentDiversion->SnarfOutput = NULL;
    self->v_CopyMode = FALSE;
    Add2Buf(b,'\0');

    macro = strdup(Buf2Str(b));

    (self->Commands)->Delete(name);
    existing = (char *)(self->Macros)->Delete(name);
    if (existing)
        free(existing);

    DEBUG(1, (stderr,"--Appending To Macro (%s)--\n",name));

    (self->Macros)->Store(name,macro);
    (self->Commands)->Store(name,(char *)DoMacro);
    free(name);
    free(end);
    FreeBuf(b);
/*    DEBUG(1, stderr,"---%s\n<----------------\n",macro);*/
}

/* divert output into a macro */

void di_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    char *macro;
    static BUF b = NULL;
    char *existing;

    b = NewBuf();

    if (argc > 1) { 

        ClearBuf(b);

        DEBUG(1, (stderr,"--Diverting to Macro (%s)------>\n",argv[1]));

/*        tmp = self->v_CopyMode;
        self->v_CopyMode = TRUE;*/
        PushDiversion(self);
	self->CurrentDiversion->SnarfOutput = b;
        self->CurrentDiversion->name = strdup(argv[1]);
    }
    else{
        if (self->v_DiversionLevel > 0) {
	    b =	self->CurrentDiversion->SnarfOutput;
	    self->CurrentDiversion->SnarfOutput = NULL;
            /*        self->v_CopyMode = tmp;*/
            Add2Buf(b,'\0');

            macro = strdup(Buf2Str(b));

            existing = (char *)(self->Macros)->Delete(self->CurrentDiversion->name);
            if (existing)
                free(existing);
            (self->Commands)->Delete(self->CurrentDiversion->name);

            (self->Macros)->Store(self->CurrentDiversion->name,macro);
            (self->Commands)->Store(self->CurrentDiversion->name,(char *)DoMacro);
            DEBUG(1, (stderr,"\n<----------------divert (%s)\n",self->CurrentDiversion->name));
            PopDiversion(self);
	    FreeBuf(b);
	    putregister(self, "dn", 1, reg_Single, 0, 0);
        }
    }
}

/* divert and append output into a macro */
void da_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    char *macro;
    char *existing;
    const char *init;
    BUF	b;

    b =	NewBuf();

    if (argc > 1) { 

        ClearBuf(b);

	init = getstring(self, argv[1]);
	while(*init) Add2Buf(b, *init++);
        DEBUG(1, (stderr,"--Diverting and appending to Macro (%s)---(%d)--->\n",argv[1], self->v_DiversionLevel));

/*        tmp = self->v_CopyMode;
        self->v_CopyMode = TRUE;*/
        PushDiversion(self);
	self->CurrentDiversion->SnarfOutput =	b;
        self->CurrentDiversion->name = strdup(argv[1]);
    }
    else{
        if (self->v_DiversionLevel > 0) {
	    b =	self->CurrentDiversion->SnarfOutput;
	    self->CurrentDiversion->SnarfOutput = NULL; 
            /*        self->v_CopyMode = tmp;*/
            Add2Buf(b,'\0');

            macro = strdup(Buf2Str(b));

            existing = (char *)(self->Macros)->Delete(self->CurrentDiversion->name);
            if (existing)
                free(existing);
            (self->Commands)->Delete(self->CurrentDiversion->name);

            (self->Macros)->Store(self->CurrentDiversion->name,macro);
            (self->Commands)->Store(self->CurrentDiversion->name,(char *)DoMacro);
            DEBUG(1, (stderr,"\n<----------------divert (%s)\n",self->CurrentDiversion->name));
            PopDiversion(self);
	    FreeBuf(b);
	    putregister(self, "dn", 1, reg_Single, 0, 0);
	}
    }
}


/* debugging */
void c0_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    self->v_CopyMode = 0;
}

void c1_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    self->v_CopyMode = 1;
}


/* if-else  -- special syntax */

void ie_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int c,delim,delimcount;
    char *str, *string2;
    const char *string3;
    boolean sense = FALSE,result;
    int dresult;
    static BUF b = NULL;

    b = NewBuf();

    /* read leading whitespace */
    do {
        c = get(self,t);
    } while((c==' ')||(c=='\t'));

    /* tell what to do by first char */

    if (c=='!') {
        sense = TRUE;
        do {
            c = get(self,t);
        } while((c==' ')||(c=='\t'));
    }
    if (isdigit(c) || c == '(') { /* numeric compare */
        ClearBuf(b);
        while((c!= ' ') && (c != '\t') && (c != '\n')) {
            Add2Buf(b,c);
            c = get(self,t);
        }
        Add2Buf(b,'\0');
        str = Buf2Str(b);
        DEBUG(1, (stderr,"Comparing numeric value (%s)\n",str));
        EvalString(self,str,&dresult,scale_u,NULL,NULL);
        result = (dresult > 0);
    }
    else {
        switch(c) {
            case 'o': result = TRUE;
                break;
            case 'e': result = FALSE;
                break;
            case 't': result = self->RoffType;
                break;
            case 'n': result = !self->RoffType;
                break;
            default: /* string compare */
                /* read first argument into string */ /* this is inadequate */
                ClearBuf(b);
                delimcount = 0;
                delim = c;
                c = get(self,t);
                if (c == delim)
                    delimcount++;

                while((c != '\n') && (delimcount < 2)) {
                    Add2Buf(b,c);
                    c = get(self,t);
                    if (c == delim)
                        delimcount++;
                }
                Add2Buf(b,'\0');
                str = Buf2Str(b);
                DEBUG(1, (stderr,"====>if: delim '%c', string argument is (%s)\n",delim,str));


                string2 = strchr(str,delim);
                if (string2 == NULL)
                    string3 = "";
                else {
                    *string2++ = '\0';
		    string3 = string2;
                }
                DEBUG(1, (stderr,"Comparing (%s) (%s)\n",str,string3));
                result = (strcmp(str,string3)==0);

                break;
        }
        c = get(self,t); /* get first char afterwards because number does */

    }
    ClearBuf(b);
    DEBUG(1, (stderr,"if: sense %d, result %d\n",sense,result));
    /* if condition is false, munch to end of conditional input */
    if (result == sense) {
	int oldlev = self->v_InCond;
        self->v_RawMode = TRUE;
        DEBUG(1, (stderr,"-->Munch->"));
	while(c	!= EOF && ((c != '\n') || (self->v_InCond > oldlev))) {
            c = get(self,t);
        }
        DEBUG(1, (stderr,"<-Munch<--"));
        self->v_RawMode = FALSE;
    }
    else {
        /* read spaces */
        while((c ==' ') || (c == '\t'))
            c = get(self,t);
        ung(self,c,t);
        Set_BOL(self); /* next char is BOL */
    }

    self->v_LastIfResult = (sense != result);
    FreeBuf(b);
}


/* if command -- special syntax */
void if_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    boolean tmp = self->v_LastIfResult;
    ie_cmd(self,t,br,argc,argv);
    self->v_LastIfResult = tmp;
}

/* else command -- special syntax */
void el_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int c;

    DEBUG(1, (stderr,"else: previous result was %d\n",self->v_LastIfResult));
    if (self->v_LastIfResult) { /* munch to end of conditional input */
	int oldlev = self->v_InCond;
        self->v_RawMode = TRUE;
        DEBUG(1, (stderr,"-->Munch->"));
        do {
            c = get(self,t);
        } while(c != EOF && ((c != '\n') || (self->v_InCond > oldlev)));
        DEBUG(1, (stderr,"<-Munch<--"));
        self->v_RawMode = FALSE;
    }
    else {
        do {
            c = get(self,t);
        } while((c ==' ')||(c == '\t')); /* munch whitespace */
        ung(self,c,t);
        Set_BOL(self);
    }
    self->v_LastIfResult = TRUE;
}



/* table macros */

/* create a table */
void Ct_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int i;
    self->Tbl = new table;
    (self->Tbl)->ChangeSize( 0, 0);
    self->v_TblMode = 1;
    self->v_TblTab = '\t';
    for (i = 0; i < MAX_COLS; i++) self->colWidth[i] = 1;
}

static char gettblopts(int  argc, char  argv[80][80] )
{
    int i;
    for (i = 0; i < argc; i++) {
	if (strncmp(argv[i], "tab(", 4) == 0)
	    return argv[i][4];
    }
    return '\t';
}

static void gettblfmt(int  argc, char  argv[80][80] )
{
}

static char *parsetbl(char  *line	/* the input */, const char  *sep	/* separators */, int  multi	/* if 1, skip multiple separators */, char  *out )
{	/* the output */
    char *p = line;
    *out = *p;
    if (*p == '\0') return NULL;
    while (*p != '\0' && !strchr(sep, *p)) *out++ = *p++;  /* copy till separator */
    *out = '\0';
    if (!multi) {
	if (*p != '\0')	p++;				    /* skip one separator */
    } else {
	while (*p != '\0' && strchr(sep, *p)) p++;	    /* skip all separators */
    }
    return p;
}


/* insert a row in the table */
void InsertTbl(class rofftext  *self,Trickle  t)
{
    int row, col, colmax;
    char line[500], tabstr[10];
    char argv[80][80]; static int argc = 0;
    char c, last, *p = line;
    if (self->v_TblMode == 0 || self->Tbl == NULL) return;

    while ((c =	get(self, t)) != '\n') *p++ = self->Berliz[(int)c];    /* copy the line */
    *p-- = '\0';
DEBUG(1, (stderr, "Tbl reading [%s] in state %d\n", line, self->v_TblMode));
    while (p > line && (*p == ' ' || *p == '\t')) p--;
    if (p <= line) last	= '\0';			    /* remember the last char */
    else last = *p;
    p = line;
    if (self->v_TblMode == 1) {	/* look for global options */
	self->v_TblMode = 2;
	if (last == ';') {
	    while ((p = parsetbl(p, " \t,", TRUE, argv[argc])) != NULL) argc++;	/* parse it */
	    self->v_TblTab = gettblopts(argc, argv);
	    argc = 0;
	    return;
	}
    }
    if (self->v_TblMode == 2) {	/* look for formats */
	while ((p = parsetbl(p, " \t", TRUE, argv[argc])) != NULL) {
	    argc++;	/* parse it */
	}
	gettblfmt(argc, argv);
	if (last == '.') {
	    self->v_TblMode = 3;
	}
	argc = 0;
	return;
    }
    if ((strcmp(line, "=") == 0) || (strcmp(line, "_") == 0)) return;	  /* ignore horiz lines */
    tabstr[0] = self->v_TblTab;	/* to parse table */
    tabstr[1] = '\0';
DEBUG(1, (stderr, "about to parse [%s]\n", line));
    while ((p = parsetbl(p, tabstr, FALSE, argv[argc])) != NULL) {
	if(!strcmp(argv[argc], "T{") || !strcmp(argv[argc], "T}"))
	    return;	/* skip it -- string inside a "T{...}T" construct */
	else argc++;	/* parse it */
    }
    row	= (self->Tbl)->NumberOfRows()+1;	/* size the table */
    colmax = (self->Tbl)->NumberOfColumns();
    if (colmax < argc) colmax = argc;
    (self->Tbl)->ChangeSize( row, colmax);
    for (col = 0; col < argc; col++) {
	struct cell *c = (self->Tbl)->GetCell( row-1, col);
	char buf[2000];
	int width = strlen(argv[col]);
	if (width > self->colWidth[col]) self->colWidth[col] = width;
	buf[0]='\"';
	strcpy(buf+1, argv[col]);	/* force to string */
	(self->Tbl)->ParseCell( c, buf);
    }
    argc = 0;	/* reset argc when we drop through because it's static */
}

/* end the table */
void Et_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int colmax = (self->Tbl)->NumberOfColumns();
    int col;
    for (col = 0; col < colmax; col++) 
	self->Tbl->col[col].thickness = self->colWidth[col]*9;
 
    (self->textm)->AddView( self->pos++, (self->Tbl)->ViewName(), self->Tbl);
    (self->textm)->InsertCharacters( self->pos++, "\n", 1);
    self->v_TblMode = 0;
}

/* Heading command */
void Hd_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int style = 0;
    int lev;
    char *p;
    const char *str;
    if (argc < 3) return;
    lev = atoi(argv[1]);
    switch (lev) {
	case 0: 
	case 1:	str = "chapter";
	    break;
	case 2:	str = "section";
	    break;
	case 3:	str = "subsection";
	    break;
	case 4:	
	case 5: str = "paragraph";
	    break;
	case 6:	
	case 7:	
	    break;
	case 8:	str = "majorheading";
	    break;
	case 9:	str = "heading";
	    break;
	default: str = "";
	    break;
    }
    if (str[0] != '\0') style = BeginStyle(self, str);
    p = argv[2];
    while (*p != '\0') put(self, *p++);
    if (str[0] != '\0') EndStyle(self, style);
}

/* Begin Page command */
void bp_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    put(self, '\n');	    /* this is necessary; otherwise font changes are one off */
    (self->textm)->InsertObject( self->pos++, "bp", "bpv");
    put(self, '\n');
}

/* Footnote command */
void Fn_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int sw = 0;
    if (argc > 1) sw = atoi(argv[1]);
    if (sw != 0) {		    /* start a footnote */
	self->Fn = new fnote;
	self->fnpos = 0;
    } else if (self->Fn	!= NULL) {  /* end a footnote */
	(self->textm)->AddView( self->pos++, (self->Fn)->ViewName(), self->Fn);
	self->Fn = NULL;
    }
}

/*  Turn off escape processing in GC mode */
void Gc_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int newval = 1;
    if (argc > 1) newval = atoi(argv[1]);
    self->gc_mode = newval;
}

/*  Pic command */
void Ps_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    class link *l;
    int open = 0;
    if (argc > 1) open = atoi(argv[1]);
    if (!open) {
	/* closing */
	char cmd[400];
	char filename[400];
	if (self->filename != NULL) strcpy(filename, self->filename);
	else if (ATK::IsTypeByName((self)->GetTypeName(), "mmtext")) {
		    class mmtext *doc = (class mmtext *) self;
		    (doc)->GetFilename( filename);
	} else filename[0] = '\0';
	sprintf(cmd, "$tail <%s +%ld|head -%ld|pic|troff|preview", filename,
		self->picBegin,	t->t->LineNumber - self->picBegin+1);	/* include .PE */
	(self->picButton)->SetLink( cmd);
	return;
    }
    /* opening */
    l = new link;
    (l)->SetText( "Click here to see PIC figure");
    self->picBegin = t->t->LineNumber -	1;  /* include the .PS */
    self->picButton= l;
    (self->textm)->AddView( self->pos++, (l)->ViewName(), l);
}

/*  Hypertext link buttons */
void Bu_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    class link * l;
    if (argc < 3) return;
    l = new link;
    (l)->SetText( argv[1]);
    (l)->SetLink( argv[2]);
    (self->textm)->AddView( self->pos++, (l)->ViewName(), l);
}

#ifdef TROFF_TAGS_ENV
/*  Tag command -- definition */
void Tag_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    char *tag;
    int count;
    /* definition -- make table entry */
    /* fix up references later */
    /* argv[1] is "G", because the command is actually .TAG */
    if (argc < 3) return;
    tag = malloc(strlen(argv[2])+1);
    if (tag == NULL) return;
    strcpy(tag, argv[2]);
    if (self->tag_count >= MAX_TAG) return;
    count = self->tag_count++;
    self->tags[count].tag = tag;
    self->tags[count].def = TRUE;
    self->tags[count].pos = self->pos;
}

/*  Tag reference */
void Tag_ref(class rofftext  *self,Trickle  t, char  *sym)
{
    class link * l;
    char *tag;
    int count;
    char filename[200];
    char taglabel[100];
    /* tag being referred to -- put in a button */
    /* need to do SetPos to put in destination */
    tag = malloc(strlen(sym)+1);
    if (tag == NULL) return;
    strcpy(tag, sym);
    if (self->tag_count >= MAX_TAG) return;
    if (self->filename != NULL) strcpy(filename, self->filename);
    else if (ATK::IsTypeByName((self)->GetTypeName(), "mmtext")) {
	class mmtext *doc = (class mmtext *) self;
	(doc)->GetFilename( filename);
    } else strcpy(filename, t->t->filename);
    count = self->tag_count++;
    self->tags[count].tag = tag;
    l = new link;
    self->tags[count].def = FALSE;
    self->tags[count].l = l;
    sprintf(taglabel, " [-->>%s] ", tag);
    (l)->SetText( taglabel);
    (l)->SetLink( filename);
    (self->textm)->AddView( self->pos++, (l)->ViewName(), l);
}

/*
 *  Fix up tags
 */
void Tag_fixup(class rofftext  *self )
{
    int i;
    for (i = 0; i < self->tag_count; i++) {
	if (! self->tags[i].def) {
	    /* search for the definition */
	    int j;
	    for (j = 0; j < self->tag_count; j++) {
		if (self->tags[j].def && strcmp(self->tags[j].tag, self->tags[i].tag) == 0) {
		    /* found definition matching the reference */
		    /* j points to the def, i to the ref */
/*		    link_SetPos(self->tags[i].l, self->tags[j].pos); */
		}
	    }
	}
    }
}
#endif /* TROFF_TAGS_ENV */	

/* print macro -- for debugging */
void PM_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    const char *m;
    printf("Macro named (%s) ----->\n",argv[1]);
    m = (self->Macros)->Lookup(argv[1]);
    if (m)
        printf("%s",m);
    else
        printf("(Could not find %s)\n",argv[1]);
    printf("\n--------------End\n");
}


void PA_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    printf("::::::::::::::::Printing all macros:::::::::::::::::\n");
    ((self->Macros))->Debug();
    printf("::::::::::::::::DONE::::::::::::::::\n");
}


/* title command -- special syntax */

void tl_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    register int c,style;
    char *str,*string1=NULL,*string2=NULL,*string3=NULL,*end=NULL;
    int tmp = self->CurrentDiversion->OutputDone;
    static BUF b = NULL;

    b = NewBuf();
    ClearBuf(b);
    c = get(self,t);
    /* read leading whitespace */
    while((c==' ')||(c=='\t'))
        c = get(self,t);
    /* read stuff into string */
    while((c != EOF) && (c != '\n')) {
        Add2Buf(b,c);
        c = get(self,t);
    }
    Add2Buf(b,'\0');
    str = Buf2Str(b);

    style = BeginStyle(self,"majorheading");
    /* now get three parts */
    string1 = str+1;
    string2 = strchr(string1,*str);
    if (string2) {
        *string2++ = '\0';
        string3 = strchr(string2,*str);
        if (string3) {
            *string3++ = '\0';
            end = strchr(string3,*str);
            if (end)
                *end = '\0';
        }
    }
    if (string2) {
        while(*string1 != '\0') {
            if ((c = *string1++) != '%')
                put(self,c);
        }
        put(self,' '); put(self,' ');
        put(self,' '); put(self,' ');
    }
    if (string3) {
        while(*string2 != '\0') {
            if ((c = *string2++) != '%')
                put(self,c);
        }
        put(self,' '); put(self,' ');
        put(self,' '); put(self,' ');
    }
    if (end) {
        while(*string3 != '\0') {
            if ((c = *string3++) != '%')
                put(self,c);
        }
        put(self,'\n');
    }
    EndStyle(self,style);
    FreeBuf(b);
    self->CurrentDiversion->OutputDone = tmp; /* don't screw up breaks */

}

int SortTraps(struct trap  *trap1,struct trap  *trap2)
{
    if (trap1->loc > trap2->loc)
        return 1;
    else if (trap1->loc < trap2->loc)
        return -1;
    else return 0;
}

/* set a trap -- beginning or end of page */
void wh_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int result;
    boolean absolute,relative;

    /* check location for trap */
    EvalString(self,argv[1],&result,scale_u,&absolute,&relative);
    if (!relative && !absolute && (result == 0)) {
        DEBUG(1, (stderr,"Setting beginning trap\n"));
        self->v_Trap = strdup(argv[2]);
    }
    /* foo */
    if (relative && (result < 0)) {
        struct trap *trap = (struct trap *)malloc(sizeof(struct trap));
        trap->loc = result;
        trap->macro = strdup(argv[2]);
        (self->EndMacros)->InsertSorted((char *)trap,(glist_greaterfptr)SortTraps);
    }

}

/* space, the final frontier (ugh) */
void sp_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    if (self->CurrentDiversion->NoSpaceMode) {
        DEBUG(1, (stderr,"Space: no space mode is on...\n"));
        return;
    }
    else
        sv_cmd(self,t,br,argc,argv);
}

void sv_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int result;
    int lines = 1;

    /* do a break */
    if (br)
        DoBreak(self);

    if (argc > 1) {
        EvalString(self,argv[1],&result,scale_v,NULL,NULL);
        if (result > 0) {
            lines = (int)((result / (self->ScaleFactor[(int)scale_v])) + (0.5));
        }
        else
            lines = 0;
    }
    DEBUG(1, (stderr,"SPACE: spacing %d from a result of %g\n",lines,result));
    if (lines > 0) {
        while(lines-- > 0)
            put(self,'\n');
        self->CurrentDiversion->OutputDone = 0;
    }
}


void it_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int result;

    if (argc<3) {
        if (self->CurrentEnviron->InputTrapCmd)
            free(self->CurrentEnviron->InputTrapCmd);
        self->CurrentEnviron->InputTrapCmd = NULL;
        self->CurrentEnviron->NextInputTrap = 0;
    }
    else {
        self->CurrentEnviron->InputTrapCmd = strdup(argv[2]);
        EvalString(self,argv[1], &result, scale_u,NULL,NULL);
        self->CurrentEnviron->NextInputTrap = result + self->v_InputLineCount;
    }
}

/* set global font */

void ft_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    const char *font;
    font = "P";
    if (argc>1)font=argv[1];
    if (self->v_DiversionLevel>0) { /* yuck-o, handle fonts in diversions */
        int tmp = self->CurrentDiversion->OutputDone;
        put(self,'\\');
        put(self,'f');
	put(self,*font);
        self->CurrentDiversion->OutputDone = tmp;
        return;
    }
    if (isdigit(font[0])) {
        int digit = font[0]-'1';
        if ((digit >= 0) && (digit <= 5))
            font = self->Fonts[digit];
        else {
            DEBUG(1, (stderr,"********WARNING: bogus font (%s)********\n)",font));
            font = self->Fonts[0];
        }
    }
    else
        switch(font[0]) {
            case 'B': font = "bold";
                break;
            case 'I': font = "italic";
                break;
            case 'S': font = "special";
                break;
            case 'P': font = self->CurrentEnviron->prevFont;
                break;
	    case 'G':
            case 'L': font = "typewriter";
                break;
            default: font = "roman";
                break;

        }
    self->CurrentEnviron->prevFont = self->CurrentEnviron->font;
    self->CurrentEnviron->font = font;
    self->CurrentEnviron->fontStyle = ChangeStyle(self,self->CurrentEnviron->fontStyle,font);

}

/* set global indent */

void in_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    /* this is bogus for now */
    int result;
    boolean relative;
    int temp;

    if (br)
	DoBreak(self);

    temp = self->CurrentEnviron->indent;

    if (argc>1) {
        DEBUG(1, (stderr,"Indenting (%s)\n",argv[1]));

        EvalString(self,argv[1],&result,scale_m,NULL,&relative);
        if (relative)
            self->CurrentEnviron->indent += result;
        else
            self->CurrentEnviron->indent = result;
    }
    else {
        self->CurrentEnviron->indent = self->CurrentEnviron->prevIndent;
    }
    self->CurrentEnviron->prevIndent = temp;
    self->CurrentEnviron->tempIndent = self->CurrentEnviron->indent;

    if (self->CurrentEnviron->indent != temp)
        SetIndent(self,self->CurrentEnviron->indent);
}

/* temporary indent */
void ti_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    /* this is bogus for now */
    int result;
    boolean absolute,relative;

    if (br)
        DoBreak(self);

    if (argc>1) {
        DEBUG(1, (stderr,"Temp. indenting (%s)\n",argv[1]));

        EvalString(self,argv[1],&result,scale_m,&absolute,&relative);
        if (relative)
            self->CurrentEnviron->tempIndent = self->CurrentEnviron->indent + result;
        else
            self->CurrentEnviron->tempIndent = result;
        if (self->CurrentEnviron->tempIndent < 0)
            self->CurrentEnviron->tempIndent = 0;
        if (self->CurrentEnviron->tempIndent != self->CurrentEnviron->indent)
            SetTempIndent(self,self->CurrentEnviron->tempIndent);
    }

}


/* no-fill mode */
void nf_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    if (br)
        DoBreak(self);
    self->CurrentEnviron->fill = FALSE;
}

/* fill mode */
void fi_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    if (br)
        DoBreak(self);
    self->CurrentEnviron->fill = TRUE;
}


/* set no-space mode */
void ns_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    DEBUG(1, (stderr,"===Turning on no-space mode===\n"));
    self->CurrentDiversion->NoSpaceMode = TRUE;
}


/* restore spacing */
void rs_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    DEBUG(1, (stderr,"=Resetting Space Mode=\n"));
    self->CurrentDiversion->NoSpaceMode = FALSE;
}


/* center text */
void ce_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    if (br)
        DoBreak(self);
}

/* center text */
void Ce_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int result;
    static int styleID = 0;

    if (br)
        DoBreak(self);
    DEBUG(1, (stderr, "Center command argc (%d) \n", argc));
    if (argc > 1) {
	EvalString(self,argv[1],&result,scale_u,NULL,NULL);
	if (result > 0) {
	    styleID = BeginStyle(self, "center");
	    return;
	}
    }
    if (styleID) EndStyle(self, styleID);
    styleID = 0;
}

/* ignore input */
void ig_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    char *end = strdup(argv[1]);
    static BUF b = NULL;

    b = NewBuf();

    ClearBuf(b);

    self->v_CopyMode = TRUE;
    self->CurrentDiversion->SnarfOutput = b;

    Scan(self,t,((argc<2)?".":end));

    self->CurrentDiversion->SnarfOutput = NULL;
    self->v_CopyMode = FALSE;
    /* ignore completely! */
    FreeBuf(b);
    free(end);
}


/* translate characters  - special syntax */
void tr_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    register int c;
    unsigned char source,new_c;
    char temp[3],name[3];

    /* read leading spaces */

    self->v_RawMode = TRUE;
    do {
        c = get(self,t);
    } while ((c==' ')||(c == '\t'));

    /* now translate, checking for special chars */

    while( (c != '\n') && (c != EOF)) {
        if (c == '\\') { /* check for special */
            c = get(self,t);
            if (c == '(') {
                name[0] = get(self,t);
                name[1] = get(self,t);
                name[2] = '\0';
                DEBUG(1, (stderr,"tr: found special char %s\n",name));
                /* change special char */
            }
            else {
                temp[0] = '\\';
                temp[1] = c;
                temp[2] = '\0';
                tpush(self,t,NULL,NULL,temp,FALSE,0,NULL);
                self->v_RawMode = FALSE;
                c = get(self,t);
                name[0] = '\0';
                DEBUG(1, (stderr,"tr: found special char %s, which maps to %c\n",temp,c));
            }
        }
        else
            name[0] = '\0';
        source = c;
        self->v_RawMode = FALSE;

        if (c == '\n') /* in case a translated character was null */
            break;

        /* should really handle special chars here, too */
        new_c = c = get(self,t);
        if (new_c == '\n')
            new_c = ' ';
        if (name[0] == '\0') {
            DEBUG(1, (stderr,"tr: translating %c to %c\n",source,new_c));
            self->Berliz[source] = new_c;
        }
        else {
            temp[0] = new_c;
            temp[1] = '\0';
            /* we won't worry about keeping track of mallocs here */
            (self->SpecialChars)->Store(name,strdup(temp));
            DEBUG(1, (stderr,"tr: renaming %s to %s\n",name,temp));
        }
        self->v_RawMode = TRUE;
	if (c != '\n') c = get(self,t);
    }
    self->v_RawMode = FALSE;
}

/* switch environment */
void ev_cmd(class rofftext  *self,Trickle  t,boolean  br,int  argc,char  *argv[])
{
    int result;
    int i;

    if (argc > 1) {
        EvalString(self,argv[1],&result,scale_u,NULL,NULL);
        i = result;
        if ((i >= 0) && (i <= 2))
            PushEnviron(self,i);
    }
    else
        PopEnviron(self);
}
