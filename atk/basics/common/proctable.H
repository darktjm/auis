ATK_INTER
#ifndef _proctable_H_
#define _proctable_H_
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


 

/* proctbl.ch -- A module that manages a mapping from strings to procedure pointers.
December, 1986 */

enum proctable_type {
	proctable_Void,		/* does not return a type */
	proctable_Boolean,  	/* class.c:boolean */
	proctable_Char,		/* 8 bit unsigned character */
	proctable_Short,		/* 16 bit integer */	
	proctable_Long,		/* 32 bit integer */
	proctable_Double,		/* double floating value */
	proctable_Object,		/* pointer to an object */
	proctable_NessMarker,	/* object of type nessmark */
	proctable_StaticString,	/* returns a pointer to a string which must
					NOT be freed by the recipient */
	proctable_DisposeString	/* returns a pointer to a string which 
					MUST be freed by recipient */
};


struct proctable_Entry;
struct proctable_Description;
struct proctable_DescriptionWithType;

#include <ATK.H>
class aaction;
class avalueflex;
typedef long (*proctable_fptr)(ATK *, long c);
typedef void (*proctable_efptr)(struct proctable_Entry *, char *rock);

/* Error codes when calling a proc. */
enum proctable_CallError {
    proctable_CallOk,		/* proc called ok */
    proctable_CallNoProc,		/* no such proc could be found */
    proctable_CallObjectTypeMismatch,	/* proc needs different object */
    proctable_CallArgTypeMismatch,  // The arguments disdn't match.
    proctable_CallFailed // A generic failure, the out list should have more information
    // in the element named 'error'.
    
};

class proctable : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();
	static inline boolean Defined(const struct proctable_Entry * self); 		
	static inline char *GetName(const struct proctable_Entry * self);		
	static inline proctable_fptr GetFunction(const struct proctable_Entry * self);		
	static inline struct ATKregistryEntry *GetType(const struct proctable_Entry * self);		
	static inline enum proctable_type GetReturnType(const struct proctable_Entry * self);	
	static inline char *GetModule(const struct proctable_Entry * self);		
	static inline char *GetDocumentation(const struct proctable_Entry * self); 	
	
	static boolean InitializeClass();
	/* called automagically */
	/* Only the name field is required.  Later calls to DefineProc will update an existing entry. */
	static struct proctable_Entry * DefineProc(char *name, proctable_fptr proc, struct ATKregistryEntry  *type, char *module, char *doc)  ;
	static struct proctable_Entry * DefineAction(char *name, aaction *proc, struct ATKregistryEntry  *type, char *module, char *doc)  ;
	static void DefineProcs(struct proctable_Description *procs);
	static void DefineActions(struct proctable_ActionDescription *procs);
	static struct proctable_Entry * DefineTypedProc(char *name, proctable_fptr proc, struct ATKregistryEntry  *type,
			char *module, char *doc, enum proctable_type returntype) 
			 ;
	static void DefineProcsWithTypes(struct proctable_DescriptionWithType *procs);
	static struct proctable_Entry * Lookup(const char *name)  ;
	static void Enumerate(proctable_efptr proc, char *procdata);
	static void ForceLoaded(const struct proctable_Entry *pe);
	static proctable_CallError Call(const struct proctable_Entry *pe, ATK *obj, const avalueflex *in, avalueflex *out=NULL);
	static proctable_CallError Call(const struct proctable_Entry *pe, ATK *obj, const avalueflex &in, avalueflex *out=NULL);
	static proctable_CallError Call(const char *cpe, ATK *obj, const avalueflex &in, avalueflex *out);
	static inline aaction *GetAction(const struct proctable_Entry *pe);
	static void DeleteProc(const char *cpe);
	static void DeleteProc(const struct proctable_Entry *pe);
	static void Preload(const char *name);
};


struct proctable_Entry {
	struct proctable_Entry *hnext;  /* hash table link */
	char *name;			/* name of the function */
	proctable_fptr proc;		/* pointer to it */
	struct ATKregistryEntry  *type;	/* type of object to apply it to */
	char *module;		/* what to dynamically load to get it */
	char *doc;			/* prose description of its function */
	enum proctable_type returntype;
	aaction *act;  /* the new way to specify the function is to derive from aaction
	 and implement operator()(ATK *obj, const avalueflex &in, avalueflex &out); */
};

struct proctable_ActionDescription {
	char *name;			/* name of the function */
	aaction *act;		/* pointer to it */
	struct ATKregistryEntry  *type;	/* type of object to apply it to */
	char *doc;			/* prose description of its function */
	char *module;		/* what to dynamically load to get it */
};

struct proctable_Description {
	char *name;			/* name of the function */
	 proctable_fptr proc;		/* pointer to it */
	struct ATKregistryEntry  *type;	/* type of object to apply it to */
	char *doc;			/* prose description of its function */
	char *module;		/* what to dynamically load to get it */
};

struct proctable_DescriptionWithType {
	char *name;			/* name of the function */
	 proctable_fptr proc;		/* pointer to it */
	struct ATKregistryEntry  *type;	/* type of object to apply it to */
	char *doc;			/* prose description of its function */
	char *module;		/* what to dynamically load to get it */
	enum proctable_type returntype;
};

inline boolean proctable::Defined(const struct proctable_Entry * self) 		{ return ((self)->proc !=  0); }
inline char *proctable::GetName(const struct proctable_Entry * self)		{ return ((self)->name); }
inline proctable_fptr proctable::GetFunction(const struct proctable_Entry * self)		{ return ((self)->proc); }
inline struct ATKregistryEntry *proctable::GetType(const struct proctable_Entry * self)		{ return ((self)->type); }
inline enum proctable_type proctable::GetReturnType(const struct proctable_Entry * self)	{ return ((self)->returntype); }
inline char *proctable::GetModule(const struct proctable_Entry * self)		{ return ((self)->module); }
inline char *proctable::GetDocumentation(const struct proctable_Entry * self) 	{ return ((self)->doc); }
inline aaction *proctable::GetAction(const struct proctable_Entry * self) 	{ return ((self)->act); }
#endif /* _proctable_H_ */
