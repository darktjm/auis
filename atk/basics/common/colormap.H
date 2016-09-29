#ifndef _colormap_H_
#define _colormap_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
