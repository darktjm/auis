/* ppm2atkimage.C -- convert from PPM format data to ATK image format. */

/*
	Copyright Carnegie Mellon University 1992,1994 - All rights reserved
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

#include <andrewos.h> /* strings.h */
#include <stdio.h>
#include <ctype.h>
#include <pbm.H>
#include <gif.H>
#include <jpeg.H>
#include <im.H>

int main(int  argc, char  **argv)
        {
    long ret, saveQuality = -1;
    class pbm *self;
    FILE *f = stdin;
    const char *saveformat = NULL;
    boolean qualityComing = FALSE;

    if(argc > 1) {
	char *arg, *lastarg = NULL;
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
	    lastarg = arg;
	}	
    }

    ATKregister(pbm);

/* Need the following because we might have to Load either one as specified by the SaveFormat preference or arg to this program. */
    ATKregister(gif);
    ATKregister(jpeg);

    ATK::LoadClass("pbm");
    if((self = new pbm)->Load( NULL, f) == 0) {
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
