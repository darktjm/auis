/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("tree23int.H")
#include <tree23int.H>

/* Crank out structs in 4k blocks. */
#define DESIREDBLOCKSIZE 4096
/* Number of marks per block */
#define NUMPERBLOCK DESIREDBLOCKSIZE / sizeof(class tree23int)
#define BLOCKSIZE NUMPERBLOCK * sizeof(class tree23int)

ATKdefineRegistryNoInit(tree23int, ATK);

#if 0
static class tree23int *freeList = NULL;
static class tree23int *lastBlock = NULL;

class tree23int *tree23int::Allocate()
{

    static int lastIndex = NUMPERBLOCK; /* Force a block malloc on first call. */

    if (freeList) {
        class tree23int *temp = freeList;
        freeList = (class tree23int *) freeList->header.tree23int_methods;
        return temp;
    }
    if (lastIndex >= NUMPERBLOCK) {
        lastBlock = (class tree23int *) malloc(BLOCKSIZE);
        lastIndex = 0;
    }
    return &lastBlock[lastIndex++];
}

void tree23int::Deallocate(class tree23int  *self)
    {
    self->header.tree23int_methods = (struct basicobject_methods *) freeList;
    freeList = self;
}
#endif

tree23int::tree23int()
{
    this->leaf = FALSE;
    this->bump = 0;
    this->data = 0;
    this->parent = NULL;
    this->nKids = 0;
    THROWONFAILURE( TRUE);
}
class tree23int *tree23int::Delete()
{
    class tree23int *parent = this->parent;

    if (parent == NULL) return  this;
    return (parent)->Remove( this);
}
long tree23int::Eval()
{
    int i;
    class tree23int *self=this;
    for (i = 0; self != NULL; i+= self->bump, self =  self->parent);
    return i;
}
class tree23int *tree23int::Insert(long  key, long  data  )
{
    class tree23int *newnode;
    
    newnode = new tree23int;
    newnode->leaf = TRUE;
    newnode->data = data;
    newnode->bump = key;
    (this)->AddIn( 0, newnode);
    return newnode;
}

void tree23int::AddIn(long  offset, class tree23int  *newnode  )
{
    long value;
    long i;

    if(this->leaf) {
    	(this->parent)->Jam( newnode);
	return;
	}

    offset += this->bump;
    newnode->bump -= this->bump;
    value = newnode->bump;
/*     leaf already checked above
 */    if (this->nKids >= 3 && this->kid[2]->bump <= value)  { 
	(this->kid[2])->AddIn(offset,newnode);
	return;
    }
    if (this->nKids >= 2 && this->kid[1]->bump <= value)  {
	(this->kid[1])->AddIn(offset,newnode);
	return;
    }
    if (this->nKids >= 1 && this->kid[0]->bump <= value)  {
	(this->kid[0])->AddIn(offset,newnode);
	return;
    }

/*     If we make it this far, there's no real home for the new item->
    We either add it as the new leftmost child (if there's room for 1 more)
    or we add it to the old leftmost child, which may result in a split
    further down the line.
 */   
    if (this->nKids < 3)  {
/* 	there's room in this node
	use slot 0; we're adding this as the leftmost child
 */	i = this->nKids;
	while (i>0)  {
	    this->kid[i] = this->kid[i-1];
	    i--;
	}
	this->kid[0] =  newnode;
	newnode->parent = this;
	this->nKids++;
	(this)->Twiddle( newnode);
    }
    else  {
	/* otherwise we add it to the bottom */
	(this->kid[0])->AddIn(offset,newnode);
    }
}

void tree23int::Apply(tree23int_applyfptr  proc  )
{
    if(this->leaf){ 
	(*proc)(this->data); 
	return;
    }
    if (this->nKids >= 1) (this->kid[0])->Apply(proc);
    if (this->nKids >= 2) (this->kid[1])->Apply(proc);
    if (this->nKids >= 3) (this->kid[2])->Apply(proc);
}

