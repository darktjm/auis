#ifndef _textflow_H_
#define _textflow_H_

/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
*/
/** \addtogroup libfigure
 * @{ */
#include <figure.H>

class textflow : public figure {
    public:
      TRACED_CLASS(textflow);

      textflow();

      virtual const char *ViewName();
};
/** @} */
#endif /* _textflow_H_ */
