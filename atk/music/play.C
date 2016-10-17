/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* play.c
	proctable entry for playing music on RT and RS/6000

	some bits are from pcontrol.c

	Tempo is expressed in the number of quarter notes per minute.
	Note durations are computed in milliseconds.
	The duration sent to speaker is in 1/128 th seconds.
	At a tempo of 120, a quarter note takes 500 milliseconds.
	The Retard value is applied after all other duration computations.

	The global Volume is added to the local volume, according to volTable.
	Volume values are entered in the range 0...9, but are coded down to
	the range 0...3 for the RT.
*/

#include <andrewos.h>
#include <stdio.h>

#include <ATK.H>
#include <proctable.H>
#include <play.H>

#include <portaudio.h>
#include <math.h>

ATKdefineRegistry(play, ATK, play::InitializeClass);

#define SAMPLE_SIZE 100
#define BUFFER_SAMPLES 500

static double sample[SAMPLE_SIZE];
// tjm - fixme: assume short == int16 for now
static short buffer[BUFFER_SAMPLES];

#define WHOLENOTE  2000	/* milliseconds in a whole note at tempo 120 */

static double freq[85];		/* frequencies for various notes (0 is rest) */

static PaStreamParameters stream_desc = {};
static PaStream *stream = NULL;
static double sample_rate;

static long Retard;		/* slow down the music
			Applied after all other note duration computation.
			The note length becomes  length(1+Retard/50) */
static long Volume;		/* global volume */

static unsigned int speak_refcnt = 0;

	boolean
play::OpenSpeaker(long vol)
{
	ATKinit;

	if(speak_refcnt)
		return TRUE;
	PaError err = Pa_OpenStream(&stream, NULL, &stream_desc, sample_rate,
				    BUFFER_SAMPLES, paClipOff, NULL, NULL);
	if(err != paNoError) {
		fprintf(stderr, "PortAudio failed to open stream (%d): %s\n",
			(int)err, Pa_GetErrorText(err));
		return FALSE;
	}
	err = Pa_StartStream(stream);
	if(err != paNoError) {
		fprintf(stderr, "PortAudio failed to open stream (%d): %s\n",
			(int)err, Pa_GetErrorText(err));
		Pa_CloseStream(stream);
		return FALSE;
	}
	speak_refcnt++;
	return TRUE;
}

void play::CloseSpeaker(void)
{
	ATKinit;

	if(!speak_refcnt || --speak_refcnt)
		return;
	PaError err = Pa_StopStream(stream);
	if(err != paNoError)
		fprintf(stderr, "PortAudio failed to stop stream (%d): %s\n",
			(int)err, Pa_GetErrorText(err));
	err = Pa_CloseStream(stream);
	if(err != paNoError)
		fprintf(stderr, "PortAudio failed to close stream (%d): %s\n",
			(int)err, Pa_GetErrorText(err));
	speak_refcnt--;
}

/* play(note, length, tempo, vol)
	plays a single note of the given length and volume
	note is 0..84.  0 is rest;  1..84 are keys (O0C .. O6B)
	length is duration in milliseconds at tempo 120
*/
	void
play::PlayOneNote(double freq, long length, long tempo, long vol)
{
	ATKinit;

	if(!tempo || !length || !speak_refcnt)
		return;
	double act_len = (double) length * 120 / tempo * (1.0 + (double)::Retard / 50);
	int bufcount = sample_rate * act_len / 1000 / BUFFER_SAMPLES;
	double sinpos = 0.0, freq_skip;
	// old code had weird volume mapping table.  I'm just going to scale
	// incoming volume by global volume, but set 0 to 0.
	double vol_adj;
	if(!vol)
		vol_adj = 0.0;
	else
		// too quiet at vol == 3/3
		// vol_adj = pow(10.0, vol - 9) * pow(10.0, ::Volume - 9);
		vol_adj = (double)(::Volume + 1) * vol / 90;

	if(freq > 0.0 && vol_adj > 0.0)
		freq_skip = freq / (sample_rate / SAMPLE_SIZE);
	else {
		freq_skip = 0.0;
		memset(buffer, 0, sizeof(buffer));
	}

	for(int i = 0; i < bufcount; i++) {
		if(freq > 0.0 && vol_adj > 0.0)
			for(int j = 0; j < BUFFER_SAMPLES; j++) {
				buffer[j] = 32768 * vol_adj * sample[(int)sinpos];
				sinpos += freq_skip;
				if((int)sinpos >= SAMPLE_SIZE)
					sinpos = 0.0;
			}
		PaError err = Pa_WriteStream(stream, buffer, BUFFER_SAMPLES);
		if(err != paNoError) {
			fprintf(stderr, "PortAudio failed to write to stream (%d): %s\n",
				(int)err, Pa_GetErrorText(err));
			return;
		}
	}
}


