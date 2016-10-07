#define NESS_INTERNALS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
\* ********************************************************************** */

/* interp.c
	interpret byte code representation of ness programs

	Entry points:

	initializeEnvt()  -  initialize the run-time environment
	stackArg(argvalue, type) - puts an argument on the stack
	interpretNess(func, ness)  -  interpret a call on func
*/

/*
 * $Log: interp.C,v $
 * Revision 1.15  1996/08/19  18:15:51  wjh
 * BUG
 * fixed bug in `system' command which truncated its output
 * NB.:  fread can return zero before feof is true
 *
 * Revision 1.14  1996/05/17  20:53:12  robr
 * Fixed to include aaction.H and use Destroy method instead of delete.
 *
 * Revision 1.13  1995/12/07  16:41:27  robr
 * Support for exporting ness functions to the proctable.
 *
 * Revision 1.12  1995/11/07  20:17:10  robr
 * OS/2 port
 *
 * Revision 1.11  1995/11/01  19:26:47  susan
 * changed UniqueID to im::GetWriteID in Write function
 *
 * Revision 1.10  1995/09/08  02:49:48  wjh
 * adapted 'z' (swap) to avoid a possible overlapping memory assignment
 *
 * Revision 1.9  1995/04/05  01:48:15  rr2b
 * don't export internally used inline functions to ness client code
 *
 * Revision 1.8  1994/08/11  02:58:16  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.7  1994/03/21  17:00:38  rr2b
 * bzero->memset
 * bcopy->memmove
 * index->strchr
 * rindex->strrchr
 * some mktemp->tmpnam
 *
 * Revision 1.6  1993/09/30  16:59:44  rr2b
 * added braces around the case of a switch statement which introduced
 * a variable.  Otherwise we hit the "can't" jump past initializer rule
 * of C++
 *
 * Revision 1.5  1993/07/23  00:23:42  rr2b
 * Split off a version of CopyText which will copy surrounding
 * styles as well as embedded styles.
 *
 * Revision 1.4  1993/06/29  18:22:36  wjh
 * 	made SigHandler static
 *
 * Revision 1.3  1993/06/28  13:50:16  rr2b
 * Fixed signal handler code.
 * Removed extraneous &s.
 *
 * Revision 1.2  1993/06/21  20:43:31  wjh
 * fixed ungetMark problems
 *
 * Revision 1.1  1993/06/21  19:30:36  wjh
 * Initial revision
 *
 * Revision 1.46  1993/03/25  20:31:24  wjh
 * fix core dump for: writefile(..., "")
 *
 * Revision 1.45  1993/02/17  06:10:54  wjh
 * uservis: added FUNCVAL to grammar.  Now functions can be values.
 * uservis: added FORWARD decls of functions
 * uservis: added subseq as a synonym for marker
 * uservis: fixed to avoid coredumps that can happen after a syntax error
 * trivial(progvis): make Sym a part of all callnodes
 * 		so dumpstack can say what value is there
 * ignore: removed 'paren' field from varnode
 * ignore: fixed bug in which builtin function names would compile as values but would get runtime error.  Now a compile error.
 * ignore: removed use of ntFUNCTION, etc. in favor of FUNCTION, etc.
 * uservis: check the type of values in RETURN stmts.  This may produce compilation errors in programs which compiled successfully in the past.
 * uservis: change error from ParseReal to be -999.0.  length(WhereItWas()) will be zero.
 * progvis: changed nessruna's -d to -x as switch to dump symbol table
 * progvis: added nessruna switch -y to set debugging
 * trivial: removed disclaimer from ness warning text
 * trivial: removed the restriction in Ness which prevents storing a NaN value
 * trivial: Fixed so the proper function is reported for an error even if warning message is added or removed.  (used a mark in funcnode)
 * trivial: Rewrote the line number calculation (ness.c:LineMsg) so it works when there is a wrapped warning text.  (The warning text is excluded from the count.)
 * adminvis: Added file testness.n ($ANDREWDIR/lib/ness/demos) for testing many facets of Ness
 * uservis: In Ness, provide simple implementations on all platforms for acosh(), asinh(), atanh(), cbrt(), expm1(), and log1p().
 * uservis: Provide Ness predefined identifier 'realIEEE' which is True if isnan(), finite(), and lgamma() are implemented
 * uservis: In Ness, accept isnan(), finite(), and lgamma() during compilation, but give a runtime error if they are executed on a platform where they are not implemented.  (See predefined identifier realIEEE.)
 * ignore: changed eventnode to correspond to change in funcnode
 *
 * e
 *
 * Revision 1.44  1992/12/17  20:09:19  rr2b
 * added #include of sys/param.h for MAXPATHLEN
 * .
 *
 * Revision 1.43  1992/12/16  04:12:38  wjh
 * Ness version 1.7
 * Added readrawfile and writerawfile.
 * Readfile, readrawfile, writefile, writerawfile, and writeobject
 * 	all canonicalize the file name argument.  It may have leading ~
 * 	or embedded $environment variable.
 * Nessruna also canonicalizes the ness file argument.
 * If the argument to readfile is an ATK object data stream,
 * 	the result is a text with one element--the object.
 * Error messages are printed with line numbers.
 * The location of errors is relative to the beginning of the script
 * 	regradless of whether there is a warning text around it.
 *
 * Revision 1.42  1992/11/26  02:42:25  wjh
 * converted CorrectGetChar to GetUnsignedChar
 * moved ExtendShortSign to interp.h
 * remove xgetchar.h; use simpletext_GetUnsignedChar
 * nessrun timing messages go to stderr
 * replaced curNess with curComp
 * replaced genPush/genPop with struct compilation
 * created compile.c to handle compilation
 * moved scope routines to compile.c
 * converted from lex to tlex
 * convert to use lexan_ParseNumber
 * truncated logs to 1992 only
 * use bison and gentlex instead of yacc and lexdef/lex
 *
 *
 * Revision 1.41  92/06/05  16:39:31  rr2b
 * Added use of sysmarknextptrs.
 * 
 * Revision 1.40  1992/05/28  17:56:08  wjh
 * fixed the (internal) swap operator to deal with any object
 * 		(it is used by writeObject())
 * added writeobject() - function to write an object to a file
 * 
 . . . log elided Dec 92 -wjh

 * Revision 1.1  88/10/21  10:58:35  wjh
 * Initial revision
 * 
 * Creation 0.0  88/03/29 10:16:00  wjh
 * Initial creation by WJHansen
 * 
*/

#include <andrewos.h>
#include <sys/param.h> /* for MAXPATHLEN */
#include <signal.h>
#include <setjmp.h>

#include <dataobject.H>
#include <view.H>
#include <text.H>
#include <simpletext.H>
#include <textview.H>
#include <stylesheet.H>
#include <environment.H>
#include <keystate.H>
#include <im.H>
#include <proctable.H>
#include <celview.H>
#include <message.H>
#include <attribs.h>
#include <environ.H>
#include <path.H>

#include <ness.H>
#include <nessmark.H>
#include <oflex.H>
#include <avalue.H>

#define APPENDSLOP 100		/* bytes to increase append rcvr beyond required */
#define FUNCSTACKSLOP 55	/* number of markers to leave space for after entering
					a function */

#define GLOBADDR(elt) (globloc = (TGlobRef)(*iar++ << 8),  \
		globloc += (TGlobRef)(*iar++),  \
		&ness_Globals[globloc].e.elt)

/* defining instances of globals  from envt.h */

NO_DLL_EXPORT class simpletext *ObjectCode = NULL;	/* the compiled bytecode for the functions */

NO_DLL_EXPORT struct globelt *ness_Globals = NULL;
NO_DLL_EXPORT TGlobRef ness_GlobFree = 0;
NO_DLL_EXPORT long ness_GlobSize = 0;

NO_DLL_EXPORT struct frameelt *FramePtr = NULL;	/* points to current stack frame */
NO_DLL_EXPORT union stackelement *NSPstore = NULL, 
		*NSPbase = NULL, *NSLowEnd = NULL, *NSHiEnd = NULL;
