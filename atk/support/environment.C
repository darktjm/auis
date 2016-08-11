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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/support/RCS/environment.C,v 3.4 1996/09/05 15:22:07 robr Exp $";
#endif


 


#include <andrewos.h>
ATK_IMPL("environment.H")
#include <environment.H>
#include <tree23int.H>
#include <viewref.H>

/* Crank out structs in 4k blocks. */
#define DESIREDBLOCKSIZE 4096
/* Number of marks per block */
#define NUMPERBLOCK DESIREDBLOCKSIZE / sizeof(class environment)
#define BLOCKSIZE NUMPERBLOCK * sizeof(class environment)

static class environment *freeList = NULL;
static class environment *lastBlock = NULL;


ATKdefineRegistry(environment, nestedmark, NULL);
#ifndef NORCSID
#endif
static long AlterEnvironmentSize(class environment  *self, struct removestruct  *data);
void environment__Dump(class environment  *self, int  level);

#if 0
class environment *environment::Allocate()
{

    static int lastIndex = NUMPERBLOCK; /* Force a block malloc on first call. */

    if (freeList) {
        class environment *temp = freeList;
        freeList = (class environment *) freeList->header.environment_methods;
        return temp;
    }
    if (lastIndex >= NUMPERBLOCK) {
        lastBlock = (class environment *) malloc(BLOCKSIZE);
        lastIndex = 0;
    }
    return &lastBlock[lastIndex++];
}

void environment::Deallocate(class environment  *self)
        {
    self->header.environment_methods = (struct basicobject_methods *) freeList;
    freeList = self;
}
#endif

environment::environment()
        {
    this->type = environment_None;
    this->data.style = NULL;

    THROWONFAILURE( TRUE);
}

environment::~environment()
        {
    switch (this->type)  {
	case environment_View:
	    if(this->data.viewref != NULL)
		(this->data.viewref)->Destroy();
	    break;
    }
}
	
class nestedmark *environment::NewButSimilar()
{
    class environment *sib=(class environment *)(this)->nestedmark::NewButSimilar();
    sib->type=this->type;
    sib->data.style=this->data.style;
    return sib;
}

class environment *environment::Wrap(long  pos, long  length, enum environmenttype  type, union environmentcontents  data)
                    {
    class environment *newenv, *nm1, *nm2, *cp;

    nm1 = (class environment *)(this)->GetInnerMost( pos);
    nm2 = (class environment *)(this)->GetInnerMost( pos + length - 1);
    cp = (class environment *)(nm1)->GetCommonParent( nm2);

    if (cp->type == environment_View) {
        long oldpos, oldlen;
        union environmentcontents olddata;

        /* Hack to prevent non-view envs from get inside view envs; */
        /* removes the view env, adds new env, adds back view env. */

        oldpos = (cp)->Eval();
        oldlen = (cp)->GetLength();
        olddata = cp->data;

        /* Null out cp->data so environment_Delete leaves viewref alone! */
        cp->data.viewref = NULL;
        (cp)->Delete();

        if ((newenv = (class environment *)(this)->Add( pos, length)) != NULL) {
            newenv->type = type;
            newenv->data = data;
            if (type == environment_View) {
                (newenv)->SetStyle( FALSE, FALSE);
                if(newenv->data.viewref) newenv->data.viewref->SetEnvironment(newenv);
            }
        }

        cp = (class environment *)(this)->Add( oldpos, oldlen);
        if (cp != NULL) {
            cp->type = environment_View;
            cp->data = olddata;
            (cp)->SetStyle( FALSE, FALSE);
            if(cp->data.viewref) cp->data.viewref->SetEnvironment(cp);
        }
    } else if ((newenv = (class environment *)(this)->Add( pos, length)) != NULL) {
	newenv->type = type;
	newenv->data = data;
        if (type == environment_View) {
            (newenv)->SetStyle( FALSE, FALSE);
            if(newenv->data.viewref) newenv->data.viewref->SetEnvironment(newenv);
        }
    }
    return newenv;
}

class environment *environment::WrapStyle(long  pos, long  length, class style  *style)
{

    union environmentcontents data;

    data.style = style;
    return (this)->Wrap( pos, length, environment_Style, data);
}

class environment *environment::WrapView(long  pos, long  length, class viewref  *viewref)
{

    union environmentcontents data;

    data.viewref = viewref;
    return (this)->Wrap( pos, length, environment_View, data);
}

class environment *environment::Insert(long  rpos			/* relative position of the environment */, enum environmenttype  type, union environmentcontents  data, boolean  doinsert)
{
    class environment *newenv;
    
    newenv = (class environment *)this->NewFromObject();
    newenv->parent = (class nestedmark *) this;
    newenv->type = type;
    newenv->data = data;
    if (type == environment_View) {
        (newenv)->SetStyle( FALSE, FALSE);
        if(newenv->data.viewref) newenv->data.viewref->SetEnvironment(newenv);
    }
    if (doinsert) {
	if (this->children == NULL)
	    this->children = new tree23int;
	newenv->position = (class tree23int *) (this->children)->Insert( rpos, (long) newenv);
    }
    return newenv;
}

