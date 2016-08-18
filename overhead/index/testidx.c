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

#include <andrewos.h> /* sys/types.h sys/file.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/index/RCS/testidx.c,v 2.10 1993/07/02 14:30:39 rr2b Stab74 $";
#endif


 

#include "index.h"

static listerproc(ai, ac, arock)
register struct Index *ai;
register struct indexComponent *ac;
register char *arock;
{
    if ((long) arock != 17) printf("arock was trashed\n");
    printf("Record keyed by %s\n", ac->name);
}

main(argc, argv)
int argc;
char **argv;
{
    if (argc < 2) return printf("test: usage is 'test <opcode> <operands>\n");
    if (!strcmp(argv[1], "cr")) {
	index_Create(argv[2], 25);
    }
    else if (!strcmp(argv[1], "ap")) {
	register struct Index *idx;
	idx = index_Open(argv[2]);
	if (!idx) return printf("open failed\n");
	index_AddPrimary(idx, argv[3], argv[4]);
	index_Close(idx);
    }
    else if (!strcmp(argv[1], "ls")) {
	register struct Index *idx;
	idx = index_Open(argv[2]);
	if (!idx) return printf("open failed\n");
	index_Enumerate(idx, listerproc, (char *) 17);
    }
    else if (!strcmp(argv[1], "as")) {
	register struct Index *idx;
	register struct recordSet *ts;
	idx = index_Open(argv[2]);
	if (!idx) return printf("open failed\n");
	ts = index_GetPrimarySet(idx, argv[3]);
	if (ts->count == 0) printf("no records match key %s\n", argv[3]);
	else if (ts->count != 1) printf("ambiguous primary key %s\n", argv[3]);
	else {
	    index_AddSecondary(idx, &ts->data[0], argv[4]);
	}
	recordset_Free(ts);
	index_Close(idx);
    }
    else if (!strcmp(argv[1], "pr")) {
	register struct Index *idx;
	register struct recordSet *ts;
	register long i, code;
	char buffer[1024];

	idx = index_Open(argv[2]);
	if (!idx) return printf("open failed\n");
	ts = index_GetAnySet(idx, argv[3]);
	printf("Printing individual records:\n");
	for(i=0;i<ts->count;i++) {
	    printf("Record %ld.%ld", ts->data[i].word1, ts->data[i].word2);
	    code = index_GetData(idx, &ts->data[i], buffer, sizeof(buffer));
	    if (code) {
		printf("...failed with code %ld\n", code);
	    }
	    else printf(" data='%s'\n", buffer);
	}
	recordset_Free(ts);
	index_Close(idx);
    }
    else if (!strcmp(argv[1], "dp")) {
	register struct Index *idx;
	register struct recordSet *ts;
	register long i;
	idx = index_Open(argv[2]);
	if (!idx) return printf("open failed\n");
	ts = index_GetPrimarySet(idx, argv[3]);
	if (ts->count == 0) printf("no records match key %s\n", argv[3]);
	else {
	    for(i = 0; i<ts->count;i++) {
		index_DeletePrimary(idx, &ts->data[i]);
	    }
	}
	recordset_Free(ts);
	index_Close(idx);	
    }
    else if (!strcmp(argv[1], "ds")) {
	register struct Index *idx;
	register struct recordSet *ts;
	register long i;
	idx = index_Open(argv[2]);
	if (!idx) return printf("open failed\n");
	ts = index_GetPrimarySet(idx, argv[3]);
	if (ts->count == 0) printf("no records match key %s\n", argv[3]);
	else {
	    for(i = 0; i<ts->count;i++) {
		index_DeleteSecondary(idx, &ts->data[i], argv[4]);
	    }
	}
	recordset_Free(ts);
	index_Close(idx);	
    }
    else if (!strcmp(argv[1], "dump")) {
	register struct Index *idx;
	idx = index_Open(argv[2]);
	if (!idx) return printf("open failed\n");
	index_Dump(idx);
	index_Close(idx);
    }
    else printf("bad opcode %s\n", argv[1]);
}
