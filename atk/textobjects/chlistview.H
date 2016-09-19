ATK_INTER
#ifndef _chlistview_H_
#define _chlistview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <textview.H>
class chlistview: public textview {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	chlistview();
      
	virtual ~chlistview();
    
       class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
       class environment * GetStyleInformation(struct text_statevector *sv, long pos, long *length)  ;
    
      virtual void ActivateItem(int pos);
      virtual void HighlightItem(long index);
      virtual void UnhighlightItem(long index);
      virtual class style * GetRegionStyle(long regionID, boolean highlighted)  ;
      virtual void SetRegionStyles(long regionID, class style *normalStyle, class style *highlightStyle);

      virtual void SetUpdateRegion(long pos, long len);
    
      long highlightedItem;
      long numStylesAllocated;
      class style **normalStyles;
      class style **highlightedStyles;
};

#endif /* _chlistview_H_ */
