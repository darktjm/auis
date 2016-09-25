/* ppm2atkimage.C -- convert from PPM format data to ATK image format. */

/*
	Copyright Carnegie Mellon University 1992,1994 - All rights reserved
*/

#include <andrewos.h> /* strings.h */
#include <stdio.h>
#include <ctype.h>
#include <imageio.H>
#include <im.H>

int main(int  argc, char  **argv)
        {
    long saveQuality = -1;
    class imageio *self;
    FILE *f = stdin;
    const char *saveformat = NULL;
    boolean qualityComing = FALSE;

    if(argc > 1) {
	char *arg;
	while(argc > 1) {
	    arg = argv[--argc];
	    switch(*arg) {
		case '-':
		    switch(*(arg+1)) {
			    case 'g':
			    case 'G':
				if( strncmp(arg+1, "GIF", 3) == 0 ||
				   strncmp(arg+1, "gif", 3) == 0) 
				    saveformat = "gif";
				break;
			    case 'j':
			    case 'J':
				if( strncmp(arg+1, "jpeg", 3) == 0 ||
				   strncmp(arg+1, "JPEG", 3))
				    saveformat = "jpeg";
				break;
			    case 'q':
				if(*(arg+2) != (char) 0)
				    saveQuality = atoi(arg+2);
				else qualityComing = TRUE;
				break;
			    default:
				break;
		    }
		    break;
		default:
		    if(qualityComing) {
			saveQuality = atoi(arg);
		    }
		    else {
			if((f = fopen(arg, "r")) == NULL) {
			    fprintf(stderr, "ppm2atkimage: couldn't open %s for reading.\n", arg);
			    exit(-1);
			}
		    }
		    break;
	    }
	}	
    }

    ATKregister(imageio);
    ATK::LoadClass("imageio");
    if((self = new imageio)->Load( NULL, f) == 0) {
	if(saveQuality > 0)
	    (self)->SetJPEGSaveQuality( saveQuality);
	if(saveformat)
	    (self)->SetSaveFormatString( saveformat);
	if((self)->Write( stdout, im::GetWriteID(), -1) == dataobject_NOREADERROR)
	    exit(0);
    }
    else {
	exit(-1);
    }
}
