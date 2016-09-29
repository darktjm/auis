#ifndef _fontsel_H_
#define _fontsel_H_
/** \addtogroup libfigure
 * @{ */
/* fontsel.H - font selection inset dataobject */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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
/** @} */
#endif /* _fontsel_H_ */