NO_DLL_EXPORT long NSSize = InitialNSSize;		/* the number of words in the stack. */
/* long nArgsGiven = 0;		number of arguments on stack when interpreter is called */
NO_DLL_EXPORT struct ATKregistryEntry  *celClass = NULL;
NO_DLL_EXPORT struct ATKregistryEntry  *textClass = NULL;
NO_DLL_EXPORT struct ATKregistryEntry  *textviewClass = NULL;
NO_DLL_EXPORT struct ATKregistryEntry  *dataobjectClass = NULL;
NO_DLL_EXPORT struct ATKregistryEntry  *lpairClass = NULL;
NO_DLL_EXPORT struct ATKregistryEntry  *viewClass = NULL;
NO_DLL_EXPORT struct ATKregistryEntry  *celviewClass = NULL;
NO_DLL_EXPORT struct ATKregistryEntry  *valueviewClass = NULL;
NO_DLL_EXPORT struct ATKregistryEntry  *imClass = NULL;
NO_DLL_EXPORT class ness *InterpretationInProgress = NULL;	/* used in ness.c */

static jmp_buf ExecutionExit;
static unsigned char *iarStore;
static const char *ErrorMsg;
static struct errornode *SavedMsg;


boolean RunError(const char  *msg, const unsigned char *iar);
static SIGNAL_RETURN_TYPE SigHandler(int);
void initializeEnvt();
union stackelement * popValue(union stackelement  *NSP);
static struct callnode * fetchaddr(unsigned char *iar);
static void InterruptNess(class ness  *ness);
static void QueryReadOnly(class ness  *ness, class simpletext  *s, const char  *msg, 
		unsigned char *iar);
struct errornode * interpretNess(short  func, class nessmark  *arg, class ness  *ness);

#if 0
boolean stackArg(union  argType  *arg, TType  type);
#endif


/* XXX the iar is no longer needed as an argument since it is saved in memory on every cycle */
/*RunError(msg, iar)
	execution errors call this routine, which passes the error along.
	'msg' describes the error.  If 'msg' begins with ':' it is a constant;
	if with a '*', it has been malloced especially for this error and
	will be freed when the error is discarded.
	'iar' is the index of the opcode that failed (it has been adjusted backward).
*/
	boolean	/* type is to fool the conditional expr in PTSTOMARK */
RunError(const char  *msg, const unsigned char *iar)  {
/* printf("RunError @ %d: %s\n", iar, msg); */
	ErrorMsg = msg;
	longjmp (ExecutionExit, 99);
	return TRUE;		/* more stuff to fool compiler */
}

	static SIGNAL_RETURN_TYPE
SigHandler(int) {
	RunError(":! ! !   Disastrous Ness error in this function.   Quit soon.   !!!!!!!!!!", 0); 
}


/* initializeEnvt()
	initialize all facets of the Ness environment
*/
	void
initializeEnvt() {
	static boolean Initialized = FALSE;
	if (Initialized) 
		return;
	Initialized  = TRUE;

	celClass = ATK::LoadClass("cel");
	textClass = ATK::LoadClass("text");
	textviewClass = ATK::LoadClass("textview");
	dataobjectClass = ATK::LoadClass("dataobject");
	lpairClass = ATK::LoadClass("lpair");
	viewClass = ATK::LoadClass("view");
	celviewClass = ATK::LoadClass("celview");
	valueviewClass = ATK::LoadClass("valueview");
	imClass = ATK::LoadClass("im");

	ObjectCode = new simpletext;

	/* ness_Globals array */
	ness_Globals = (struct globelt *)malloc(sizeof(struct globelt));
	ness_GlobSize = 1;  /* this initial one is never used */
	ness_Globals[0].e.i.hdr = idleHdr;
	ness_Globals[0].next = 0;
	ness_GlobFree = 0;

	/* Ness Execution Stack */
	NSSize = InitialNSSize;
	NSLowEnd = (union stackelement *)calloc(NSSize, sizeof(union stackelement));
	NSHiEnd = NSLowEnd + NSSize;
	NSPstore = NSHiEnd;

	/* create an initial frame record */
	NSPstore = (union stackelement *)((unsigned long)NSPstore - sizeof(struct frameelt));
	NSPstore->f.hdr = frameHdr;
	NSPstore->f.returnaddress = NULL;  
	NSPstore->f.prevframe = NULL;

	NSPbase = NSPstore;
	FramePtr = &NSPbase->f;
	/*  nArgsGiven = 0; */
}


static text *amarktext=NULL;
DEFINE_OFLEX_CLASS(amarkt,nessmark,5U);
static amarkt amarks;
/* popValue(NSP)
	removes topmost value from stack
	unlinks marks and frames
*/
	union stackelement *
popValue(union stackelement  *NSP) {
	switch (NSP->l.hdr) {
	      case (longHdr):
		NSPopSpace(longstkelt);
		break;
	      case (boolHdr):
		NSPopSpace(boolstkelt);
		break;
	      case (dblHdr):
		NSPopSpace(dblstkelt);
		break;
	      case (ptrHdr):
		NSPopSpace(ptrstkelt);
		break;
	      case (funcHdr):
		NSPopSpace(funcstkelt);
		break;
	      case (frameHdr):
		if (((long)NSP) == ((long)FramePtr))   /* (was '>=' ) */
			FramePtr = NSP->f.prevframe;
		else
			RunError(":stack is confused at illegal frame mark",
					iarStore);
		NSPopSpace(frameelt);
		break;
	    case (seqHdr): {
		/* remove link to this mark.  */
		class nessmark *n = NSP->s.v;
		(n)->ungetMark();
		NSPopSpace(seqstkelt);
		break;
		}
	      default:
		/* ERROR: just discard one long word */
		NSPstore=NSP=(union stackelement *)(((unsigned long)NSP) 
						+ sizeof(long));
		break;
	}
	return NSPstore = NSP;
}


NO_DLL_EXPORT atom_def ness_booleanatom("boolean");


size_t ness_InitArgMarks() {
    if(amarktext==NULL) {
	amarktext=new text;
	amarktext->SetReadOnly( TRUE);
    }
    return amarks.GetN();
}

void ness_ClearArgMarks(size_t bottom) {
    if(bottom<amarks.GetN()) amarktext->AlwaysDeleteCharacters(amarks[bottom].GetPos(), amarktext->GetLength()-amarks[bottom].GetPos());
    for(size_t i=amarks.GetN();i>bottom;) {
	i--;
	nessmark &m=amarks[i];
	m.DetachFromText();
	amarks.Remove(i);
    }
}

boolean ness_StackArgs(const avalueflex &args) {
    stackelement *NSP=NSPstore;
    size_t i=0;
    static ATKregistryEntry *nessmarkent=ATK::LoadClass("nessmark");
    while(i<args.GetN()) {
	if(NSP-1<=NSLowEnd) return FALSE;
	const avalue &arg=args[i];
	const atom *a=arg.Type();
	if(a==avalue::integer) {
	    NSPushSpace(longstkelt);
	    NSP->l.hdr = longHdr;
	    NSP->l.v = arg.Integer();
	} else if(a==avalue::cstring) {
	    const char *str=arg.CString();
	    nessmark *m=NULL;
	    if(str==NULL || *str=='\0')  {
		m=new nessmark;
		m->MakeConst("");
	    } else {
		long len = strlen(str);
		long pos=amarktext->GetLength();
		m=amarks.Append();
		m->SetStyle(FALSE, FALSE);
		amarktext->AlwaysInsertCharacters( pos, (char *)str, len);
		m->Set(amarktext, pos, len);
	    }
	    PUSHMARK((nessmark *)m);
	} else if(a==ness_booleanatom) {
	    NSPushSpace(longstkelt);
	    NSP->l.hdr = boolHdr;
	    NSP->l.v = arg.Integer(ness_booleanatom);
	} else if (a==avalue::real) {
	    NSPushSpace(dblstkelt);
	    NSP->d.hdr = dblHdr;
	    NSP->d.v = arg.Real(); 
	} else if (a==avalue::atkatom || ATK::IsTypeByName(a->Name(), "ATK")) {
	    /* some other kind of pointer */
	    ATK *obj=arg.ATKObject(a);
	    if(nessmarkent==obj->ATKregistry()) {
		NSPushSpace(seqstkelt);
		NSP->s.hdr=seqHdr;
		(NSP->s.v)->InitFrom((nessmark *)obj);
	    } else {
		NSPushSpace(ptrstkelt);
		NSP->p.hdr = ptrHdr;
		NSP->p.v = obj;
	    }
	}
	    
	
	i++;
    }
    return TRUE;
}

