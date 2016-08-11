#ifndef _color_H_
#define _color_H_
/* Copyright 1995 Carnegie Mellon University All rights reserved.
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
#include <mflex.H>
#include <observable.H>
#include <atom.H>

struct ddcolor_list;
class ddcolor;
class colormap;
// This is the class that application programmers should use
// to represent a color.
// Just create a color object, and pass it wherever a color
// is required.  color objects can be assigned a const char *,
// or another color object.  E.g.:
// color obj;
// obj="Pink";
// obj=Pink_atom;
// or
// obj=color(65535, 0, 0);
// To free the underlying color just assign NULL to the color:
// obj=NULL;
// Note that the underlying color will be freed automatically
// when the color is destroyed.
class color {
    ddcolor_list *devcolors;
    color *next;
  public:
    const atom *name;
    unsigned short r, g, b;
    color();
    color(const color &);
    color(const char *name);
    color(const atom *name);
    color(unsigned short r, unsigned short g, unsigned short b);
    ~color();
    color &operator=(const color &);
    color &operator=(const char *);
    color &operator=(const atom  *);
    void AddMapping(ddcolor *device_color);
    void DeleteMapping(colormap *dm);
    ddcolor *FindMapping(colormap *dm);
    inline color *Next() {
	return next;
    }
    inline const char *Name() {
	return name?name->Name():NULL;
    }
    inline void RGB(unsigned short &ra, unsigned short &ga, unsigned short &ba) {
	ra=r;
	ga=g;
	ba=b;
    }
    void HardwareRGB(colormap *dm, unsigned short &ra, unsigned short &ga, unsigned short &ba);	
    static char *ColorName(char *result, char *name,  unsigned char rgb[3]);
		// stores a name for the color into result;  returns result
		// uses 'name' if not null;  otherwise uses #xxxxxx from rgb
		// assumes 'result' has space for at least 100 bytes
};

// icolor and icolormap are used by the graphic class to maintain a list of
// the colors allocated implicitly by old style foreground and background
// setting calls.
class icolor : public color {
    unsigned long refs;
    icolor *next;
  public:
    icolor();
    icolor(const char *name);
    icolor(const atom *name);
    icolor(unsigned short r, unsigned short g, unsigned short b);
    ~icolor();
    static icolor *Create(const char *name);
    static icolor *Create(const atom *name);
    static icolor *Create(unsigned short r, unsigned short g, unsigned short b);
    static icolor *Find(const char *name);
    static icolor *Find(const atom *name);
    static icolor *Find(unsigned short r, unsigned short g, unsigned short b);
    static icolor *Find(const color *c);
    void Destroy();
    inline void Reference() { refs++; }
};

DEFINE_MFLEX_CLASS(icolormap_base, icolor *, 0);
class icolormap : public icolormap_base {
  public:
   icolormap();
    ~icolormap();
    icolor *Alloc(const char *name, unsigned short r, unsigned short g, unsigned short b);
    icolor *Alloc(const atom *name, unsigned short r, unsigned short g, unsigned short b);
    icolor *Alloc(const char *name);
    icolor *Alloc(const atom *name);
    icolor *Alloc(unsigned short r, unsigned short g, unsigned short b);
    void Clear();
};

// These are the classes which should form the basis for the window system dependent
// portions of the code.

//  A ddcolor represents a particular color allocated in a particular colormap in the window system.
// ddcolor objects must be allocated with new, and should usually be destroyed with Destroy.
// If a colormap is being cleared or destroyed it will notify it's observers that the colormap
// is being flushed and then it will call delete on all the ddcolors.
class atom;
class ddcolor {
  public:
    ddcolor();
    ddcolor(colormap *map, const atom *name, unsigned short r, unsigned short g, unsigned short b);
    virtual ~ddcolor();
    virtual void Destroy();
    inline void Reference() {
	refs++;
    }
    virtual void GetRGB(unsigned short &ra, unsigned short &ga, unsigned short &ba);
    inline void SetColormap(class colormap *colormap) { ((this)->cmap = (colormap)); }
    inline class colormap  * &GetColormap() { return ((this)->cmap); }
    unsigned short ir, ig, ib;
    const atom *iname;
    unsigned long refs;
    colormap *cmap;
};

struct ddcolor_list {
    ddcolor *dcolor;
    ddcolor_list *next;
};
#define colormap_FLUSH 1

DEFINE_MFLEX_CLASS(ddcolors,ddcolor *, 10);
// colormap represents a particular colormap in the window system.
// It maintains a list of ddcolor objects for each color the application
// has allocated in the colormap.
// If the colormap is being extensively modified Flush should be
// called first.
class colormap : public observable {
  public:
    ATKregistryEntry *ATKregistry();
    colormap();
    virtual ~colormap();
    void Flush();
    virtual ddcolor *Allocate(const color *c); 
    virtual ddcolor *Allocate(const char *name, unsigned short r, unsigned short g, unsigned short b);
    virtual ddcolor *Allocate(const atom *name, unsigned short r, unsigned short g, unsigned short b);
    virtual ddcolor *Allocate(const char *name);
    virtual ddcolor *Alloc(const atom *name);
    virtual ddcolor *Alloc(unsigned short r, unsigned short g, unsigned short b);
    // Note that you must call Flush before Free'ing any colors which
    // might be in use.
    virtual void Free(ddcolor *dc);
    ddcolors colors;
};
#endif /* _color_H_ */

