ATK_INTER
#ifndef _fontsel_H_
#define _fontsel_H_
/* fontsel.ch - font selection inset dataobject */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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

#define	fontsel_DATACHANGED	(1)

#define	fontsel_Style	(0)
#define	fontsel_Size	(1)
#define	fontsel_Family	(2)

#define	fontsel_default_Style	(0)
#define	fontsel_default_Size	(12)
#define	fontsel_default_Family	("andy")

#include <dataobject.H>

class fontsel : public  dataobject  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      
	static boolean InitializeClass();

      
	fontsel();
      
	virtual ~fontsel();

    
      virtual void SetStyle(long mask);
      virtual void SetSize(short val);
      virtual void SetFamily(const char *val);

    
      inline long   GetStyle()  { return ((this)->style); }
      inline short   GetSize()  { return ((this)->size); }
      inline const char  * GetFamily()  { return ((this)->family); }
      inline long IsActive(int val)  { return ((this)->active & ((unsigned long)1<<(val))); }
      inline void UnsetStyle()  { (((this))->SetStyle( fontsel_default_Style), (this)->active &= ~((unsigned long)1<<fontsel_Style)); }
      inline void UnsetSize()  { (((this))->SetSize( fontsel_default_Size), (this)->active &= ~((unsigned long)1<<fontsel_Size)); }
      inline void UnsetFamily()  { (((this))->SetFamily( fontsel_default_Family), (this)->active &= ~((unsigned long)1<<fontsel_Family)); }

    
      long style;
      short size;
      char *family;

      long active;
};

#endif /* _fontsel_H_ */