enum interval {I76, I77, I90, I92, I100, I112, I114, I117};

	/* cent: 1.00057778950655485929   1200'th root of 2 */
static const double Ifactor[] = {
	1.04487715286087075242,		/* cent ^ 76 */
	1.04548087191543268121,		/* cent ^ 77 */
	1.05336103595483586152,		/* cent ^ 90 */
	1.05457862951601300325,		/* cent ^ 92 */
	1.05946309435929526455,		/* cent ^ 100  12'th root of 2 */
	1.06683224294535754535,		/* cent ^ 112 */
	1.06806540804785154947,		/* cent ^ 114 */
	1.06991782890027806748,		/* cent ^ 117 */
};

static const enum interval *TemperTable;	/* points to one of the following four 
					temper tables */

/* each temper table is a list of the (geometric) intervals between 
   successive keys of the chromatic scale, starting with C. 
An entry in the table is an index into Ifactor, which gives the factor. */

static const enum interval Pythagorean[] = {
	I90, I114, I90, I90, I114, I90, I114, I90, I114, I90, I90, I114
}; 

static const enum interval JustIntonation[] = {
	I112, I92, I112, I90, I92, I112, I92, I112, I92, I90, I112, I92
}; 	/* also called Ptolmeaic */

static const enum interval MeanTone[] = {
	I117, I76, I117, I117, I76, I117, I76, I117, I76, I117, I117, I76
}; 	 /* sum is 1199.  octaves are shorter */

static const enum interval EqualTemper[] = {
	I100, I100, I100, I100, I100, I100, I100, I100, I100, I100, I100, I100
}; 

/* InitFreq(tbl)
	initialize frequency table according TemperTable
*/
	static void
InitFreq(const enum interval *tbl)
{
	long note;
	if (tbl == TemperTable) 
		return;
	TemperTable = tbl;
	/* start at A (note 46) = 440 and work in both directions */
	freq[46] = 440.0;
	for (note=47; note <= 84; note++)
		freq[note] = freq[note-1] 
				* Ifactor[(long)TemperTable[(note-2)%12]];
	for (note=45; note>=1; note--)
		freq[note] = freq[note+1] 
				/ Ifactor[(long)TemperTable[(note-1)%12]];
	freq[0] = 0.0;
}

	void
play::Volume(long v)
{
	ATKinit;

	if (v < 0) ::Volume = 0;
	else if (v > 9) ::Volume = 9;
	else ::Volume = v;
}

	void
play::Retard(long v)
{
	ATKinit;

	if (v < -30) ::Retard = -30;
	else if (v > 40) ::Retard = 40;
	else ::Retard = v;
}

/* play::Tuning(name)
	choose tuning from among Pythagorean, Just, Mean, or Equal 
	(only the first letter is tested) 
*/
	void
play::Tuning(const char *name)
{
	ATKinit;

	switch (*name) {
	case 'E':		InitFreq(EqualTemper);	break;
	case 'P':		InitFreq(Pythagorean);	break;
	case 'J':		InitFreq(JustIntonation);	break;
	case 'M':	InitFreq(MeanTone);	break;
	}		
}

	static void
play_tone(double freq, long duration)
{
	long vol = 3;
	if (duration < 0) return;
	if (duration > 10000) 
		/* if more than 10 seconds, give 5 seconds */
		duration = 5000;
	if ( ! play::OpenSpeaker(vol))
		return;
	play::PlayOneNote(freq, duration, 120, vol);
	play::CloseSpeaker();
}

	void
