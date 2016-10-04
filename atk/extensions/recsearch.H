#ifndef _recsearch_H_
#define _recsearch_H_

/** \addtogroup librecsearch Object recsearch
 *  A dynamically loadable ATK class file.  Contains recsearch.
 * @{ */
#include <ATK.H>

class recsearch : public ATK  {
    public:
      virtual ATKregistryEntry *ATKregistry();

      static boolean InitializeClass();

};
/** @} */
#endif /* _recsearch_H_ */
