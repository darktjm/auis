/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* sys/types.h sys/file.h */
#include <stdio.h>

#include "index.h"

/* given an index and a record id, copy out key into abuffer, a buffer of max size alen */
int index_GetKey(struct Index *ai, struct recordID *arid, char *abuffer, long alen)
{
    struct indexBucket *tb;
    struct indexComponent *tc;
    long rval;
    tb = index_CGetHash(ai, rhash(*arid));
    if (!tb) return INDEXNOENT;
    tc = index_FindID(tb, arid);
    if (tc) {
	strncpy(abuffer, tc->name, alen);
	rval = 0;
    }
    else {
	rval = INDEXNOENT;
    }
    index_CPut(ai, tb);
    return rval;
}

/* given an index and a record id, copy out data into abuffer, a buffer of max size alen */
long index_GetData(struct Index *ai, struct recordID *arid, char *abuffer, long alen)
{
    struct indexBucket *tb;
    struct indexComponent *tc;
    long rval;
    tb = index_CGetHash(ai, rhash(*arid));
    if (!tb) return INDEXNOENT;
    tc = index_FindID(tb, arid);
    if (tc) {
	strncpy(abuffer, tc->data, alen);
	rval = 0;
    }
    else {
	rval = INDEXNOENT;
    }
    index_CPut(ai, tb);
    return rval;
}

/* internal routine: given a bucket, tell if there are any references from a secondary
    * record the given record id.
    */
static int index_RecordInUse(struct indexBucket *ab, struct recordID *arid)
{
    struct indexComponent *tc;
    for(tc = ab->list; tc; tc=tc->next) {
	if (tc->primary == 0 && req(*arid, tc->id)) return 1;
    }
    return 0;
}

/*
  * Internal routine: given a bucket, return a pointer to the named primary record, or null
  * if none exist.
      */
struct indexComponent *index_FindID(struct indexBucket *ab, struct recordID *arid)
{
    struct indexComponent *tc;
    for(tc = ab->list; tc; tc=tc->next) {
	if (tc->primary == 1 && req(*arid, tc->id)) return tc;
    }
    return (struct indexComponent *) 0;
}

/*
  * Internal routine: given a bucket pointer and a record id, generate the next unique
  * record id for records placed in that bucket.
      */
static void index_GenerateKey(struct indexBucket *ab, struct recordID *arid)
{
    arid->word1 = ab->hashIndex;
    arid->word2 = ab->nextID++;
}

/*
  * Given an index and a key, add a new primary record with the provided associated data.
  * This routine always creates a new record, even if there is already a primary record
      * with the same key.
      */
int index_AddPrimary(struct Index *ai, char *akey, char *adata)
{
    struct indexBucket *tb;
    struct indexComponent *tc;
    tb = index_CGet(ai, akey);
    tc = (struct indexComponent *) malloc(sizeof (struct indexComponent));
    tc->primary = 1;
    index_GenerateKey(tb, &tc->id);
    tc->hashes = index_NewHL();
    tc->name = strdup(akey);
    tc->data = strdup(adata);
    tb->modified = 1;
    tc->next = tb->list;
    tb->list = tc;
    index_CPut(ai, tb);
    return 0;
}

/*
  * Given an index, a record id of a primary record and a new key, this function
  * creates a new secondary record pointing to the named primary record.
  */
int index_AddSecondary(struct Index *ai, struct recordID *arid, char *akey)
{
    struct indexBucket *tb;
    struct indexComponent *tc;
    long idUsed, keyHash;
    tb = index_CGet(ai, akey);
    idUsed = index_RecordInUse(tb, arid);
    tc = (struct indexComponent *) malloc(sizeof (struct indexComponent));
    tc->primary = 0;
    rset(tc->id, *arid);
    tc->name = strdup(akey);
    tb->modified = 1;
    tc->next = tb->list;
    tb->list = tc;
    /* check if need to add to hash list */
    index_CPut(ai, tb);
    if (!idUsed) {	/* if id was not previously referenced from this bucket, add this bucket in */
	/* now add to primary's hash list */
	keyHash = index_Hash(akey, ai->hashTableSize);
	tb = index_CGetHash(ai, rhash(*arid));
	tc = index_FindID(tb, arid);
	if (tc) {
	    if (!index_HashPresent(tc->hashes, keyHash)) {
		index_HashAdd(tc->hashes, keyHash);
		tb->modified = 1;
	    }
	}
	index_CPut(ai, tb);
    }
    return 0;
}

static void index_PurgeBucket (struct Index *ai, long ahash, struct recordID *arid);

/*
  * Given an index and a primary record id, this routine deletes the record and all secondary
  * records pointing to the specified primary id.
  */
