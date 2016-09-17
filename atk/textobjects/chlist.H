ATK_INTER
#ifndef _chlist_H_
#define _chlist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

#endif /* _chlist_H_ */
