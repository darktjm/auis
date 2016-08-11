/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/*
 * Copyright 1989, 1990, 1991 Jim Frost
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  The author makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/imageaux.C,v 3.2 1993/07/29 20:15:08 rr2b Stab74 $";
#endif
#include <andrewos.h>

#ifndef NORCSID
#endif
unsigned long doMemToVal(unsigned char *p, unsigned int len);
unsigned long doValToMem(unsigned long   val, unsigned char *p, unsigned int len);
unsigned long doMemToValLSB(unsigned char *p, unsigned int len);
unsigned long doValToMemLSB(unsigned long   val, unsigned char *p, unsigned int len);
void flipBits(unsigned char *p, unsigned int len);


unsigned long doMemToVal(unsigned char *p, unsigned int len)
          { unsigned int  a;
  unsigned long i;

  i= 0;
  for (a= 0; a < len; a++)
    i= (i << 8) + *(p++);
  return(i);
}

unsigned long doValToMem(unsigned long   val, unsigned char *p, unsigned int len)
               { int a;

  for (a= len - 1; a >= 0; a--) {
    *(p + a)= val & 0xff;
    val >>= 8;
  }
  return(val);
}

unsigned long doMemToValLSB(unsigned char *p, unsigned int len)
          { int val, a;

  val= 0;
  for (a= len - 1; a >= 0; a--)
    val= (val << 8) + *(p + a);
  return(val);
}

/* this is provided for orthagonality
 */

unsigned long doValToMemLSB(unsigned long   val, unsigned char *p, unsigned int len)
               {
  while (len--) {
    *(p++)= val & 0xff;
    val >>= 8;
  }
  return(val);
}

/* this flips all the bits in a byte array at byte intervals
 */

void flipBits(unsigned char *p, unsigned int len)
          { static int init= 0;
  static unsigned char flipped[256];

  if (!init) {
    int a, b;
    unsigned char norm;

    for (a= 0; a < 256; a++) {
      flipped[a]= 0;
      norm= a;
      for (b= 0; b < 8; b++) {
	flipped[a]= (flipped[a] << 1) | (norm & 1);
	norm >>= 1;
      }
    }
  }

  while (len--)
    p[len]= flipped[p[len]];
}

