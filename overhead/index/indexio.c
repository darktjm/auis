/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* sys/types.h sys/file.h */
#include <stdio.h>
#include <sys/stat.h>


#include "index.h"

/*
 * Internal routine: given a FILE * and an integer, write the integer in a machine-independent
 * form to the file.  Make sure this works for vaxes as well as normal machines.
 */
static void writeInteger(FILE *afile, long ai)
{
    long tc;

    tc = (ai >> 24) & 0xff;
    putc(tc, afile);
    tc = (ai >> 16) & 0xff;
    putc(tc, afile);
    tc = (ai >> 8) & 0xff;
    putc(tc, afile);
    tc = ai & 0xff;
    putc(tc, afile);
}

/*
  * Internal routine: given a FILE * and a record id pointer, write the external representation
  * of the record ID to the file.
  */
static void writeRecordID(FILE *afile, struct recordID *arid)
{
    long code;
    long tc;

    code = arid->word1;
    tc = (code >> 24) & 0xff;
    putc(tc, afile);
    tc = (code >> 16) & 0xff;
    putc(tc, afile);
    tc = (code >> 8) & 0xff;
    putc(tc, afile);
    tc = code & 0xff;
    putc(tc, afile);

    code = arid->word2;
    tc = (code >> 24) & 0xff;
    putc(tc, afile);
    tc = (code >> 16) & 0xff;
    putc(tc, afile);
    tc = (code >> 8) & 0xff;
    putc(tc, afile);
    tc = code & 0xff;
    putc(tc, afile);
}

/*
  * Internal routine: given a FILE * and a pointer to an integer, read the external representation
  * of the integer from the file and return it in the place provided.
  */
static void readInteger(FILE *afile, long *ai)
{
    long code;
    long tc;

    code = 0;
    tc = getc(afile);
    code = tc << 24;
    tc = getc(afile);
    code |= tc<<16;
    tc = getc(afile);
    code |= tc<<8;
    tc = getc(afile);
    code |= tc;

    *ai = code;
}

/*
  * Internal routine: given a FILE *, read a record id from the specified file, and
  * return a pointer to a static recordID structure containing the newly-read
  * recordID.
  */
static struct recordID *readRecordID(FILE *afile)
{
    long code;
    long tc;
    static struct recordID tid;

    code = 0;
    tc = getc(afile);
    code = tc << 24;
    tc = getc(afile);
    code |= tc<<16;
    tc = getc(afile);
    code |= tc<<8;
    tc = getc(afile);
    code |= tc;
    tid.word1 = code;

    code = 0;
    tc = getc(afile);
    code = tc << 24;
    tc = getc(afile);
    code |= tc<<16;
    tc = getc(afile);
    code |= tc<<8;
    tc = getc(afile);
    code |= tc;
    tid.word2 = code;
    return &tid;
}

/*
  * Internal routine: read a set of hash bucket ids from a file (FILE *), and return a hashList *
  * pointer to the newly-created list.
  */
static struct hashList *readHashList(FILE *afile)
{
    long temp;
    struct hashList *tlist, *clist;

    /* NEVER return null */
    clist = (struct hashList *) malloc(sizeof (struct hashList));
    clist->nentries = 0;
    clist->next = (struct hashList *) 0;
    while (1) {
	temp = (getc(afile)) << 8;
	temp |= getc(afile);

	/* are we done? */
	if (temp == 0xffff) {
	    return clist;
	}

	/* is the record full? */
	if (clist->nentries >= MAXHL) {
	    tlist = (struct hashList *) malloc(sizeof (struct hashList));
	    tlist->next = clist;
	    tlist->nentries = 0;
	    clist = tlist;
	}

	clist->entries[clist->nentries++] = temp;
    }
}

/*
  * Internal routine: given a FILE * and a hashList * pointer to a list of hash buckets, write
  * the hash list representation to the file.
  */
static void writeHashList(FILE *afile, struct hashList *alist)
{
    long i;
    long temp, tc;
    for(;alist;alist=alist->next) {
	for(i=0;i<alist->nentries;i++) {
	    temp = alist->entries[i];
	    if (temp == 0xffff) continue;		/* don't write out deleted entries */
	    tc = (temp>>8) & 0xff;
	    putc(tc, afile);
	    tc = temp & 0xff;
	    putc(tc, afile);
	}
    }
    putc(0xff, afile);
    putc(0xff, afile);
}

