/*                                                  HTGuess: Guess content-type from a stream
                                   CONTENT-TYPE GUESSER
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This stream is a one that reads first a chunk of stuff, tries to figure out the format,
   and calls HTStreamStack().  This is a kind of lazy-evaluation of HTStreamStack().
   
   This could be extended arbitrarily to recognize all the possible file formats in the
   world, if someone only had time to do it.
   
   This module is implemented by HTGuess.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTGUESS_H
#define HTGUESS_H

#include "HTStream.h"
#include "HTFormat.h"

#ifdef SHORT_NAMES
#define HTGuess_new     HTGuessN
#endif

extern HTStream * HTGuess_new   (HTRequest *     req,
                                        void *          param,
                                        HTFormat        input_format,
                                        HTFormat        output_format,
                                        HTStream *      output_stream);


#endif  /* !HTGUESS_H */
/*

   End of file HTGuess.h. */
