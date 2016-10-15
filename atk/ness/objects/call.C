#define NESS_INTERNALS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *  Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved *
\* ********************************************************************** */

#include <andrewos.h>
#include <filetype.H>
#include <environ.H>
#include <proctable.H>
#include <dataobject.H>
#include <tlex.H>
#include <im.H>
#include <view.H>
#include <text.H>
#include <viewref.H>
#include <arbiterview.H>
#include <arbiter.H>
#include <celview.H>

#include <nessmark.H>
#include <ness.H>
ATK_IMPL("prochook.H")
#include <prochook.H>

#include "nessint.h"

#define BUFLEN 300



/* defining occurrence for declaration in ness.hn */
/* NO_DLL_EXPORT */ const char * const callvarietyname[] = {
	"callC",
	"callPE",
	"callSym",
	"callNess",
	"callBltn",
	"callMeth",
	"callClPr",
	"callGet",
	"callSet",
	"callNone",
	NULL
};

static struct libnode *LibList = NULL;
NO_DLL_EXPORT nesssym_scopeType LibScope = nesssym_GLOBAL;

/* {{fields name and defn should be unsigned char, but the Sun considers
	character constants to be *signed* (bletch) }} */
static const struct builtindef builtintable[] = {
	{"next", "n", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"start", "o", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"base", "p", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"newbase", "q", {Tstr, Tend}, ness_codeOrange, NULL},
	{"replace", "r", {Tstr, Tstr, Tstr, Tend}, ness_codeYellow, NULL},
	{"extent", "x", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},

	{"previous", "wp", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"finish", "no", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"front", "on", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"rest", "monnzx", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"first", "mmonnzxox", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"second", "monnzxmmonnzxox", {Tstr, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"last", "Z", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"allprevious", "ompzx", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"allnext", "nmpx", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"advance", "monnoznnox", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"length", "wl", {Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"nextn", "wn", {Tstr, Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"toend", "mpx", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"copy", "qzr", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},

	/* F* search.c */
	/* SearchOp's with two marker args */
	{"search", "Fa", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"match", "Fb", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"anyof", "Fc", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"span", "Fd", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"token", "Fe", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"regsearch", "Ff", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"regsearchreverse", "Fg", {Tstr, Tstr, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"searchforstyle", "Fh", {Tstr, Tstr, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"definestyle", "zFi", {Tstr, Tstr, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"addstyles", "Fj", {Tstr, Tstr, Tstr, Tend}, ness_codeYellow, NULL},
	{"hasstyles", "Fk", {Tbool, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"addstylebyname", "Fl", {Tstr, Tstr, Tstr, Tend}, 
						ness_codeYellow, NULL},
	/* SearchOp's with one marker arg */
	{"parseint", "Fp", {Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"parsereal", "Fq", {Tdbl, Tstr, Tend}, ness_codeOrange, NULL},
	{"firstobject", "Fr", {Tptr, Tstr, Tend}, ness_codeOrange, NULL},
	{"nextstylegroup", "Fs", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"enclosingstylegroup", "Ft", {Tstr, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"clearstyles", "Fu", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"nextstylesegment", "Fv", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	/* SearchOp with no arg */
	{"whereitwas", "Fw", {Tstr, Tend}, ness_codeOrange, NULL},
	/* SearchOp with three args */
	{"replacewithobject", "Fx", {Tstr, Tstr, Tptr, Tstr, Tend},
						ness_codeYellow, NULL},

	/* interp.c */
	{"textimage", "L", {Tstr, Tunk, Tend}, ness_codeOrange, NULL},
	{"readfile", "ia", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"readrawfile", "ir", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"writefile", "zWf", {Tstr, Tstr, Tstr, Tend}, ness_codeGreen, NULL},
	{"writerawfile", "zWr", {Tstr, Tstr, Tstr, Tend}, 
						ness_codeGreen, NULL},
	{"writeobject", "zWo", {Tptr, Tptr, Tstr, Tend}, ness_codeGreen, NULL},
	{"print", "j", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"printline", "jN", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"system", "X", {Tstr, Tstr, Tend}, ness_codeGreen, NULL},
	{"launchapplication", "UP", {Tvoid, Tbool, Tstr, Tstr, Tstr, Tend}, 
						ness_codeGreen, NULL},
	{"isreadonly", "UR", {Tbool, Tstr, Tend}, ness_codeOrange, NULL},
	{"telluser", "UT", {Tvoid, Tstr, Tend}, ness_codeOrange, NULL},
	{"askuser", "UU", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},

	{"currentselection", "Ss", {Tstr, Tunk, Tend}, ness_codeOrange, NULL},
	{"currentmark", "Sm", {Tstr, Tunk, Tend}, ness_codeOrange, NULL},
	{"setcurrentselection", "Y", {Tvoid, Tstr, Tptr, Tend}, 
						ness_codeOrange, NULL},
	{"queueanswer", "UQ", {Tvoid, Tstr, Tend}, ness_codeGreen, NULL},
	{"queuecancellation", "UC", {Tvoid, Tend}, ness_codeGreen, NULL},
	{"dohit", "UH", {Tptr, Tlong, Tlong, Tlong, Tptr, Tend}, 
						ness_codeYellow, NULL},
	{"focus", "G", {Tvoid, Tptr, Tend}, ness_codeOrange, NULL},
	{"dokeys", "Kk", {Tvoid, Tstr, Tptr, Tend}, ness_codeGreen, NULL},
	{"domenu", "Km", {Tvoid, Tstr, Tptr, Tend}, ness_codeGreen, NULL},

	/* U*  nevent.c */
	{"value_getvalue", "Un", {Tlong, Tptr, Tend}, ness_codeOrange, NULL},
	{"value_getarraysize", "Uo", {Tlong, Tptr, Tend}, 
						ness_codeOrange, NULL},
	{"value_getstring", "Up", {Tstr, Tptr, Tend}, ness_codeOrange, NULL},
	{"value_getarrayelt", "Uq", {Tstr, Tlong, Tptr, Tend}, 
						ness_codeOrange, NULL},
	{"value_setvalue", "Ur", {Tvoid, Tlong, Tptr, Tend}, 
						ness_codeYellow, NULL},
	{"value_setarraysize", "Us", {Tvoid, Tlong, Tptr, Tend}, 
						ness_codeYellow, NULL},
	{"value_setstring", "Ut", {Tvoid, Tstr, Tptr, Tend}, 
						ness_codeYellow, NULL},
	{"value_setarrayelt", "Uu", {Tvoid, Tstr, Tlong, Tptr, Tend}, 
						ness_codeYellow, NULL},
	{"value_setnotify", "Uv", {Tvoid, Tbool, Tptr, Tend}, 
						ness_codeYellow, NULL},

	/* J*  call.c */
	{"im_forceupdate", "Jq", {Tvoid, Tend}, ness_codeOrange, NULL},
	{"inset", "Jr", {Tptr, Tstr, Tend}, ness_codeOrange, NULL},
	{"new", "Js", {Tptr, Tptr, Tend}, ness_codeOrange, NULL},
	{"class", "Jt", {Tptr, Tunk, Tend}, ness_codeOrange, NULL},

	/* H*  real.c */
	/* unary real operators */
	{"acos", "Ha", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"asin", "Hc", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"atan", "He", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"cos", "Hi", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"cosh", "Hj", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"erf", "Hk", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"erfc", "Hl", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"exp", "Hm", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"fabs", "Ho", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"j0", "Hr", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"j1", "Hs", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"log", "Hu", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"log10", "Hv", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"sin", "Hy", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"sinh", "Hz", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"sqrt", "HA", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"tan", "HB", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"tanh", "HC", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"y0", "HD", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"y1", "HE", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},

	{"lgamma", "Ht", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"acosh", "Hb", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"asinh", "Hd", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"atanh", "Hf", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"cbrt",  "Hg", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"expm1", "Hn", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"log1p", "Hw", {Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},

	/* M*  real.c */
	/* binary and other real operators */
	{"atan2",  "Ma", {Tdbl, Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"hypot",  "Mb", {Tdbl, Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},
	{"pow", "Mx", {Tdbl, Tdbl, Tdbl, Tend}, ness_codeOrange, NULL},	/* Mx out of order */

	/* real to integer */
	{"round",  "Mc", {Tlong, Tdbl, Tend}, ness_codeOrange, NULL},
	{"floor",  "Md", {Tlong, Tdbl, Tend}, ness_codeOrange, NULL},
	{"ceil",  "Me", {Tlong, Tdbl, Tend}, ness_codeOrange, NULL},

	/* real to boolean */
	{"isnan",  "Mf", {Tbool, Tdbl, Tend}, ness_codeOrange, NULL},
	{"finite",  "Mg", {Tbool, Tdbl, Tend}, ness_codeOrange, NULL},

	/* integer to real */
	{"float",  "Mj", {Tdbl, Tlong, Tend}, ness_codeOrange, NULL},
	{"double",  "Mj", {Tdbl, Tlong, Tend}, ness_codeOrange},  /* TELMAT */

	/* (integer, real) => real */
	{"jn",  "Mk", {Tdbl, Tdbl, Tlong, Tend}, ness_codeOrange, NULL},
	{"yn",  "Ml", {Tdbl, Tdbl, Tlong, Tend}, ness_codeOrange, NULL},

	/* "Mx" is above */	

	/* rexf.c */
	{"centerx", "Da", {Tstr, Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"centerpadx", "Db", {Tstr, Tstr, Tlong, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"leftx", "Dc", {Tstr, Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"padx", "Dd", {Tstr, Tstr, Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"prepadx", "De", {Tstr, Tstr, Tlong, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"prestripx", "Df", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"rightx", "Dg", {Tstr, Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"stripx", "Dh", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"substrx", "Di", {Tstr, Tlong, Tlong, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"subwordx", "Dj", {Tstr, Tlong, Tlong, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"wordx", "Dk", {Tstr, Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"copiesx", "Do", {Tstr, Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"delstrx", "Dp", {Tstr, Tlong, Tlong, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"delwordx", "Dq", {Tstr, Tlong, Tlong, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"justifyx", "Dr", {Tstr, Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"reversex", "Ds", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"spacex", "Dt", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"tolowerx", "Du", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"toupperx", "Dv", {Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"insertx", "DA", {Tstr, Tlong, Tstr, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"overlayx", "DB", {Tstr, Tlong, Tstr, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"translatex", "DC", {Tstr, Tstr, Tstr, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"xrangex", "DH", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"findx", "DK", {Tlong, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"indexx", "DL", {Tlong, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"posx", "zDL", {Tlong, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"verifyx", "DO", {Tlong, Tstr, Tstr, Tend}, ness_codeOrange, NULL},
	{"wordindexx", "DS", {Tlong, Tlong, Tstr, Tend}, 
						ness_codeOrange, NULL},
	{"wordsx", "DT", {Tlong, Tstr, Tend}, ness_codeOrange, NULL},
	{"abbrevx", "DW", {Tbool, Tstr, Tstr, Tend}, ness_codeOrange, NULL},


	{NULL, NULL}
};

static const struct builtindef  predefinedTable[] = {
	{"mousex", "Ux", {Tlong}, ness_codeOrange, NULL},
	{"mousey", "Uy", {Tlong}, ness_codeOrange, NULL},
	{"mouseaction", "Uw", {Tlong}, ness_codeOrange, NULL},
	{"mouseleftdown", "Ua", {Tlong}, ness_codeOrange, NULL},
	{"mouseleftup", "Ub", {Tlong}, ness_codeOrange, NULL},
	{"mouseleftmove", "Uc", {Tlong}, ness_codeOrange, NULL},
	{"mouserightdown", "Ud", {Tlong}, ness_codeOrange, NULL},
	{"mouserightup", "Ue", {Tlong}, ness_codeOrange, NULL},
	{"mouserightmove", "Uf", {Tlong}, ness_codeOrange, NULL},
	{"lastkeys", "Uk", {Tstr}, ness_codeOrange, NULL},
	{"lastmenu", "Um", {Tstr}, ness_codeOrange, NULL},
	{"defaulttext", "E", {Tptr}, ness_codeOrange, NULL},
	{"currentinputfocus", "UF", {Tptr}, ness_codeOrange, NULL},
	{"currentinset", "UI", {Tptr}, ness_codeOrange, NULL},
	{"currentwindow", "UJ", {Tptr}, ness_codeOrange, NULL},
#if       ! SY_U5x && ! SY_AIXx && !defined(VAX_ENV) && !defined(PMAX_ENV)
	{"realieee", "1" /* TRUE */,  {Tbool}, ness_codeOrange, NULL},
#else  /* ! SY_U5x && ! SY_AIXx && !defined(VAX_ENV) && !defined(PMAX_ENV) */
	{"realieee", "9" /* FALSE */, {Tbool}, ness_codeOrange, NULL},
#endif /* ! SY_U5x && ! SY_AIXx && !defined(VAX_ENV) && !defined(PMAX_ENV) */

	{NULL, NULL}
};


void callInit(nesssym_scopeType  Gscope, int  idtok, class nesssym  *proto);
void callPredefId(class nesssym  *var);
struct callnode * callLoadFuncval(struct varnode  *var);
static const char * argcounterror(const char  *format, long  n);
static char * argtypeerror(long  n, Texpr  formal , Texpr  actual);
static Texpr builtincall(struct varnode  *fnode, struct exprnode  *argtypes);
void checkargtypes(class nesssym  *func, struct exprnode  *fexpr, 
		class nesssym  *formal, struct exprnode  *actual);
static boolean callCheckProcTable(struct varnode  *varnode, struct exprnode *args);
struct exprnode * callFunc(struct varnode * varnode, struct exprnode  *argtypes);
void callUnknown(class nesssym  *sym);
void callCheck (struct callnode  *call, unsigned char *iar, class ness  *ness);
static union stackelement *startPush(union stackelement  *NSP, 
		long  size, TType  hdr, long  v);
void callCheat(unsigned char op, unsigned char *iar, class ness  *ness);
static void recordFiles(char *dir, class nesssym  *proto);
void callCompLib(struct libnode  *lnode);
enum libstate callCheckLib(char *fun, struct funcnode  **fnode);
struct errornode * callInitSubTree(class ness  *ness);
struct errornode * callInitAll(class ness  *ness);
long ReadTextFileStream(class text  *text, const char *name, FILE  *f, boolean  objok);


/* callInit(Gscope, idtok, proto)
	initialize the tables used by procedures in this file
	Only needs to be called once.
	'Gscope' is the GrammarScope in which to define builtins
	'idtok' is the token number for identifiers
	'proto' is a struct nesssym value
	defines the LibScope for storing names of ness library functions
*/
	void
callInit(nesssym_scopeType  Gscope, int  idtok, class nesssym  *proto) {
	const struct builtindef *b;
	class nesssym *sym;

	for (b = builtintable; b->name != NULL; b++) {
		sym = nesssym::NDefine((char *)b->name, proto, Gscope);
		nesssym_NSetINode(sym, builtindef, b);
		sym->toknum = idtok;
		sym->type = b->types[0];
		sym->flags = flag_function | flag_builtin;
	}
	for (b = predefinedTable; b->name != NULL; b++) {
		sym = nesssym::NDefine((char *)b->name, proto, Gscope);
		nesssym_NSetINode(sym, builtindef, b);
		sym->toknum = idtok;
		sym->type = b->types[0];
		sym->flags = flag_var | flag_builtin;
	}
	LibScope = nesssym::NNewScope(nesssym_GLOBAL);
}


/* callPredefId(var)
	generate code for predefined identifier
*/
	void
callPredefId(class nesssym  *var) {
	struct builtindef *b = nesssym_NGetINode(var, builtindef);
	unsigned char *defn;
	for (defn = (unsigned char *)b->defn ; *defn; defn++)
		genop(*defn);
}


/* callLoadFuncval(var)
	Compile code to load onto the stack a value representing the
function given by the variable.

var->sym is either undefined or a function.  Look around for a function
	definition.  If there is a function definition, ensure there is a
	callnode and compile '(p' followed by ptr to callnode.
   Returns True if found a definition; false otherwise.

   Possible sorts of function definition
	builtin func
	ness func defined already
	proctable entry

 relevant var->flags values  {inode value}

flag_function | flag_ness	top level function in a script	{funcnode}
flag_function | flag_ness | flag_forward  ness func declared FORWARD {funcnode}
flag_function | flag_ness | flag_xfunc	function within an 'extend' {funcnode}
flag_function | flag_ness | flag_xfunc | flag_forward	fwd func in 'extend' {funcnode}
	for the above four, construct a callnode from info in funcnode
	and attach to funcnode
flag_function | flag_builtin	a builtin function 		{builtindef}
	construct a callnode from builtindef and attach to builtindef
flag_function | flag_proctable	function from proctable		{callnode}
	callnode in inode(sym) is created by callCheckProctable
flag_function | flag_classdefn  function from class	 	{callnode}
	callnode in inode(sym) is created by callCheckClass

fail for the following
flag_function | flag_undef	undefined function		{NULL or 1}
		(the value 1 indicates that it is a for-sure undefined func)
flag_function | flag_forward	possibly a forward func call	{callnode}

	type in returned callnode is    functype<<$ | Tfunc

 */

	static struct exprnode specialexprnode;
	struct callnode *
callLoadFuncval(struct varnode  *var) {
	class nesssym *sym = var->sym;
	struct callnode *call = NULL;
	struct funcnode *fnode = NULL;

	if (sym->flags == (flag_function | flag_undef) ||
			sym->flags == (flag_function | flag_forward))
		return NULL;

	if (sym->flags & flag_undef) {
		if (callCheckLib((sym)->NGetName(), &fnode) == OK
				&& fnode != NULL) {
			/* found name in a Ness library */
			nesssym_NSetINode(sym, funcnode, fnode);
			sym->flags = flag_function | flag_ness;
		}
		else if (callCheckProcTable(var,&specialexprnode)) {
			/* ifsucceeded, var has callnode for proctable entry */
		} else return NULL;
	}

	switch(sym->flags) {
	case flag_function | flag_ness:
	case flag_function | flag_ness | flag_xfunc:
	case flag_function | flag_ness | flag_forward:
	case flag_function | flag_ness | flag_xfunc | flag_forward:   {
		struct funcnode *fnode = nesssym_NGetINode(sym, funcnode);
		if (fnode->call)
			call = fnode->call;
		else {
			/* build callnode from funcnode */
			int i;
			class nesssym *psym;
			call = (struct callnode *)malloc(
						sizeof(struct callnode));
			call->variety = callNess;
			call->Sym = sym;
			call->where.Nproc = fnode->SysGlobOffset;
			call->requiredclass = NULL;
			call->rettype = fnode->functype;
			for (i = 0, psym = fnode->parmlist; psym; 
						i++, psym = psym->next) {}
			call->nargs = i;
			for (psym = fnode->parmlist; i--; psym = psym->next)
				call->argtype[i] = psym->type;
			fnode->call = call;
		}
	} break;
	case flag_function | flag_builtin: {
		struct builtindef *b = nesssym_NGetINode(sym, builtindef);
		if (b->call)
			call = b->call;
		else {
			/* build callnode from builtindef */
			int i;
			call = (struct callnode *)malloc(
						sizeof(struct callnode));
			call->variety = callBuiltin;
			call->Sym = sym;
			call->where.bid = b;
			call->requiredclass = NULL;
			call->rettype = b->types[0];
			for (i = 0; b->types[i+1] != Tend; i++) {}
			call->nargs = i;
			while (i--)
				call->argtype[i] = b->types[i+1];
			b->call = call;
		}
	} break;
	case flag_function | flag_undef:
		return NULL;
	case flag_function | flag_proctable:
	case flag_function | flag_classdefn:
		call = nesssym_NGetINode(sym, callnode);
	}

	genop('(');
	refAddress(getcurfuncmark(), 'p', call);
	return call;
}



static const char toomanyargs[] = "*function call has %d extra argument%s";
static const char toofewargs[] = "*function call needs %d more argument%s";

	static const char *
argcounterror(const char  *format, long  n) {
	char *msg = (char *)malloc(50);
	sprintf(msg, format, n, (n>1) ? "s" : "");
	return msg;
}
	static char *
argtypeerror(long  n, Texpr  formal , Texpr  actual) {
	char *msg = (char *)malloc(60);
	if ((formal&0xf) == Tend) formal = Tunk;
	if ((actual&0xf) == Tend) actual = Tunk;
	sprintf(msg, "*argument %ld should have type %s(%ld), but is type %s(%ld)", 
			n, TypeName[formal&0xf], formal, TypeName[actual&0xf], actual);
	return msg;
}


/* builtincall(fnode, argtypes)
	generate a call on builtin function referenced by sym
	check types
*/
	static Texpr
builtincall(struct varnode  *fnode, struct exprnode  *argtypes) {
	struct builtindef *b = nesssym_NGetINode(fnode->sym, builtindef);
	unsigned char *defn;
	Texpr *type;
	long nargs;
	struct exprnode *targs;
	long loc, tloc, len;
	boolean sendnargs;

	loc = fnode->loc;	/* start of function name */
	tloc = (curComp->tlex)->RecentPosition( 0, &len);  /* closing ')' */
	len = tloc + len - loc;	/* length of call */
	sendnargs = FALSE;

	/* count number of args */
	nargs = 0;
	for (targs = argtypes;  targs != NULL;  targs = targs->next) 
		nargs++;

	/* the type list at b->types begins with the return type and then has
		argument types in reverse order.  We start with the
		last type and go right to left */
	type = b->types + 1;
	if (*type == Tunk) {
		/* XXX special case for  currentselection(), currentmark(),  textimage(),
			cheat_callmethod(),   cheat_callclassproc(),  class()*/
		if (strcmp(b->name, "textimage") == 0) {
			if (argtypes == NULL || argtypes->next != NULL) 
				SaveError(":textimage must have only one argument", 
						loc, len);
		}
		else if (strncmp(b->name, "current", 7) == 0) {
			if (argtypes == NULL)
				/*  no arg, insert defaulttext */
				genop('E');
			else if (argtypes->next != NULL) 
				SaveError(":cannot have more than one argument", 
						loc, len);
			else if ((argtypes->type&0xf) != Tptr) 
				ExprError(":argument should be a pointer value",
						argtypes);
		}
		else if (strncmp(b->name, "cheat_call", 10) == 0) {
			/* method or class proc call: send along arg count */
			sendnargs = TRUE;
		}
		else if (strcmp(b->name, "class") == 0) {
			/* class() arg is either Tptr or Tstr */
			if (argtypes == NULL || argtypes->next != NULL) 
				SaveError(":class() must have one argument", 
						loc, len);
			if ((argtypes->type&0xf) != Tptr  &&  (argtypes->type&0xf) != Tstr) 
				ExprError(":argument should be a marker or pointer",
						argtypes);
		}
		else {} 
	}
	else {
		/* check match of types in argtypes and type  */
		long argnum = nargs;
		while (argtypes != NULL && *type != Tend) {
			/* check a type */
			if ((*type&0xf) != (argtypes->type&0xf))
				SaveError(argtypeerror(argnum, 
					*type, argtypes->type),    loc, len);
			/* advance */
			argtypes = argtypes->next;
			type++;
			argnum --;
		}
		if (argtypes != NULL)
			SaveError(":too many args", loc, len);
		else if (*type != Tend)
			SaveError(":too few args", loc, len);
	}

	if (curComp->ness->accesslevel >= b->minok) {
		/* now (finally) we generate the code for the function */
		for (defn = (unsigned char *)b->defn ; *defn; defn++)
			genop(*defn);
		if (sendnargs) genop(nargs);
	}
	else {
		/* does not meet the security test.  Pop the args and return 0 */
		long argnum;
		for (argnum = nargs; argnum > 0; argnum--)
			genop('y');
		genop ('0');
		if (b->minok >= ness_codeGreen)
			SaveError(":This function may destroy your files", 
					loc, len);	
		else
			SaveError(":This function may modify a file", 
					loc, len);	
	}
	return *b->types;
}


	void
checkargtypes(class nesssym  *funcXXX	/* the function symbol */, 
		struct exprnode  *fexpr	/* the function expr */, 
		class nesssym  *formal, 
		struct exprnode  *actual) {
	class nesssym *tformal;
	struct exprnode *tactual;
	long n, nargs;

	/* check number of arguments */
	tformal = formal;
	tactual = actual;
	nargs = 0;
	while (tformal != NULL && tactual != NULL) {
		tformal = tformal->next;
		tactual = tactual->next;
		nargs++;
	}
	if (tformal != NULL || tactual != NULL) {	/* wrong number of arguments */
		const char *msg;
		n = 0;
		if (tformal != NULL)  {	/* too few arguments */
			while (tformal != NULL) n++, tformal = tformal->next;
			msg = argcounterror(toofewargs, n);
		}
		else {	/* too many arguments */
			while (tactual != NULL) n++, tactual = tactual->next;
			msg = argcounterror(toomanyargs, n);
		}
		ExprError(msg, fexpr);
		return;
	}
	/* check types of arguments  (note that lists are the same length) */
	tformal = formal;
	tactual = actual;
	n = 0;
	while (tformal != NULL) {
		if ((tactual->type&0xf) != (tformal->type&0xf))
			ExprError(argtypeerror(nargs - n, tformal->type, tactual->type), tactual);
		tformal = tformal->next;
		tactual = tactual->next;
		n++;	/* update n late so that arg number is  (nargs - n)  */
	}
	}


static struct hack {
	char procmod[15];
	char modname[11];
} ModuleHack [] = {
	{"bodies", "text822v"},
	{"folder", "folders"},
	{"frame", "framecmd"},
	{"self", "zipedit"},
	{"self_insert", "spread"},
	{"wraplook", "stylesheet"},

	{"exit", "im"},   /* obsolete */
	{"redraw", "im"},   /* obsolete */
	{"start", "im"},   /* obsolete */
	{"stop", "im"},   /* obsolete */
	{"raster", "rasterv"},	/* obsolete */
	{0, 0}
};

static Texpr AtomToNessType(const atom *t) {

    if(t==avalue::integer) return Tlong;
    else if(t==avalue::real) return Tdbl;
    else if(t==avalue::cstring) return Tstr;
    else if(t) {
	ATKregistryEntry *e=ATK::LoadClass(t->Name());
	if(e) return Tptr;
	else return Tunk;
    }
    return Tunk;
}

	static callnode *
ProcToCall(proctable_Entry *pe, exprnode *argtypes, nesssym *func) {
	int nargs=0;
	int i;
	exprnode *e;
	proctable::ForceLoaded(pe);
	if ( ! proctable::Defined(pe)) return NULL;
	aaction *a=proctable::GetAction(pe);
	if (a == NULL) {
		if(argtypes != &specialexprnode) {
			for (nargs=0, e=argtypes; 
					e != NULL;  
					nargs++, e = e->next) {}
		} 
		else nargs = MAXARGS;
	} 
	else 
		nargs=a->ArgTypes().GetN()+1;
	callnode *call = (struct callnode *)
			malloc(sizeof(struct callnode)
				+ (nargs-MAXARGS) * sizeof(Texpr));
	if (a == NULL) {
		if(argtypes != &specialexprnode) {
			for (i = nargs, e=argtypes; e != NULL;  e = e->next) 
				call->argtype[--i] = e->type;
		} 
		else {
			call->argtype[0]=Tptr;
			call->argtype[1]=Tunk;
		}
	} 
	else {
		call->argtype[0]=Tptr;
		for(i=0; i<nargs-1; i++) {
			const atom *t=a->ArgTypes()[i].Atom();
			call->argtype[i+1]=AtomToNessType(t);
		}
	}
	call->nargs = nargs;
	call->variety = callPE;
	call->Sym = func;
	call->where.pe = pe;
	if(a==NULL) 
		call->rettype=Tlong;
	else {
		if(a->RetTypes().GetN() == 0) call->rettype=Tvoid;
		else call->rettype 
				= AtomToNessType(a->RetTypes()[0].Atom());
	}
	call->requiredclass = NULL;
	return call;
}

/* callCheckProcTable(func)
	check to see if function called is in the proctable  
	(try loading if needed)
	if found, attach a callnode to var
	return TRUE if succeed and FALSE otherwise
*/
	static boolean
callCheckProcTable(struct varnode  *varnode, struct exprnode *argtypes) {
	class nesssym *func;
	struct proctable_Entry *pe;
	char procname[BUFLEN];
	char modname[BUFLEN];
	char *dash;
	struct callnode *call;
	struct hack *gack;

	func = varnode->sym;

	/* copy nessname to procname, changing _ to - */
	strncpy(procname, (func)->NGetName(), BUFLEN);
	procname[BUFLEN-1] = '\0';	/* truncate to BUFLEN characters */
	dash = (char *)strchr(procname, '_');
	
	if(dash!=NULL) {
	    strncpy(modname, (func)->NGetName(), dash - procname);
	    modname[dash - procname] = '\0';
	} else strcpy(modname, procname);
	
	for (dash=procname; (dash=(char *)strchr(dash, '_')); )
		*dash = '-';
	
	if ((pe=proctable::Lookup(procname)) == NULL) {
		for (gack = &ModuleHack[0];  gack->procmod[0] != 0;  gack++)
			if (strcmp(modname, &gack->procmod[0]) == 0) {
				strcpy(modname, &gack->modname[0]);
				break;
			}
		ATK::LoadClass(modname);
		if ((pe=proctable::Lookup(procname)) == NULL) 
			return FALSE;
	}
	if (curComp->ness->accesslevel < ness_codeGreen) 
		/* illegal at this accesslevel */
		SaveError(":This function may modify or destroy your files", 
			varnode->loc, varnode->len);
	call=ProcToCall(pe, argtypes, func);
	if (call==NULL) 
	    SaveError(":could not load proctable procedure", 
		      varnode->loc, varnode->len);
	nesssym_NSetINode(func, callnode, call);
	func->flags = flag_function | flag_proctable;
	return TRUE;
}


/* callFunc(varnode, argtypes)
	called during compilation to generate object code for a function call
	'varnode' is the function reference
	'argtypes' is a list of exprnodes, each giving the type of an arg (reverse order)
     Possible kinds of function:
	flag_var && (type&0xF) == Tfunc		call a funcval
	flag_function | flag_builtin		builtin - use builtincall
	flag_function | flag_ness		Ness library - find out so with callCheckLib
	flag_function | flag_undef		unknown - may be a forward reference
	flag_function | flag_ness		previously defined Ness func
	flag_function | flag_ness | flag_forward  FORWARD declared
	flag_function | flag_ness | flag_xfunc	defined in an 'extend'
	flag_function | flag_ness | flag_xfunc | flag_forward  FORWARD in 'extend'
	flag_function | flag_proctable  	proctable function
	flag_function | flag_classdefn  	method, class procedure, Get, or Set
*/
	struct exprnode *
callFunc(struct varnode * varnode, struct exprnode  *argtypes) {
	class nesssym *varbl = varnode->sym;
	struct exprnode *val;	/* return value */
	long loc, len;
	Texpr rettype = Tstr;
	struct funcnode *fnode;
	struct callnode *cnode;
	struct exprnode *e;
	long n;
	const char *msg;

	loc = (curComp->tlex)->RecentPosition( 0, &len);
	val = exprnode_Create(Tstr, NULL, FALSE, 
			varnode->loc, loc+len - varnode->loc);
	if (varbl == NULL) {
		val->type = Tunk;
		return val;
	}

	/* first try to find the definition  and do extensive lookups
		if not previously found */

	if ((varbl->flags & flag_var)  &&  (varbl->type & 0xF) == Tfunc) {
		/* Tfunc, check vardefnode->call->callnode */
		cnode = nesssym_NGetINode(varbl, vardefnode)->sig;
		if ((cnode->nargs == MAXARGS && cnode->argtype[0]==Tptr && cnode->argtype[1]==Tunk) || cnode->nargs==-99) {
			n = 0;
			for (e=argtypes; e != NULL; n++, e = e->next) {}
			cnode->nargs = n;
			for (e=argtypes; e != NULL; e = e->next) 
				cnode->argtype[--n] = e->type;
		}
	}
	else if ( (fnode=nesssym_NGetINode(varbl, funcnode)) == NULL) {
		/* call to unknown function */

		/* move to the outerscope for the current compilation, 
			so future references to the same function succeed */
		((class sym *)varbl)->SetScope( curComp->ness->outerScope); 

		if (callCheckLib((varbl)->NGetName(), &fnode) == OK
				&& fnode != NULL) {
			/* found name in a Ness library */
			nesssym_NSetINode(varbl, funcnode, fnode);
			varbl->flags = flag_function | flag_ness;
		}
		else if (callCheckProcTable(varnode, argtypes)) {
			/* it is a call on the proctable */
			cnode = nesssym_NGetINode(varbl, callnode);
		} else {
			/* 1st call to an undefined function: build callnode */
			varbl->flags = flag_function | flag_forward;
			varbl->type = (Tstr << 4) | Tfunc;
			for (n=0, e=argtypes; e != NULL;  n++, e = e->next) {}
			cnode = (struct callnode *)malloc(
				sizeof(struct callnode)
				+  sizeof(Texpr) * (n - MAXARGS));
			cnode->nargs = n;
			for (e=argtypes; e != NULL; e = e->next) 
				cnode->argtype[--n] = e->type;
			cnode->variety = callSym;
			cnode->requiredclass = NULL;
			cnode->Sym = varbl;
			cnode->where.offset = 0L;
			cnode->rettype = Tstr;
			nesssym_NSetINode(varbl, callnode, cnode);
		}
	}
	else {
		/* the inode value is either a funcnode or a callnode */
		fnode = nesssym_NGetINode(varbl, funcnode);
		cnode = (struct callnode *)fnode;
	}

	
	/* now check argument types and generate code */
	
	switch (varbl->flags) {

	/* ness code (has fnode) */
	case flag_function | flag_ness:
	case flag_function | flag_ness | flag_forward :
	case flag_function | flag_ness | flag_xfunc:
	case flag_function | flag_ness | flag_xfunc | flag_forward: {
	    /* call to previously defined ness function (or library func)*/
	    struct exprnode node;
	    if(fnode->oldcall) {
		node.next=argtypes;
		node.IsCat=FALSE;
		node.loc=0;
		node.len=0;
		node.type=Tfunc;
		argtypes=&node;
		genop('(');
		refAddress(getcurfuncmark(), 'p', fnode->oldcall);
	    }
		checkargtypes(varbl, val, fnode->parmlist, argtypes);
		refSysGlob(getcurfuncmark(), 'O', fnode->SysGlobOffset);
		rettype = fnode->functype;
		if(fnode->oldcall) {
		    argtypes=node.next;
		}
		break;
	    }
	/* builtin function (has builtindef) */
	case flag_function | flag_builtin:
		rettype = builtincall(varnode, argtypes);
		break;

	default:  /* uses callnode */
		/* test number and type of arguments */
		for (n=0, e=argtypes; e != NULL;  n++, e = e->next) {}

		if ((cnode->nargs == MAXARGS && cnode->argtype[0]==Tptr && cnode->argtype[1]==Tunk) || cnode->nargs==-99)
			msg = ":defered args inexplicably encountered";
		else if (n < cnode->nargs)
			msg = argcounterror(toofewargs, 
				cnode->nargs - n);
		else if (n > cnode->nargs)
			msg = argcounterror(toomanyargs, 
				n - cnode->nargs);
		else msg = NULL;
		if (msg) 
			SaveError(msg, varnode->loc, varnode->len);
		else for (e=argtypes; n--, e != NULL; e=e->next) 
			if ((e->type&0xf) != (cnode->argtype[n]&0xf))
				SaveError(argtypeerror(n+1, 
					 cnode->argtype[n], e->type),
				    varnode->loc, varnode->len);

		/* generate the call on the callnode */
		if (varbl->flags & flag_var) {
			/* funcval */
			int argsize = 0;
			genop ('(');
			refAddress(getcurfuncmark(), 'C', cnode);
			/* output a byte giving the stack size of args
				so run time can find the funcval 
				below the args */
			for (e=argtypes; e != NULL; e = e->next) {
				switch(e->type & 0xF) {
				case Tfunc: 
					argsize += sizeof(struct funcstkelt);
					break;
				    case Tlong:
					argsize += sizeof(struct longstkelt);
					break;
				case Tbool:
				case Tbra:
					argsize += sizeof(struct boolstkelt);
					break;
				case Tdbl:
					argsize += sizeof(struct dblstkelt);
					break;
				case Tstr:
					argsize += sizeof(struct seqstkelt);
					break;
				    case Tptr:
					argsize += sizeof(struct ptrstkelt);
					break;
				}
			}
			genop(argsize);

			/* For a non Tvoid function, the stack after the '(C'
				has the value and then the funcval;
			   for a Tvoid function, the stack has the funcval.
			   In either case, we must get rid of the funcval. */
			if (cnode->rettype != Tvoid)
				genop('z');	/* swap value and funcval */
			genop('y');		/* pop funcval */
		}
		else {
			/* other callnode functions */
			/* case flag_function | flag_proctable: */
			/* case flag_function | flag_classdefn: */
			/* case flag_function | flag_forward: */

			refAddress(getcurfuncmark(), 'C', cnode);
		}


		rettype = cnode->rettype;
		break;

	case flag_function | flag_undef:
		break;
	}

	/* delete argtypes list elements */
	while (argtypes != NULL) {
		struct exprnode *t = argtypes->next;
		exprnode_Destroy(argtypes);
		argtypes = t;
	}
	varnode_Destroy(varnode);
	val->type = rettype;
	return val;
}

/* callUnknown(sym)
	make the sym a call on an unknown function
	the node value will later point to a callnode
	(called from varIsFunction)
*/
	void
callUnknown(class nesssym  *sym) {
	sym->flags = flag_function | flag_undef;
	nesssym_NSetINode(sym, callnode, NULL);
}





/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = *\
 *
 *	Run Time
 *
\* = = = = = = = = = = = = = = = = = = = = = = = = = = = = */




/* callCheck (call, NSP, iar, ness)
	call->variety is callSym
	This is used only for forward function calls.
	Resolve the function reference indicated by 'call' and check argument types
	Use value of 'NSP' to find arguments.  Use 'iar' in RunError calls.
	Will convert to callNess if function was a forward reference;
	otherwise signals a runtime error.

	If the function was defined, the nesssym referenced by the four bytes 
	following the opcode will now have flags  flag_function|flag_ness

XXX this function is no longer needed: everything but forward references are done
at compile time.  Forward references should be done with fixups at the end of the compilation
(and should use 'O' instead of 'C')

XXX this function does not test the types of the values on the stack

*/
	void
callCheck (struct callnode  *call, unsigned char *iar, class ness  *ness) {
	class nesssym *func = call->Sym;	/* the nesssym for desired function */
	class nesssym *formals;
	long n;
	struct funcnode *fnode;

	if (func->flags != (flag_function | flag_ness))
		/* function not found, issue warning */
		RunError(":call to undefined proc", iar);
	fnode = nesssym_NGetINode(func, funcnode);

	/* count formals and check types against actuals */
	for (n=0, formals = fnode->parmlist; formals != NULL;
			formals = formals->next, n++) {}
	if (call->nargs > n)
		RunError(argcounterror(toomanyargs, call->nargs - n), iar);
	else if (call->nargs < n)
		RunError(argcounterror(toofewargs, n - call->nargs), iar);
	else for (formals = fnode->parmlist; --n, formals != NULL;
			formals = formals->next) 
		if ((formals->type&0xf) != (call->argtype[n]&0xf)) 
			RunError(argtypeerror(call->nargs - n, 
				formals->type,  call->argtype[n]),  iar);

	/* we have found the desired function.  Convert to callNess. */
	call->variety = callNess;
	call->where.Nproc = fnode->SysGlobOffset;
}
// #if (sizeof(union argtype) != 4)
//	}}}}}}  /* assumption:  union argtype values are four bytes */
//		/* If this assumption is false, the offending
//		element of the union will have to be removed. */
// #endif

	static union stackelement *
startPush(union stackelement  *NSP, long  size, TType  hdr, long  v) {
	NSP = (union stackelement *)(((unsigned long)NSP) - size);
	NSP->l.hdr = hdr;
	NSP->l.v = v;
	NSPstore = NSP;
	return NSP;
}


/* callCfunc(call, NSP, iar, ness)
	call the  proctable entry indicated by callnode 'call'
 	call is limited to 10 argument words
 	proctable procedures may only be called with an object and a string,
 	and return a long.  (which may mean nothing)
 	proctable actions may be called with integers, markers, reals and
 	objects they may return the same types.
 
	steps:
		1. get args into an array
		2. get the proc
		3. check first arg if req'd
		4. call the proc
		5. free allocated values
		6. push return value onto stack
*/
	void
callCfunc(struct callnode  *call, unsigned char *iar, class ness  *ness) {
	union stackelement *NSP = NSPstore;
	avalueflex arg;	/* arguments (from stack) */
	avalueflex retval;	/* returned value (push to stack) */
	boolean malloced[10];	/* T if arg[i] pts to malloced space */
	long n;		/* local var */
	long nargs;	/* actual number of arg words in array */
	TType lasttype;		/* local var */
	boolean checkFirstArg;	/* whether first arg must be object */
	class view *v, *v2;		/* first arg if any */
	class dataobject *d;
	class text *textp;
	boolean createdview;
	long argmax;		/* number of argument words actually passed */

	nargs = call->nargs;
	argmax = nargs;

	/* 1. get a pointer to the function to call */
	checkFirstArg = TRUE;
	createdview = FALSE;

	if(call->variety!=callPE) return;


	call->requiredclass = proctable::GetType(call->where.pe);
	checkFirstArg = (call->requiredclass != NULL);
	/* 2. get C versions of arguments into array.  
		pop the stack
		XXX if we ever are able to pass nessmarks to C functions,
			we must be careful that popping the stack
			does not delete the text pointed at by a nessmark
		'n' counts the args as required by the target function
		args are on stack with the last one on top
	 */
	// The first arg doesn't go into the argument list proper.
	if(nargs>1) {
	    arg.Insert(0,nargs-1);
	    for (n = nargs-2; n >= 0; n--)  {
		malloced[n] = FALSE;
		lasttype = NSP->l.hdr;
		/* load arg 'n' from stack */
		/* XXX should we check type?
		 it should be call->argtype[n - FirstIndex] */
		switch (lasttype) {
		    case ptrHdr:
			arg[n].Set((void *)NSP->p.v);
			NSPopSpace(ptrstkelt);
			break;
		    case longHdr:
		    case boolHdr:
			arg[n].Set((long)NSP->l.v);
			NSPopSpace(longstkelt);
			break;
		    case seqHdr:
			arg[n].Set((NSP->s.v)->ToC());
			malloced[n] = TRUE;
			NSP=popValue(NSP);	/* use popValue to unget nessmark */
			break;
		    default:
			arg[n].Set(0L);
			/* ERROR: unknown arg type */
			RunError(":unknown arg type", iar);
			break;
		}
	    }
	}
	if(nargs>0 && NSP->l.hdr==ptrHdr) {
	    v=(view *)NSP->p.v;
	    NSPopSpace(ptrstkelt);
	} else v=NULL;
	
	/* 3. if required, check first arg and perhaps invent it */
	if (checkFirstArg) {
	    if (nargs == 0) {
		/* there are no args, use input focus or currentinset */
		v = (class view *)im::GetLastUsed();
		if (call->requiredclass != imClass)
		    v = (ness->CurrentInset != NULL) 
		      ? ness->CurrentInset 
		      : (v != NULL) ? ((class im *)v)->GetInputFocus()
		      : NULL;
		argmax++;
		lasttype = ptrHdr;
	    }
	    else if (call->argtype[0] != Tptr)  {
		/* oops, first arg should be a pointer, but isn't */
		RunError(":first argument should be a pointer, but isn't", iar);
	    }
	    /* XXX should somehow be able to deal with undisplayed insets */
	     /* for the nonce we ignore proctable calls with NULL first
	     arg on the notion that this probably comes
	     from a call of inset() for an inset 
	     whose view has never been exposed  */
	    if (v != NULL)  {
		/* find view appropriate to type required by proc */
		v2 = (view *)ProperPtr((ATK  *)v, call->requiredclass);
		if (v2 == NULL && (v)->IsType( dataobjectClass)  
		    && (call->requiredclass)->IsType( viewClass)) {
		    /* wants a view on the data object.  Generate one. */
		    /* XXX need to cache the generated views */
		    /* XXX it is pretty bogus to insert the view into the im */
		    d = (class dataobject *)v;
		    v = (class view *)ATK::NewObject((d)->ViewName());
		    if ((v)->IsType( call->requiredclass)) {
			im *iptr=im::GetLastUsed();
			if(iptr==NULL) iptr=new im;
			static struct rectangle playpen = {0,0,0,0};
			(v)->SetDataObject( d);
			(v)->InsertView( (class view *)iptr,
					 &playpen);
			createdview = TRUE;
		    }
		    else (v)->Destroy();
		} else v=v2;
		if (v == NULL) {
		    char *buf, *wantname;
		    wantname = (char *)(proctable::GetType(call->where.pe)   )->GetTypeName(
											    );
		    buf = (char *)malloc(60 + strlen((v)->GetTypeName()) 
					 + strlen(wantname));
		    sprintf(buf, 
			    "*first arg to proctable call is  /%s/, but should be /%s/",
			    (v)->GetTypeName(),  wantname);
		    RunError(buf, iar);
		}
	    }

	    /* 4. call the function */
	    proctable::Call(call->where.pe, v, arg, &retval);
	}
	/* 5.  free allocated values */
	if(nargs>1) for (n = nargs-2; n >= 0; n--) 
		if (malloced[n]) 
			free((char *)arg[n].CString());
	if (createdview) {
		/* XXX should cache the view */
		/*  XXX should set data object  in the view to NULL,
			but view.c is broken:
			view::SetDataObject(v, NULL);  */
		(v)->Destroy();
	}

	/* 6. push retval */
	if(retval.GetN()>0) {
	    switch (call->rettype) {
		case Tlong:
		    NSP = startPush(NSP, sizeof(struct longstkelt), longHdr, retval[0].Integer());
		    break;
		case Tbool:
		    NSP = startPush(NSP, sizeof(struct boolstkelt), boolHdr, retval[0].Integer());
		    break;
		case Tptr:
		    NSP = startPush(NSP, sizeof(struct ptrstkelt), ptrHdr, (long)retval[0].VoidPtr());
		    break;
		case Tstr:
		    textp = new text;
		    (textp)->InsertCharacters(0, (char *)retval[0].CString(), strlen(retval[0].CString()));
		    (PUSHMARK(NULL))->Set(textp, 0, (textp)->GetLength());
		    break;
		default:
		    NSP = startPush(NSP, sizeof(struct longstkelt), longHdr, 0);
		    break;
	    }
	} else {
	    NSP = startPush(NSP, sizeof(struct longstkelt), longHdr, 0);
	}
	    
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = *\
 *
 *	The Cheat functions
 *
\* = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/* callCheat(op, iar, ness)
	implement the  cheat_  functions
*/
	void
callCheat(unsigned char op, unsigned char *iar, class ness  *ness) {
	union stackelement *NSP = NSPstore;
	switch (op) {

	/* eventually the following ought to move to nevent.c */

	case 'q':	im::ForceUpdate();  break;

	case 'r':	{				/* inset() */
		class celview *v;
		/* load to stack top a pointer to the object named by the
			string now at stack top */
		if ((ness)->GetArbiter() == NULL) 
			v = NULL;
		else {
			char *s;
			/* put name from mark at NSP into buf */ 
			s = (NSP->s.v)->ToC();
			/* get ptr to named inset */
			v = arbiterview::GetNamedCelview((ness)->GetArbiter(), s);
			free(s);
		}
		NSP = popValue(NSP);		/* discard string */
		NSPushSpace(ptrstkelt);
		NSP->p.hdr = ptrHdr;
		NSP->p.v = (ATK  *)v;
	}	break;
	case 's':	{				/* new() */
		/* arg is ptr for the class */
		struct ATKregistryEntry  *ci;
		if (NSP->p.hdr != ptrHdr  ||  NSP->p.v == NULL)  
			RunError(":arg was not a non-NULL pointer", iar);
		ci = (struct ATKregistryEntry  *)NSP->p.v;
		NSP = popValue(NSP);
		NSPushSpace(ptrstkelt);
		NSP->p.hdr = ptrHdr;
		NSP->p.v = (ci)->New();		/* ???
			ci points to a Registry entry
			is this how I get a new thing of that type */
	}	break;
	case 't':	{				/* class */
		/* arg is string naming the class */
		struct ATKregistryEntry  *cip = NULL;
		if (NSP->s.hdr == seqHdr) {
			char *s = (NSP->s.v)->ToC();
			cip = ATK::LoadClass(s);  /* NULL if fail */
			free(s);
		}
		else if (NSP->p.hdr == ptrHdr) {
			if (NSP->p.v == NULL) 
				RunError(":arg is NULL", iar);
			else
				cip = (NSP->p.v)->ATKregistry();
		}
		else
			RunError(
			   ":arg isn't subseq or object (is it initialized?)", 
					iar);
		NSP = popValue(NSP);
		NSPushSpace(ptrstkelt);
		NSP->p.hdr = ptrHdr;
		NSP->p.v = (ATK  *)cip;
	}	break;

	} /* end switch(op) */
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = *\
 *
 *	Find all files in library path
 *
\* = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


/* recordFiles(path, end)
	make a libnode for each .n file in the directory 'dir'
*/
	static void
recordFiles(char *dir, class nesssym  *proto) {
	char *dirname;
	long len;
	DIR *dirp;
	DIRENT_TYPE *entry;

	len = strlen(dir);
	dirname = (char *)malloc(len+2);
	strcpy(dirname, dir);
	if (*(dirname+len-1) != '/')
		strcat(dirname+len, "/");
	dirp = opendir(dirname);
	if (dirp == NULL) return;
	while ((entry=readdir(dirp)) != NULL) {
		/* process a directory entry */
		char *fname, *symname, *t;
		class nesssym *s;
		boolean new_c;

		t = (char *)(entry->d_name + DIRENT_NAMELEN(entry));
		if (*(t-2) != '.' ||  *(t-1) != 'n')
			/* skip all files other than *.n */
			continue;
		fname = (char *)malloc(DIRENT_NAMELEN(entry)+1);
		symname = (char *)malloc(DIRENT_NAMELEN(entry)+1);
		strncpy(fname, entry->d_name, DIRENT_NAMELEN(entry));
		fname[DIRENT_NAMELEN(entry)] = '\0';
		/* build symbol name */
		strcpy(symname, fname);
		symname[DIRENT_NAMELEN(entry)-2] = '\0';	/* remove .n */
		for (t=symname; *t; t++)		/* smash case */
			if (isupper(*t)) *t = tolower(*t);
		s = nesssym::NLocate(symname, proto, LibScope, &new_c);
		free(symname);
		if (new_c) {
			/* we only record this file name if we do
				not previously have a library file of
				the same name */
		    LibList = libnode_Create(dirname, fname, NULL,
					NotRead, -1, LibList);
			nesssym_NSetINode(s, libnode, LibList);
			s->flags = flag_library;
		}
		else free (fname);
	}
	closedir(dirp);
}

/* callDirLibs()
	read directories on NessPath and find out what '.n' files exist
	build the LibList.
*/
	void
callDirLibs() {
	char *path, *colon;
	char dirname[MAXPATHLEN+1], tc;
	class nesssym *proto = new nesssym;

	path = (char *)environ::GetProfile("nesspath");
	if (path == NULL) 
		path = (char *)environ::AndrewDir("/lib/ness/");
	/* path is a colon separated list of directories */
	while (TRUE) {
		while (isspace(*path) || *path == ':') path++;
		if (*path == '\0') {
			delete proto;
			return;
		}
		colon = (char *)strchr(path, ':');
		if (colon == NULL)
			colon = path + strlen(path);
		/* colon points just beyond end of path */

		tc = *colon;
		*colon = '\0';


		filetype::CanonicalizeFilename(dirname, path, MAXPATHLEN);
		recordFiles(dirname, proto);

		*colon = tc;
		path = colon;
	}

/* xxxx need to check classpath and make a list of all .ch files available
for callCheckClass  */

}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = *\
 *
 *	Compile Time: look for function in library
 *
\* = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/* callCompLib(lnode)
	create a ness for 'lnode'
	compile the file referenced by 'lnode'
	set the status field in 'lnode' appropriately
*/
	void
callCompLib(struct libnode  *lnode) {
	char fullname[MAXPATHLEN+1];

	if (lnode->status == ReadFailed)
		return;
	if (lnode->ness == NULL) {
		lnode->ness = new ness;
		(lnode->ness)->SetName( lnode->filename);
		lnode->ness->libnode = lnode;
	}

	(lnode->ness)->SetAccessLevel( ness_codeUV);
	if (lnode->status == NotRead) {
		/* open the file and read it into the ness 
			(it may be a text data stream) */
		strcpy(fullname, lnode->path);
		strcat(fullname, lnode->filename);
		if ((lnode->ness)->ReadNamedFile( fullname) 
				!= dataobject::NOREADERROR) {
			lnode->ness->ErrorList = errornode_Create(lnode->ness,
					0, 0, 0, ":file read failed", 
					FALSE, NULL);
			(lnode->ness)->Expose();	
			lnode->status = ReadFailed;
			return;
		}
		lnode->status = NotCompiled;
	}

	/* compile the ness */
	if (lnode->status == NotCompiled) {
		lnode->status = Compiling;
		if ((lnode->ness)->Compile() != NULL) {
			lnode->status = CompiledWithError;
			(lnode->ness)->Expose();
		}
		else
			lnode->status = OK;
	}
}


/* callCheckLib(fun, fnode)
	looks to see if the function named 'fun' is in the library
	and loads it if so
	returns enum libstate value: OK if found and parsed the library
	If OK and find desired function, set *fnode to its funcnode
	(OK does not necessarily mean the function was found.)

	All function names are assumed to have an underline; the part 
	preceding the underline is taken to be the file name.
	If the file has a function named filename_Init, that function
	is called after a successful compile.
*/
	enum libstate
callCheckLib(char *fun, struct funcnode  **fnode) {
	char fname[MAXNAMLEN+1];
	char *under;
	long nmlen;
	class nesssym *libsym, *funcsym;
	struct libnode *libnode;
	struct libusenode *libuse;
	boolean found;

	*fnode = NULL;

	/* get file name */
	under = (char *)strchr(fun, '_');
	if (under == NULL)
		return NotRead;   /* not valid library function name */
	nmlen = MIN(MAXNAMLEN, under - fun);
	strncpy(fname, fun, nmlen);
	fname[nmlen] = '\0';
	libsym = nesssym::NFind(fname, LibScope);
	if (libsym == NULL) 
		return NotRead;		/* no such library file */
	libnode = nesssym_NGetINode(libsym, libnode);

	ness *n = curComp ? curComp->ness : NULL;

	if (n && libnode->useid != n->compilationid) {
		/* add it to the list of libraries used by ness now being compiled*/
		libnode->useid = n->compilationid;

		/* check to see if it is on the libuselist yet */
		found = FALSE;
		for (libuse = n->libuseList; libuse != NULL;
				libuse = libuse->next)
			if (libuse->used == libnode) {
				found = TRUE;
				break;
			}
		if ( ! found) 
			/* add it to libraries used by ness now being compiled*/
			n->libuseList = libusenode_Create(libnode,
					n->libuseList);
	}

	if (libnode->status != OK)
		callCompLib(libnode);

		/* note: no 'else' here.  we test the outcome of this
		or an earlier compile */
	if (libnode->status != OK) 
		return libnode->status;
	
	/* find the desired function in the file */
	funcsym = nesssym::NFind(fun, libnode->ness->outerScope);
	if (funcsym != NULL && funcsym->flags == (flag_function | flag_ness))
		/* BINGO! */
		*fnode = nesssym_NGetINode(funcsym, funcnode);
	return OK;
}


static long InitId;	/* which initialization cycle are we doing */

/* callInitSubTree(ness)
	calls init() in subtree headed by ness
	does not call it if its compilationid is InitId, to avoid circular call graphs
*/
	struct errornode *
callInitSubTree(class ness  *ness) {
	struct libusenode *t;
	struct errornode *err;
	if (ness->ErrorList != NULL)
	    return ness->ErrorList;
	if (ness->needsInit && InitId == ness->compilationid) {
		/* somewhere higher in the call chain, this ness is being 
			initialized,  But now it also needs to be initialized.
			Therefore there is a circularity.  Error 

			This situation can arise when there is a forward reference within 
			a library package to a routine in that same package.
		*/
		char buf[300];
		sprintf(buf, "*Circular initializations involving %s\n", ness->name);
		return ness->ErrorList = 
			errornode_Create(ness, 0, 0, 0, strdup(buf), TRUE, ness->ErrorList);
	}

	ness->compilationid = InitId;
	/* init all children on the off chance that the init calls a child */
	for (t = ness->libuseList;  t != NULL;  t = t->next) {
		err = callInitSubTree(t->used->ness);
		if (err != NULL)
			return err;
	}
	/* init this ness */
	if (ness->needsInit) {
	/* find and interpret init()
			MapRunErrors if needed
		*/
		ness->ErrorList = interpretNess(
				nesssym_NGetINode(ness->InitFunc, funcnode)
						->SysGlobOffset, 
				NULL, ness);
		if (ness->ErrorList == NULL) {
			/* COMPATABILITY with 00: accept init() for initialization */
			class nesssym *funcsym;
			for (funcsym = ness->globals; funcsym != NULL; 
					funcsym = funcsym->next)
				if (strcmp(funcsym->name, "init") == 0) 
					break;
			if (funcsym != NULL) 
				ness->ErrorList = interpretNess(
					nesssym_NGetINode(funcsym, funcnode)
						->SysGlobOffset, 
					NULL, ness);
		}
		if (ness->ErrorList != NULL) {
			neventUnpost(ness, FALSE);	/* remove old postings */
			MapRunError(ness);
			(ness)->Expose();
		}
		ness->needsInit = FALSE;
	}
	return ness->ErrorList;
}


/* callInitAll(ness)
	call init() for 'ness' and all libraries it calls
*/
	struct errornode *
callInitAll(class ness  *ness) {
	InitId = im::GetWriteID();
	return callInitSubTree(ness);
}


/* ReadTextFileStream(text, name, f)
	read a text from an already open file
	the file name is required so it can be checked to see
		what the extension is, and thus what kind of object
	if the file is an arbiter object surrounding a text object, 
		the latter is extracted from the former
	if the file is a non-text object it is an error, unless objok is TRUE
	if the file is a non-text object and objok is TRUE,
		the object is returned as the only element in the text
*/
	long
ReadTextFileStream(class text  *text, const char *name, FILE  *f, boolean  objok) {
	long objectID;
	long val;
	char *objectType;
	struct attributes *attributes;

	objectType = filetype::Lookup(f, name, &objectID, &attributes);
	if (objectType != NULL) {
		if (ATK::IsTypeByName(objectType, "arbiter")) {
			/* XXX there is an arbiter wrapped around it */
			class text *t;
			class arbiter *shit = new arbiter;
			val = (shit)->Read( f, objectID);
			if (val != dataobject::NOREADERROR)
				return val;
			t = (class text *)(shit)->GetObject();
			if ((t)->IsType( textClass)) {
				if (attributes != NULL)
					(text)->SetAttributes( attributes);
				(text)->AlwaysCopyTextExactly( 0, 
					t, 0, (t)->GetLength());
				val = dataobject::NOREADERROR;
			}
			else 
				val = dataobject::BADFORMAT;
			(shit)->Destroy();
			return val;
		}
		else if ( ! ATK::IsTypeByName(objectType, "text")) {
			if (objok) {
				class viewref *vr;
				class dataobject *dobj;
				vr = (text)->InsertObject( 0, 
						objectType, NULL);
				if (vr == NULL) 
					return dataobject::BADFORMAT;
				dobj = vr->dataObject;
				if (attributes != NULL)
					(dobj)->SetAttributes( 
						attributes);
				return (dobj)->Read( f, objectID);
			}
			else {
			  /*	fprintf(stderr, "%s, it's a %s\n", 
					"File is not a ness",
					objectType);  */
				return dataobject::BADFORMAT;
			}
		}
	}
	if (attributes != NULL)
		(text)->SetAttributes( attributes);
	return (text)->Read( f, objectID);
}

#if 0
}
#endif


static atom_def subseq("subseq");
static atom_def unknown("unknown");
static atom_def funcatom("function");

	static void 
AddType(avalueflex &l, Texpr type) {
	avalue *arg = l.Append();
	*arg = (const atom *)unknown;
	switch(type&0xf) {
	case Tend:
		break;
	case Tbool:
		*arg = (const atom *)ness_booleanatom;
		break;
	case Tshrt:
	case Tlong:
	case Tchr:
		*arg = avalue::integer;
		break;
	case Tbra:
	case Tdbl:
		*arg = avalue::real;
		break;
	case Tstr:
		*arg = (const atom *)subseq;
		break;
	case Tptr:
		*arg = avalue::atkatom;
		break;
	case Tfunc:
		*arg=(const atom *)funcatom;
		break;
	case Tvoid:
		*arg=avalue::voidatom;
		break;
	case Tunk:
		break;
	case Tcstr:
		*arg=avalue::cstring;
		break;
	}
}

	static void 
AddArgs(avalueflex &args, nesssym *parms) {
	if(parms) {
		if(parms->next) AddArgs(args, parms->next);
		AddType(args, parms->type);
	}
}

	static void 
PopReturn(avalueflex &out) {
	static atom_def booleanatom("boolean");
	stackelement *NSP=NSPstore;
	TType type=NSPstore->l.hdr;
	switch (type) {
	case (longHdr):
	case (boolHdr):
		if (type == boolHdr) out.add(NSP->l.v, NULL, booleanatom);
		else out.add(NSP->l.v);
		NSPopSpace(longstkelt);
		break; 
	case (dblHdr):
		out.add(NSP->d.v);
		NSPopSpace(dblstkelt);
		break;
	case seqHdr:
		out.add(NSP->s.v, NULL, subseq);
		NSPopSpace(seqstkelt);
		break;
	case (ptrHdr):
		out.add(NSP->p.v, NULL, avalue::atkatom);
		NSPopSpace(ptrstkelt);
		break;
	}

}

extern class ness *InterpretationInProgress;

static atom_def nessatom("ness");

	static void 
ness_ProcRunObj(ATK *obj, const avalueflex &aux, 
		const avalueflex &in, avalueflex &out) {
	funcnode *fn=(funcnode *)aux[0].VoidPtr();
	ness *self=(ness *)aux[1].ATKObject(nessatom);
	callnode *oldcall=(callnode *)aux[2].VoidPtr();
	
	if(self->ErrorList != NULL 
				|| (self->ErrorList=callInitAll(self))) {
		MapRunError(self);
		self->Expose();		
		if(oldcall && oldcall->variety==callPE 
					&& oldcall->where.pe) 
			proctable::Call(oldcall->where.pe, obj, in, &out);
	} 
	else {
		self->CurrentInset=NULL;

		ness_InitInterp(self);
		stackelement *NSP=NSPstore;
		NSPushSpace(ptrstkelt);
		NSP->p.hdr = ptrHdr;
		NSP->p.v = obj;
		size_t bottom=ness_InitArgMarks();
		ness_StackArgs(in);
		NSP=NSPstore;
		if(oldcall && oldcall->variety == callPE 
				&& oldcall->where.pe) {
			NSPushSpace(funcstkelt);
			NSP->c.hdr=funcHdr;
			NSP->c.call=oldcall;
		}
		if (self->ErrorList == NULL)
			self->ErrorList = interpretNess(
						fn->SysGlobOffset, 
						NULL, self);
		if (self->ErrorList != NULL) {
			neventUnpost(self, FALSE); // remove old posts
			MapRunError(self);
			self->Expose();
			if(oldcall && oldcall->variety==callPE 
						&& oldcall->where.pe) {
				proctable::Call(oldcall->where.pe, 
					obj, in, &out);
			}
		}
		PopReturn(out);
		ness_ClearArgMarks(bottom);
		InterpretationInProgress=NULL;
	}
}

	static void 
ness_ProcRunNoObj(ATK *obj, const avalueflex &aux, 
			const avalueflex &in, avalueflex &out) {
	funcnode *fn=(funcnode *)aux[0].VoidPtr();
	ness *self=(ness *)aux[1].ATKObject(nessatom);
	callnode *oldcall=(callnode *)aux[2].VoidPtr();
	   
	if (self->ErrorList != NULL 
				|| (self->ErrorList=callInitAll(self))) {
		MapRunError(self);
		self->Expose();
		if(oldcall && oldcall->variety==callPE 
					&& oldcall->where.pe) 
			proctable::Call(oldcall->where.pe, obj, in, &out);
	}
  
	ness_InitInterp(self);
	size_t bottom=ness_InitArgMarks();
	ness_StackArgs(in);
	stackelement *NSP=NSPstore;

	if(oldcall && oldcall->variety==callPE 
					&& oldcall->where.pe) { 
		NSPushSpace(funcstkelt);
		NSP->c.hdr=funcHdr;
		NSP->c.call=oldcall;
	}
	
	if (self->ErrorList == NULL)
		self->ErrorList = interpretNess(
						fn->SysGlobOffset, 
						NULL, self);

	if (self->ErrorList != NULL) {
		neventUnpost(self, FALSE); // remove old postings
		MapRunError(self);
		self->Expose();
		if (oldcall && oldcall->variety == callPE 
					&& oldcall->where.pe)
			  proctable::Call(oldcall->where.pe, obj, in, &out);
	}
	PopReturn(out);
	ness_ClearArgMarks(bottom);
	InterpretationInProgress = NULL;
	return;
}

boolean ness_ProcHookType::TypeCheck(const avalueflex &in) {
	const avalueflex &argtypes=ArgTypes();
	avalueflex_citer iter(argtypes);  // iterate over the argument 
							// types to be checked
	avalueflex_citer aiter(in);  // iterate over the arguments.
	while( ! iter.done() &&  ! aiter.done()) {
		if (iter->Atom() != aiter->Type()) {
			const atom *iat = iter->Atom();
			const atom *aity = aiter->Type();
			if (iat != subseq || aity != avalue::cstring) {
				if( ! ATK::IsTypeByName(aity->Name(), 
							iat->Name())) {
					fprintf(stderr, "%s between %s and %s.\n",
							"Type mismatch",
							aity->Name(), iat->Name());
					return FALSE;
				}
			}
		}
		iter++;
		aiter++;
	}
	if( ! iter.done() ||  ! aiter.done()) {
		fprintf(stderr, "Too %s arguments.\n", 
				iter.done()?"many":"few");
		return FALSE;
	}
	return TRUE;
}

	void 
ness_ExportFunction(ness *self, nesssym *ns, funcnode *fn) {
	avalueflex argtypes, rettype;
	AddArgs(argtypes, fn->parmlist);
	boolean hasfunc=FALSE;
	if (argtypes.GetN() > 0) {
		nesssym *la=fn->parmlist;
		if(la && (la->type&0xf) == Tfunc && la->GetName() 
				&& strcmp(la->GetName(), "oldfunc") == 0) {
			argtypes.Remove(argtypes.GetN()-1);
			hasfunc=TRUE;
		}
	}
	AddType(rettype, fn->functype);
	char *pname = new char[strlen(ns->GetName())+1];
	strcpy(pname, ns->GetName());
	for(char *q=pname;*q;q++) if(*q == '_') *q = '-';
	static ATKregistryEntry *atkent = ATK::LoadClass("ATK");
	proctable_Entry *keeppe = NULL;
	callnode *keepcall = NULL;
	if (hasfunc) {
		proctable_Entry *oldpe = proctable::Lookup(pname);
		if (oldpe) {
			keeppe = new proctable_Entry(*oldpe);
			keepcall = ProcToCall(keeppe, &specialexprnode, 
					NULL);
		}
	}
	aaction *act = NULL;
	avalueflex auxargs;
	auxargs
	  .add((void *)fn)
	  .add(self)
	  .add(keepcall);
	  
	if(argtypes.GetN()>0 
			&& argtypes[0].Atom() == avalue::atkatom) {
		argtypes.Remove((size_t)0);
		act = new ness_ProcHookType(ness_ProcRunObj, 
				auxargs, argtypes, rettype);
	} else {
		act = new ness_ProcHookType(ness_ProcRunNoObj, 
				auxargs, argtypes, rettype);
	}
	proctable_Entry *pe = proctable::DefineAction(pname, act, 
			atkent, "Function provided by ness code.\n", NULL);
	/* sigh, now we add a little hack which records the pointer 
		to the proctable function name if needed, 
		or frees the name if it is no longer needed. -robr */
	if(proctable::GetName(pe) != pname) {
		fn->procname = NULL;
		delete pname;
	} else fn->procname = pname;	
	fn->pe = pe;
	fn->oldcall = keepcall;
}

#if 0 /* use commented out below */
	static long 
ScanForProcs(sym *s, long rock) {
	libnode *lnode = (libnode *)rock;
	nesssym *ns = (nesssym *)s;
	char *dot = strchr(lnode->filename, '.');
	size_t nmlen;
	if (dot) nmlen = dot-lnode->filename;
	else nmlen = strlen(lnode->filename);
	if ((ns->type&0xf) != Tfunc 
			|| s->GetScope()!=lnode->ness->outerScope 
			|| strncmp(lnode->filename, s->GetName(), 
					nmlen) != 0) 
		return 0;
	funcnode *fn = nesssym_NGetINode(ns,funcnode);
	if(fn && fn->pe==NULL) 
		ness_ExportFunction(lnode->ness, ns, fn);
	return 0;
}
#endif

	static void 
ness_ProcHookFunc(ATK *, const avalueflex &aux, 
		const avalueflex &in, avalueflex &out) {
	const char *fun = in[0].CString();
	char fname[MAXNAMLEN+1];
	char *under;
	long nmlen;
	class nesssym *libsym;
	struct libnode *libnode;
	struct libusenode *libuse;
	boolean found;

	/* get file name */
	under = (char *)strchr(fun, '-');
	if (under) nmlen = MIN(MAXNAMLEN, under - fun);
	else nmlen = MIN(MAXNAMLEN, strlen(fun));
	strncpy(fname, fun, nmlen);
	fname[nmlen] = '\0';
	libsym = nesssym::NFind(fname, LibScope);
	if (libsym == NULL) return;
	libnode = nesssym_NGetINode(libsym, libnode);
	strncpy(fname, fun, MAXNAMLEN);
	fname[MAXNAMLEN]='\0';
	for (under = fname; *under; under++) 
			if (*under == '-') *under='_';
	static int id=im::GetWriteID();
	static struct libusenode *libuseList=NULL;  // list of libs used 
	if (libnode->useid != id) {
		// append to list of libs used by ness now being compiled
		libnode->useid = id;

		/* check to see if it is on the libuselist yet */
		found = FALSE;
		for (libuse = libuseList; libuse != NULL;
		libuse = libuse->next)
			if (libuse->used == libnode) {
				found = TRUE;
				break;
			}
		if ( ! found) {
			// append to list of libs used by current ness
			libuseList = libusenode_Create(libnode, libuseList);
			// initialize it.  It would be better to defer
			// this until a function from the library is
			// called, but I don't feel like implementing that
			// at this time
			// Actually, this may crash, for all I know - tjm
			callInitSubTree(libnode->ness);
		}
	}

	if (libnode->status != OK) 
		callCompLib(libnode);
	/* note: no 'else' here.  we test the outcome of this
	 		or an earlier compile */
	if (libnode->status != OK) {
		return;
	}

//	nesssym::Enumerate(libnode->ness->outerScope, 
//		ScanForProcs, (long)libnode);
}

NO_DLL_EXPORT 	traced_ptr<ness_ProcHookType>
ness_ProcHook(new ness_ProcHookType(ness_ProcHookFunc, 0L, avalue::cstring, 
	avalue::voidatom));


