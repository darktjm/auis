#ifndef _toksym_H_
#define _toksym_H_
/* ********************************************************************** *\
 *     Copyright IBM Corporation 1988,1991, 1993 - All Rights Reserved    *
\* ********************************************************************** */

/** \addtogroup libsym
 * @{ */
/* toksym.ch
 *
 * A toksym object is a subclass of sym.  Its adds four fields:
 *	toknum	-  the token number for compiler
 *	loc, len 	-  position of the token in the source stream
 *	info	-  type dependant information
 *
 */


#include "sym.H"	
/* This include generates an error message claiming that sym.ih is included twice.  
   The conflict is with the include generated by class as part of toksym.ih, but
   the automatically generated include does NOT define the routines. */


/* the following doesn't work !?!?!  (in the old class system.  Maybe it works in C++)*/
/*	typedef sym_scopeType  toksym_scopeType;   */

typedef  long   toksym_scopeType;

#define toksym_GLOBAL sym_GLOBAL


union infotype {
	long intval;
	double *realval;
	char *str;
	long (*proc)();
	struct object *obj;
	struct node *node;
};


class toksym : public  sym
{
public:
	virtual ATKregistryEntry *ATKregistry();

	virtual char *ToC(class text *t, char *buf, long maxlen);
		/* copies the token from the text to the buffer, up to maxlen bytes
			returns buf */



/* the initial T in each of these procedure names is to get around a bug in the class system */

	static inline toksym_scopeType TNewScope(toksym_scopeType enclosingScope) {
		return ((toksym_scopeType)sym::NewScope(enclosingScope)); }

	static inline void TDestroyScope(toksym_scopeType scope) { 
		(sym::DestroyScope(scope)); }

	static inline toksym_scopeType TParentScope(toksym_scopeType scope) { 
		return ((toksym_scopeType)sym::ParentScope(scope)); }

	static inline class toksym * TDefine(const char *name, 
				class toksym *proto, toksym_scopeType scope) { 
		return ((class toksym *)sym::Define(name, proto, scope)); }

	static inline boolean TUndefine(const char *name, toksym_scopeType scope) { 
		return (sym::Undefine(name, scope)); }

	static inline class toksym *TFind(const char *name, toksym_scopeType scope) { 
		return ((class toksym *)sym::Find(name, scope)); }

	static inline class toksym *TLocate(const char *name, class toksym *proto,
 			toksym_scopeType scope, boolean *new_c) { 
		return ((class toksym *)sym::Locate(name, proto, scope, new_c)); }

	static inline long TFindAll(const char *name, toksym_scopeType scope, 
			sym_findfptr proc, long rock) { 
		return (sym::FindAll(name, scope, proc, rock)); }


	inline const char * TGetName()	{ return ((this)->name); }
	inline toksym_scopeType TGetScope()  {
		return ((toksym_scopeType)(this)->scope); }
	inline long GetLoc()		{ return ((this)->loc); }
	inline long GetLen()		{ return ((this)->len); }

	
	toksym();
	virtual ~toksym();


	short toknum;	/* token code for yacc */
	long loc, len;	/* position of token in source */
	union infotype info;	/* type dependent */

};
/** @} */
#endif /* _toksym_H_ */
