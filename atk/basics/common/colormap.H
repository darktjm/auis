ATK_INTER
#ifndef _colormap_H_
#define _colormap_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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

#include <dataobject.H>

class colormap  : public  dataobject  {
public:
  virtual ATKregistryEntry *ATKregistry();

  static boolean InitializeClass();

  colormap();
  virtual ~colormap();

  const char * ViewName();

  virtual int SetColor(class color *color, boolean needpixel)  ;
  virtual class color * LookupColor(const char *name, unsigned int red, unsigned int green, unsigned int blue, boolean needpixel)  ;
  virtual int LookupColorCell(class color *c)  ;
  virtual int Copy(class colormap *source)  ;
  virtual int Merge(class colormap *other)  ;
  virtual class color * AllocColor(char *name, unsigned int red, unsigned int green, unsigned int blue, boolean needpixel)  ;
  virtual int ChangeColor(class color *c)  ;
  virtual void DestroyColor(class color *c);

  inline int   Size() { return ((this)->size); }
  inline int   Used() { return ((this)->used); }
  inline class color *NthColor(int n) { return ((this)->colors[n]); }

  int size, used;
  class color **colors;
};


#endif /* _colormap_H_ */