int index_DeletePrimary(struct Index *ai, struct recordID *arid)
{
    struct indexBucket *tb;
    struct indexComponent *tc;
    int i;
    struct indexComponent **lc;
    struct hashList *mh = NULL;
    struct hashList *th;
    tb = index_CGetHash(ai, rhash(*arid));
    lc = &tb->list;
    for(tc = *lc; tc; tc=tc->next) {
	if (req(*arid, tc->id) && tc->primary) {
	    /* found the record */
	    *lc = tc->next;	/* splice it out */
	    /* free everything but the hashList */
	    mh = tc->hashes;
	    free(tc->name);
	    free(tc->data);
	    free(tc);
	    tb->modified = 1;
	}
	else lc = &tc->next;
    }
    index_CPut(ai, tb);
    if (mh != NULL){
	for(th=mh;th;th=th->next) {
	    for(i=0;i<th->nentries;i++) {
		index_PurgeBucket(ai, th->entries[i], arid);
	    }
	}
    }
    else return(1);
	
    index_FreeHL(mh);	/* finally free the hash list */
    return 0;
}

/*
  * Internal routine: given an index, a record id and a hash bucket, purge the bucket of all
  * references to the speicfied record id.
  */
static void index_PurgeBucket (struct Index *ai, long ahash, struct recordID *arid)
{
    struct indexComponent *tc, *nc;
    struct indexBucket *tb;
    struct indexComponent **lc;
    tb = index_CGetHash(ai, ahash);
    lc = &tb->list;
    for(tc=tb->list; tc; tc=nc) {
	nc = tc->next;
	if (req(*arid, tc->id) && tc->primary == 0) {
	    *lc =nc;
	    tb->modified = 1;
	    free(tc);
	}
	else lc = &tc->next;
    }
    index_CPut(ai, tb);
}

/*
  * Given an index, a primary record's record id, and a key, delete the secondary record
	 * with the specified key that refers to the given primary record id.
	 */
int index_DeleteSecondary(struct Index *ai, struct recordID *arid, char *akey)
{
    struct indexComponent *tc, *nc;
    struct indexComponent **lc;
    struct indexBucket *tb;
    char flag;

    tb = index_CGet(ai, akey);
    lc = &tb->list;
    flag = 0;
    for(tc=tb->list; tc; tc=nc) {
	nc = tc->next;
	if (req(*arid, tc->id) && strcmp(akey, tc->name) == 0 && tc->primary == 0) {
	    *lc =nc;
	    tb->modified = 1;
	    free(tc);
	    flag = 1;
	    break;
	}
	lc = &tc->next;
    }
    if (flag == 0) return 0;		/* didn't do anything */
    flag = index_RecordInUse(tb, arid);	/* find out if any references are left */
    index_CPut(ai, tb);
    if (!flag) {		/* none left? delete pointer to this bucket */
	tb = index_CGetHash(ai, rhash(*arid));
	if (tb) {
	    tc = index_FindID(tb, arid);
	    if (tc) index_HashRemove(tc->hashes, index_Hash(akey, ai->hashTableSize));
	    tb->modified = 1;
	    index_CPut(ai, tb);
	}
    }
    return 0;
}

/*
  * Internal routine: compute the hash for a string, given a hash table size (usually
									       * found in the index structure).
	*/
long index_Hash(char *astring, short hashSize)
{
    long aval;
    short tc;
    aval = 0;
    while ((tc  = *astring++)) {
	aval *= 173;
	aval += tc;
    }
    aval = aval % hashSize;
    if (aval < 0) aval += hashSize;
    return aval;
}

/*
  * Internal routine: given an index, a hash bucket and a flag indicating whether the
  * file is to be opened for reading or writing, open the appropriate bucket file and return
      * a standard I/O FILE * for the file.  If opening for writing, the new file will be created
	  * if necessary, and truncated.
	      */
FILE *index_HashOpen(struct Index *ai, long ahash, long awrite)
{
    char tpath[1024];
    char tbuffer[20];
    strcpy(tpath, ai->pathName);
    strcat(tpath, "/");
    sprintf(tbuffer, "H%ld", ahash);
    strcat(tpath, tbuffer);
    return fopen(tpath, (awrite? "w+" : "r"));
}

/*
  * Given an index, enumerate all of the records in the index.  Takes an index, a proc and
  * a rock, and calls the proc with the index, the record (struct indexComponent) and rock.
  */
