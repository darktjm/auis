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

#include <andrewos.h>
ATK_IMPL("nestedmark.H")
#include <nestedmark.H>
#include <tree23int.H>

static boolean GlobalIsolation = FALSE;

/* These statics are used for generating the next change in the nested marks.
Set in GetInnerMost and used in GetNextChange.
 */
static class nestedmark *lastSelf;	
static long lastPos;
static long ncrlength;


ATKdefineRegistry(nestedmark, ATK, NULL);
static void DoFreeTree(class nestedmark  *self);
static void FilterProc(class nestedmark  *self, struct filterstruct  *data, class tree23int  *t, class tree23int  *which  );
class nestedmark *splitOffRight(class nestedmark  *self,int  rpos);


nestedmark::~nestedmark() {
}

nestedmark::nestedmark()
{
    this->children = NULL;
    this->position = NULL;
    this->length = 999999999;
    this->parent = NULL;
    this->includeEnding = TRUE;
    this->includeBeginning = FALSE;

    THROWONFAILURE( TRUE);
}

class nestedmark *nestedmark::NewButSimilar()
{
    class nestedmark *sib=(class nestedmark *)this->NewFromObject();
    sib->includeBeginning=this->includeBeginning;
    sib->includeEnding=this->includeEnding;
    return sib;
}

static void DoFreeTree(class nestedmark  *self)
{
    (self)->FreeTree();
}

void nestedmark::FreeTree()
{
    if (this->children)  {
	(this->children)->Apply( (tree23int_applyfptr) DoFreeTree);
	(this->children)->Free();
    }
    delete this;
}
/* owns for communication with filter proc
 */
struct filterstruct {
    class tree23int *fptree;
    class nestedmark *fpparent;
};

static void FilterProc(class nestedmark  *self, struct filterstruct  *data, class tree23int  *t, class tree23int  *which  )
{
    self->position = t;
    if (which == data->fptree)
	self->parent = data->fpparent;
}

class nestedmark *splitOffRight(class nestedmark  *self,int  rpos)
{
    class tree23int *node;
    class nestedmark *right=(self)->NewButSimilar();

    if(self->children!=NULL)
	node=(self->children)->GetLeftMostNode();
    else
	node=NULL;

    if(node!=NULL)
	right->children=new tree23int;

    while(node!=NULL){
	class nestedmark *child=((class nestedmark *)node->data);
	class tree23int *nextnode = (self->children)->GetNextNode(node);
	int pos=(node)->Eval(),
	    len=child->length;

	if(pos+len>rpos)
	    if(pos >= rpos){
		(node)->Delete();
		child->position= (right->children)->Insert( pos-rpos, (long) child);
		delete node;
		child->parent=right;
	    }else{
		class nestedmark *halfchild=
		  splitOffRight(child,rpos-pos);
		child->length=rpos-pos;
		halfchild->length=len-child->length;
		halfchild->position= (right->children)->Insert( 0, (long) halfchild);
		halfchild->parent=right;
	    }		
	node=nextnode;
    }

    right->length=self->length-rpos;
    self->length=rpos;

    return right;
}

class nestedmark *nestedmark::Split(long  rpos)
{
    class nestedmark *right=splitOffRight(this,rpos), *parent=this->parent;

    if(parent!=NULL){
	int prpos=(this->position)->Eval();
	right->position=
	  (parent->children)->Insert( prpos+rpos, (long) right);
	right->parent=parent;
    }

    return right;
}

class nestedmark *nestedmark::Add(long  pos, long  length)
{
    register class nestedmark *cp;
    register class nestedmark *nm1;
    register class nestedmark *nm2;
    long eorg;
    long rpos;
    class tree23int *tt1;
    class tree23int *tt2;
    struct filterstruct procdata;
    class nestedmark *newnm;

    if (length <= 0) return FALSE;

    nm1 = (this)->GetInnerMost( pos);
    nm2 = (this)->GetInnerMost( pos+length-1);
    cp = (nm1)->GetCommonParent(nm2);
    eorg = (cp)->Eval();
    rpos = pos - eorg;	/* store position relative to start of enviro */

    if(nm1!=cp){
	int crpos;
	while(nm1->parent!=cp)
	    nm1=nm1->parent;
	crpos=(nm1->position)->Eval();
	if(rpos>crpos)
	    (void)(nm1)->Split(rpos-crpos);
    }

    if(nm2!=cp){
	int crpos;
	while(nm2->parent!=cp)
	    nm2=nm2->parent;
	crpos=(nm2->position)->Eval();
	if(rpos+length<crpos+nm2->length)
	    (void)(nm2)->Split(rpos+length-crpos);
    }

