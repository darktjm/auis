#ifndef _ctext_H_
#define _ctext_H_
/** \addtogroup libsrctext
 * @{ */
/* File ctext.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   ctext: a text subclass specialized for dealing with C code. */

/* kinds of styles that are used in hash table */
#define KEYWRD 1     /* keywords */
#define PREPRC 2     /* preprocessor directives */

#include "srctext.H"
class ctext: public srctext {
  public:
    TRACED_CLASS_NODEST(ctext);

    int Indentation(long pos);
    boolean IsTokenChar(char ch);
    boolean Quoted(long pos);
    void RedoStyles();
    void SetAttributes(struct attributes *atts);
    void SetupStyles();

    virtual long BackwardSkipDelimited(long pos, char begin, char end);
    virtual long BackwardSkipJunk(long pos);
    virtual long CheckPreproc(long start);
    virtual long MaybeBackwardSkipCppLines(long pos);
    virtual long UnstyledCommentStart(long pos);

    static boolean InitializeClass();
    ctext();

    /* Variables used to control the indenting style. */
    int braceIndent, switchLabelUndent, switchLevelIndent;
    boolean outdentPreproc;
};
/** @} */
#endif /* _ctext_H_ */
