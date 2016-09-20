/* Copyright 1992 Carnegie Mellon University All rights reserved. */
/** \addtogroup libshadows libshadows
 * Support library for adding 3D shading to graphics.
 * @{ */

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

extern void shadows_SetPreferences(int  (*getint)());
extern void shadows_ComputeColor(unsigned int  br , unsigned int  bg , unsigned int  bb, unsigned short  *rr , unsigned short  *rg , unsigned short  *rb, int  color);

#define shadows_PRESSED 0
#define shadows_TOPSHADOW 1
#define shadows_BOTTOMSHADOW 2

ENDCPLUSPLUSPROTOS
/** @} */
 