void tree23int::Filter(long  offset, class tree23int  *left, class tree23int  *right, long  lowval, long  highval, tree23int_filterfptr  proc, char  *procdata)
  {
    offset += this->bump;

    if(this->leaf ){
	class tree23int *newnode;
    	if (offset >= lowval && offset < highval)  {
		newnode = (right)->Insert( offset, this->data);
		(*proc)(this->data, procdata, newnode, right);
    	}
    	else  {
		newnode = (left)->Insert( offset, this->data);
		(*proc)(this->data, procdata, newnode, left);
    	}
    	return;
    }
    if (this->nKids >= 1) (this->kid[0])->Filter(offset,left,right,lowval,highval, proc, procdata);
    if (this->nKids >= 2) (this->kid[1])->Filter(offset,left,right,lowval,highval, proc, procdata);
    if (this->nKids >= 3) (this->kid[2])->Filter(offset,left,right,lowval,highval, proc, procdata);
}

long tree23int::FindL(long  offset, long  key  )
{
    int nKids;
    class tree23int *self=this;
    if(self->leaf ) return self->data;
    while (self != NULL)  {
	if (self->leaf) return self->data;
	offset += self->bump;
	if ((nKids=self->nKids) >= 3 && offset+self->kid[2]->bump <= key) 
	    self = self->kid[2];
	else if (nKids >= 2 && offset+self->kid[1]->bump <= key) 
	    self = self->kid[1];
	else if (nKids >= 1 && offset+self->kid[0]->bump <= key) 
	    self = self->kid[0];
	else return 0;
    }
    return 0;		/* there's no other possibility */
}

long tree23int::FindR(long  offset, long  key  )
{
    class tree23int *lastright;
    int lastrightx = 0;
    class tree23int *self=this;
    if(self->leaf ) return 0;

    lastright = NULL;
    while (self != NULL)  {
	offset += self->bump;
	if (self->leaf) break;
	if (self->nKids >= 3 && offset+self->kid[2]->bump <= key)
	    self = self->kid[2];
	else if (self->nKids >= 2 && offset+self->kid[1]->bump <= key)  {
	    if (self->nKids >= 3)  {
		lastright = self;
		lastrightx = 2;
	    }
	    self = self->kid[1];
	}
	else if (self->nKids >= 1 && offset+self->kid[0]->bump <= key)  {
	    if (self->nKids >= 2)  {
		lastright = self;
		lastrightx = 1;
	    }
	    self = self->kid[0];
	}
	else if (self->nKids >= 1)  {
	    lastright = self;
	    lastrightx = 0;
	    break;
	}
	else break;
    }
    if (lastright != NULL)
	return (lastright->kid[lastrightx])->FindL( 0, lastright->kid[lastrightx]->bump);
    return 0;
}

void tree23int::Free()
{
    if (this->nKids >= 1) (this->kid[0])->Free();
    if (this->nKids >= 2) (this->kid[1])->Free();
    if (this->nKids >= 3) (this->kid[2])->Free();
    delete this;
}

void tree23int::Merge(class tree23int  *ancestor, long  offset, tree23int_mergefptr  proc, char  *procdata)
{
    offset = offset+this->bump;
    /*     recurse down the tree, adding stuff to the root */
    if(this->leaf) {
	class tree23int *tp;
	tp = (ancestor)->Insert(offset,this->data);
	(*proc)(this->data,procdata,tp,ancestor);
	return;
    }
    if (this->nKids >= 1)
	(this->kid[0])->Merge( ancestor, offset, proc, procdata);
    if (this->nKids >= 2)
	(this->kid[1])->Merge( ancestor, offset, proc, procdata);
    if (this->nKids >= 3)
	(this->kid[2])->Merge( ancestor, offset, proc, procdata);
}

