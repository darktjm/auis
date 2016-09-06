/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#include <andrewos.h>
ATK_IMPL("chlist.H")
#include <chlist.H>


ATKdefineRegistry(chlist, text, NULL);
void Clear(class chlist  *self);


chlist::chlist()
{
    this->ItemList = NULL;
    this->numitems = 0;
    this->numitemsallocated = 0;
    this->freeProc = NULL;
    this->numRegions = 1;
    this->numRegionsAllocated = 4;
    this->strRegionNum = 0;
    (this)->SetReadOnly( TRUE);
    THROWONFAILURE((TRUE));
}

void Clear(class chlist  *self)
{
    long i, j;
    struct listitem *item;

    for (i = self->numitems - 1; i >= 0; i--) {
	item = &self->ItemList[i];
	if (self->freeProc != NULL && item->rock != 0) {
	    (*self->freeProc)(item->rock);
	}
	if (item->regionStrings != NULL) {
	    for (j = 0; j < self->numRegions; j++) {
		if (item->regionStrings[j] != NULL) {
		    free(item->regionStrings[j]);
		}
	    }
	    free(item->regionStrings);
	}
	else if (self->ItemList[i].str != NULL)  {
	    free(self->ItemList[i].str);
	}
    }
    if (self->ItemList != NULL) {
	free(self->ItemList);
    }
}	

chlist::~chlist()
{
    ::Clear(this);
}

void chlist::Clear()
{
    ::Clear(this);
    (this)->text::Clear();

    this->ItemList = NULL;
    this->numitems = 0;
    this->numitemsallocated = 0;
    this->freeProc = NULL;
    this->numRegions = 1;
    this->numRegionsAllocated = 4;
    this->strRegionNum = 0;
    (this)->SetReadOnly( TRUE);
}
    
void chlist::SetRegionStringByIndex(long  index, long  regionNum, const char  *regionStr)
{
    struct listitem *item;
    long len, oldLen, changeLen = 0;
    long startPos, i;
    char *newstr;

    if (index < 0 || index >= this->numitems) {
	return;
    }
    if (regionNum < 0 || regionNum >= this->numRegions) {
	return;
    }

    item = &this->ItemList[index];

    len = 0;
    newstr = NULL;

    if (regionStr != NULL)  {
	len = strlen(regionStr);
	if (len != 0) {
	    newstr = (char *) malloc(len + 1);
	    strcpy(newstr, regionStr);
	}
    }

    startPos = item->loc;
    if (item->regionStrings != NULL || regionNum != this->strRegionNum) {
	if (item->regionStrings == NULL)  {
	    item->regionStrings = (char **) calloc(this->numRegionsAllocated, sizeof(char *));
	    item->regionStrings[this->strRegionNum] = item->str;
	}

	for (i = 0; i < regionNum; i++) {
	    if (item->regionStrings[i] != NULL) {
		startPos += strlen(item->regionStrings[i]);
	    }
	}

	if (item->regionStrings[regionNum] == NULL) {
	    /* New string */
	    if (len != 0) {
		(this)->AlwaysInsertCharacters( startPos, newstr, len);
		changeLen = len;
	    }
	}
	else {
	    oldLen = strlen(item->regionStrings[regionNum]);
	    if (len != 0)  {
		(this)->AlwaysReplaceCharacters( startPos, oldLen, newstr, len);
		changeLen = len - oldLen;
	    }
	    else {
		/* Need to check if enivironment goes away */
		(this)->AlwaysDeleteCharacters( startPos, oldLen);
		changeLen = -oldLen;
	    }
	    free(item->regionStrings[regionNum]);
	}

	item->regionStrings[regionNum] = newstr;
	if (regionNum == this->strRegionNum) {
	    item->str = newstr;
	}
    }
    else {
	if (item->str != NULL) {
	    oldLen = strlen(item->str);
	    free(item->str);
	}
	else {
	    oldLen = 0;
	}

	if (len != 0) {
	    (this)->AlwaysReplaceCharacters( startPos, oldLen, newstr, len);
	    changeLen = len - oldLen;
	}
	else {
	    (this)->AlwaysDeleteCharacters( startPos, oldLen);
	    changeLen = -oldLen;
	}
	item->str = newstr;
    }

    if (changeLen != 0) {
	for (i = index + 1; i < this->numitems; i++) {
	    this->ItemList[i].loc += changeLen;
	}
    }
    (this)->NotifyObservers( 0);
}

void chlist::SetRegionString(const char  *str, long  regionNum, const char  *regionStr)
{
    (this)->SetRegionStringByIndex( (this)->GetIndexByString( str), regionNum, regionStr);

}

