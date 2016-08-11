ATK_INTER
#ifndef _recsearch_H_
#define _recsearch_H_

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/recsearch.H,v 1.2 1994/11/30 20:42:06 rr2b Stab74 $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidrecsearch_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/recsearch.H,v 1.2 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

#include <ATK.H>

class recsearch : public ATK  {
    public:
      virtual ATKregistryEntry *ATKregistry();

      static boolean InitializeClass();

};

#endif /* _recsearch_H_ */
