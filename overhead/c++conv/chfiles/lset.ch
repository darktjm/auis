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


 

#define STRING 1
#define LONG 2
#define FLOAT 3

class lset : dataobject[dataobj] {
overrides:
    Read (FILE *file, long id) returns long;
    Write (FILE *file, long writeid, int level) returns long;
    GetModified() returns long;
classprocedures:
    InitializeClass() returns boolean;
    InitializeObject(struct lset *self) returns boolean;
methods:
    InsertObject (char *name,char *viewname);
    registername(char *rf) returns char *;
data:
	int type;
	int pct;
	char dataname[32];
	char viewname[32];
	char refname[64];
    struct dataobject *dobj;
    struct dataobject *left,*right;
    int application;
    struct text *pdoc;
    int revision;
};