    newnm = (class nestedmark *)this->NewFromObject();
    newnm->parent = cp;
    tt1 = new tree23int;
    tt2 = new tree23int;
    procdata.fptree=tt2;
    procdata.fpparent=newnm;
    if (cp->children == NULL)
	cp->children = new tree23int;
    (cp->children)->Filter( 0, tt1, tt2, rpos, rpos+length, (tree23int_filterfptr) FilterProc, (char *) &procdata);
    (cp->children)->Free();
    cp->children = tt1;

/*     optimization: if tt2 is empty, put a null in here and ttt_free(tt2)

 */    newnm->children = tt2;

/*     now tt2 is too high by rpos, so decrease it

 */    (tt2)->Update(-rpos,-rpos);
    newnm->position = (class tree23int *) (tt1)->Insert( rpos, (long) newnm);
    (newnm)->SetLength(length);

    return newnm;
}

void nestedmark::Delete()
{
    class nestedmark *pp;
    int relleft;
    struct filterstruct procdata;

    relleft = (this->position)->Eval();
    pp = this->parent;
    if (pp == NULL) return;	/* failed */
    (this->position)->Delete();	/* delete from parent's 23 tree */
    (this->position)->Free();
    if (this->children != NULL)  {
	procdata.fptree = pp->children;		/* not null, since e is a child */
	procdata.fpparent = pp;
	(this->children)->Merge( pp->children, relleft, (tree23int_mergefptr) FilterProc, (char *) &procdata);
	(this->children)->Free();
    }
    delete this;
}

void nestedmark::Update(long  pos, long  length)
{
    register class nestedmark *up, *tp;
    long tpos, tsize;
    boolean includebeginning;
    boolean includeending;
    long rpos = 0;
    long len;
    boolean ib;
    int offset = 0;

    tp = (this)->GetEnclosing(pos);
    if (tp == NULL) tp = this;
    includebeginning = ( ! GlobalIsolation) && tp->includeBeginning;
    includeending = ( ! GlobalIsolation) && tp->includeEnding;

    /* fix up the lengths  */

    if (length > 0)  {
        while (tp != NULL)  {
            tpos = (tp)->Eval();
            ib = FALSE;
            if (tp->parent == NULL
                    || (pos > tpos && pos < tpos+tp->length) 
                    || (ib = (includebeginning && pos == tpos)) 
                    || (includeending && pos == tpos + tp->length))  {
                tp->length += length;
                if (tp->children != NULL)
                    (tp->children)->Update(pos - tpos + offset,length);
            }
            if (tp->position != NULL)
                rpos = tp->position->bump;
            len = tp->length;
            tp = tp->parent;
            if (tp != NULL)  {
                offset = (ib) ? 1 : 0;
                includebeginning = ( ! GlobalIsolation) 
                        && (tp->includeBeginning || ((rpos == 0) ? includebeginning : FALSE));
                includeending = ( ! GlobalIsolation) 
                        && (tp->includeEnding 
                                || ((rpos + len == tp->length) ? includeending : FALSE));
            }
        }
    }
    else  {
        while (length < 0)  {
            up = (this)->GetInnerMost(pos);
	    tsize = -ncrlength;
	    if (tsize < length) tsize = length;
	    if (up == NULL) up = this;
	    while (up)  {
		tpos = (up)->Eval();
		if (pos >= tpos && pos < tpos+up->length)
		    if (pos-tsize > tpos + up->length)
			up->length = pos-tpos;
		    else up->length += tsize;
		if (up->length == 0)  {
		    tp = up;
                    up = up->parent;
                    if (up != NULL)
			(tp)->Delete();
		}
		else  {
		    if (up->children != NULL)
			(up->children)->Update(pos-tpos,tsize);
		    up = up->parent;
		}
	    }
	    length -= tsize;
	}
    }
}

class nestedmark *nestedmark::GetInnerMost(long  pos  )
{
    register class nestedmark *tp;
    long  eleft;

/*     first check if it is within our range
 */
    if (this == NULL) return this;
    if (this->parent == 0 && this->length<999999999) this->length = 999999999;
    ncrlength = this->length-pos;
    if (this->children)  {
	tp = (class nestedmark *) (this->children)->FindL(0,pos);	/* look for the sub-environment */
	if (tp != NULL)  {
	    eleft = (tp->position)->Eval();
	    if (pos >= eleft && pos < eleft+tp->length)  {
		tp = (tp)->GetInnerMost(pos-eleft);
		lastSelf = this;
		lastPos = pos;
		return tp;
	    }
	}
	tp = (class nestedmark *) (this->children)->FindR(0,pos);
	if (tp != NULL)
	    ncrlength = (tp->position)->Eval()-pos;
    }
    lastSelf = this;
    lastPos = pos;
    return this;
}

