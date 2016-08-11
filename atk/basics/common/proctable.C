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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/proctable.C,v 3.13 1996/05/17 17:49:47 robr Exp $";
#endif


 

/* proctbl.c -- A module that manages a mapping from strings to procedure pointers.
 */


#include <andrewos.h>
ATK_IMPL("proctable.H")
#include <util.h>
#include <ctype.h>
#include <proctable.H>
#include <message.H>
#include <aaction.H>
#include <environ.H>

#define HASHMAX	128		/* must be power of two */
#define HASHMASK	(HASHMAX-1)

static struct proctable_Entry *hashTable[HASHMAX];


/* Initialize the package. 
*/
	
ATKdefineRegistry(proctable, ATK, proctable::InitializeClass);

static int ModuleClear(struct proctable_Entry  *pe, char  *module);
static int HashName(const char  *name);
static struct proctable_Entry *LookupHash(const char  *name, int  hash);

boolean 
proctable::InitializeClass()
	{
	int i;
	
	for (i = 0; i < HASHMAX; ++i)
		hashTable[i] = NULL;
	return TRUE;
}

/* Add an entry to the table.  Only the name needs to be defined, 
	but all storage pointed to must be permanent 
	(do the mallocs in the caller or pass literals). 
*/
	struct proctable_Entry *
proctable::DefineProc(char  *name		/* these match the fields in an entry */, proctable_fptr proc, struct ATKregistryEntry   *type, char  *module, char  *doc)
						{
	ATKinit;

	int hash;
	struct proctable_Entry *pe = NULL;

	if (name == NULL) {
		pe = (struct proctable_Entry *)malloc(sizeof(struct proctable_Entry));
		pe->name = NULL;
		pe->proc = proc;
		pe->act=NULL;
		pe->type = type;
		pe->module = module;
		pe->doc = doc;
		pe->returntype = proctable_Void;
	}
	else {
		hash = HashName(name);
		pe = LookupHash(name, hash);
		if (pe == NULL) {
			pe = (struct proctable_Entry *)malloc(sizeof(struct proctable_Entry));
			if (pe == NULL)
				return NULL;
			pe->hnext = hashTable[hash];
			hashTable[hash] = pe;
			pe->name = name;
			pe->type = NULL;
			pe->module = NULL;
			pe->doc = NULL;
			pe->proc = NULL;
			pe->act=NULL;
			pe->returntype = proctable_Void;
		}
		if (proc != NULL)
		{
		    pe->proc = proc;
		    pe->act=NULL;
		}
		if (type != NULL)
			pe->type = type;
		if (module != NULL)
			pe->module = module;
		if (doc != NULL)
			pe->doc = doc;
	}
	return pe;
}

void proctable::DefineProcs(struct proctable_Description  *procs)
{
	ATKinit;

	while(procs->name!=NULL){
		proctable::DefineProc(procs->name, procs->proc, procs->type, 
				procs->module, procs->doc);
		procs++;
	}
}

/* Add a typed entry to the table.  Only the name needs to be defined, 
	but all storage pointed to must be permanent (do the mallocs in the caller or pass literals). 
*/
	struct proctable_Entry *
proctable::DefineTypedProc(char  *name		/* these match the fields in an entry */, proctable_fptr proc, struct ATKregistryEntry   *type, 
		char  *module, char  *doc, enum proctable_type  returntype)
							{
	ATKinit;

	struct proctable_Entry *pe;
	pe = proctable::DefineProc(name, proc, type, module, doc);
	if (pe != NULL)
		pe->returntype = returntype;
	return pe;
}

void proctable::DefineProcsWithTypes(struct proctable_DescriptionWithType  *procs)
{
	ATKinit;

	while(procs->name!=NULL){
		proctable::DefineTypedProc(procs->name, procs->proc, procs->type, 
					procs->module, procs->doc, procs->returntype);
		procs++;
	}
}

