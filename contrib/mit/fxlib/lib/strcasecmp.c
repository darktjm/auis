

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/*
 * This file is stolen from the Athena OLH system
 * It contains the definition of strcasecmp, which isn't present on the
 * *^&)(&* broken PS/2's.
 *
 */

#if defined(_I386) && defined(_BSD)

#include <stdio.h>

int strcasecmp(str1,str2)
     char *str1,*str2;
{
  int a;

  if (str1 == NULL) {
    if (str2 == NULL)
      return(0);
    else
      return(-1);
  }
  if (str2 == NULL)
    return(1);
  while ((*str1 != '\0') && (*str2 != '\0')) {
    a = (*str1++ - *str2++);
    if ((a != 0) && (a != 'A' - 'a') && (a != 'a' -'A')) return(a);
  }
  if (*str1 == *str2) return(0);
  if (*str1 ==  '\0') return(-1); else return(1);
}

#endif 
