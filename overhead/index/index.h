#ifndef _andrew_index_h_
#define _andrew_index_h_
 /* C++ified by magic !@#%&@#$ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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


 

/*
 * here are the various record headers.  Records come in two major types, the 
 * primary records, which consist of an IPrimary subrecord, an IBucketList subrecord
 * and an IData subrecord, in that order, and a secondary record, consisting of an
 * ISecondary subrecord.
 
 * An IPrimary record is followed by an 8 byte UID, and a null-terminated keyword.
 * A bucket-list is followed by an array of 16 bit (network byte order, kiddies)
 * integers, terminated by an 0xffff entry.
 * An IData subrecord is followed by an uninterpreted null-terminated byte string.

 * Note that no byte strings, keys or names may contain any characters
 * less than or equal to IMax.
 
 * The database itself is comprised of a dir full of two types of files.  One file, named
 * V<version number>.<hash-table size> describes the version number and hash
 * table size.  The others, named H<number> contain the various hash buckets.
 * Each hash bucket contains a set of primary and secondary records whose keys
 * hash to the appropriate bucket.
 */

#define MAXSTRLENGTH		1024		/* max string size */
#define INDEXVERSION		1

struct recordID {
    long word1;		/* hash value */
    long word2;		/* counter */
};

#define IPRIMARY			1
#define IBUCKETLIST		2
#define IDATA			3

#define ISECONDARY		8
/* leave 9 unused, it is a tab */

#define IMAX				26

struct Index {
    long version;
    char *pathName;
    short hashTableSize;
    struct indexBucket *blist;
};

#define MAXHL		10
struct hashList {
    struct hashList *next;
    long nentries;
    short entries[MAXHL];
};

struct indexBucket {
    struct indexBucket *next;
    long hashIndex;
    long nextID;
    struct indexComponent *list;
    char modified;
};

struct indexComponent {
    struct indexComponent *next;
    char *name;
    struct hashList *hashes;
    struct recordID id;
    char *data;
    char primary;				/* true iff primary record */
};

#define INDEXOK			0
#define INDEXNOVERSION	(-1)
#define IBADV			(-2)
#define INDEXNOENT		(-3)

#define rset(a,b) (a).word1 = (b).word1; (a).word2 = (b).word2;
#define req(a,b) ((a).word1 == (b).word1 && (a).word2 == (b).word2)
#define rhash(a) (a).word1

struct recordSet {
    long count;				/* in-use slots */
    long acount;				/* allocated slots */
    struct recordID *data;
};

typedef void (*index_efptr)(struct Index *ai, struct indexComponent *tc, char *arock);
extern struct indexBucket *index_CGet(struct Index  *ai, register char  *akey);
extern struct hashList *index_NewHL();
extern struct indexComponent *index_FindID(register struct indexBucket  *ab, register struct recordID  *arid);
extern FILE *index_HashOpen(register struct Index  *ai, register long  ahash, register long  awrite);
extern struct Index *index_Open(register const char  *apath);
extern void index_Close(struct Index *ai);
extern struct recordSet *index_GetPrimarySet(register struct Index  *ai, register char  *akey);
extern struct recordSet *index_GetAnySet(register struct Index  *ai, register char  *akey);
extern struct indexBucket *index_ReadIndex(register FILE  *afile);
extern struct indexBucket *index_CGetHash(struct Index  *ai, register long  ahash);
extern struct indexBucket *index_CGet(struct Index  *ai, register char  *akey);
extern struct recordSet *recordset_New(int asize);
extern long index_Hash(register char  *astring, register int  hashSize);
void index_Enumerate(struct Index *ai, index_efptr aproc, char *arock);
long index_GetData(struct Index *ai, struct recordID *arid, char *abuffer, long alen);
void recordset_Free(struct recordSet *aset);
ENDCPLUSPLUSPROTOS
 
#endif