class environment *environment::InsertStyle(long  rpos			/* relative position of the environment */, class style  *style, boolean  doinsert)
{

    union environmentcontents data;

    data.style = style;

    return (this)->Insert( rpos, environment_Style, data, doinsert);
}

class environment *environment::InsertView(long  rpos			/* relative position of the environment */, class viewref  *viewref, boolean  doinsert)
{

    union environmentcontents data;

    data.viewref = viewref;

    return (this)->Insert( rpos, environment_View, data, doinsert);
}


class environment *environment::GetRootEnvironment()
{
    class environment *newenv;
    
    newenv = new environment;
    newenv->length = 2;	/* Probably should be proper setting of includebeginning and ending */
    return newenv;
}

struct removestruct {
    long pos;
    long length;
    enum environmenttype type;
    boolean deleteall;
    boolean anyChange;
};

static long AlterEnvironmentSize(class environment  *self, struct removestruct  *data)
        {
    long pos;
    if (self->type != data->type && data->type != environment_Any)
	return 0;
    
    pos = (self)->Eval();
    if (pos < data->pos + data->length && pos + self->length > data->pos)  {
	data->anyChange = TRUE;
	if (pos < data->pos)  {
	    /* Delete end of the environment */
	    class environment *endPart;

	    if(data->pos + data->length < pos + self->length) (self)->Split( data->pos - pos + data->length);
	    endPart=(class environment *)(self)->Split(data->pos-pos);

	    /* Delete child environments if necessary. */
	    if (data->deleteall && endPart->children != NULL) 
	    (endPart->children)->Enumerate( (tree23int_efptr) AlterEnvironmentSize, (char *) data);

	    (endPart)->Delete();
	}
	else if (pos + self->length > data->pos + data->length)  {
	    class environment *env;
	    /* Delete beginning of the environment */
	    /* Is this split position corect? */
	    (void)(self)->Split( data->pos - pos + data->length);
            if (data->deleteall && self->children != NULL)
                (self->children)->Enumerate( (tree23int_efptr) AlterEnvironmentSize, (char *) data);

            (self)->Delete();
	}
	else  {
	    /* Delete the entire environment */

	    if (data->deleteall && self->children != NULL)
		(self->children)->Enumerate( (tree23int_efptr) AlterEnvironmentSize, (char *) data);
	    (self)->Delete();
	}
    }
    return 0;
}

boolean environment::Remove(long  pos, long  length, enum environmenttype  type, boolean  deleteall)
                    {
    class environment *beginenv, *endenv;
    class environment *parentenv;
    class environment *env, *bottomenv;
    struct removestruct procdata;
    long envpos;

    procdata.anyChange = FALSE;
    beginenv = (class environment *)(this)->GetInnerMost( pos);
    endenv = (class environment *)(this)->GetInnerMost( pos + length - 1);

    parentenv = (class environment *)(beginenv)->GetCommonParent( endenv);

    for (envpos = (parentenv)->Eval(); (pos == envpos || pos + length == envpos + parentenv->length); envpos = (parentenv)->Eval())  {
	if (parentenv->parent == NULL) break;
	parentenv = (class environment *) parentenv->parent;
	if (! deleteall) break;
    }

    if (parentenv->children != NULL)  {
	procdata.pos = pos;
	procdata.length = length;
	procdata.deleteall = deleteall;
	procdata.type = type;
	(parentenv->children)->Enumerate( (tree23int_efptr) AlterEnvironmentSize, (char *) &procdata);
	if(procdata.anyChange && !deleteall)
	    return TRUE;
	if(parentenv->parent!=NULL) {
	    AlterEnvironmentSize(parentenv,&procdata);
	    if(procdata.anyChange && !deleteall)
		return TRUE;
	}
    }

    /* no child envs were modified, so apply to parent instead */

    if(parentenv->parent==NULL)
	return procdata.anyChange;	/* can't plainer top environment */

    bottomenv=parentenv;

    do{
	env=parentenv;
	parentenv=(class environment *)parentenv->parent;
    }while(parentenv->parent!=NULL && deleteall);


    procdata.pos = pos;
    procdata.length = length;
    procdata.deleteall = deleteall;
    procdata.type = type;
    AlterEnvironmentSize(env, &procdata);

    return procdata.anyChange;
}

void environment__Dump(class environment  *self, int  level)
        {
    class nestedmark *nself = (class nestedmark *) self;
    register int i = level;

    while (i-- > 0)
	printf("    ");
    printf("Env %x (%x ^) position: %d length: %d ib: %c ie: %c type: %d value %d\n", nself, nself->parent, (self)->Eval(), nself->length, (nself->includeBeginning) ? 'T' : 'F', (nself->includeEnding) ? 'T' : 'F', self->type, self->data.style);
    if (nself->children != NULL)
        (nself->children)->Enumerate( (tree23int_efptr) environment__Dump, (char *) level + 1);
}