play::Tone(double freq, long duration, long volume)
{
	ATKinit;

	if (volume < 0) volume = 0;
	else if (volume > 9) volume = 9;
	if (duration < 0) return;
	if (duration > 10000) duration = 5000;	/* if more than 10 seconds, give 5 seconds */
	if ( ! OpenSpeaker(volume))
		return;
	PlayOneNote(freq, duration, 120, volume);
	CloseSpeaker();
}



	static long 
getint(const char **codepp, long min, long max)
{
	long v;
	char c;

	v = 0;
	if (*((*codepp)+1) == '=') {
		/* skip the Basic   =var;   phrase */
		while (TRUE) {
			c = *((*codepp) + 1);
			if (c == '\0'  ||  c == ';') break;
			(*codepp)++;
		}
		return min;
	}
	while (TRUE) {
		c = *((*codepp) + 1);
		if (c == '\0'  ||  c < '0'  ||  c > '9') break;
		v = 10 * v  +  c - '0';
		(*codepp)++;
	}

	if (v < min) return min;
	if (v > max) return max;
	return v;
}

/* here are the possible notes: 
	"C", 1,	"C#", 2,	"C+", 2,
	"D-", 2,	"D", 3,	"D#", 4,	"D+", 4,
	"E-", 4,	"E", 5,
	"F", 6,	"F#", 7,	"F+", 7,
	"G-", 7,	"G", 8,	"G#", 9,	"G+", 9,
	"A-", 9,	"A", 10,	"A#", 11,	"A+", 11,
	"B-", 11,	"B", 12
*/

/* for each letter, this table gives the basic note */
static const long NoteVal[] = {
	/* A */	10, 
	/* B */	12, 
	/* C */	1, 
	/* D */	3, 
	/* E */	5, 
	/* F */	6, 
	/* G */	8
};


/* getnote(codepp, lenp)
	*codep points to a note letter
	lenp points to a temporary containing its intended length
	check for subsequent  #, +, -, and .    advancing codepp
	adjust note and length
*/
	static long
getnote(const char **codepp, long *lenp)
{
	long note;
	char c;

	note = NoteVal[**codepp - 'A'];
	c = *((*codepp) + 1);
	if (c == '#'  ||  c == '+') {
		note++;
		(*codepp)++;
	}
	else if (c == '-') {
		note--;
		(*codepp)++;
	}

	while (*((*codepp) + 1) == '.') {
		*lenp  =  *lenp * 3 / 2;
		(*codepp)++;
	}
	return note;
}
	
/* play_notes(s)
	 Play the sequence of notes in the string s.
	The string has codes as defined for Microsoft BASIC:

		Ox		(capital Oh) choose octave
				each octave starts at C and ascends to B
				O3 starts with middle C
				(0 <= x <= 6)  default 4
		<		change to next lower octave		
		>		change to next higher octave

		Lx		length of note is 1/x
				x is any value with 1 <= x <= 64
				default 4
			L1	whole note
			L2	half note
			L4	quarter note
			L8	eighth note
			L16 L32 L64  etc.

		Px		(Pause) rest for length x,
				where x is as for Lx

		C, C#, D, D#, E, F, F#, G, G#, A, A#, B
				play the note in the current octave
				with the current volume and duration.
				C# may be written as C+ or D-
				and similarly for E#, F#, G#, and A# 

		. 		immediately after a note, multiples its duration
				by 3/2 (multiple dots repeat the multiplication)

		Tx		Tempo.  sets the number of quarter notes in a
				minute (32 <= x <= 255)  default 120

		Nx		Note.  plays x'th note  (0 <= x <= 84) 
				n=0 is rest.  Notes 1-12 are in octave 0,
				13-24 in octave 1, ...,  73-84 in octave 6
				middle C is note 37

		Vx		relative volume for subsequent notes
				(0 <= x <= 9)  default 3
				(the RT/PC only distinguishes three levels)

		Mc		ignored (where c is one character)
		Xvariable;	ignored

	spaces newlines tabs and other undefined characters are ignored
	if any x above is of form  =variable;  it is ignored
 */
	void
