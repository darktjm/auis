#ifndef _idltext_H_
#define _idltext_H_
/** \addtogroup libsrctext
 * @{ */
/* File idltext.H created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   idltext: a text subclass specialized for dealing with SOM IDL code. */

#include "cpptext.H"
class idltext: public cpptext {
  public:
    TRACED_CLASS_NODEST(idltext);

    static boolean InitializeClass();
    idltext();
};
/** @} */
#endif /* _idltext_H_ */
