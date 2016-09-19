ATK_INTER
#ifndef _fontselview_H_
#define _fontselview_H_
/* fontselv.ch - font selection inset view */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#include <lpair.H>

class fontselview  : public  lpair {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      
	static boolean InitializeClass();
 
      
	fontselview();
      
	virtual ~fontselview();

    
       void ObservedChanged(class observable *fontsel, long status);
       void SetDataObject(class dataobject *dobj);

    
      virtual void ShowExtraOption();
      virtual void SetExtraOptionString(char *val);

    
      class lpair *lp1, *lp2;
      class fontsample *sample;
      boolean showdefault;
      char *defaultstring;

      class stringtbl *familytbl;
      class strtblview *vfamilytbl;
      short *familyacc, familyextra;
      char **familylist;
      int families_num;
      int familynum;

      class stringtbl *styletbl;
      class strtblview *vstyletbl;
      short *styleacc, styleextra;
      long *stylelist;
      int styles_num;
      long style_mask;

      class stringtbl *sizetbl;
      class strtblview *vsizetbl;
      short *sizeacc, sizeextra;
      short *sizelist;
      int sizes_num;
      int sizenum;

};

#endif /* _fontselview_H_ */