void chlist::DefineRegion(long  regionNum)
{
    long i;

    if (regionNum >= this->numRegionsAllocated) {
	/* Need to reallocate region strings */
	this->numRegionsAllocated += 4;

	for (i = 0; i < this->numitems; i++) {
	    if (this->ItemList[i].regionStrings != NULL) {
		this->ItemList[i].regionStrings = (char **) realloc(this->ItemList[i].regionStrings, this->numRegionsAllocated * sizeof(char *));
	    }
	}
    }
    if (regionNum >= this->numRegions) {
	this->numRegions = regionNum + 1;
    }
}

void chlist::DefineStringRegion(long  regionNum)
{
    long i;

    if (this->strRegionNum != regionNum && regionNum < this->numRegions && regionNum >= 0) {
	for (i = 0; i < this->numitems; i++) {
	    if (this->ItemList[i].regionStrings != NULL) {
		(this)->SetRegionStringByIndex( i, regionNum, this->ItemList[i].str);
		(this)->SetRegionStringByIndex( i, this->strRegionNum, NULL);
	    }
	    this->ItemList[i].str = this->ItemList[i].regionStrings[regionNum];
	}
	this->strRegionNum = regionNum;
    }
}

boolean chlist::AddItemAtIndex(long  index, const char  *str, chlist_itemfptr proc, long  data)
{
    char *mycopy;
    struct listitem *newbuf;
    long pos, len, i;

    if (this->numitems >= this->numitemsallocated) {
	this->numitemsallocated += 25;
	newbuf = (struct listitem *) malloc(this->numitemsallocated * sizeof(struct listitem));
	if (newbuf == NULL) {
	    return(FALSE);
	}
	if (this->ItemList != NULL) {
	    memmove(newbuf, this->ItemList, (this->numitemsallocated - 25) * sizeof(struct listitem));
	    free(this->ItemList);
	}
	this->ItemList = newbuf;
    }
    len = strlen(str);
    mycopy = (char *)malloc(1+len);
    if (!mycopy) {
	--this->numitems;
	return(FALSE);
    }
    strcpy(mycopy, str);

    if (index >= this->numitems) {
	index = this->numitems;
	pos = (this)->GetLength();
    }
    else {
	pos = this->ItemList[index].loc;
	for (i = this->numitems; i > index; i--) {
	    this->ItemList[i] = this->ItemList[i-1];
	    this->ItemList[i].loc += len + 1;
	}
    }

    (this)->AlwaysInsertCharacters( pos, str, len);
    (this)->AlwaysInsertCharacters( pos+len, "\n", 1);
    this->ItemList[index].str = mycopy;
    this->ItemList[index].proc = proc;
    this->ItemList[index].rock = data;
    this->ItemList[index].loc = pos;
    this->ItemList[index].regionStrings = NULL;
    ++this->numitems;

    (this)->NotifyObservers( 0);

    return(TRUE);
}


boolean chlist::AddItemToEnd(const char  *str, chlist_itemfptr proc, long  data)
{
    return (this)->AddItemAtIndex( this->numitems, str, proc, data);
}

struct listitem * chlist::FindItem(const char  *str)
{
    long i;
    i = (this)->GetIndexByString(str);
    if (i>= 0) return(&this->ItemList[i]);
    return(NULL);
}

struct listitem * chlist::FindItemByIndex(unsigned long  index)
{
    if (index < (unsigned)this->numitems) {
        return (&this->ItemList[index]);
    }
    return NULL;
}

chlist_freefptr chlist::SetFreeProcedure(chlist_freefptr  proc)
{
    chlist_freefptr oldproc = this->freeProc;

    this->freeProc = proc;

    return(oldproc);
}

long chlist::GetIndexByString(const char  *str)
{
    long i;
    for (i=0; i<this->numitems; ++i) {
	if (!strcmp(this->ItemList[i].str, str)) {
	    return(i);
	}
    }
    return(-1);
}

long chlist::GetIndexByData(long  data)
{
    long i;
    for (i=0; i<this->numitems; ++i) {
	if (data == this->ItemList[i].rock) {
	    return(i);
	}
    }
    return(-1);
}