#if 0
/* we do not need stackArg for now.  The single arg passed to interpretNess 
	is enough for nessrun the code is useful because it shows how to stack things */

union  argType {
	long l;
	unsigned long b;
	double d;
	ATK  *p;
	class nessmark *m;
};

/* stackArg(arg, type)
	 puts the argument onto NSP and keeps track of the number of arguments supplied
*/
	boolean
stackArg(union  argType  *arg, TType  type) {
	union stackelement *NSP = NSPstore;
	if (NSP-1 <= NSLowEnd) {
		/* stack overflow */
		/* XXX */
		return FALSE;
	}

	switch (type) {
	case (longHdr):
	case (boolHdr):
		NSPushSpace(longstkelt);
		NSP->l.hdr = type;
		NSP->l.v = arg->l; 
		break; 
	case (dblHdr):
		NSPushSpace(dblstkelt);
		NSP->d.hdr = type;
		NSP->d.v = arg->d; 
		break;
	case (ptrHdr):
		if ((TType)arg->m->header.nessmark_methods
				== nessmarkHdr) {
			NSPushSpace(nessmark); 
			(&NSP->s)->InitFrom( arg->s);
		}
		else {
			/* some other kind of pointer */
			NSPushSpace(ptrstkelt);
			NSP->p.hdr = type;
			NSP->p.v = arg->p; 
		}
		break;
	}
	nArgsGiven ++;
	return TRUE;
}
#endif

#define FETCHADDR(iar) (iar+=sizeof(void *), fetchaddr(iar-sizeof(void *)))
	static struct callnode *
fetchaddr(unsigned char *iar) {
	unsigned long symloc = 0, i;
	for(i = 0; i < sizeof(void *); i++) {
		symloc <<= 8;
		symloc += *iar++;
	}
	return (struct callnode *)symloc;
}


	static void
InterruptNess(class ness  *ness) {
	message::DisplayString(im::GetLastUsed(), 0, "Interrupted !");
	im::ForceUpdate();
	ness->ToldUser = TRUE;
	longjmp(ExecutionExit, 1);	/* normal exit */
}

	static void
QueryReadOnly(class ness  *ness, class simpletext  *s, 
		const char  *msg, unsigned char *iar) {
	if (s == (class simpletext *)(ness)->GetDefaultText()) {
		/* see if user wants to make defaulttext read/write */
		long choice;
		static const char * const choices[] = {
			"Cancel - Leave text read-only", 
			"Read-Write - Let the Ness script modify the text", 
			NULL
		};
		if (message::MultipleChoiceQuestion(NULL, 50, 
			"The main text is read-only, but the script is trying to modify it.  Okay?",
				0, &choice, choices, "cr") >= 0 
				&& choice == 1) {
			/* ok, make it read write */
			(s)->SetReadOnly( FALSE);
			return;
		}
	}
	RunError(msg, iar);
}

	static long
ness_stkeltsize(union stackelement *s) {
	switch(s->i.hdr) {
		case frameHdr: return sizeof(struct frameelt);
		case longHdr: return sizeof(struct longstkelt);
		case boolHdr: return sizeof(struct boolstkelt);
		case dblHdr: return sizeof(struct dblstkelt);
		case ptrHdr: return sizeof(struct ptrstkelt);
		case seqHdr: return sizeof(struct seqstkelt);
		case funcHdr: return sizeof(struct funcstkelt);
		case idleHdr: return sizeof(struct idleelt);
		default:
			RunError("Internal error: size of unknown elt", 0);
			return sizeof(long);
	}
}
	void ness_InitInterp(ness *n) {
	    if (InterpretationInProgress == NULL) {	
		/* reinitialize stack pointer */
		NSPstore = NSPbase;

		/* XXX The next two lines attempt to move the gap to the end of ObjectCode 
		 XXX so iar++ will sequence through the code */

		long objlen=ObjectCode->GetLength();
		(ObjectCode)->InsertCharacters( objlen, " ", 1);
		(ObjectCode)->DeleteCharacters( objlen, 1);
	    }
	    InterpretationInProgress = n;
	}

/* interpretNess(func, ness)  -- interpret the code for the ness function func

	'func' is the location value returned by makeFunction
	presumably a compiler has put bytecodes within this mark.

	A single marker argument is supplied to the top level function.

	'ness' is used for GetArbiter,  GetDefaultText,  and GetCelScope
*/
	struct errornode *