static boolean guard=FALSE;
void proctable::Preload(const char *name) {
    if(guard) return;
    guard=TRUE;
    char buf[256];
    const char *p=strchr(name, '-');
    size_t len;
    if(p) len=(p-name);
    else len=strlen(name);
    if(len>sizeof(buf)-1) len=sizeof(buf)-1;
    if(len==0) {
	guard=FALSE;
	return;
    }
    strncpy(buf, name, len);
    buf[len]='\0';
    if(ATK::LoadClass(buf)) {
	proctable_Entry *pe=proctable::Lookup(name);
	if(pe && proctable::Defined(pe)) {
	    guard=FALSE;
	    return;
	}
    }
    static boolean tryness=environ::GetProfileSwitch("AutoLoadNessProcs", TRUE);
    if(tryness) {
	static ATKregistryEntry *nessent=ATK::LoadClass("ness"); 
	// at first glance this line might seem to invite mutual recursion, but the 'static' saves us.
	static proctable_Entry *pe=proctable::Lookup("ness-proctable-hook");
	if(pe) proctable::Call(pe, NULL, name, NULL);
    }
    guard=FALSE;
}

/* Given a name, look up its entry. */
struct proctable_Entry *proctable::Lookup(const char  *name)
{
    ATKinit;
    register int hash;
    hash = HashName(name);
    proctable_Entry *result=LookupHash(name, hash);
    if(result==NULL && !guard) {
	proctable::Preload(name);
	result=LookupHash(name, hash);
    }
    return result;
}

void proctable::DeleteProc(const char *cpe) {
    int hash=HashName(cpe);
    proctable_Entry *pe, **ppe=&hashTable[hash];
    for (pe = hashTable[hash]; pe != NULL; pe = pe->hnext) {
	if (FOLDEDEQ(pe->name, (char *)cpe)) {
	    (*ppe)=pe->hnext;
	    free(pe);
	}
	ppe=&pe->hnext;
    }
}

void proctable::DeleteProc(const struct proctable_Entry *pe) {
    int hash=HashName(pe->name);
    proctable_Entry *spe, **ppe=&hashTable[hash];
    for (spe = hashTable[hash]; spe != NULL; spe = spe->hnext) {
	if (spe==pe) {
	    (*ppe)=spe->hnext;
	    free(spe);
	}
	ppe=&spe->hnext;
    }
}

/* Call the proc with each entry and with the rock. */
void proctable::Enumerate(proctable_efptr proc, char  *procdata)
			{
	ATKinit;

	int hash;
	struct proctable_Entry *pe;

	for (hash = 0; hash < HASHMAX; ++hash)
		for (pe = hashTable[hash]; pe != NULL; pe = pe->hnext)
			(*proc)(pe, procdata);
}

/* Force the package for this function to be loaded if possible. */
void proctable::ForceLoaded(const struct proctable_Entry  *pe)
		{
	ATKinit;

	if (proctable::Defined(pe) || pe->module == NULL)
		return;
	(void) proctable::Preload(pe->module);
	/* Go ahead and mark all other procs for this module as loaded. */
	proctable::Enumerate((proctable_efptr)ModuleClear, pe->module);
}

/* Potentially clear the module pointer for this entry. */
static int ModuleClear(struct proctable_Entry  *pe, char  *module)
		{
	if (pe != NULL && pe->module != NULL && strcmp(pe->module, module) == 0)
		pe->module = NULL;
	return(0);
}


/* Compute the hash function for this name. */
static int HashName(const char  *name)
	{
	register int hash = 0;

	while (*name != 0) {
		hash = hash + isupper(*name)?tolower(*name):*name;
		name++;
	}
	return hash & HASHMASK;
}

/* Given a name and a hash index, look up the name. */
static struct proctable_Entry *LookupHash(const char  *name, int  hash)
		{
	struct proctable_Entry *pe;

	for (pe = hashTable[hash]; pe != NULL; pe = pe->hnext)
		if (FOLDEDEQ(pe->name, (char *)name))
			return pe;
	return NULL;
}


static long ActionInterface(ATK *obj, long rock) {
    if(rock>=0 && rock<=255) {
	message::DisplayString(NULL, 0, "This function requires it's rock to be it's name.");
	return 0; 
    }
    char *name=(char *)rock;
    proctable_Entry *pe=proctable::Lookup(name);
    if(pe==NULL || pe->act==NULL) {
	message::DisplayString(NULL, 0, "The function name was invalid.");
	return 0;
    }
    avalueflex out;
    avalueflex in;
    (*pe->act)(obj, in, out);
    return 0;
}