/*
  * Internal routine: Given a FILE * and a hash bucket, write the external representation of
  * the entire hash bucket to the file.
  */
static void index_WriteIndex(FILE *afile, struct indexBucket *ab)
{
    struct indexComponent *c;
    writeInteger(afile, ab->nextID);
    for(c=ab->list; c; c=c->next) {
	if (c->primary) {
	    putc(IPRIMARY, afile);
	    writeRecordID(afile, &c->id);
	    fputs(c->name, afile);
	    putc(0, afile);
	    writeHashList(afile, c->hashes);
	    fputs(c->data, afile);
	    putc(0, afile);
	}
	else /* secondary record */ {
	    putc(ISECONDARY, afile);
	    writeRecordID(afile, &c->id);
	    fputs(c->name, afile);
	    putc(0, afile);
	}
    }
}

/*
  * Internal routine: given a FILE *, read the hash bucket (header information plus all
							     * primary and secondary records) from the file, and return a ponter to the struct
  * hashBucket * just created.
  */
struct indexBucket *index_ReadIndex(FILE *afile)
{
    long tc;
    char charBuffer[MAXSTRLENGTH];
    struct recordID *idp;
    long code;
    struct hashList *tlist;
    struct indexComponent *enchilada;
    struct indexComponent *ti;
    struct indexBucket *tb;
    long nextID;

    readInteger(afile, &nextID);
    enchilada = (struct indexComponent *) 0;
    while (1) {
	tc = getc(afile);
	if (tc == EOF) break;
	switch (tc) {
	    case IPRIMARY:
		/* first read the record id */
		idp = readRecordID(afile);	/* returns pointer to static structure */
		/* now read the index name */
		for(code=0;code<MAXSTRLENGTH;code++) {
		    tc = charBuffer[code] = getc(afile);
		    if (tc == 0) break;
		}
		charBuffer[MAXSTRLENGTH-1] = '\000';
		tlist = readHashList(afile);
		if (!tlist) return (struct indexBucket *) 0;
		ti = (struct indexComponent *) malloc(sizeof(struct indexComponent));
		ti->next = enchilada;
		enchilada = ti;
		ti->name = strdup(charBuffer);
		ti->hashes = tlist;
		ti->primary = 1;
		rset(ti->id, *idp);
		for(code=0;code<MAXSTRLENGTH;code++) {
		    tc = charBuffer[code] = getc(afile);
		    if (tc == 0) break;
		}
		charBuffer[MAXSTRLENGTH-1] = '\000';	    
		ti->data = strdup(charBuffer);
		break;

	    case ISECONDARY:
		idp = readRecordID(afile);
		/* now read the index name */
		for(code=0;code<MAXSTRLENGTH;code++) {
		    tc = charBuffer[code] = getc(afile);
		    if (tc == 0) break;
		}
		charBuffer[MAXSTRLENGTH-1] = '\000';	    
		ti = (struct indexComponent *) malloc(sizeof(struct indexComponent));
		ti->next = enchilada;
		enchilada = ti;
		ti->name = strdup(charBuffer);
		ti->primary = 0;
		rset(ti->id, *idp);
		break;

	    default:
		printf("bad index file in bucket\n");
		return (struct indexBucket *) 0;
	}
    }
    tb = (struct indexBucket *) malloc(sizeof (struct indexBucket));
    tb->list = enchilada;
    tb->nextID = nextID;
    tb->modified = 0;
    tb->hashIndex = 0;
    return tb;
}

/*
  * Internal routine: Given a pointer to a hash bucket, free it.
  */
void index_FreeIndex(struct Index *ai, struct indexBucket *abucket)
{
    struct indexComponent *idx, *nidx;
    struct indexBucket *tbucket, **lbucket;

    /* remove bucket from chain */
    lbucket = &ai->blist;
    for(tbucket = *lbucket; tbucket; tbucket=tbucket->next) {
	if (tbucket == abucket) {
	    *lbucket = tbucket->next;
	    break;
	}
	lbucket = &tbucket->next;
    }

    idx = abucket->list;
    free(abucket);
    for(;idx;idx=nidx) {
	nidx=idx->next;
	free(idx->name);
	if (idx->primary) {
	    struct hashList *th, *nh;
	    for(th=idx->hashes;th;th=nh) {
		nh = th->next;
		free(th);
		free(idx->data);
	    }
	    free(idx);
	}
    }
}

