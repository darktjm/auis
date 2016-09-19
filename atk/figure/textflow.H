ATK_INTER
#ifndef _textflow_H_
#define _textflow_H_

/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
*/
#include <figure.H>

class textflow : public figure {
    public:
      virtual ATKregistryEntry *ATKregistry();

      textflow();
      virtual ~textflow();

      virtual const char *ViewName();
};

#endif /* _textflow_H_ */
