/*                                                              HTTee:  Tee stream for libwww
                                        TEE STREAM
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   The Tee stream just writes everything you put into it into two oter streams. One use
   (the only use?!) is for taking a cached copey on disk while loading the main copy,
   without having to wait for the disk copy to be finished and reread it.
   
   This module is implemented by HTTee.c, and it is a part of the Library of Common Code.
   
 */
#include "HTStream.h"

extern HTStream * HTTee PARAMS((HTStream* s1, HTStream* s2));
/*

   End of definition  */
