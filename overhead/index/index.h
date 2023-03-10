#ifndef _andrew_index_h_
#define _andrew_index_h_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libindex libindex
 * A library for storing an on-disk hash table.
 * @{ */

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

#include <atkproto.h>
#include <stdio.h>
BEGINCPLUSPLUSPROTOS

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

extern void index_HashAdd(struct hashList *alist, long ahash);
extern int index_HashRemove(struct hashList *alist, long ahash);
extern int index_HashPresent(struct hashList *alist, long ahash);
extern void index_FreeHL(struct hashList *alist);

extern int index_Create(char *apath, long aHashSize);
extern void index_CWrite(struct Index *ai, struct indexBucket *ab);
extern void index_CPut(struct Index *ai, struct indexBucket *ab);
extern void index_FreeIndex(struct Index *ai, struct indexBucket *abucket);

extern int index_GetKey(struct Index *ai, struct recordID *arid, char *abuffer, long alen);
extern int index_AddPrimary(struct Index *ai, char *akey, char *adata);
extern int index_AddSecondary(struct Index *ai, struct recordID *arid, char *akey);
extern int index_DeletePrimary(struct Index *ai, struct recordID *arid);
extern int index_DeleteSecondary(struct Index *ai, struct recordID *arid, char *akey);
extern void index_Dump(struct Index *ai);

typedef void (*index_efptr)(struct Index *ai, struct indexComponent *tc, char *arock);
extern struct hashList *index_NewHL();
extern struct indexComponent *index_FindID(struct indexBucket  *ab, struct recordID  *arid);
extern FILE *index_HashOpen(struct Index  *ai, long  ahash, long  awrite);
extern struct Index *index_Open(const char  *apath);
extern void index_Close(struct Index *ai);
extern struct recordSet *index_GetPrimarySet(struct Index  *ai, char  *akey);
extern struct recordSet *index_GetAnySet(struct Index  *ai, char  *akey);
extern struct indexBucket *index_ReadIndex(FILE  *afile);
extern struct indexBucket *index_CGetHash(struct Index  *ai, long  ahash);
extern struct indexBucket *index_CGet(struct Index  *ai, char  *akey);
extern struct recordSet *recordset_New(int asize);
extern long index_Hash(char  *astring, short  hashSize);
extern void index_Enumerate(struct Index *ai, index_efptr aproc, char *arock);
extern long index_GetData(struct Index *ai, struct recordID *arid, char *abuffer, long alen);
extern void recordset_Free(struct recordSet *aset);
extern void recordset_Add(struct recordSet *aset, struct recordID *arid);
/** @} */
ENDCPLUSPLUSPROTOS
 
#endif
