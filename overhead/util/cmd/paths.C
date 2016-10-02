/* Copyright 1993 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>

#include <util.h>

static const char usage[]= "usage: andrewdirs [andrewdir] [xlibdir] [afsbasedir] [localdir]\n";
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
