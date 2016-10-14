#ifndef _hlptext_H_
#define _hlptext_H_
/** \addtogroup libsrctext
 * @{ */
/* File hlptext.H created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   hlptext: a text subclass specialized for creating help files and other non-source text views requiring the use of dogtags. */

#include <text.H>
class hlptext: public text {
  public:
    TRACED_CLASS_NODEST(hlptext);

    long Read(FILE *file, long id);

    hlptext();
};
/** @} */
#endif /* _hlptext_H_ */
