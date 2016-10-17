/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987, 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
playtest.c

	test the play package
*/


#include <stdio.h>
#include <ATK.H>
#include <text.H>

#include <play.H>	

#include <filetype.H>
#include <attribs.h>

#include <observable.H>
#include <proctable.H>
#include <dataobject.H>


class text *text;	/* the source text */


int main(int argc, const char **argv)
{
	FILE *inputfile;

	long objectID;
	char *objectType;
	struct attributes *attributes;
	char tuning[] = "E";
	boolean playtones = FALSE;

	doStaticLoads();

	char buf[500];
	long i, max;

	while (argc > 1  &&  *(argv[1]) == '-') {
		argv++;
		argc --;
		switch (*(argv[0]+1)) {
		case 't':	playtones = TRUE;   break;
		case 'E':  *tuning = 'E';  break;
		case 'J':  *tuning = 'J';  break;
		case 'M':  *tuning = 'M';  break;
		case 'P':  *tuning = 'P';  break;
		}
	}

	text = new class text;

	inputfile = 0;

	if (argc == 2 || (inputfile=fopen(argv[1], "r" )) == NULL) {
		printf("File %s not found\n", argv[1]);
	}

	attributes = NULL;
	if (inputfile && (objectType = filetype::Lookup(inputfile, argv[1], &objectID, &attributes))
				!= NULL) 
		/* NULL means use default object. Text in this case. */
		/* If not default, make sure that the object type of the file is compatible
		 * with text.  */
		if (!ATK::IsTypeByName(objectType, "text")) {
			fprintf(stderr, "File is not a text object, its a %s\n", objectType);
			exit(1);
		}
	if (attributes != NULL)
		/* Gets things like readonly and others. Omit if you don't need the
		 * attributes. (You can pass NULL into filetype_Lookup instead of the
		 * attributes parameter.  */
		text->SetAttributes(attributes);

	play::Volume(3);
	play::Tuning(tuning);
	play::Retard(0);

	printf("inputfile: %p\n", inputfile);

	if (inputfile && playtones) {
		/* each line of input file specifies a tone with
			frequency duration(msec) volume(0..9) */
		double freq;
		long dura, vol;
		while (fscanf(inputfile, "%lf %ld %ld", &freq, &dura, &vol) == 3) {
			play::Tone(freq, dura, vol);
		}
	}
	else if (inputfile) {
		text->Read(inputfile, objectID);
		max = text->GetLength();
		if (max > (int)sizeof(buf) - 2) max = sizeof(buf) -2;
		for (i = 0; i < max; i ++)
			buf[i] = text->GetChar(i);
		buf[i] = '\0';
		play::Notes(buf);
	}
	else
		/* play three blind mice */
		play::Notes("T180 L2 E D C  P4     E D C P4  \
			G L4 F F L2 E P4    G L4 F F L2 E P4  \
			L4 G >C   C < B A B  >C<  G G P4  \
			G >C   C < B A B  >C<   G G P4  \
			G L1 E D C");
        /* hacky wait for queue to flush before exiting */
	play::Notes("P128 P128 P128 P128 P128");
}
