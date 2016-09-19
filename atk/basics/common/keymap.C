/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* keymap.c -- A class that provides mappings of keys to procedures.
December, 1986 */


#include <andrewos.h>
ATK_IMPL("keymap.H")
#include <keymap.H>
#include <proctable.H>

#define KEYMASK	(keymap_MAXKEYS-1)


ATKdefineRegistry(keymap, ATK, NULL);
struct keymap_fulltable *NewFullTable();
static void DoInitialize(class keymap  *self, boolean  sparsep);
static boolean bindKey(class keymap  *self,const unsigned char *keys,ATK   *obj,long  rock,enum keymap_Types  type);
static void ExpandTable(class keymap  *self);


struct keymap_fulltable *NewFullTable()
{

    int i;
    struct keymap_fulltable *newTable = (struct keymap_fulltable *)
                                 malloc(sizeof(struct keymap_fulltable));

    for (i = 0; i < keymap_MAXKEYS; ++i) {
        newTable->types[i] = keymap_Empty;
	newTable->objects[i] = NULL;
	newTable->rocks[i]=0;
    }
    return newTable;
}

static void DoInitialize(class keymap  *self, boolean  sparsep)
        {

    self->sparsep = sparsep;
    if (sparsep) {
        self->table.sparse = (struct keymap_sparsetable *)
                             malloc(sizeof(struct keymap_sparsetable));
        self->table.sparse->numValid = 0;
    }
    else
        self->table.full = NewFullTable();
}

keymap::keymap()
        {
    DoInitialize(this, TRUE);
    THROWONFAILURE( TRUE);
}

keymap::keymap(const keymap &o) {
    sparsep=o.sparsep;
    if(sparsep) {
	table.sparse=(struct keymap_sparsetable *)
	  malloc(sizeof(struct keymap_sparsetable));
	*table.sparse=*o.table.sparse;
    } else {
	table.full=(struct keymap_fulltable *)                                  malloc(sizeof(struct keymap_fulltable));
	*table.full=*o.table.full;
    }
}
	
keymap::~keymap()
{
    int i;
    if(this->sparsep) {
	for (i = 0; i < this->table.sparse->numValid; ++i) {
	    if(this->table.sparse->types[i]==keymap_Keymap) delete (class keymap *)this->table.sparse->objects[i];
	}
	if(this->table.sparse) free(this->table.sparse);
    } else {
	for (i = 0; i < keymap_MAXKEYS; ++i) {
	    if(this->table.full->types[i] == keymap_Keymap)
		delete (class keymap *)this->table.full->objects[i];
	}
	if(this->table.full) free(this->table.full);
    }
}

static boolean bindKey(class keymap  *self,const unsigned char *keys,ATK   *obj,long  rock,enum keymap_Types  type)
                    {
    enum keymap_Types e;
    const unsigned char *p;
    class keymap *km1, *km2;
    unsigned char c;

    for (p = keys, km1 = self; p[1] != 0; ++p) {
	c = (*p == 128)? 0 : *p; /*  allows nulls in string to be represented by 128 */
	e = (km1)->Lookup( c, (ATK **)&km2,NULL );
	if (e == keymap_Empty) {
	    e = keymap_Keymap;
	    km2 = new keymap;
	    (km1)->InsertObject( c, km2, rock, keymap_Keymap);
	}
	else if (e != keymap_Keymap)
	    return FALSE;
	km1 = km2;
    }
    c = (*p == 128)? 0 : *p; /*  allows nulls in string to be represented by 128  */
	
    (km1)->InsertObject(c,obj,rock,type);

    return TRUE;
}

boolean keymap::BindToKey(const char  *keys, const struct proctable_Entry  *pe, long  rock)
                {

    if (keys == NULL || *keys == 0)
	return FALSE;
    bindKey(this,(unsigned char *)keys,(ATK  *) pe,rock,keymap_Proc);
    return TRUE;
}

void keymap::RemoveBinding(const char  *keys)
{
    bindKey(this,(const unsigned char *)keys,NULL,0,keymap_Empty);
}

static void ExpandTable(class keymap  *self)
    {

/* Don't even try to expand a full table... */
    if (self->sparsep) {

        int i;
        struct keymap_fulltable *newTable;

        newTable = NewFullTable();
        for (i = 0; i < self->table.sparse->numValid; ++i) {

            int key = self->table.sparse->keys[i];

            newTable->types[key] = self->table.sparse->types[i];
            newTable->objects[key] = self->table.sparse->objects[i];
	    newTable->rocks[key]=self->table.sparse->rocks[i];
	}
        free(self->table.sparse);
        self->table.full = newTable;
        self->sparsep = FALSE;
    }
}

void keymap::InsertObject(long  slot, ATK   *object, long  rock, enum keymap_Types  type)
                    {

    slot &= KEYMASK;
    if (this->sparsep) {

        int i, numValid = this->table.sparse->numValid;

        for (i = 0; i < numValid && this->table.sparse->keys[i] != slot; ++i)
            ;
        if (i != numValid || (numValid < keymap_SPARSESIZE && ++this->table.sparse->numValid)) {
            this->table.sparse->keys[i] = slot & KEYMASK;
            this->table.sparse->types[i] = type;
	    this->table.sparse->objects[i] = object;
	    this->table.sparse->rocks[i]=rock;
            return;
        }
        else
            ExpandTable(this);
    }

    this->table.full->types[slot & KEYMASK] = type;
    this->table.full->objects[slot & KEYMASK] = object;
    this->table.full->rocks[slot&KEYMASK]=rock;
}

enum keymap_Types keymap::Lookup(char  key, ATK   **object,long  *rockP)
                {
    key &= KEYMASK;

    if (this->sparsep) {

        int i;

        for (i = 0; (i < this->table.sparse->numValid) && (this->table.sparse->keys[i] != key); i++)
            ;
        if (i < this->table.sparse->numValid) {
            if (object != NULL)
                *object = this->table.sparse->objects[i];
	    if(rockP!=NULL)
		*rockP=this->table.sparse->rocks[i];
            return this->table.sparse->types[i];
        }
        else
            return keymap_Empty;
    }

    if (object != NULL)
        *object = this->table.full->objects[(unsigned char)key];
    if(rockP!=NULL)
	*rockP=this->table.full->rocks[(unsigned char)key];
    return this->table.full->types[(unsigned char)key];
}
