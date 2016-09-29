#ifndef _nestedmark_H_
#define _nestedmark_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupport
 * @{ */
#define nestedmark_VERSION 1

/* FreeTree: recursively frees the tree starting at self.
Add: adds in a mark in the nested mark tree headed by self. Returns new mark if completed.
Delete: removes the nested mark from the tree it is in.
Update: change the position of the marks where length items are
	added/delete.
GetInnerMost: returns the innermost nested mark surrounding the item at pos.
GetEnclosing: returns the innermost nested mark paying attention to the includebeginning and includeending flags.
Eval: returns the position assocated with the nested mark.
GetCommonParent: finds the common parent in the tree of self and nmark.
SetLength: sets the length of the nested mark.
GetNextChange: returns the next change fof the mark passed in by self and the pos.  If they are the same values as the last call to GetInnerMost then the value computed by GetInnerMost is used.
Distance: returns the distance from self to nmark.  If nmark is above self the value returned is positive - If self is above nmark the value returned is negative.  If they are equal the value returned is zero and if they are not directly related the value returned is nestedmark_UNRELATED.
SetStyle: Sets the includebegiing and includeending flags.
NewButSimilar: returns a new mark with the include{Ending,Beginning} fields copied from the old one.  This allows subclasses (i.e., environment) to copy relevant fields when we want a "copy" of a mark.
Split: Splits the mark given (and any children) in  two at the given point (relative the beginning of the mark), and returns the right half (the left half is the old mark, modified).
SetGlobalIsolation: If argument is True, Updates will behave as though all individual styles are False.  Returns prior value of GlobalStyle.
 */
#include <ATK.H>

class nestedmark : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    inline long GetLength() { return (this->length); }
    inline class nestedmark * GetParent() { return (this->parent); }

    virtual void FreeTree();
    virtual class nestedmark * Add(long pos, long length)  ;
    virtual void Delete();
    virtual void Update(long pos, long length);
    virtual class nestedmark * GetInnerMost(long pos)  ;
    virtual class nestedmark * GetEnclosing(long pos)  ;
    virtual long Eval()  ;
    virtual class nestedmark * GetCommonParent(class nestedmark *nmark)  ;
    virtual void SetLength(long length);
    virtual long GetNextChange(long pos)  ;
    virtual long Distance(class nestedmark *nmark)  ;
    virtual void SetStyle(boolean includebeginning, boolean includeending);
    virtual class nestedmark * GetChild(long position)  ;
    virtual class nestedmark * GetNextChild(class nestedmark *current, long pos)  ;
    virtual class nestedmark * GetPreviousChild(class nestedmark *current, long pos)  ;
    virtual long NumberOfChildren()  ;
    virtual class nestedmark * NewButSimilar()  ;
    virtual class nestedmark * Split(long rpos)  ;

    nestedmark();
    virtual ~nestedmark();
	static boolean SetGlobalIsolation(boolean dontextend)  ;

    class tree23int *children;		/* The children of this mark */
    class tree23int *position;		/* This mark in the superior 23-tree */
    long length;			/* Length of this mark */
    class nestedmark *parent;		/* enclosing mark */
    boolean includeEnding;
    boolean includeBeginning;
};

#define nestedmark_UNRELATED -999999999
/** @} */
#endif /* _nestedmark_H_ */
