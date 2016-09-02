ATK_INTER
#ifndef _recsearch_H_
#define _recsearch_H_

#include <ATK.H>

class recsearch : public ATK  {
    public:
      virtual ATKregistryEntry *ATKregistry();

      static boolean InitializeClass();

};

#endif /* _recsearch_H_ */