/* Add an entry to the table.  Only the name needs to be defined, 
	but all storage pointed to must be permanent 
	(do the mallocs in the caller or pass literals). 
*/
	struct proctable_Entry *
proctable::DefineAction(char  *name		/* these match the fields in an entry */, aaction *act, struct ATKregistryEntry   *type, char  *module, char  *doc)
						{
	ATKinit;

	int hash;
	struct proctable_Entry *pe = NULL;

	if (name == NULL) {
		pe = (struct proctable_Entry *)malloc(sizeof(struct proctable_Entry));
		pe->name = NULL;
		pe->proc = ActionInterface;
		pe->act=act;
		pe->type = type;
		pe->module = module;
		pe->doc = doc;
		pe->returntype = proctable_Void;
	}
	else {
		hash = HashName(name);
		pe = LookupHash(name, hash);
		if (pe == NULL) {
			pe = (struct proctable_Entry *)malloc(sizeof(struct proctable_Entry));
			if (pe == NULL)
				return NULL;
			pe->hnext = hashTable[hash];
			hashTable[hash] = pe;
			pe->name = name;
			pe->type = NULL;
			pe->module = NULL;
			pe->doc = NULL;
			pe->proc = NULL;
			pe->act=NULL;
			pe->returntype = proctable_Void;
		}
		if (act != NULL)
		{
		    pe->proc = ActionInterface;
		    pe->act=act;
		}
		if (type != NULL)
			pe->type = type;
		if (module != NULL)
			pe->module = module;
		if (doc != NULL)
			pe->doc = doc;
	}
	return pe;
}


void proctable::DefineActions(struct proctable_ActionDescription  *procs)
{
	ATKinit;

	while(procs->name!=NULL){
		proctable::DefineAction(procs->name, procs->act, procs->type, 
				procs->module, procs->doc);
		procs++;
	}
}
proctable_CallError proctable::Call(const char *cpe, ATK *obj, const avalueflex &in, avalueflex *out) {
    proctable_Entry *pe=proctable::Lookup((char *)cpe);
    if(pe==NULL) return proctable_CallNoProc;
    return Call(pe, obj, &in, out);
}
proctable_CallError proctable::Call(const struct proctable_Entry *pe, ATK *obj, const avalueflex &in, avalueflex *out) {
    return Call(pe, obj, &in, out);
}

proctable_CallError proctable::Call(const struct proctable_Entry *pe, ATK *obj, const avalueflex *in, avalueflex *out) {
    if(pe->type && obj && !obj->IsType(pe->type)) {
	// the wrong object type.
	return proctable_CallObjectTypeMismatch;
    }
    avalueflex args;
    avalueflex result;
    if(pe->act) {
	if(out==NULL) out=&result;
	if(in==NULL) in=&args;
	(*pe->act)(obj, *in, *out);
    } else if (in==NULL || in->GetN()==0) {
	pe->proc(obj,0);
    } else {
	if((*in)[0].Type()==avalue::integer) {
	    pe->proc(obj,(*in)[0].Integer());
	} else if((*in)[0].Type()==avalue::cstring) {
	    pe->proc(obj, (long)(*in)[0].CString());
	} else {
	    return proctable_CallArgTypeMismatch;
	}
    }
    if(out) {
	// The element named 'error' indicates what sort of error may have occurred.
	// If the type is avalue::integer it is a proctable_CallError value.
	// Otherwise only a failure indication is returned.
	static atom_def error("error");
	avalue *val=NULL;
	size_t ind=out->GetN();
	if(ind==0) return proctable_CallOk;
	if((*out)[ind-1].Name()==error) {
	    // a little optimization hack. silly really. -rr2b
	    val=&(*out)[ind-1];
	} else val=out->Find(error);
	if(val) {
	    if((val->Type()==avalue::integer)) {
		return (proctable_CallError)val->Integer();
	    } else {
		return proctable_CallFailed;
	    }
	}
    }
    return proctable_CallOk;
}
