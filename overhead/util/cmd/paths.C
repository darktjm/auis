/* Copyright 1993 Carnegie Mellon University All rights reserved.
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

#include <andrewos.h>

#include <util.h>

static const char usage[]= "usage: andrewdirs [andrewdir] [xlibdir] [afsbasedir] [localdir] [andydir]\n";
int main(int argc, char **argv)
{
    int i, len;
    const char *r=NULL;
    if(argc<2) {
	fprintf(stderr, "andrewdirs: warning: no dirs requested!\n");
	fprintf(stderr, usage);
	exit(1);
    }
    for(i=1;i<argc;i++) {
	if(strcmp(argv[i], "andrewdir")==0) r=AndrewDir("");
	else if(strcmp(argv[i], "afsbasedir")==0) {
	    r=getenv("AFSBASEDIR");
	    if(r==NULL) r=AFSBASEDIR;
	}
	else if (strcmp(argv[i], "xlibdir")==0) r=XLibDir("");
	else if (strcmp(argv[i], "localdir")==0) r=LocalDir("");
	else if (strcmp(argv[i], "andydir")==0) r=AndyDir("");
	else {
	    fprintf(stderr, usage);
	    return 1;
	}
	if (!r) continue;
	len = strlen(r);
	printf("%s%s\n", r, ((len>0 && r[len-1] != '/') ? "/" : ""));
    }
    return 0;
}