interpretNess(short  func, ATK  *arg, class ness  *ness) {
	union stackelement *NSP;	/* stack pointer */
	unsigned char *iar;	/* next opcode to execute */
	unsigned char *iarzero;	/* point to first byte of object code */
	unsigned char *PrevAddr;	/* former value of iar before goto or call */
	long objlen = (ObjectCode)->GetLength();
	long nextiar;		/* beginning of func */
	long lengthgotten;
	long CondCode = 0;		/* set by compares; tested by branches
					0: EQ    1:GT   -1:LT   -2:error */
	SIGNAL_RETURN_TYPE (*oldBus)(int)=NULL, (*oldSeg)(int)=NULL; 
				/* save signal hndlrs */
	TGlobRef globloc;
	boolean Success;
	long gocount;
	struct osi_Times starttime;
	boolean Gmessaged = FALSE;
	long exitcode;
	jmp_buf SaveExecExit;
	struct frameelt *SaveFramePtr = FramePtr;
	class ness *SaveInterpInProg = InterpretationInProgress;

	/* SaveExecExit = ExecutionExit; */
	memmove((char *)SaveExecExit, (char *)ExecutionExit, sizeof(jmp_buf));
	if(!InterpretationInProgress) ness_InitInterp(ness);
	NSP = NSPstore;

	   FramePtr = NULL;	/* terminate the unstacking loop */
	PrevAddr = NULL;	/* this will terminate the while loop */
	/* the first frame will be built by the EnterFunction ('P') operator 
		at the beginning of the first function */
	static ATKregistryEntry *nessmarkent=ATK::LoadClass("nessmark");
	if (arg != NULL  && arg->ATKregistry()==nessmarkent) PUSHMARK((nessmark *)arg);
 
	nextiar = ness_Globals[func].e.s.v->GetPos();

	iar = (unsigned char *)(ObjectCode)->GetBuf( nextiar, 
		objlen - nextiar, &lengthgotten);
	iarzero = iar - nextiar;		/* for debugging */
	/* printf("iarzero: %d\n", iarzero); */
 
	
     if(arg!=NULL && arg->ATKregistry()!=nessmarkent) {
	    NSPushSpace(ptrstkelt);
	    NSP->p.hdr = ptrHdr;
	    NSP->p.v = arg;
     }
     if (lengthgotten < objlen-nextiar) 
		return LocateErrorFunc(iar, iarzero, 
			":Object code management failure !!  Quit soon!", ness);
	gocount = 0;
	osi_GetTimes(&starttime);

	if ((exitcode=setjmp(ExecutionExit)) != 0) {
		struct errornode * volatile msg = NULL;

		/* return here from longjmp after execution terminates
			 either an error or normal end of execution */

		/* reset the destination of the error longjmp
				(in case popValue fails) */
/* printf("ExitCode: %d\n", exitcode); */
		if (setjmp(ExecutionExit) == 0) {
			/* setjmp returns 0 when first called */
			InterpretationInProgress = NULL;

			if (exitcode == 1)  
				msg = NULL;	/* normal exit */
			else if (exitcode == 8)
				msg = SavedMsg;
			else 
				msg = LocateErrorFunc(iarStore, iarzero, 
						ErrorMsg, ness);

			/* pop stack.  Necessary to free up marks to text */
			NSP = NSPstore;
			while (FramePtr != NULL)
				NSP = popValue(NSP);
		}

		/* restore error traps */
		signal(SIGBUS, oldBus);
		signal(SIGSEGV, oldSeg);

		/* restore global variables */
		/* ExecutionExit = SaveExecExit; */
		memmove((char *)ExecutionExit, (char *)SaveExecExit,
				sizeof(jmp_buf));
		InterpretationInProgress= SaveInterpInProg;
		FramePtr = SaveFramePtr;

		if (exitcode != 1 && 
				InterpretationInProgress != NULL) {
			/* we erred in a nested execution
			  (most likely from an event within
			  launchApplication)
			  exit from the outer ness. */
/* printf("ReExit for: %s\n", msg->msg); */
			SavedMsg = msg;
			longjmp (ExecutionExit, 8);
		}

		return msg;
	}
	/* when first called setjmp returns zero so execution initially continues here */

	/* catch disasterous errors */
	oldBus = signal(SIGBUS, SigHandler);
	oldSeg = signal(SIGSEGV, SigHandler);

while (TRUE)  {
    iarStore = iar;
    switch(*iar++) {
	default:  RunError(":Illegal Opcode.  Compiler failure!", iar-1);   break;
	case '\n':	break;	/* No-Op */
	case '^': 		/* push NULL */
		NSPushSpace(ptrstkelt);
		NSP->p.hdr = ptrHdr;
		NSP->p.v = NULL;
		break;
	case '_':		/* unary minus */
		if (NSP->l.hdr != longHdr)
			RunError(":operand is not an integer value", iar-1);
		NSP->l.v = - NSP->l.v; 
		break;
	case '+':
	case '-':
	case '*':	
	case '/':
	case '%':  {
		struct longstkelt *left ;
		if (NSP->l.hdr != longHdr)
			RunError(":right operand is not an integer value", iar-1);
		left = &(&(NSP->l))[1];
		if (left->hdr != longHdr)
			RunError(":left operand is not an integer value", iar-1);
		switch (*(iar-1)) {
		case '+':  left->v += NSP->l.v;   break;
		case '-':  left->v -= NSP->l.v;   break;
		case '*':  left->v *= NSP->l.v;   break;
		case '/':  if (NSP->l.v == 0) RunError(":divide by zero", iar-1); 
			else left->v /= NSP->l.v;   break;
		case '%':  if (NSP->l.v == 0) RunError(":divide by zero", iar-1); 
			else left->v %= NSP->l.v;   break;
		}
		NSP = popValue(NSP);	/* discard right operand */
	}	break;
	case '(': {		/* FUNCVAL operations */
		unsigned char *opiar = iar-1;
		unsigned char op = *iar++;
		struct funcstkelt *src, *dest;
		unsigned long sysloc;
		static unsigned char *saveiar = NULL;  /* to call builtindef */

		switch (op) {
		case 'k':		/* load from ness_Globals */
			sysloc = (unsigned long)*iar++ << 8;
			sysloc += (unsigned long)*iar++;
			src = &ness_Globals[sysloc].e.c;
			goto Funload;
		case 'l':		/* load from stack */
			src = (struct funcstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
			goto Funload;
		Funload:
			if (src->hdr != funcHdr) {
				RunError(":tried to load non-function", opiar);
			}
			NSPushSpace(funcstkelt); 
			NSP->c = *src;
			break;
		case 's':		/* store to stack */
			dest = (struct funcstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
			goto Funstore;
		case 'v':		/* store to ness_Globals */
			sysloc = (unsigned long)*iar++ << 8;
			sysloc += (unsigned long)*iar++;
			dest = &ness_Globals[sysloc].e.c;
			goto Funstore;
		Funstore:
			if (NSP->c.hdr == ptrHdr && NSP->p.v == NULL)
				/* accept NULL ptr as funcval */
				NSP->c.hdr = funcHdr;
			else if (NSP->c.hdr != funcHdr)
				RunError(":tried to store non-function",opiar);
			/* else if we had destination callnode, we could
				check its signature versus NSP->c.call XXX */
			*dest = NSP->c;
			NSPopSpace(funcstkelt);   /*(ptrstkelt is same size)*/
			break;
		case 't':		/* compare */
			src = &(&(NSP->c))[1];
			if (NSP->c.hdr != funcHdr)
				RunError(":right operand is not a function", 
						opiar);
			if (src->hdr != funcHdr)
				RunError(":left operand is not a function", 
						opiar);

			CondCode = (src->call == NSP->c.call) ? 0 : 1;

			NSP = popValue(NSP);	/* discard right operand */
			NSP = popValue(NSP);	/* discard left operand */
			break;
		case 'C': {
			/* call the function whose callnode is on the stack.  
			  check versus dummy callnode pted at from op stream */
			struct callnode *dummy = FETCHADDR(iar);
			struct funcstkelt *callloc = 
				(struct funcstkelt *)((long)NSP+(long)*iar++);
			struct callnode *call;
			int i;
			if (callloc->hdr != funcHdr) {
				RunError(":funcval value is not a function",
						opiar);
			}
			call = callloc->call;
			if (call == NULL)
				RunError(":funcval value is NULL", opiar);

			/* if argtypes in call defered (call->nargs is -99),
				fill them in from dummy */
			if ((call->nargs == MAXARGS && call->argtype[0]==Tptr && call->argtype[1]==Tunk) || call->nargs==-9) {
				call->nargs = dummy->nargs;
				for (i=call->nargs; --i >= 0; )
					call->argtype[i] = dummy->argtype[i];
			}

			/* check number of operands and types */
			i = dummy->nargs;
			if (i != call->nargs) i = -9;
			while (--i >= 0) {
				if (dummy->argtype[i] != call->argtype[i]
						&& dummy->argtype[i] != Tunk)
						/* Tunk if arg was a call to 
							a funcval */
					break;
			}
			if (i != -1)
				RunError(
	":supplied arguments don't match function requirements", opiar);

			/* call function from 'call' (code similar to 'C') */
			NSPstore = NSP;
			if (call->variety == callSym)
				/* go find out what is being called
					This will CHANGE call->variety */
				callCheck(call, opiar, ness);

			/* now execute function as C code or Ness code */
			if (call->variety == callNess) {
				PrevAddr = iar;
				nextiar = (ness_Globals[
					call->where.Nproc].e.s.v)->GetPos();
				iar = (unsigned char *)(ObjectCode)->GetBuf(
						 
						nextiar, 1, &lengthgotten);
			}
			else if (call->variety == callBuiltin) {
				/* to call a builtin function, 
				  we append its defn to a dummy string
				  and set iar pting to it.  A final op
				  in the string returns to '(Q' 
				  which restores iar */
				static unsigned char tmpstream[50];
				if (saveiar != NULL) 
					RunError(":recursive builtin", opiar);
				PrevAddr = iar;
				saveiar = iar;
				strcpy((char *)tmpstream, call->where.bid->defn);
				strcat((char *)tmpstream, "(Q");
				iar = tmpstream;
			}
			else {
				/* callC, callPE, callGet, callSet, 
					callMethod, callClProc */
				callCfunc(call, opiar, ness);
				NSP = NSPstore;
			}
		}	break;
		case 'Q':
			/* restore after call of builtin */
			iar = saveiar;
			saveiar = NULL;
			break;
		case 'p': {
			/* push callnode from opstream onto stack */
			struct callnode *call = FETCHADDR(iar);
			NSPushSpace(funcstkelt); 
			NSP->c.hdr = funcHdr;
			NSP->c.call = call;
		} break;
		} } break;

	case '0': 		/* push zero long */
		NSPushSpace(longstkelt);
		NSP->l.hdr = longHdr;
		NSP->l.v = 0;
		break;
	case '1': 		/* push TRUE */
		NSPushSpace(boolstkelt);
		NSP->b.hdr = boolHdr;
		NSP->b.v = TRUE;	/* use C version for now */
		break;
	case '9': 		/* push FALSE */
		NSPushSpace(boolstkelt);
		NSP->b.hdr = boolHdr;
		NSP->b.v = FALSE;	/* use C version for now */
		break;

	case 'a':		/* branch LT */
		Success = (CondCode == -1);
		goto brancher;
	case 'b':		/* branch GT */
		Success = (CondCode == 1);
		goto brancher;
	case 'c':		/* branch LE */
		Success = (CondCode != 1);
		goto brancher;
	case 'd':		/* branch GE */
		Success = (CondCode != -1);
		goto brancher;
	case 'e':		/* branch EQ */
		Success = (CondCode == 0);
		goto brancher;
	case 'f':		/* branch NE */
		Success = (CondCode != 0);
		goto brancher;
	case 'g':		/* goto */
		Success = TRUE;
		goto brancher;
	case 'h':		/* branch error */
		Success = (CondCode == -2);
		goto brancher;

brancher: {
		long offset;
		unsigned char chi, clo;
		if (gocount++ > 600) {
			if (im::CheckForInterrupt()) 
				InterruptNess(ness);
			if (! Gmessaged) {
				struct osi_Times now;
				osi_GetTimes(&now);
				if (now.Secs > starttime.Secs + 7) {
					message::DisplayString(im::GetLastUsed(), 0, 
	"You can use control-G if your script is stuck in a loop");
					im::ForceUpdate();
					Gmessaged = TRUE;
				}
			}
			gocount = 0;
		}
		chi = *iar++;
		clo = *iar++;
		offset = ExtendShortSign((chi<<8) + clo);
		if (Success) {
			/* set iar to loc indicated by offset from addr of branch */
			PrevAddr = iar - 3;
			iar = PrevAddr + offset;
		}
	}	break;

	/* {"readfile", "ia", {Tstr, Tstr, Tend}, ness_codeOrange},
	   {"readrawfile", "ir", {Tstr, Tstr, Tend}, ness_codeOrange}, */

	case 'i':	{	/* read named file to stack top */
		class text *t;
		FILE *f;
		char *s;
		char fullName[MAXPATHLEN+1];
		const char * volatile fname;
		long val;
		unsigned char subop = *iar++;	/* a - ATK;   r - raw */

		/* canonicalize file name and open file */
		s = (FETCHMARK(NSP))->ToC();
		if (*s == '\0')
			f = NULL;
		else {
			fname = path::UnfoldFileName(s, fullName, 0);
			f = fopen(fname, "r");
		}
		free(s);
		NSP = popValue(NSP);		/* discard filename */

		/* create a new mark for the file to read into */
		t = new text;		
		PUSHMARK(NULL)->Set( t, 0, 0);

		if (f == NULL) 
			fprintf(stderr,  "Ness: cannot read file \"%s\"\n", 
					fname);
		else if (subop == 'a') {
			/* read an ATK file into mark on top of stack */
			val = ReadTextFileStream(t, fname, f, TRUE);
			if (val != dataobject::NOREADERROR)
				fprintf(stderr, 
					"Ness: file not in ATK format \"%s\"",
					fname);
			fclose(f);
		}
		else {	
			/* 'r' - read raw file into mark on top of stack */
			(t)->Read( f, 0);
			fclose(f);
		}
		(FETCHMARK(NSP))->SetLength( (t)->GetLength());
	}	break;

	case 'j':	{	/* print string */
		long i, end;
		class simpletext *t;
		class nessmark *n = FETCHMARK(NSP);
		t = (n)->GetText();
		i = (n)->GetPos();
		end = i + (n)->GetLength();
		while (i < end)
			putchar((t)->GetUnsignedChar( i)), i++;
		/* leave value on stack because print is a function and all such return values. */
	}	break;
	case 'k':	{	/* load string from ness_Globals */
		struct seqstkelt *s = GLOBADDR(s);
		PUSHMARK(FETCHMARK((union stackelement *)s));
	}	break;
	case 'l':	{	/* load string from stack */
			/* operand is index of arg from FramePtr */
		union stackelement *s
			= (union stackelement *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
		PUSHMARK(FETCHMARK(s));
	}	break;
	case 'm':		/* dup */
		PUSHMARK(FETCHMARK(NSP));	
		break;
	case 'n':	{	/* string next() */
		long pos;
		class nessmark *n = FETCHMARK(NSP);
		pos = (n)->GetPos() + (n)->GetLength();
		(n)->SetPos(pos);
		(n)->SetLength((pos < ((n)->GetText())->GetLength()) ? 1 : 0);
	}	break;
	case 'o':		/* string start() */
		(FETCHMARK(NSP))->SetLength( 0);
		break;
	case 'p':	{	/* string base() */
		class nessmark *n = FETCHMARK(NSP);
		(n)->SetPos( 0);
		(n)->SetLength(
			((n)->GetText())->GetLength());
	}	break;
	case 'q':	{	/* string newbase() */
		PUSHMARK(NULL)->Set(new text, 0, 0);
	}	break;
	case 'r':	{	/* string replace()  top arg is 2nd operand*/
		class nessmark *left, *right;
		class simpletext *stext;
		right = FETCHMARK(NSP);
		left = FETCHMARK((union stackelement *)(&(&NSP->s)[1]));
		stext = (left)->GetText();
		if ((stext)->GetReadOnly()) {
			NSPstore = NSP;
			QueryReadOnly(ness, stext, 
				":cannot replace any part of constant", iar-1);
		}
		(left)->Replace( right);

		NSP = popValue(NSP);	/* discard right operand */
	}	break;
	case 's':	{	/* store string to a variable on the stack */
			/* operand is index of arg from FramePtr */
		union stackelement *m		/* where to store */
			= (union stackelement *)((unsigned long)FramePtr 
					+ sizeof(struct frameelt)
					+ (unsigned long)*iar++);
		class nessmark *n = FETCHMARK(NSP);
		if (m->s.hdr != seqHdr) {
			/* we are storing into an area which was not a mark */
			m->s.hdr = seqHdr;
			m->s.v = n;		/* continue to use mark */
			NSPopSpace(seqstkelt);
		}
		else {
			(m->s.v)->SetFrom(n);
			NSP = popValue(NSP);	/* discard value stored */
		}
	}	break;
	case 't':	{	/* compare strings */
		long i, j, k;
		long len, ilen, jlen, d = 0;
		class simpletext *itext, *jtext;
		class nessmark *left, *right;

		right = FETCHMARK(NSP);
		left = FETCHMARK((union stackelement *)(&(&NSP->s)[1]));
		i = (left)->GetPos();
		ilen = (left)->GetLength();
		j = (right)->GetPos();
		jlen = (right)->GetLength();
		len = (jlen < ilen) ? jlen : ilen;
		itext = (left)->GetText();
		jtext = (right)->GetText();

		for (k=0;  k < len;  k++, i++, j++)
			if ((d = ((long)(itext)->GetUnsignedChar( i)) 
					- ((long)(jtext)->GetUnsignedChar( j))) != 0)
				break;
		if (k == len) 
			/* both equal until go off the end of one 
				the longer is the greater */
			CondCode = (ilen > jlen)  ?  1  :  (ilen == jlen)  ?  0  :  -1;
		else 
			/* reached an unequal character */
			CondCode = (d>0)  ?  1  :  -1;

		NSP = popValue(NSP);	/* discard right operand */
		NSP = popValue(NSP);	/* discard left operand */
	}	break;
	case 'u':		/* compare string to EMPTY  (is = or >) */
		CondCode = ((FETCHMARK(NSP))->GetLength() == 0)  ?  0  :  1;
		NSP = popValue(NSP);	/* discard operand */
		break;
	case 'v':	{	/*store string to Ness_Globals */
		struct seqstkelt *m = GLOBADDR(s);
		class nessmark *n = FETCHMARK(NSP);

		if (m->hdr != seqHdr) {
			/* we are storing into an area which was not a mark */
			m->hdr = seqHdr;
			m->v = n;		/* continue to use mark */
			NSPopSpace(seqstkelt);
		}
		else {
			(m->v)->SetFrom(n);
			NSP = popValue(NSP);	/* discard value stored */
		}
	}	break;
	case 'w':	{	/* previous(), nextn(), length()*/
		unsigned char subop = *iar++;
		long n;
		switch (subop) {
		case 'p':  {
			class nessmark *n = FETCHMARK(NSP);
			if ((n)->GetPos() > 0) {
				(n)->SetPos((n)->GetPos() - 1);
				(n)->SetLength( 1);
			}
			else (n)->SetLength( 0);
		}	break;
		case 'n':
			if (NSP->l.hdr != longHdr)
				RunError(":tried to nextn with non-integer value", iar - 2);
			n = NSP->l.v;
			NSP = popValue(NSP);
			/* leave mark on stack and revise its value */
			(FETCHMARK(NSP))->NextN( n);
			break;
		case 'l':
			n = (FETCHMARK(NSP))->Length();
			NSP = popValue(NSP);
			/* push n */
			NSPushSpace(longstkelt); 
			NSP->l.hdr = longHdr;
			NSP->l.v = n;
			break;
		}
	}	break;
	case 'x':	{	/* string extent()  top arg is 2nd operand*/
		class nessmark *left, *right;
		right = FETCHMARK(NSP);
		left = FETCHMARK((union stackelement *)(&(&NSP->s)[1]));

		if ((left)->GetText() != (right)->GetText())
			(left)->MakeConst("");	/* EmptyText IS UNIQUE */
		else {
			int start = (left)->GetPos();
			int end = (right)->GetPos() 
				+ (right)->GetLength();
			if (end < start)
				start = end;
			(left)->SetPos( start);
			(left)->SetLength( end - start);
		}
		NSP = popValue(NSP);	/* discard right operand */
	}	break;
	case 'y':			/*  pop  */
		NSP = popValue(NSP);
		break;
	case 'z':	{		/* swap top two operands */
		union stackelement temp, *second;
		long topsize, secondsize;

		topsize = ness_stkeltsize(NSP);
		second = (union stackelement *)(((long)NSP) + topsize);
		secondsize = ness_stkeltsize(second);
		memcpy(&temp, NSP, topsize);
		memmove(NSP, second, secondsize);
		second = (union stackelement *)(((long)NSP) + secondsize);
		memcpy(second, &temp, topsize);
	}	break;	


	case 'A':	{	/* append top value on stack to second */
		class nessmark *source, *rcvr;
		long rcvrlen;
		class simpletext *stext;
		source = FETCHMARK(NSP);
		rcvr = FETCHMARK((union stackelement *)(&(&NSP->s)[1]));

		stext = (rcvr)->GetText();
		if ((stext)->GetReadOnly()) {
			NSPstore = NSP;
			QueryReadOnly(ness, stext, 
				":cannot append to constant", iar-1);
		}

		/* ensure that rcvr is the finish of its base */
		rcvrlen = (stext)->GetLength();
		(rcvr)->SetPos( rcvrlen);
		(rcvr)->SetLength( 0);

		/* put source into rcvr and compute base()*/
		(rcvr)->Replace( source);
		(rcvr)->SetPos( 0);
		(rcvr)->SetLength( rcvrlen + (source)->GetLength());

		NSP = popValue(NSP);	/* discard source */
	}	break;
	case 'B':	{	/* boolean operations */
		unsigned char *opiar = iar-1;
		unsigned char op = *iar++;
		struct boolstkelt *src, *dest;
		switch (op) {
		case 'k':		/* load from ness_Globals */
			src = GLOBADDR(b);
			goto Bload;
		case 'l':		/* load from stack */
			src = (struct boolstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
		Bload:
			if (src->hdr != boolHdr)
				RunError(":tried to load non-Boolean value", opiar);
			NSPushSpace(boolstkelt); 
			NSP->b = *src;
			break;
		case 's':		/* store to stack */
			dest = (struct boolstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
			goto Bstore;
		case 'v':		/* store to ness_Globals */
			dest = GLOBADDR(b);
			goto Bstore;
		Bstore:
			if (NSP->b.hdr != boolHdr)
				RunError(":tried to store non-Boolean value", opiar);
			*dest = NSP->b;
			NSPopSpace(boolstkelt);
			break;
		case 't':		/* compare */
			src = &(&(NSP->b))[1];
			if (NSP->b.hdr != boolHdr)
				RunError(":right operand is not a boolean value", opiar);
			if (src->hdr != boolHdr)
				RunError(":left operand is not a boolean value", opiar);
			CondCode = (NSP->b.v == src->v) ? 0 :
					(src->v < NSP->b.v) ? -1 : 1;
			NSP = popValue(NSP);	/* discard right operand */
			NSP = popValue(NSP);	/* discard left operand */
			break;
		}
	}	break;
	case 'C':	{	/* call an unknown function on the object atop stack */
			/* operand is four bytes giving address of a callnode  */
		struct callnode *call = FETCHADDR(iar);

		NSPstore = NSP;
		if (call->variety == callSym)
			/* go find out what is being called and check arg types
				This will CHANGE call->variety */
			callCheck(call, iar-sizeof(void *)-1, ness);

		/* now execute function as C code or Ness code */
		if (call->variety == callNess) {
			PrevAddr = iar;
			nextiar = ness_Globals[call->where.Nproc].e.s.v->
						GetPos();
			iar = (unsigned char *)(ObjectCode)->GetBuf( 
					nextiar, 1, &lengthgotten);
		}
		else {
			/* callC, callPE, callGet, callSet, 
				callMethod, callClProc */
			callCfunc(call, iar-1-sizeof(void *), ness);
			NSP = NSPstore;
		}
	}	break;
	case 'D':{	/* Rexx-like functions */
		unsigned char op = *iar++;
		DoRex(op);
		NSP = NSPstore;
	}	break;
	case 'E':		/* load to stack top a pointer to the current textview object */
		if ((ness)->GetDefaultText() == NULL)
			RunError(":there is no default text", iar-1);
		NSPushSpace(ptrstkelt);
		NSP->p.hdr = ptrHdr;
		NSP->p.v = (ATK  *)(ness)->GetDefaultText(); 
		break;
	case 'F': {		/* one of the search functions */
		unsigned char op = *iar++;
		SearchOp(op, iar-2);
		NSP = NSPstore;
	}	break;
	case 'G':	 {	/* make inset have input focus */
		class view *v = NULL;
		if (NSP->p.hdr != ptrHdr || NSP->p.v == NULL 
				|| ! (NSP->p.v)->IsType( viewClass)) 
			 RunError(":Arg was not a pointer to a view", iar-1);
		else v = (class view *)NSP->p.v;
		NSP = popValue(NSP);	/* discard view pointer */
		(v)->WantInputFocus( v);	  /* try to give it the input focus */
	}	break;
	case 'H': {	/* unary operations on real values */
		unsigned char op = *iar++;
		realUnary(op, iar-2);
	}	break;
	case 'I':	{	/* integer operations */
		unsigned char *opiar = iar-1;
		unsigned char op = *iar++;
		struct longstkelt *src, *dest;
		long t;
		switch (op) {
		case 'k':		/* load from ness_Globals */
			src = GLOBADDR(l);
			goto Iload;
		case 'l':		/* load from stack */
			src = (struct longstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
		    Iload:
			if (src->hdr != longHdr)
				RunError(":tried to load non-integer value", opiar);
			NSPushSpace(longstkelt); 
			NSP->l = *src;
			break;
		case 's':		/* store to stack */
			dest = (struct longstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
			goto Istore;
		case 'v':		/* store to ness_Globals */
			dest = GLOBADDR(l);
			goto Istore;
		Istore:
			if (NSP->l.hdr != longHdr)
				RunError(":tried to store non-integer value", opiar);
			*dest = NSP->l;
			NSPopSpace(longstkelt);
			break;
		case 't':		/* compare */
			src = &(&(NSP->l))[1];
			if (NSP->l.hdr != longHdr)
				RunError(":right operand is not an integer value", opiar);
			if (src->hdr != longHdr)
				RunError(":left operand is not an integer value", opiar);
			t = src->v - NSP->l.v;
			CondCode = (t == 0) ? 0 : (t<0) ? -1 : 1;
			NSP = popValue(NSP);	/* discard right operand */
			NSP = popValue(NSP);	/* discard left operand */
			break;
		}
	}	break;
	case 'J':	{	/* call one of various object functions */
		unsigned char op = *iar++;
		callCheat(op, iar-2, ness);
		NSP = NSPstore;
	}	break;
	case 'K':	{	/* dokeys(obj, keys)  or DoMenu(obj, item) */
		class view *OldIF;
		struct ptrstkelt *p;
		class view *t;
		class im *im;
		char *option;
		unsigned char op = *iar++;

		/* (here we assume top is seqstkelt; will test later) */
		p = (struct ptrstkelt *)((unsigned long)NSP + 
				sizeof(struct seqstkelt));
		t = (class view *)p->v;
		if (p->hdr != ptrHdr  || t == NULL 
				|| ! (t)->IsType( viewClass))
			 RunError(":first argument was not a view", iar-2);
		/*  can't use ProperPtr because we do not know 
			the type of the child of the cel */
		if (t == ness->CurrentInset  &&  (t)->IsType( celviewClass)) {
			/* is celview:  use application or truechild */
			if (((class celview *)t)->GetApplication() != NULL)
				t = ((class celview *)t)->GetApplication();
			else if (((class celview *)t)->GetTrueChild() != NULL)
				t = ((class celview *)t)->GetTrueChild();
		}
		im = (t)->GetIM();
		if (im == NULL) goto pop1;
		OldIF = (im)->GetInputFocus();
		/* is all this input focus stuff really necessary? -rr2b */
		if (t != OldIF) {
			/* try to give the input focus to t */
			(t)->WantInputFocus( t);
			if (!(t)->IsAncestor((im)->GetInputFocus()))
				/* t did not get IF.  Give up ? XXX */
				goto pop1;
		}
		option = (FETCHMARK(NSP))->ToC();
		if (op == 'k')
			(im)->DoKeySequence( option);
		else if (op == 'm')
			(im)->DoMenu( option);
		free(option);

		if (t != OldIF)
			/* restore input focus */
			(OldIF)->WantInputFocus( OldIF);
	pop1:
		NSP = popValue(NSP);	/* discard string operand,
			leave object as "value" of function */
	}	break;
	case 'L':	{	/* textimage() - convert value to string */
		char buf[50];
		switch (NSP->i.hdr) {
		case ptrHdr:
			if (NSP->p.v == NULL)
				sprintf(buf, "NULL");
			else  sprintf(buf, "0x%p", NSP->p.v);
			break;
		case funcHdr:
			if (NSP->c.call == NULL)
				sprintf(buf, "NULL function");
			else  sprintf(buf, "function %s", (char *)NSP->c.call->Sym);
			break;
		case boolHdr:
			if (NSP->b.v == TRUE) strcpy(buf, "True");
			else if (NSP->b.v == FALSE) strcpy(buf, "False");
			else sprintf(buf, "0x%lx", NSP->b.v);
			break;
		case longHdr:
			sprintf(buf, "%ld", NSP->l.v);
			break;
		case dblHdr: {
			char *bx;			
#if !defined(VAX_ENV) && !defined(PMAX_ENV)
#if (! SY_U5x && ! SY_AIXx && !SY_OS2)
			if (isnan(NSP->d.v) == 1)
				sprintf(buf, "*NaN*");
			else
#endif /* (!SYSV && !AIX) */
#endif /* !defined(VAX_ENV) && !defined(PMAX_ENV) */
			    sprintf(buf, "%0.5g", NSP->d.v);
			for (bx = buf; *bx != '\0' 
					&& *bx != '.'
					&& *bx != 'e'  
					&& *bx != 'N';    bx++)  {}
			if (*bx == '\0')
				/* value does not indicate it is real */
				strcat(buf, ".");   /* make it real */
		}	break;
		case seqHdr:
			/* leave the arg as the value
				XXX should probably make a copy  */
			goto leaveitalone;
		default:
			RunError(":cannot convert value to string", iar-1);
		}
		NSP = popValue(NSP);	/* discard value */
		PUSHMARK(NULL)->MakeConst( buf);
	leaveitalone:
		break;
	}
	case 'M': {	/* operations on real numbers */
		unsigned char op = *iar++;
		realOther(op, iar-2);
		NSP = NSPstore;
	}	break;
	case 'N':		/* print a newline */
		putchar('\n');
		break;
	case 'O':	{	/* call a function */
			/* operand is two bytes giving index into Ness_Globals */
		long lengthgotten;
		if (gocount++ >600) {
			if (im::CheckForInterrupt()) 
				InterruptNess(ness);
			else gocount = 0;
		}
		nextiar = GLOBADDR(s)->v->GetPos();
		PrevAddr = iar;
		iar = (unsigned char *)(ObjectCode)->GetBuf( 
				nextiar, 1, &lengthgotten);
	}	break;
	case 'P':	{	/* enter a function */
			/* operand is number of bytes of locals */
		unsigned long NlocBytes = (unsigned long)*iar++;
		unsigned long *t = (unsigned long *)NSP;

		NSP = (union stackelement *)((unsigned long)NSP - NlocBytes);
		/* NSPstore gets set with NSPushSpace(frameelt) below */

		if (NSP - FUNCSTACKSLOP <= NSLowEnd)
			RunError(":stack overflow on function entry", iar-2);

		/* zero out the locals */
			/* bzero(NSP, NlocBytes); */
		while (--t >= (unsigned long *)NSP) 
			*t = 0L; 

		NSPushSpace(frameelt);	
		NSP->f.hdr = frameHdr;
		NSP->f.returnaddress = PrevAddr;  
		NSP->f.prevframe = FramePtr;
		FramePtr = &NSP->f;
	}	break;
	case 'Q':  {	/* return from function call */
		long eltsize;
		union stackelement *tsp = NSP,
			*targ = (union stackelement *)(((unsigned long)FramePtr) 
					+ sizeof(struct frameelt)
					+(unsigned long)*iar++);
		iar = FramePtr->returnaddress;
		eltsize = ness_stkeltsize(tsp);

		/* pop everything off stack, unlinking marks
			and resetting FramePtr  */
		/* but first skip over return value */
		NSP = (union stackelement *)((unsigned long)NSP + eltsize); 
		while (NSP < targ) 
			NSP = popValue(NSP);

		/* copy return value, last word first */
		long nwords = eltsize/sizeof(unsigned long);
		unsigned long *src, *dest;
		src = (unsigned long *)tsp + nwords;
		dest = (unsigned long *)NSP;
		NSP = (union stackelement *)((unsigned long) NSP - eltsize);
		NSPstore = NSP;
		/* it is (allegedly) slower to unroll the following loop
				because it fits in the instruction cache */
		while (src > (unsigned long *)tsp) 
			 *--dest = *--src;
		if (iar == 0) 
			longjmp(ExecutionExit, 1);
		if (FramePtr < &NSP->f || FramePtr > &NSPbase->f)
			RunError(":frame error on function exit", iarStore);
	} 	break;	
	case 'R': {	/* operations on real numbers */
		unsigned char *opiar = iar-1;
		unsigned char op = *iar++;
		struct dblstkelt *src, *dest;
		double t;
		switch (op) {
		case 'k':		/* load from ness_Globals */
			src = GLOBADDR(d);
			goto Rload;
		case 'l':		/* load from stack */
			src = (struct dblstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
			goto Rload;
		Rload:
			if (src->hdr != dblHdr)
				RunError(":tried to load non-real value", opiar);
			NSPushSpace(dblstkelt); 
			NSP->d = *src;
			break;
		case 's':		/* store to stack */
			dest = (struct dblstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
			goto Rstore;
		case 'v':		/* store to ness_Globals */
			dest = GLOBADDR(d);
			goto Rstore;
		Rstore:
			if (NSP->d.hdr != dblHdr)
				RunError(":tried to store non-real value", opiar);
			*dest = NSP->d;
			NSPopSpace(dblstkelt);
			break;
		case 't':		/* compare */
			src = &(&(NSP->d))[1];
			if (NSP->d.hdr != dblHdr)
				RunError(":right operand is not a real value", opiar);
			if (src->hdr != dblHdr)
				RunError(":left operand is not a real value", opiar);
			t = src->v - NSP->d.v;
			CondCode = (t == 0) ? 0 : (t<0) ? -1 : 1;
			NSP = popValue(NSP);	/* discard right operand */
			NSP = popValue(NSP);	/* discard left operand */
			break;
		}
	}	break;
	case 'S':	{	/* get current selection as a marker */
		class textview *v = NULL;
		unsigned char *opiar = iar-1;
		unsigned char op = *iar++;
		if (NSP->p.hdr != ptrHdr ||  
				(v=(class textview *)ProperPtr((ATK  *)NSP->p.v,
						textviewClass)) == NULL) 
			 RunError(":Arg was not a pointer to a textview", opiar);
		NSP = popValue(NSP);	/* discard view pointer */
		PUSHMARK(NULL);
		if (v == NULL)  {}
		else if (op == 's') {
			/* currentselection(textview) */
			(NSP->s.v)->Set( 
				(class simpletext *)v->dataobject,
				(v)->GetDotPosition(),
				(v)->GetDotLength());
		}
		else {
			/* op == 'm' :  currentmark(textview) */ 
			(NSP->s.v)->Set( 
				(class simpletext *)v->dataobject,
				(v->atMarker)->GetPos(),
				(v->atMarker)->GetLength());
		}
	}	break;	
	case 'T':		/* test TOS for TRUE (or FALSE) */
		if (NSP->b.hdr != boolHdr) 
			RunError(":Not a pointer to a mark", iar);
		CondCode = (NSP->b.v == TRUE) ? 0 : -1;
		NSP = popValue(NSP);	/* discard value */
		break;
	case 'U':	{	/* call a user interface information function */
		unsigned char op = *iar++;
		neventInfo(op, iar-2, ness);
		NSP = NSPstore;
	}	break;
	case 'V':	{	/* pointer operations */
		unsigned char *opiar = iar-1;
		unsigned char op = *iar++;
		struct ptrstkelt *src, *dest;
		switch (op) {
		case 'k':		/* load from ness_Globals */
			src = GLOBADDR(p);
			goto Vload;
		case 'l':		/* load from stack */
			src = (struct ptrstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
		Vload:
			if (src->hdr != ptrHdr)
				RunError(":tried to load non-pointer value", opiar);
			NSPushSpace(ptrstkelt); 
			NSP->p = *src;
			break;
		case 's':		/* store to stack */
			dest = (struct ptrstkelt *)((unsigned long)FramePtr
					+ sizeof(struct frameelt) 
					+ (unsigned long)*iar++);
			goto Vstore;
		case 'v':		/* store to ness_Globals */
			dest = GLOBADDR(p);
			goto Vstore;
		Vstore:
			if (NSP->p.hdr != ptrHdr)
				RunError(":tried to store non-pointer value", opiar);
			*dest = NSP->p;
			NSPopSpace(ptrstkelt);
			break;
		case 't':		/* compare */
			src = &(&(NSP->p))[1];
			if (NSP->p.hdr != ptrHdr)
				RunError(":right operand is not a pointer value", opiar);
			if (src->hdr != ptrHdr)
				RunError(":left operand is not a pointer value", opiar);
			CondCode = (src->v == NSP->p.v) ? 0 : 1;
			NSP = popValue(NSP);	/* discard right operand */
			NSP = popValue(NSP);	/* discard left operand */
			break;
		}
	}	break;	

	/* {"writefile", "zWf", {Tstr, Tstr, Tstr, Tend}, ness_codeGreen},
	    {"writerawfile", "zWr", {Tstr, Tstr, Tstr, Tend}, ness_codeGreen},
	    {"writeobject", "zWo", {Tobj, Tobj, Tstr, Tend}, ness_codeGreen},
	*/
	case 'W':{	/* write second stack elt to file named by topmost */
			/* the writefile() and writeobject() functions have
				filename as 1st arg;  a 'swap' is compiled prior
				to the call on this operator, so the contents can
				be returned as the value of the writefile */
		unsigned char op = *iar++;
		class nessmark * volatile contents;
		ATK  * volatile obj;
		class text *t, *tempt;
		char *s;
		FILE *f;
		long pos, len;
		char fullName[MAXPATHLEN+1];
		struct ptrstkelt * volatile p;

		/* (assume seqstkelt at top;  check later) */
		if (op != 'o') {
			contents = FETCHMARK((union stackelement *)
					(&(&NSP->s)[1]));
		}
		else {
			p = (struct ptrstkelt *)(&(&NSP->s)[1]);
			if (p->hdr != ptrHdr)
				RunError(":2nd arg should be object", 0);
			obj = p->v;
			if (obj == NULL)
				RunError(":attempted to write NULL object", 0);
			obj = ProperPtr(obj, dataobjectClass);
			if (obj == NULL)
				RunError(":cannot find dataobject for object",
						 0);
		}

		/* open the file */
		s = (FETCHMARK(NSP))->ToC();
		if (*s == '\0')
			f = NULL;
		else 
			f = fopen(path::UnfoldFileName(s, fullName, 0), "w");
		free(s);
		if (f == NULL) {
			NSP = popValue(NSP);	/* discard filename */
			/* file failure: return null string or null object*/
			if (op != 'o')	/* nessmark_Start */
				(contents)->SetLength( 0);
			else
				p->v = NULL;
			break;
		}
		if (op == 'o') {
			/* write object to file */
			((class dataobject *)obj)->Write( f,
							  im::GetWriteID(), 0);
			fclose(f);
			NSP = popValue(NSP);	/* discard filename */
			break;
		}

		/* write marker contents to file */
		pos = (contents)->GetPos();
		len = (contents)->GetLength();
		t = (class text *)(contents)->GetText();
		if (op == 'r') {
			((class simpletext *)t)->WriteSubString( 
					pos, len, f, FALSE);
			fclose(f);
			NSP = popValue(NSP);	/* discard filename */
			break;
		}

		if ((t)->IsType( textClass) && (t->rootEnvironment)->NumberOfChildren() > 0
				&& t->templateName == NULL) {
			/*  There's at least one style, and no template.  Read one.  */
			class stylesheet *s =(t)->GetStyleSheet();
			char *sstpl;
			(t)->ReadTemplate( 
				(s != NULL
					&&  (sstpl=(s)->GetTemplateName()) 
						!= NULL
					&&  *sstpl != '\0')
				    ? sstpl : "default",      FALSE);
		}

		if (pos == 0  &&  len == (t)->GetLength()) 
			/* write the whole thing */
			(t)->Write( f, im::GetWriteID(), 0);
		else {
			/* copy to a temp text to do the substring 
			   (WriteSubString does not put on \begindata...) */
			tempt = new text;
			(tempt)->AlwaysCopyTextExactly( 0, t, pos , len);
			(tempt)->Write( f, im::GetWriteID(), 0);
			(tempt)->Destroy();
		}
		fclose(f);
		NSP = popValue(NSP);	/* discard filename */

	}	break;
	case 'X':	 {		/* system() */
		char *buf, *combuf;
		FILE *output;
		char frombuf[1025];
		class simpletext *t;
		long i, end;
		class nessmark *n;
		boolean allowexec = 
			! environ::GetProfileSwitch("SecurityConscious",
						FALSE);
				
		buf = (FETCHMARK(NSP))->ToC();
		i = strlen(buf);
		combuf = (char *)malloc(i+20);
		sprintf(combuf, "(%s)</dev/null", buf);		/* supply empty stdin */
		free(buf);
		NSP = popValue(NSP);	/* discard the argument */

		/* create a new mark for the file to read into  */
		t = new simpletext;
		n = PUSHMARK(NULL);
		n->Set(t, 0, 0);
		if (allowexec) 
			output = popen(combuf, "r");
		else output = NULL;

		free(combuf);
		end = 0;
		if (output != NULL) {
			/* now read the file into mark on top of stack */
			while (! feof(output)) {
				i = fread(frombuf, sizeof(*frombuf), 
					sizeof(frombuf), output);
				if (i <= 0) continue;
						// sometimes i is zero
						// before eof  -wjh
				(t)->InsertCharacters( end, frombuf, i);
				end += i;
			}
			pclose(output);
		}
		else {
			static const char msg[] = "ERROR: Could not execute command";
			(t)->InsertCharacters( end, msg, strlen(msg));
			end += strlen(msg);
		}
		(n)->SetLength( end);
	}	break;

	case 'Y':	{	/*  setcurrentselection(textview, mark) */
		struct ptrstkelt *p;
		class textview *t = NULL;
		class nessmark *n = FETCHMARK(NSP);
		p = (struct ptrstkelt *)(&(&NSP->s)[1]);
		if (p->hdr != ptrHdr  ||
				(t=(class textview *)ProperPtr((ATK  *)p->v,
					textviewClass)) == NULL)
			 RunError(":first argument was not a textview", iar-1);
		if ((class simpletext *)t->dataobject
					== (n)->GetText())  {
			(t)->SetDotPosition( (n)->GetPos());
			(t)->SetDotLength( (n)->GetLength());
			(t)->FrameDot( (n)->GetPos());
		}
		else
			 RunError(":marker was not for text in the given object", iar-1);
		NSP = popValue(NSP);	/* discard selection marker */
		NSP = popValue(NSP);	/* discard object */
	}	break;
	case 'Z': {		/* last() */
		class nessmark *n = FETCHMARK(NSP);
		if ((n)->GetLength() > 0) {
			(n)->SetPos((n)->GetPos() + (n)->GetLength() -1);
			(n)->SetLength( 1);
		}
	}	break;

    }	/* end switch(*iar++) */
}	/* end while(TRUE) */

}	/* end interpretNess() */