boolean chlist::DeleteItemByIndex(long  i)
{
    long len, j;

    if (i<0 || i >= this->numitems) return(FALSE);
    if ((i+1) < this->numitems) {
	len = this->ItemList[i+1].loc - this->ItemList[i].loc;
    } else {
	len = (this)->GetLength() - this->ItemList[i].loc;
    }
    (this)->AlwaysDeleteCharacters( this->ItemList[i].loc, len);
    if (this->freeProc != NULL && this->ItemList[i].rock != 0)  {
        (*this->freeProc)(this->ItemList[i].rock);
    }
    if (this->ItemList[i].regionStrings != NULL) {
	for (j = 0; j < this->numRegions; j++) {
	    if (this->ItemList[i].regionStrings[j] != NULL) {
		free(this->ItemList[i].regionStrings[j]);
	    }
	}
	free(this->ItemList[i].regionStrings);
    }
    else if (this->ItemList[i].str != NULL) {
	free(this->ItemList[i].str);
    }
	
    this->numitems --;
    while (i<this->numitems) {
	this->ItemList[i].str = this->ItemList[i+1].str;
	this->ItemList[i].loc = this->ItemList[i+1].loc - len;
	this->ItemList[i].proc = this->ItemList[i+1].proc;
	this->ItemList[i].rock = this->ItemList[i+1].rock;
	this->ItemList[i].regionStrings = this->ItemList[i+1].regionStrings;
	++i;
    }
    (this)->NotifyObservers( 0);
    return(TRUE);
}

boolean chlist::DeleteItem(const char  *str)
{
    long i = (this)->GetIndexByString( str);

    return (this)->DeleteItemByIndex( i);
}

boolean chlist::ChangeItemByIndex(long  index, const char  *newstr)
{
    (this)->SetRegionStringByIndex( index, this->strRegionNum, newstr);

    return(TRUE);
}

boolean chlist::ChangeItem(const char  *oldstr , const char  *newstr)
{
    return (this)->ChangeItemByIndex( (this)->GetIndexByString( oldstr), newstr);
}

boolean chlist::ChangeDataByIndex(long  i, long  rock)
{
    if (i<0 || i >= this->numitems) return(FALSE);

    if (this->freeProc != NULL && this->ItemList[i].rock != 0)  {
        (*this->freeProc)(this->ItemList[i].rock);
    }

    this->ItemList[i].rock = rock;

    (this)->NotifyObservers( 0);

    return(TRUE);
}

boolean chlist::ChangeData(const char  *oldstr, long  rock)
{
    return (this)->ChangeDataByIndex( (this)->GetIndexByString( oldstr), rock);
}

long chlist::GetRegionInfoForPosition(long  index, long  position, long  *size, long  *offset)
{
    struct listitem *item = &this->ItemList[index];
    long itemPosition = item->loc;
    long len;
    long itemLen, regionSize = 0, regionOffset = 0, regionID;

    if (index < this->numitems - 1) {
	itemLen = this->ItemList[index + 1].loc - item->loc - 1;
    }
    else {
	itemLen = (this)->GetLength() - item->loc - 1;
    }

    if (position == item->loc + itemLen) {
	regionSize = 1;
	regionOffset = 0;
	regionID = -1;
    }
    else if (item->regionStrings == NULL) {
	regionID = this->strRegionNum;
	regionSize = itemLen;
	regionOffset = itemPosition - position;
    }
    else {

	long i;

	regionID = -1;

	for (i = 0; i < this->numRegions; i++) {
	    len = (item->regionStrings[i] != NULL) ? strlen(item->regionStrings[i]) : 0;
	    if (itemPosition + len > position) {
		regionID = i;
		regionSize = len;
		regionOffset = position - itemPosition;
		break;
	    }
	    else {
		itemPosition += len;
	    }
	}
    }
    if (size != NULL) {
	*size = regionSize;
    }
    if (offset != NULL) {
	*offset = regionOffset;
    }

    return regionID;
}

long chlist::GetIndexByPosition(long  position, long  *regionID, long  *size, long  *offset)
{
    long min = 0;
    long max = this->numitems - 1;
    long split;

    while (max > min) {
	split = min + (max-min)/2;
	if (this->ItemList[split].loc > position) {
	    max = split - 1;
	} else {
	    if (min == split) {
		if (this->ItemList[max].loc <= position) {
		    min = max;
		}
		break;
	    }
	    min = split;
	}
    }
    if (min < this->numitems) {
	/* found an item */
	long region = (this)->GetRegionInfoForPosition( min, position, size, offset);

	if (regionID != NULL) {
	    *regionID = region;
	}

	return min;
    }
    return -1;
}

void chlist::EnumerateItems(long  index, long  length, chlist_efptr  proc, long  rock)
{
    long endIndex = index + length;

    if (endIndex > this->numitems) {
	endIndex = this->numitems;
    }

    while (index != endIndex) {
	if (! (*proc)(this, &this->ItemList[index], index, rock)) {
	    return;
	}
	index++;
    }
}

