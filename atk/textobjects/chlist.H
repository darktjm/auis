#ifndef _chlist_H_
#define _chlist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libtextobjs
 * @{ */
#include <text.H>
#include <view.H>

typedef void (*chlist_itemfptr)(long rock, class chlist *l, enum view_MouseAction action, long nclicks, long index, int region);
typedef void (*chlist_freefptr)(long rock);
typedef boolean (*chlist_efptr)(chlist *self, struct listitem *it, long index, long rock);

struct listitem {
    char *str;
    int loc;
    chlist_itemfptr proc;
    long rock;
    char **regionStrings;
};

class chlist: public text {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	chlist();
      
	virtual ~chlist();
    
       void Clear();
    
      virtual boolean AddItemAtIndex(long index, const char *str, chlist_itemfptr proc, long rock)  ;
      virtual boolean AddItemToEnd(const char *str, chlist_itemfptr proc, long rock)  ;
      virtual boolean DeleteItem(const char *str)  ;
      virtual boolean DeleteItemByIndex(long index)  ;
      virtual long GetIndexByString(const char *str)  ;
      virtual long GetIndexByData(long rock)  ;
      virtual struct listitem * FindItem(const char *str)  ;
      virtual struct listitem * FindItemByIndex(unsigned long index)  ;
      virtual chlist_freefptr SetFreeProcedure(chlist_freefptr freeProc)  ;
      virtual boolean ChangeItem(const char *oldstr, const char *newstr)  ;
      virtual boolean ChangeItemByIndex(long index, const char *newstr)  ;
      virtual boolean ChangeData(const char *oldstr, long rock)  ;
      virtual boolean ChangeDataByIndex(long index, long rock)  ;
      virtual void DefineRegion(long regionNum);
      virtual void DefineStringRegion(long regionNum);
      virtual void SetRegionStringByIndex(long index, long regionNum, const char *regionStr);
      virtual void SetRegionString(const char *str, long regionNum, const char *regionStr);
      virtual long GetRegionInfoForPosition(long index, long position, long *size, long *offset)  ;
      virtual long GetIndexByPosition(long position, long *regionID, long *size, long *offset)  ;
      virtual void EnumerateItems(long startIndex, long length, chlist_efptr proc, long rock);

    
      inline struct listitem * GetItemList() { return (this->ItemList); }
      inline int GetNumItems() { return (this->numitems); }
      inline int GetNumRegions() { return (this->numRegions); }
    
      struct listitem *ItemList;
      int numitems, numitemsallocated;
      chlist_freefptr freeProc;
      int numRegions, numRegionsAllocated;
      int strRegionNum;
};
/** @} */
#endif /* _chlist_H_ */
