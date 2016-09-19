/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1993 - All Rights Reserved
\* ********************************************************************** */

/* nesst.c

	test the ness object
*/


#include <andrewos.h>

#include <ness.H>


int main(int  argc, char  **argv);
void printdata(class ness  *dobj);


int main(int  argc, char  **argv) {
	class ness *dobj;
	boolean debug = TRUE;

	FILE *f;
	char *fnm;
	long i, id;

	long *lstar = (long *)malloc(4*sizeof(long));
	lstar[0] = 10;  lstar[1] = 11;  lstar[2] = 12;
	printf("0x%lx  0x%lx  %d  %d\n", 
		lstar, &lstar[1], lstar[1], lstar[2]);
	fflush(stdout);
    
	printf("Start\n"); fflush(stdout);
/* not needed anymore
	ATK_Init(AndrewDir("/dlib/atk")); 
*/

	printf("Init done\n"); fflush(stdout);

	fnm = NULL;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-')
			switch (argv[i][1]) {
				case 'f': /* Debugging. */
					debug = FALSE;
					break;
				default: /* Unknown switch. Treat it as a file... */
					fnm = argv[i]+1;
					break;
			}
		else { /* Its a file right? */
			fnm = argv[i];
		}
	}

	printf("fnm: %s\n", fnm); fflush(stdout);

	dobj = new ness;
	printdata(dobj);

	/* $$$ establish some initial contents for the data object */
	if (fnm && (f=fopen(fnm, "r")))
		(dobj)->Read(f, 0);
	else {
		static char *small = 
			"function main(arg) printline(arg); end function";
		(dobj)->InsertCharacters( 0, small, strlen(small));
	}

	printdata(dobj);
	printf("writing t1\n");  fflush(stdout);
	(dobj)->Write( f=fopen("/tmp/t1", "w"), 12, 0);
	fclose(f);
	(dobj)->Destroy();
	dobj = new ness;
	printf("reading t1\n");  fflush(stdout);
	f = fopen("/tmp/t1", "r");
	fscanf(f, "\\begindata{ness,%d} ", &id);
	printf("read: %d\n",(dobj)->Read( f, id));
	fclose(f);
	printdata(dobj);
	printf("writing t2\n");  fflush(stdout);
	(dobj)->Write( f=fopen("/tmp/t2", "w"), 12, 0);
	return 0;
}

	void 
printdata(class ness  *dobj) {
	printf("Origin: %s\n", dobj->Origin);
	printf("OriginalModValue: %ld\n", dobj->OriginalModValue);
	printf("hasWarningText: %ld\n", dobj->hasWarningText);
	printf("ScriptLoc: %ld\n", dobj->ScriptLoc);
	printf("AfterScriptLoc: %ld\n", dobj->AfterScriptLoc);
	printf("DeauthButtonLoc: %ld\n", dobj->DeauthButtonLoc);
	printf("ScanButtonLoc: %ld\n", dobj->ScanButtonLoc);
	printf("CompileButtonLoc: %ld\n", dobj->CompileButtonLoc);
	printf("accesslevel: %ld\n", dobj->accesslevel);
	printf("DisplayDialogBox: %ld\n", dobj->DisplayDialogBox);
	fflush(stdout);
}