class nestedmark *nestedmark::GetEnclosing(long  pos  )
{
    register class nestedmark *tp;
    long  eleft;

/*     first check if it is within our range
 */
    if (this == NULL) return this;
    if (this->parent == 0 && this->length<999999999) this->length = 999999999;
    ncrlength = this->length-pos;
    if (this->children != NULL)  {
	tp = (class nestedmark *) (this->children)->FindL(0,pos);	/* look for the sub-environment */
	if (tp != NULL)  {
	    eleft = (tp->position)->Eval();
	    if (eleft == pos && pos > 0) {
		/* look for style that ends at same place that tp begins */

		class nestedmark *ptp = (class nestedmark *) (this->children)->FindL(0,pos - 1);
		long peleft;

/* Changed to take Global Isolation into account  -rr2b */
		if (ptp != NULL && ((peleft = (ptp->position)->Eval()) + ptp->length) == pos && ptp->includeEnding && (!GlobalIsolation)) {
		    tp = (ptp)->GetEnclosing(pos-peleft);
		    lastSelf = this;
		    lastPos = pos;
		    return tp;
		}
	    }

/* Changed to take Global Isolation into account -rr2b */
	    if ((pos > eleft && pos < eleft+tp->length) || (pos == eleft && tp->includeBeginning && (!GlobalIsolation)) || (pos == eleft + tp->length && tp->includeEnding && (!GlobalIsolation)))  {
		tp = (tp)->GetEnclosing(pos-eleft);
		lastSelf = this;
		lastPos = pos;
		return tp;
	    }
	}
	tp = (class nestedmark *) (this->children)->FindR(0,pos);
	if (tp != NULL)
	    ncrlength = (tp->position)->Eval()-pos;
    }
    lastSelf = this;
    lastPos = pos;
    return this;
}

long nestedmark::Eval()
{
    register int i;
    class nestedmark *self=this;
    i=0;
    while (self != 0)
	{
	if (self->position) i += (self->position)->Eval();
	else return i;
	self = self->parent;
	}
    return i;
}

class nestedmark *nestedmark::GetCommonParent(class nestedmark  *nmark  )
{
    register class nestedmark *tp;
    register class nestedmark *up;

    tp = this;
    while (tp != NULL)
	{
	up = nmark;
	while (up != NULL)  {
	    if (up == tp) return tp;
	    up = up->parent;
	}
	tp = tp->parent;
    }
    return NULL;	/* no common parent */
}

void nestedmark::SetLength(long  length  )
{
    this->length = length;
}

long nestedmark::GetNextChange(long  pos  )
{
    if (this != lastSelf || pos != lastPos)
	(this)->GetInnerMost( pos);
    return ncrlength;
}

/* This routine returns the distance from self to namrk.  If it is positive then
nmark is above self.  If it is negative then self is above nmark.  
If it is 0 then they are the same node and if it is nestedmark_UNRELATED
then they are not directly related. */

long nestedmark::Distance(class nestedmark  *nmark)
{
    register int i;
    register class nestedmark *tmark = this;
    
    for (i = 0, tmark = this; tmark && tmark != nmark; i++, tmark = tmark->parent);
    if (tmark) return i;
    for (i = 0, tmark = nmark; tmark && tmark != this; i++, tmark = tmark->parent);
    if (tmark) return -i;
    return nestedmark_UNRELATED;
}

void nestedmark::SetStyle(boolean  includebeginning, boolean  includeending)
            {
    this->includeBeginning = includebeginning;
    this->includeEnding = includeending;
}

class nestedmark *nestedmark::GetChild(long  pos)
        {
    class nestedmark *child;
    
    if (this->children != NULL) {
	child = (class nestedmark *) (this->children)->FindL( 0, pos - (this)->Eval());
	if (child != NULL) {
	    long childEndPos = (child)->Eval() + (child)->GetLength();
	    if (childEndPos > pos) {
		return child;
	    }
	}
    }

    return NULL;
}

class nestedmark *nestedmark::GetPreviousChild(class nestedmark  *nm, long  pos)
            {
    class tree23int *tp;

    if (this->children == NULL)
	return NULL;
    
    if (nm == NULL || nm->parent != this)
	nm = (this)->GetChild( pos);

    if (nm == NULL) {
	nm = (class nestedmark *) (this->children)->FindL( 0, pos - (this)->Eval());
	if (nm != NULL) {
	    if ((nm)->Eval() + (nm)->GetLength() <= pos) {
		return nm;
	    }
	}
	else {
	    return NULL;
	}
    }

    /* at this time nm is non-NULL */

    tp = (this->children)->GetPreviousNode( nm->position);
    if (tp != NULL) {
	return (class nestedmark *) (tp)->GetData();
    }
    else {
	return NULL;
    }
}

class nestedmark *nestedmark::GetNextChild(class nestedmark  *nm, long  pos)
            {
    class tree23int *tp;
    
    if (this->children == NULL)
	return NULL;
    
    if (nm == NULL || nm->parent != this)
	nm = (this)->GetChild( pos);

    if (nm == NULL) {
	return (class nestedmark *) (this->children)->FindR( 0, pos - (this)->Eval());
    }
    else {
	tp = (this->children)->GetNextNode( nm->position);
	if (tp != NULL) {
	    return (class nestedmark *) (tp)->GetData();
	}
	else {
	    return NULL;
	}
    }
}

long nestedmark::NumberOfChildren()
    {
    if (this->children != 0)
	return (this->children)->NumberOfLeaves();
    return 0;
}

	boolean
nestedmark::SetGlobalIsolation(boolean  dontextend)
		{
	boolean Old = GlobalIsolation;
	GlobalIsolation = dontextend;
	return Old;
}
