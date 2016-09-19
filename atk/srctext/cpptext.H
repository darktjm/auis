#ifndef _cpptext_H_
#define _cpptext_H_
/* File cpptext.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   cpptext: a text subclass specialized for dealing with C++ code. */

/* kinds of styles that are used in hash table */
/*ctext KEYWRD 1     / * keywords */
/*ctext PREPRC 2     / * preprocessor directives */
#define CLASS  3     /* class definition */

/* bit values used for indentation identification */
#define	CLASS_BIT	1   /* bit 0 */
#define	ACSCTRL_BIT	2   /* bit 1 */
#define TEMPLATE_BIT	4   /* bit 2 */
/*note: CLASS_BIT keywords do NOT get wrapped with kindStyle[CLASS], and ACSCTRL_BIT isn't used anyway, because ctext and cpptext still use the old match() method*/

#include "ctext.H"
class cpptext: public ctext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void BackwardCheckLabel(long pos);
    void BreakLine(class mark *endofline);
    long CheckWord(long i, long end);
    int Indentation(long pos);
    void RedoStyles();
    boolean ReflowComment(long pos);
    void SetAttributes(struct attributes *atts);
    void SetupStyles();

    static boolean InitializeClass();
    cpptext();

    boolean inClassDef;
    int acsctrlOutdent;
};

#endif /* _cpptext_H_ */
