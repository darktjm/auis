ATK_INTER
#ifndef _plink_H_
#define _plink_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblink
 * @{ */
#include <link.H>

class plink : public link {
    public:
      virtual ATKregistryEntry *ATKregistry();



      plink();

      virtual ~plink();

      long Read(FILE *file, long id)  ;
      long Write(FILE *file, long writeid, int level)  ;

      virtual void SetTag(char *newtag);

      inline char  * GetTag() { return (this->tag); }

      char *tag;
};
/** @} */
#endif /* _plink_H_ */