void tree23int::Update(long  pos, long  size  )
{
    int i;
    class tree23int *tp;
    boolean flag;

    pos -= this->bump;
    if (size > 0)  {
/* 	inserting chars
 */
	if (pos <= 0)  {
	    this->bump += size;
	    if (this->parent != NULL)
		(this->parent)->Twiddle( this);
	    return;
	}
	if(this->leaf) return;
/* 	otherwise handle things recursively
 */	i = this->nKids-1;	/* right to left to avoid interaction with twiddle */
	flag = FALSE;
	while (i >= 0)  {
	    tp = this->kid[i];
	    if (pos >= tp->bump) flag = TRUE;
/* 	    only i==0 call can change tp->bump, and we've already checked it.
 */	    (tp)->Update(pos,size);
	    if (flag) return;
	    i--;
	}
    }
    else {
/* 	deleting characters, size is negative
 */
	if (pos < 0)  {
/* 	    delete the stuff to the left
 */	    if (pos > size) i = pos;
	    else i = size;
	    this->bump +=  i;
	    if (this->parent != NULL)
		(this->parent)->Twiddle( this);
	    if(this->leaf) return;
	    pos -= i;
	    size -= i;
	}
	else	if(this->leaf) return;
	if (size == 0) return;
	i = this->nKids - 1;
	flag = FALSE;
	while (i >= 0)  {
	    tp = this->kid[i];
	    if (pos >= tp->bump) flag = TRUE;
/* 	    once again, only i==0 call can change tp->bump
 */	    (tp)->Update(pos,size);
	    if (flag) return;
	    i--;
	}
    }
}

void tree23int::Jam(class tree23int  *newnode  )
{
    class tree23int *tp, *up;
    class tree23int *nroot;
    int i;
    int slot, value;
/*     called to jam a new leaf node under a parent (self)
 */
    value = newnode->bump;
/*     there are, as usual, two choices: 3 nodes or less than three
 */    if (this->nKids >= 3 && value > this->kid[2]->bump) slot = 3;
    else if (this->nKids >= 2 && value > this->kid[1]->bump) slot = 2;
    else if (this->nKids >= 1 && value > this->kid[0]->bump) slot = 1;
    else slot = 0;
    if (this->nKids < 3) {
/* 	add the new node into the right spot.  If this is slot 0, be sure
	to call tree23int_Twiddle when you're done
 */	i = this->nKids;
	while (i>slot)  {
	    this->kid[i] = this->kid[i-1];
	    i--;
	}
	this->kid[slot] =  newnode;
	this->nKids += 1;
	newnode->parent = this;
	if (slot == 0) (newnode->parent)->Twiddle( newnode);
    }
    else  {
/* 	otherwise we do a split
 */	tp = new tree23int;
	up = new tree23int;
	tp->bump = this->bump;
	up->bump = this->bump;
	tp->parent = NULL;	/* just for a little while */
	up->parent = NULL;
	tp->nKids = 2;
	up->nKids = 2;

/* 	put the value in the tree
 */	if (slot > 0) tp->kid[0] = this->kid[0];
	if (slot > 1) tp->kid[1] = this->kid[1];
	else tp->kid[1] = this->kid[0];
	if (slot > 2) up->kid[0] = this->kid[2];
	else up->kid[0] = this->kid[1];
	up->kid[1] = this->kid[2];
	if (slot == 0) tp->kid[0] = newnode;
	else if (slot == 1) tp->kid[1] = newnode;
	else if (slot == 2) up->kid[0] =  newnode;
	else up->kid[1] =  newnode;

	tp->kid[0]->parent = tp;
	tp->kid[1]->parent = tp;
	up->kid[0]->parent = up;
	up->kid[1]->parent = up;
	(tp->kid[0]->parent)->Twiddle( tp->kid[0]);	/* note that tp and up ->parent is 0 */
	(up->kid[0]->parent)->Twiddle( up->kid[0]);

	if ((nroot = this->parent) != NULL)
	    value = (nroot)->Eval();	/* where is self, originally */
	else
	    value = 0;
	nroot = (this)->Delete();		/* delete the self from its parent */
	if (nroot == this)  {
/* 	    create a new self  */
	    nroot->bump = 0;
	    nroot->parent = NULL;
	    nroot->nKids = 0;
	}
	else delete this;
	tp->bump += value - (nroot)->Eval();
	(nroot)->Jam(tp);		/* note this can't cause a split, but can change nroot->bump */
	up->bump +=value - (nroot)->Eval();
	(nroot)->Jam(up);		/* this one *CAN* cause a split */
	(tp->parent)->Twiddle( tp);
    }
}

class tree23int *tree23int::Remove(class tree23int  *child  )
{
    int i;
    int j;
    class tree23int *parent;
    class tree23int *newParent;
    