play::Notes(const char *s)
{
	ATKinit;

	const char *codep;	/* points to string being sounded */
	long 	length;		/* milliseconds per note */
	long	octave;		/* 0 <= octave <= 6:  3 C is middle C */
	long	vol;		/* local volume   0 <= vol <= 3 */
	long tempo;		/* number of quarter notes per minute */
	long t, tlen;		/* temporaries */
	
	tempo = 120;	
	length = WHOLENOTE / 4;	/* at tempo 120, a quarter note is 500 milliseconds */
	octave = 4;
	vol = 3;

	if ( ! OpenSpeaker(vol)) 
		return;

	for (codep = s; *codep != '\0'; codep ++) {
		switch (*codep) {

		case 'O':	octave = getint(&codep, 0, 6);	break;

		case '>':	if (octave < 6) octave++;  		break;
		case '<':	if (octave >0 ) octave--;  		break;

		case 'L': length = WHOLENOTE
				 / getint(&codep, 1, 64);	break;

		case 'P': PlayOneNote(freq[0], WHOLENOTE 
					/ getint(&codep, 1, 64), 
				tempo, 0);		break;

		case 'C':  case 'D':  case 'E':  case 'F':  
		case 'G':  case 'A':  case 'B': 
			tlen = length;
			t = getnote(&codep, &tlen);
			PlayOneNote(freq[t + 12 * octave], 
				tlen, tempo, vol);
			PlayOneNote(freq[0], 1, tempo, 0); break;

		case 'T':	 tempo = getint(&codep, 32, 255);	break;

		case 'N':  PlayOneNote(freq[getint(&codep, 0, 84)], 
				length, tempo, vol);		break;

		case 'V':	 vol = getint(&codep, 0, 9);		
				break;

		case'M':	 codep++;			break;
		case 'X':  while (*(codep+1) && *(codep+1) != ';')
					codep++;		break;
		}
	}
	CloseSpeaker();
}

	boolean
play::InitializeClass()
{
	proctable::DefineProc("play-volume",(proctable_fptr)play::Volume, NULL, NULL,
		"set global volume for playing music   (arg is int)");
	proctable::DefineProc("play-retard",(proctable_fptr)play::Retard, NULL, NULL,
		"set retard factor for playing music   (arg is int)");
	proctable::DefineProc("play-tuning",(proctable_fptr)play::Tuning, NULL, NULL,
		"set tuning   (arg is \"E\", \"J\", \"M\", or \"P\")");
	proctable::DefineProc("play-tone",(proctable_fptr)play_tone, NULL, NULL,
		"play a single tone (args are {double}freq  &  {long}duration in 128'ths sec");
	proctable::DefineProc("play-notes",(proctable_fptr)play::Notes, NULL, NULL,
		"play music   (arg is string of codes)");

	::Retard = 0;	/* normal speed */
	::Volume = 3;
	InitFreq(EqualTemper);

	PaError err = Pa_Initialize();
	if(err != paNoError) {
		fprintf(stderr, "PortAudio failed to initialize (%d): %s\n",
			(int)err, Pa_GetErrorText(err));
		return FALSE;
	}
	if((stream_desc.device = Pa_GetDefaultOutputDevice()) == paNoDevice) {
		fputs("No default PortAudio output device\n", stderr);
		return FALSE;
	}

	stream_desc.channelCount = 1; /* mono is fine for a beep */
	stream_desc.sampleFormat = paInt16;
	stream_desc.suggestedLatency =
		/* beeps, not video games */
		Pa_GetDeviceInfo(stream_desc.device)->defaultHighOutputLatency;
	sample_rate = Pa_GetDeviceInfo(stream_desc.device)->defaultSampleRate;

	/* sample is 1 full sine wave */
	/* maybe a square wave would be more accurate (it'd definitely be easier) */
	for(int i = 0; i < SAMPLE_SIZE; i++)
		sample[i] = sin(2 * M_PI * i / SAMPLE_SIZE);
	return TRUE;
}
