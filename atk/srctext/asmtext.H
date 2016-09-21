#ifndef _asmtext_H_
#define _asmtext_H_
/** \addtogroup libsrctext
 * @{ */
/* File asmtext.H created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   asmtext: a text subclass specialized for dealing with Assembly code. */

#define MAX_BANGCHARS 32 /* arbitrary limit for how many chars can be bang-comment-starters */
#define MAX_TABSTOPS  80 /* arbitrary number for how many Tab Stops can be set in preferences */

#include "srctext.H"
class asmtext: public srctext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    long Indent(mark *range);
    char * Keywordify(char *buff, boolean checkforceupper);
    void RedoStyles();
    void SetAttributes(struct attributes *atts);

    inline void SetCComments(boolean newValue) { (((this)->CComments) = (newValue)); }
    inline boolean UseCComments() { return ((this)->CComments); }
    inline boolean HasReindentFilter() { return ((this)->reindentFilterName!=NULL && *((this)->reindentFilterName)!='\0'); }
    inline char *ReindentFilterName() { return ((this)->reindentFilterName); }

    static boolean InitializeClass();
    asmtext();

    boolean CComments;
    char bangComments[MAX_BANGCHARS+1];
    char *reindentFilterName;
};
/** @} */
#endif /* _asmtext_H_ */