    i=0;
    while (i < this->nKids)  {
	if (this->kid[i] ==  child)  {
/* 	    found the dude to zap
 */	    j = --(this->nKids);
	    while (i < j) {
		this->kid[i]=this->kid[i+1];
		i++;
	    }
	    if (this->nKids > 0)
		(this->kid[0]->parent)->Twiddle( this->kid[0]);
	    else if ((parent = this->parent) != NULL)  {
/* 		if no parent, then we would delete a root of a 2/3 tree,
		    which is probably pointed to by other stuff.  We must delete zero-child
		    nodes, otherwise FindR might attempt to go down one of these null
		    paths in order to find the dude just to the right of the value it is given.
		    This results in an incorrect return value of 0 from FindR.
 */		
		newParent = (parent)->Remove( this);
		delete this;
		return newParent;
	    }
	    return this;
	}
	i++;
    }
    return this;
}

void tree23int::Twiddle(class tree23int  *child  )
{
    long t;
    class tree23int *self=this;
    while (child->bump != 0 && self != NULL && child ==  self->kid[0])  {
	t = child->bump;
	self->bump += t;
	child->bump = 0;
	if (self->nKids >= 2) self->kid[1]->bump -= t;
	if (self->nKids >= 3) self->kid[2]->bump -= t;
	child = self;
	self = self->parent;
    }
}

class tree23int *tree23int::GetLeftMostNode()
{
    class tree23int *self=this;
    while (! self->leaf)  {
	if (self->nKids == 0) return NULL;
	self = self->kid[0];
    }
    return  self;
}

class tree23int *tree23int::GetNextNode(class tree23int  *node)
        {
    class tree23int *parent;
    
    while (node != this)  {
	parent = node->parent;
	if ( node == parent->kid[0])  {
	    if (parent->nKids >= 2)
		return (parent->kid[1])->GetLeftMostNode();
	}
	else if (parent->nKids == 3 &&  node == parent->kid[1])  {
	    return (parent->kid[2])->GetLeftMostNode();
	}
	node =  parent;
    }
    return NULL;
}

class tree23int *tree23int::GetRightMostNode()
{
    class tree23int *self=this;
    while (! self->leaf)  {
	if (self->nKids == 0) return NULL;
	self = self->kid[self->nKids - 1];
    }
    return  self;
}

class tree23int *tree23int::GetPreviousNode(class tree23int  *node)
        {
    class tree23int *parent;
    
    while (node != this)  {
	parent = node->parent;
	if ( node == parent->kid[0])  {
	    node = parent;
	}
	else if (node == parent->kid[1]) {
	    return (parent->kid[0])->GetRightMostNode();
	}
	else if (node == parent->kid[2]) {
	    return (parent->kid[1])->GetRightMostNode();
	}
    }

    return NULL;
}

long tree23int::Enumerate(tree23int_efptr  proc, char  *procdata)
            {
    class tree23int *node;
    class tree23int *nextnode;
    if(this->leaf) {
    	if ((*proc)(this->data, procdata)) return this->data;
    	return 0;
    }
    node = (this)->GetLeftMostNode();
    while (node != NULL)  {
	nextnode = (this)->GetNextNode(node);
	if ((*proc)(node->data, procdata)) return node->data;
	node = nextnode;
    }
    return 0;
}

long tree23int::NumberOfLeaves()
    {
    long count = 0;
    if(this->leaf) return 1;
    if (this->nKids > 0)
	count += (this->kid[0])->NumberOfLeaves();
    if (this->nKids > 1)
	count += (this->kid[1])->NumberOfLeaves();
    if (this->nKids > 2)
	count += (this->kid[2])->NumberOfLeaves();
    return count;
}
void tree23int::Dump(long  offset  )
{
    int i;

    printf("%8p (%8p^): ",this,this->parent);
    if(this->leaf) {
    	printf("(LEAF %lx %lx)\n",this->bump+offset, this->data);
	return;
    }
    printf("(INT (%d) %lx)\n",this->nKids,this->bump+offset);
    i = 0;
    while (i<this->nKids)  {
	(this->kid[i])->Dump(this->bump+offset);
	i++;
    }
    printf("End of %p\n",this);
}