void index_Enumerate(struct Index *ai, index_efptr aproc, char *arock)
{
    long i;
    struct indexBucket *tb;
    struct indexComponent *tc;

    for(i=0;i<ai->hashTableSize;i++) {
	tb = index_CGetHash(ai, i);
	for(tc=tb->list;tc;tc=tc->next) {
	    (*aproc)(ai, tc, arock);
	}
	index_CPut(ai, tb);
    }
}

/*
  * Given a pathname for an index file, return an open index structure for the index.
      * The pathname is the pathname of the directory containing all of the hash bucket
      * and version number files.
      */
struct Index *index_Open(const char *apath)
{
    DIR *td;
    DIRENT_TYPE *tde;
    struct Index *ti;
    long code;
    long htSize, version, foundFlag;

    td = opendir(apath);
    if (!td) {
	return (struct Index *) 0;
    }
    foundFlag = 0;
    while((tde=readdir(td))) {
	if (tde->d_name[0] == 'V') {
	    /* found the version number file */
	    if (foundFlag) {
		closedir(td);
		return (struct Index *) 0;
	    }
	    foundFlag = 1;
	    code = sscanf(tde->d_name, "V%ld.%ld", &htSize, &version);
	    if (code != 2) {
		closedir(td);
		return (struct Index *) 0;
	    }
	}
    }
    closedir(td);
    if (!foundFlag) {
	return (struct Index *) 0;
    }
    ti = (struct Index *) malloc (sizeof (struct Index));
    ti->pathName = strdup(apath);
    ti->hashTableSize = htSize;
    ti->version = version;
    ti->blist = (struct indexBucket *) 0;
    return ti;
}

/*
  * Close an open index file, freeing all associated files.
  */
void index_Close(struct Index *ai)
{
    struct indexBucket *tb, *nb;
    for(tb=ai->blist;tb;tb=nb) {
	nb = tb->next;		/* pull it out before freeing */
	if (tb->modified) {
	    index_CWrite(ai, tb);
	    index_FreeIndex(ai, tb);
	}
    }
    free(ai->pathName);
    free(ai);
}

/*
  * Given an index and a key, return a new recordSet containing the record IDs of all of the
  * primary records having the specified key.  This recordSet must be freed, using
  * recordset_Free, when the caller is finished with it.
  */
struct recordSet *index_GetPrimarySet(struct Index *ai, char *akey)
{
    struct indexBucket *tb;
    struct indexComponent *tlist;
    struct recordSet *rs;

    tb = index_CGet(ai, akey);
    rs = recordset_New(4);
    /* now we have the index in core, so we scan it */
    for(tlist = tb->list;tlist;tlist=tlist->next) {
	if (tlist->primary && strcmp(tlist->name, akey) == 0) recordset_Add(rs, &tlist->id);
    }
    index_CPut(ai, tb);
    return rs;
}

/*
  * Given an index and a key, return a new recordSet containing the record IDs of all
  * records (primary or secondary) having the specified key.  This recordSet must be freed, using
  * recordset_Free, when the caller is finished with it.
  */
struct recordSet *index_GetAnySet(struct Index *ai, char *akey)
{
    struct indexBucket *tb;
    struct indexComponent *tlist;
    struct recordSet *rs;

    tb = index_CGet(ai, akey);
    rs = recordset_New(4);
    /* now we have the index in core, so we scan it */
    for(tlist = tb->list;tlist;tlist=tlist->next) {
	if (strcmp(tlist->name, akey) == 0) recordset_Add(rs, &tlist->id);
    }
    index_CPut(ai, tb);
    return rs;
}

/*
  * Quasi-internal routine (if you need it, you need it): produces a dupm of an open index
  * on standard output.  Very useful for debugging things.
      */
void index_Dump(struct Index *ai)
{
    struct indexBucket *tb;
    struct indexComponent *tc;
    long i;
    struct hashList *th;
    long j;
    for(i=0;i<ai->hashTableSize;i++) {
	tb = index_CGetHash(ai, i);
	if (!tb) printf("Failed to get bucket %ld\n", i);
	else {
	    printf("Bucket %ld next id %ld\n", i, tb->nextID);
	    for(tc=tb->list;tc;tc=tc->next) {
		printf(" Record named '%s' ", tc->name);
		if (tc->primary) {
		    printf("id %ld.%ld data '%s'\n", tc->id.word1, tc->id.word2, tc->data);
		    printf("  hashes:");
		    for(th=tc->hashes;th;th=th->next) {
			for(j=0;j<th->nentries;j++) {
			    printf(" %d", th->entries[j]);
			}
		    }
		    printf("\n");
		}
		else {
		    printf("refers to %ld.%ld\n", tc->id.word1, tc->id.word2);
		}
	    }
	}
	index_CPut(ai, tb);
    }
}
