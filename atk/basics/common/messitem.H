ATK_INTER
#ifndef _messitem_H_
#define _messitem_H_
/* Copyright 1993 Carnegie Mellon University All rights reserved. */

/** \addtogroup libbasics
 * @{ */
 #include <ATK.H>
class messitem : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      static struct messitemi * Exists( char *str )  ;
      static const char * Replace( const char *str )  ;
      static void InitMessFile();
};
/** @} */
#endif /* _messitem_H_ */