/*
  * Given an open index file, and a hash bucket number, return a pointer to a
  * struct indexBucket containing the required info.  You may modify this structure
  * if you want (all of its components are malloc'd, so free them if you drop pointers
      * to them), but if you do, set the modified flag in the bucket header.  You tell
	    * the index package you are done with this structure by using index_CPut,
	    * not by freeing it.
	    */
struct indexBucket *index_CGetHash(struct Index *ai, long ahash)
{
    FILE *file;
    struct indexBucket *tb;
    for(tb=ai->blist;tb;tb=tb->next) {
	if (tb->hashIndex == ahash) return tb;
    }
    file = index_HashOpen(ai, ahash, 0);
    if (file == (FILE *) 0) return (struct indexBucket *) 0;
    tb = index_ReadIndex(file);
    fclose(file);
    tb->hashIndex = ahash;
    tb->modified = 0;
    tb->next = ai->blist;
    ai->blist = tb;
    return tb;
}

/*
  * Given an open index file, and a key, return a pointer to a
  * struct indexBucket containing the required info.  The key is used to determine
  * the appropriate hash bucket to read.
  * You may modify this structure
  * if you want (all of its components are malloc'd, so free them if you drop pointers
      * to them), but if you do, set the modified flag in the bucket header.  You tell
	    * the index package you are done with this structure by using index_CPut,
	    * not by freeing it.
	    */
struct indexBucket *index_CGet(struct Index *ai, char *akey)
{
    return index_CGetHash(ai, index_Hash(akey, ai->hashTableSize));
}

/*
  * Given an open index file, and a hash bucket obtained from CGet or CGetHash, relinquish
  * the required storage.
  */
void index_CPut(struct Index *ai, struct indexBucket *ab)
{
}

/*
  * Internal routine to write out a bucket.
  */
void index_CWrite(struct Index *ai, struct indexBucket *ab)
{
    FILE *tf;

    tf = index_HashOpen(ai, ab->hashIndex, 1);
    if (tf == 0) return;
    index_WriteIndex(tf, ab);
    fclose(tf);
    ab->modified = 0;
}

/*
  * Internal routine: Clear out the contents of an index file.  This routine does not
  * delete the directory itself, but does delete all of its contents.
  */
static void Purge(char *apath)
{
    DIR *td;
    DIRENT_TYPE *tde;
    char buffer[1024];
    struct stat tstat;
    long code;

    td = opendir(apath);
    if (!td) return;
    while((tde=readdir(td))) {
	strcpy(buffer, apath);
	strcat(buffer, "/");
	strcat(buffer, tde->d_name);
	if (strcmp(tde->d_name, ".") == 0 || strcmp(tde->d_name, "..") == 0)
	    continue;
	code = stat(buffer, &tstat);
	if (code == 0) {
	    if ((tstat.st_mode & S_IFMT) == S_IFDIR) continue;
	}
	else continue;
	unlink(buffer);
    }
    closedir(td);
}

/*
  * Create a new index file, with the appropriate hash table size.  If the index file
  * already exists, it is cleared out, with any data in the index being lost.
  */
int index_Create(char *apath, long aHashSize)
{
    char tbuffer[1024], sb[20];
    long i;
    FILE *tfile;
    Purge(apath);		/* clear out old junk */
    mkdir(apath, 0755);
    strcpy(tbuffer, apath);
    sprintf(sb, "/V%ld.%d", aHashSize, INDEXVERSION);
    strcat(tbuffer, sb);
    tfile = fopen(tbuffer, "w+");
    if (tfile == (FILE*) 0) return INDEXNOENT;
    fclose(tfile);
    for(i=0;i<aHashSize;i++) {
	strcpy(tbuffer, apath);
	sprintf(sb, "/H%ld", i);
	strcat(tbuffer, sb);
	tfile = fopen(tbuffer, "w+");
	if (tfile == (FILE *) 0) return 1;
	writeInteger(tfile, 0);
	fclose(tfile);
    }
    return 0;
}
