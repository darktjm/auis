/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* keystate.ch -- A class that keep track of partially evaluated sequences of keystrokes.
December, 1986 */


#include <andrewos.h>
ATK_IMPL("keystate.H")
#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <aaction.H>

ATKdefineRegistryNoInit(keystate, ATK);
void InitState(class keystate  *self);


void keystate::SetObject(ATK   *object)
        {

    this->object = object;
}

class keystate *keystate::AddBefore(class keystate  *ks)
        {
    this->next = ks;
    return this;
}

class keystate *keystate::AddAfter(class keystate  *ks)
        {
    class keystate *kp;

    this->next = NULL;
    if (ks == NULL)
	return this;
    for (kp = ks; kp->next != NULL; kp = kp->next)
	;
    kp->next = this;
    return ks;
}

void InitState(class keystate  *self)
    {
    class keystate *ks;

    for (ks = self; ks != NULL; ks = ks->next)
	ks->curMap = ks->orgMap;
}

void keystate::Reset()
    {
    InitState(this);
}

enum keystate_ApplyKeyValues keystate::ApplyKey(char  key, struct proctable_Entry  **ppe, long  *rockP, ATK   **pobject)
                    {
    class keystate *ks;
    struct proctable_Entry *pe;
    long rock;
    boolean allBad = TRUE;	/* true while all keymaps looked at have no binding for this key */
    boolean foundProc = FALSE;	/* true if we execute something */
    boolean foundMap = FALSE;	/* true if we advance one keymap */
    enum keymap_Types code;

    for (ks = this; ks != NULL; ks = ks->next) {
        if (ks->function != NULL)
            code = (*ks->function)(ks->functionData, key, &pe, &rock);
        else {
            if (ks->curMap == NULL)
                continue;
            code = (ks->curMap)->Lookup( key, (ATK **)&pe, &rock);
        }
	if (code == keymap_Empty)  {
	    ks->curMap = NULL;
	    continue;
	}
	/* Found something, either a keymap or a proc. */
	allBad = FALSE;
	if (code == keymap_Keymap) {
	    foundMap = TRUE;
	    ks->curMap = (class keymap *)pe;
	}
	else { 	/* code == keymap_Proc */
	    if (foundMap) {
		/* Sorry, an earlier keymap takes precedence. */
		ks->curMap = NULL;
		continue;
	    }
	    foundProc = TRUE;
	    if (ppe != NULL)
		*ppe = pe;
	    if (pobject != NULL)
		*pobject = ks->object;
            if (rockP != NULL)
                *rockP = rock;
	    break;
	}
    }
    if (foundProc) {
	(this)->Reset();
	if(pe == NULL)
	    return keystate_NoBinding;
	else
	    return keystate_ProcFound;
    }
    if (allBad) {
	(this)->Reset();
	return keystate_NoBinding;
    }
    return keystate_Pending;
}

/* Apply the procedure in a proctable entry to the object, or to an object of the correct type as found in the keystate chain. */
enum keystate_DoProcValues keystate::DoProc(struct proctable_Entry  *pe, long  rock, ATK   *object)
                {
    class keystate *ks;

    proctable::ForceLoaded(pe);
    if (!proctable::Defined(pe)) {
	(this)->Reset();
	return keystate_NoProc;
    }
    if (!(object)->IsType( pe->type)) {
	/* Attempt to find object of correct type. */
	for (ks = this; ks != NULL; ks = ks->next)
	    if ((ks->object)->IsType( pe->type))
		break;
	if (ks == NULL) {
	    (this)->Reset();
	    return keystate_TypeMismatch;
	}
	object = ks->object;
    }
    if(pe->act!=NULL) {
	avalueflex a;
	if(rock<0 || rock>255) {
	    const char *r=(char *)rock;
	    if(strncmp(r,"list:",5)==0) {
		r+=5;
		a.InterpretString(&r);
	    } else {
		a.add(r);
	    }
	} else {
	    a.add(rock);
	}
	proctable::Call(pe,object,&a);
    } else (*pe->proc)(object, rock);
    return keystate_ProcCalled;
}

void keystate::FreeChain()
    {
    class keystate *ks, *kp;

    for (ks = this; ks != NULL; ks = kp) {
	kp = ks->next;
	delete ks;
    }
}

/* Class methods begin here. */


/* Basic class routines.  We have our own allocation routines for speed. */

static class keystate *freeKS = NULL;

keystate::keystate()
        {
    this->next = NULL;
    this->orgMap = NULL;
    this->curMap = NULL;
    this->object = NULL;
    this->function = NULL;
    THROWONFAILURE( TRUE);
}

class keystate *keystate::Create(ATK   *object, class keymap  *keymap)
            {
    class keystate *keystatep;
    
    keystatep = new keystate;
    keystatep->orgMap = keymap;
    keystatep->curMap = keymap;
    keystatep->object = object;
    return keystatep;
}

void keystate::SetOverride(keystate_fptr function, long  functionData)
            {

    this->function = function;
    this->functionData = functionData;
}

void keystate::GetOverride(keystate_fptr *function, long  *functionData)
            {

    *function = this->function;
    *functionData = this->functionData;
}

class keystate *keystate::Allocate()
    {
    class keystate *ks;

    if (freeKS == NULL)
	ks = (class keystate *) malloc(sizeof (class keystate));
    else {
	ks = freeKS;
	freeKS = freeKS->next;
    }
    return ks;
}

void keystate::Deallocate(class keystate  *ks)
        {
    ks->next = freeKS;
    freeKS = ks;
}
