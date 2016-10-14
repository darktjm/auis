#ifndef _ptext_H_
#define _ptext_H_
/** \addtogroup libsrctext
 * @{ */
/* File ptext.H
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   ptext: a text subclass specialized for dealing with Pascal code. */

#define KEYWRD 1     /* keywords */

#define idstyle_PLAIN   0
#define idstyle_UPPER   1
#define idstyle_LOWER   2
#define idstyle_CAPIT   3

#include "srctext.H"
class ptext: public srctext {
  public:
    TRACED_CLASS_NODEST(ptext);

    void SetAttributes(struct attributes *atts);
    void SetupStyles();
    void RedoStyles();
    int Indentation(long pos);
    void BackwardCheckWord(long pos, long first);
    char *Keywordify(char *buff, boolean checkforceupper);
    boolean IsTokenChar(char ch);

    static boolean InitializeClass();
    ptext();

    /* Variables used to control the indenting style. */
    int caseLevelIndent, slashAsteriskComments, idStyle;
};
/** @} */
#endif /* _ptext_H_ */
