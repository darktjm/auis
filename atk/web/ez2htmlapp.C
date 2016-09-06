/* File ez2htmlapp.C created by R S Kemmetmueller
	(c) Copyright IBM Corp 1995.  All rights reserved. 
	Copyright Carnegie Mellon University 1996 -- All rights reserved

   ez2htmlapp: converts datastream files to HTML+ tag format. 

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

#ifndef NORCSID
#define NORCSID
static UNUSED const char ibmid[] = 
 "(c) Copyright IBM Corp.  (This work is unpublished).  All rights reserved.";
#endif

ATK_IMPL("ez2htmlapp.H")

#include <sys/stat.h>
#include <util.h>
#include <environment.H>
#include <text.H>
#include <gif.H>
#include <filetype.H>
#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>
#include <environ.H>
#include <im.H>
#include <link.H>
#include <path.H>
#include <astring.H>
#include <gentext.H>
#include <content.H>
#include <mark.H>

#include <htmlenv.H>
#include <attlist.H>
#include <htmltext.H>
#include <hidden.H>
#include <ez2htmlapp.H>

static int insetctr = 0;
static const char *helptop_subst = NULL;
static boolean use_plaintext_tag = FALSE;

enum style_actions { 
	action_NOTHING=0,
	action_REMOVE=1, 
	action_REMOVEPARENTS=2, 
	action_REMOVETEXT=4,
	action_ENSUREDINGBAT=8,
	action_CONVERTHELPTOPIC=16,
	action_MAKEHR=32 
};

struct envlist {
	htmlenv *env;
	enum style_actions action;
	struct envlist *next;
};
static struct envlist *doomedenvs;

ATKdefineRegistry(ez2htmlapp, application, NULL);
static void cleanup(htmltext *txt);
static style *findOrCreateStyle(text *txt, const char *stylename);

static const char defaultWrapper[] = 
"<|! marker title |><|! marker body |> \n"
"<|! marker html |><|! marker CleanUphtml |> \n"
"\n"
"	<| html := |>"
"<html> \n"
"<head><title>  \n"
"<| clearstyles(title) |> \n"
"</title></head> \n"
"<body> \n"
"<| body |> \n"
"</body> </html> \n"
"<| CleanUp: |> \n";


ez2htmlapp::ez2htmlapp() {
	inputFile = NULL;
	objectType = NULL;
	outPath = NULL;
	outFileNm = NULL;
	gifDir = NULL;
	gifURL = NULL;
	gifFlag = ez2html_NOGIFS;
	wrapper = NULL;
	SetMajorVersion(2);
	SetMinorVersion(961008);
	SetFork(FALSE);
	helptop_subst = 
			environ::Get("HELPTOPICANCHORSUBST");
	if (!helptop_subst)
		helptop_subst = 
			environ::GetProfile("HelptopicAnchorSubst");
	if (helptop_subst && *helptop_subst == '\0')
		helptop_subst = NULL;
    	use_plaintext_tag = (environ::Get("USEPLAINTEXTTAG")) 
			? TRUE : FALSE;
	THROWONFAILURE(TRUE);
}

ez2htmlapp::~ez2htmlapp()  {
	if (outPath) free(outPath);
	if (inputFile) free(inputFile);
	if (objectType) free(objectType);
	if (outFileNm) free(outFileNm);
	if (gifDir) free(gifDir);
	if (gifURL) free(gifURL);
	if (wrapper) wrapper->Destroy();
	return;
}

/* override */
/* temporarily pretend we're EZ in order to pick up 
		.ezinit file extension mappings 
*/
	void 
ez2htmlapp::ReadInitFile() {
	const char *oldName=GetName();
	SetName("ez");
	application::ReadInitFile();
	SetName(oldName);
}

/* override */
/* don't bother with all the color initializations 
*/
	boolean 
ez2htmlapp::Start() {
	return TRUE;
}

	static void 
ShowUsage(ez2htmlapp *self) {
	fprintf(stderr, "Usage: %s [switches] [filename]\n", 
			(self)->GetName());
	fprintf(stderr, "\t-h   Print this message\n");
	fprintf(stderr, "\t-N   No gifs in output (default: N)\n");
	fprintf(stderr, "\t-G   Put gifs in output\n");
	fprintf(stderr, "\t-F   Delete old gifs, then put gifs in output\n");
	fprintf(stderr, "\t-R   Reuse old gifs\n");
	fprintf(stderr, "\t-f outfile  Specify destination path  (default: .)\n");
	fprintf(stderr, "\t-g gifdir   Path to store gifs (default: .)\n");
	fprintf(stderr, "\t-u gifURL   Prefix for URL referring to gifs\n");
	fprintf(stderr, "\t-o dataobj   Non-text dataobject input (default: text)\n");
	fprintf(stderr, "\t-w wrapper-file   Non-standard HTML wrapper\n");
	fprintf(stderr, "\t-S   Split into sections\n");
}

	boolean 
ez2htmlapp::ParseArgs(int argc, const char *argv[]) {
	char outFileArg[MAXPATHLEN+1];
	char wrapperFileArg[MAXPATHLEN+1];
	struct stat statBuf;
	char flagSw = ' ';
	char *tx;
	boolean ofaisdir = FALSE;		// outFileArg is as directory

	outFileArg[0]='\0';
	wrapperFileArg[0]='\0';

	if ( ! application::ParseArgs(argc,argv))
		return FALSE;

	if (outPath) free(outPath);
	if (inputFile) free(inputFile);
	if (wrapper) wrapper->Destroy();
	if (objectType) free(objectType);
	if (outFileNm) free(outFileNm);
	if (gifDir) free(gifDir);
	if (gifURL) free(gifURL);
	gifFlag = ez2html_NOGIFS;
	inputFile = NULL;
	wrapper = NULL;
	outFileNm = NULL;
	outPath = NULL;
	inputFile = NULL;
	gifDir = NULL;
	gifURL = NULL;
	Split = FALSE;
	splitctr = 0;

	/* parse command-line options */
	while (*++argv && (*argv)[0]=='-') {
		switch ((*argv)[1]) {
		case 'h':	// help
			ShowUsage(this);  return FALSE;
		case 'o':	// object type
			objectType = NewString(((*argv)[2]) ? (*argv)+2
					: (--argc, *++argv));  
			break;
		case 'f':	// output file prefix
			filetype::CanonicalizeFilename(outFileArg, 
				((*argv)[2]) ? (*argv)+2 : (--argc, *++argv), 
				MAXPATHLEN);
			break;
		case 'g': {		// gifDir
			const char *trel = ((*argv)[2]) ? (*argv)+2
					: (--argc, *++argv);
			int len = strlen(trel);
			gifDir = (char *)malloc(len+2);
			strcpy(gifDir, trel);
			if (gifDir[len-1] != '/')
				strcat(gifDir+len, "/");
			} break;
		case 'u': {		// gifURL
			const char *trel = ((*argv)[2]) ? (*argv)+2
					: (--argc, *++argv);
			int len = strlen(trel);
			gifURL = (char *)malloc(len+2);
			strcpy(gifURL, trel);
			if (gifURL[len-1] != '/')
				strcat(gifURL+len, "/");
			} break;
		case 'N':		// do not convert insets to gifs
			gifFlag = ez2html_NOGIFS;  
			flagSw = 'N';    break;
		case 'G':		// convert insets to new gifs; leave old gifs
			gifFlag = ez2html_GIFIFY;  
			flagSw = 'G';    break;
		case 'F':		// delete gif files in destination area
			gifFlag = ez2html_NOOLDGIFS;  
			flagSw = 'F';    break;
		case 'R':		// retain gif files in destination area
			gifFlag = ez2html_REUSEGIFS;  
			flagSw = 'R';    break;
		case 'S':		// split file as per Table Of Contents
			Split = TRUE;
			break;
		case 'w':		// use named wrapper file
			filetype::CanonicalizeFilename(wrapperFileArg, 
				((*argv)[2]) ? (*argv)+2 : (--argc, *++argv), 
				MAXPATHLEN);
			break;
		default:
			ShowUsage(this);
			fprintf(stderr, "\t(unrecognized switch %s)\n", *argv);
			return FALSE;
		}
	}
	if (*argv) {
		inputFile = (char *)malloc(MAXPATHLEN+1);
		filetype::CanonicalizeFilename(inputFile, *argv, 
				MAXPATHLEN);
	}

	// Create wrapper gentext
	if (*wrapperFileArg) {
		wrapper = gentext::CreateFromNamedFile(
						wrapperFileArg);
		if (wrapper->GetErrors()) {
			fprintf(stderr, 
				"*** Error(s) reading wrapper file %s. %s\n",
				wrapperFileArg, "Using default.");
			wrapper->PrintAllErrors(wrapperFileArg);
			*wrapperFileArg = '\0';
		}
	}
	if ( ! *wrapperFileArg) 
		wrapper = gentext::Create(defaultWrapper);
	
	// check if outFileArg is a directory
	if (*outFileArg && stat(outFileArg, &statBuf) >= 0 &&
			(statBuf.st_mode & S_IFMT) == S_IFDIR) {
		// outFileArg is a directory;  be sure it ends w/ slash
		tx = outFileArg + strlen(outFileArg);
		if (tx[-1] != '/') strcat(tx, "/");
		ofaisdir = TRUE;
	}

	if ((! *outFileArg || ofaisdir) && ! inputFile) {		if (gifFlag == ez2html_NOGIFS && ! Split) 
			return TRUE;  // will send output to stdout
		fprintf(stderr, 
				"With -%c%s, must have -f or infile\n", 
				flagSw, (Split) ? " and -S" : "");
		ShowUsage(this);
		return FALSE;
	}

	if ( ! *outFileArg)
		strcpy(outFileArg, inputFile);	

	// compute outPath from outFileArg
	tx = strrchr(outFileArg, '/');
	if (tx) {
		char t = tx[1];
		tx[1] = '\0';
		outPath = NewString(outFileArg);
		tx[1] = t;
		tx++;		// pt to outFileNm (if any)
	}
	else {
		outPath = NewString("");
		tx = outFileArg;  	// pt to outFileNm (if any)
	}

	if (! *tx) {
		// get outFileNm from inputFile
		tx = strrchr(inputFile, '/');
		if (tx) tx++;
		else tx = inputFile;
	}
	outFileNm = NewString(tx);

	//  Strip extension from outFileNm
	tx = strrchr(outFileNm, '.');
	if (tx && strlen(tx+1) <= 4 && ! strchr(tx+1, '/'))
		*tx = '\0';

	// check that files can be written to outPath
	ofaisdir = ofaisdir || (stat(outPath, &statBuf) >= 0 &&
			(statBuf.st_mode & S_IFMT) == S_IFDIR);
	if ( ! ofaisdir || access(outPath, W_OK) != 0) {
		fprintf(stderr, "Cannot write output to %s\n", outPath);
		return FALSE;
	}

	// check gifDir   (reuse variable outFileArg for another purpose)
	if ( ! gifDir) gifDir = NewString("./");
	if (*gifDir == '/')
		strcpy(outFileArg, gifDir);
	else
		sprintf(outFileArg, "%s%s", outPath, gifDir);
	if (stat(outFileArg, &statBuf) < 0 ||
			(statBuf.st_mode & S_IFMT) != S_IFDIR
			|| access(outFileArg, W_OK) != 0) {
		fprintf(stderr, "Cannot write gif files to %s\n",
				outFileArg);
		return FALSE;
	}

	// check gifURL
	if (*gifDir == '/' && ! gifURL) {
		fprintf(stderr, "Absolute gifdir specified (-g %s) -- %s.\n",
				gifDir, 
				"must specify gif URL with -u switch");
		return FALSE;
	}

	// send a messsage to stdout in case it is redirected
	printf("Sending output to %s%s.html\n", outPath, outFileNm);
	if (Split) 
		printf("and %s %s%s.NNN.html\n", 
				"split files to", outPath, outFileNm);
	if (gifFlag != ez2html_NOGIFS) {
		printf("and %s %s%s%s.NNN.gif\n", 
			(gifFlag == ez2html_REUSEGIFS)
				? "reusing gif files from": "gif files to", 
			(*gifDir == '/') ? "" : outPath, 
			gifDir, outFileNm);
	}

	return TRUE;
}


	static FILE *
OpenInfile(ez2htmlapp *self) {
	char filename[MAXPATHLEN];
	if ( ! self->inputFile) 
		/* read from stdin */
		return stdin;

	/* read from a file */
	struct stat statBuf;
	filetype::CanonicalizeFilename(filename, self->inputFile, 
				sizeof(filename)-1);
	if (stat(filename, &statBuf) >= 0) {
		if ((statBuf.st_mode & S_IFMT) == S_IFDIR) {
			fprintf(stderr, "%s: '%s' is a directory; %s\n", 
				self->GetName(), filename,
				"cannot convert to HTML.");
			return NULL;
		}
	} 
	else {
		fprintf(stderr, "%s: '%s' not found or unreadable.\n", 
				self->GetName(), filename);
		return NULL;
	}
	free(self->inputFile);
	self->inputFile = NewString(filename);

	return fopen(self->inputFile, "r");
}

	static text *
ReadInfile(ez2htmlapp *self, FILE *infile, boolean *iscode, boolean *isplain) {
	char objectName[256];
	long objID;
	struct attributes *attribs;
	text *txt;

	if ( ! self->objectType)
		self->objectType = filetype::Lookup(infile, self->inputFile, 
					&objID, &attribs);
	/* use a local string for object name 
			(pointer returned by filetype::Lookup is flaky) */
	if ( ! self->objectType) {
		fprintf(stderr, "%s: Input has indeterminate object type\n",
				self->GetName());
		return NULL;
	}
	strcpy(objectName, self->objectType);
	/* load the class, to make sure ATK::IsTypeByName 
				does not lie due to ignorance */
	ATK::LoadClass(objectName);
	if ( ! ATK::IsTypeByName(objectName, "text")) {
		fprintf(stderr, "%s: '%s' is not a textual data type%s\n", 
				self->GetName(), objectName, 
				", cannot convert to HTML.");
		return NULL;
	} 
	else if (objectName[0]=='\0') 
		/* assume plain ASCII, via stdin or something */
		strcpy(objectName, "text");
	/* create an input object and load the file into it */
	txt = (text *)ATK::NewObject(objectName);
	if ( ! txt) {
		fprintf(stderr, "%s: Could not create object of type '%s'\n", 
				self->GetName(), objectName);
		return NULL;
	}
	txt->SetAttributes(attribs);

	if (txt->Read(infile, objID) != dataobject_NOREADERROR) {
		fprintf(stderr, "%s: Failed reading %s\n", 
			self->GetName(),
			(self->inputFile) ? self->inputFile : "stdin");
		txt->Destroy();
		return NULL;
	}

	*isplain = (objID==0 || txt->IsType("rawtext") 
			|| txt->GetWriteStyle() == text_NoDataStream 
			|| ! txt->GetExportEnvironments());
	*iscode= (txt->IsType("srctext"));

	return txt;
}

	static FILE *
OpenOutput(ez2htmlapp *self, boolean *opened) {
	*opened = FALSE;
	if ( ! self->outPath)
		return stdout;
	FILE *outf;
	char outnm[MAXPATHLEN];
	sprintf(outnm, "%s%s.html", self->outPath, self->outFileNm);
	outf = fopen(outnm, "w");
	if ( ! outf) 
		fprintf(stderr, "%s, Unable to open output file: %s\n",
				self->GetName(), outnm);
	*opened = TRUE;
	return outf;
}

/* GifInfo -  read first thirteen bytes of gif file:  check for GIF8xa
	0-5:  "GIF8xa"  for x = 7 or 9
	6-7:	width (in DEC order)
	8-9:	height (ditto)
	10:	global colormap flag (&80)
		color bits ((&70 >> 4) + 1)
		pixelbits ((&7)+1)
			global ncolors is 1<<pixelbits
			if global colormap flag is 0, each enclosed gif
				has its own colormap
			(gif file format permits multiple gifs with local maps)
	11:	background pixel
	12:	00
*/
	static boolean 
GifInfo(char *filename, int *width, int *height, int *ncolors) {
	char buf[13];
	FILE *fgif = fopen(filename, "r");
	if ( ! fgif)  return FALSE;
	if (fread(buf, 1, 13, fgif) != 13 
			|| strncmp(buf, "GIF8", 4) != 0
			|| buf[5] != 'a'
			|| buf[12] != 0x0) 
		{fclose(fgif); return FALSE;}
	if (width) *width = (buf[7]<<8) | buf[6];
	if (height) *height = (buf[9]<<8) | buf[8];
	if (ncolors) 
		*ncolors = (buf[10]&80) ? 1 << ((buf[10]&7)+1) : 0;
	fclose(fgif);
	return TRUE;
}

#if 0
/* get a filename constructed from the given name using the fmt
	fmt must have a %s for the file name 
			and a %d for a uniquifying integer
  returns pointer to static storage containing name
	(access() returns 0 for success)
*/
	static char *
neighborfile(htmltext *self, char *file, char *fmt)  {
	static char filename[MAXPATHLEN];
	int code;
	while (TRUE) {
		sprintf(filename, fmt, file, self->destfilecounter++);
		code = access(filename, F_OK);
		if (code == 0)  continue;	// file exists
		if (errno == ENOENT) break;  // try this name
				// file does not exist, but directories are okay
		return NULL;	// some other error:
				// we cannot create a neighbor to 'file'
	}

	// check that directory is writable
	char *slash = strrchr(filename, '/');
	if (slash) {
		// check that directory is writable
		*slash = '\0';
		code = access(filename, W_OK);
		*slash = '/';
	}
	else code = access(".", W_OK);

	return (code == 0) ? filename : NULL;
}
#endif

static const char * const nogif[] = {
	"fnote",
	"bp",
	"hidden",
	"table",
	"urlbutton",
	"link",
//	"gif",
//	"img",
//	"timeoday",
	NULL
};

static boolean NeedsGIF(dataobject *d) {
    int i;
    for (i = 0; nogif[i] && ! d->IsType(nogif[i]); i++) {}
    return !nogif[i];
}


	static boolean
CountInsets(htmltext *self, long pos, htmlenv *env,
	    dataobject *dobj, arbval rock) {
	boolean needsgif=NeedsGIF(dobj);
	int *np = (int *)rock.obj;
	if (needsgif) (*np)++;
	return FALSE;
}

	static boolean
ConvertInsets(htmltext *htxt, long pos, htmlenv *env,
		dataobject *dobj, arbval rock) {
	boolean needsgif = NeedsGIF(dobj);
	char gifname[200], nmbuf[MAXPATHLEN], whatt[100];
	const char *wh;
	ez2htmlapp *cvtr = (ez2htmlapp *)rock.obj;

	if ( ! needsgif) 
		return FALSE;	// continue scanning insets

	sprintf(gifname, "%s.%d.gif", 
			cvtr->outFileNm, ++insetctr);
	sprintf(nmbuf, "%s%s%s", 
			(*cvtr->gifDir == '/') ? "" : cvtr->outPath, 
			cvtr->gifDir, gifname);

	switch (cvtr->gifFlag) {
	case ez2html_NOOLDGIFS:
	case ez2html_GIFIFY:
		wh = htxt->GififyInset(pos, env, nmbuf);
		if (*wh == '*') {
			fprintf(stderr, "Gifify ERROR: %s\n", wh);
			return FALSE;
		}
		break;
	case ez2html_REUSEGIFS: {
		int width, height;
		// convert inset to image for existing gif file
		if (GifInfo(nmbuf, &width, &height, NULL))
			sprintf(whatt, "WIDTH=%d HEIGHT=%d", 
					width, height);
		else {
			fprintf(stderr, "Failed to get size from %s\n",
				nmbuf);
			*whatt = '\0';
		}
		wh = whatt;
	}	break;
	case ez2html_NOGIFS:
		break;
	}

	// replace inset at pos with image object for gif

	// replace inset with image object having appropriate attrs
	char tags[MAXPATHLEN+100];
	if (cvtr->gifURL)
		sprintf(tags, "SRC=%s%s %s", 
				cvtr->gifURL, gifname, wh);
	else   // by construction if there is no gifURL, 
		// there is a gifdir and it is relative
		sprintf(tags, "SRC=%s%s %s", 
				 cvtr->gifDir, gifname, wh);
	attlist al;
	al.MakeFromString(tags);

	image *dat = new gif;
	if ( ! dat) 
		return FALSE;
	/* dat->Load(nmbuf);	// the gut of GetImage (not really needed) */

	htmlenv *ienv = (htmlenv *)htxt->AlwaysReplaceWithView(pos, 1,
				"htmlimagev", dat);
	if (ienv) ienv->SetAttribs(&al);
	htxt->NotifyObservers(0);
	return FALSE;
}


/* scan through insets and gifify according to switches
*/
	static void
ManageGifFiles(ez2htmlapp *self, htmltext* htxt) {
	DIR *dir;
	DIRENT_TYPE *fileent;
	boolean gotgif = FALSE;
	boolean deleting = (self->gifFlag == ez2html_NOOLDGIFS);
	char pathbuf[MAXPATHLEN];
	int nmlen, ninsets = 0;

	// initialize insetctr
	insetctr = 0;

	// set up dir name and file name prefix
	sprintf(pathbuf, "%s%s", 
				(*self->gifDir == '/')? "" : self->outPath, 
				self->gifDir);
	nmlen = strlen(self->outFileNm);

	// scan directory pathbuf for files named outFileNm.NNN.gif 
	dir = opendir(pathbuf);
	while ((fileent=readdir(dir))) {
		char *dot = fileent->d_name+nmlen;
		if (*dot != '.') continue;
		if (strncmp(self->outFileNm, fileent->d_name, nmlen) 
					!= 0)
			continue;
		int val = 0;
		for (dot++; isdigit(*dot); dot++) 
			val = 10*val+(*dot-'0');
		if (strcmp(dot, ".gif") != 0)
			continue;

		//  fileent->d_name is outFileNm.NNN.gif
		switch (self->gifFlag) {
		case ez2html_GIFIFY:	// gifify insets, new gifs
			if ( ! gotgif) {
				// this is first gif.  prompt for deletions
				gotgif = TRUE;
				// XXX prompt to see if exisitng gif s.b. deled
			}
			if ( ! deleting)  break;
			// DROP THRU to delete file
		case ez2html_NOOLDGIFS:  // delete existing gifs
			sprintf(pathbuf, "%s%s%s", 
				(*self->gifDir == '/') ? "" : self->outPath, 
				self->gifDir, fileent->d_name);
			remove(pathbuf);
			break;
		case ez2html_REUSEGIFS: // already done
		case ez2html_NOGIFS:    // nothing to do
		        break;
		}
		if ( ! deleting && val > insetctr) 
			insetctr = val;
	}

	if (self->gifFlag == ez2html_REUSEGIFS) {
		// count insets that need gifs
		arbval rock;
		rock.obj = (pointer)&ninsets;
		htxt->EnumerateInsets((htmlefptr)CountInsets, rock);
		if (insetctr != ninsets) {
			fprintf(stderr, "%s: %s '%s%s%s.NNN.gif'",
				self->GetName(), 
				"-R specified, but wrong number of files",
				(*self->gifDir == '/') ? "" : self->outPath, 
				self->gifDir, self->outFileNm);
			return;
		}
		insetctr = 0;	// restart cnt for actual output
	}
}

	static int
ConvertAndWrite(ez2htmlapp *self, htmltext *htxt) {

	// process files in gifdir
	ManageGifFiles(self, htxt);
	
	// ensure existence of an im (used in htmltext.C)
	im *realim = im::Create(NULL);
	if ( ! realim) {
		fprintf(stderr, "Cannot gifify without %s :-(don't ask)",
				"X windows");
		return 1;
	}
	realim->VanishWindow();

	// use a `content' object to find titles and sections (if possible)
	content c;
	c.SetSourceText(htxt);

	if ( ! c.entry) {
		// no title, no sections.  Write one file
		boolean opened;

		arbval rock;
		rock.obj = (pointer)self;
		htxt->EnumerateInsets((htmlefptr)ConvertInsets, rock);

		FILE *outfile = OpenOutput(self, &opened);
		if ( ! outfile)  return 1;
		AString s;
		FILE *temp = s.VfileOpen("w");
		htxt->Write(temp, 1, 0);
		s.VfileClose(temp);

		self->wrapper->SetVar("title", "");
		self->wrapper->SetVar("body", (char *)s);
		self->wrapper->Instantiate("");

		self->wrapper->WriteVarToFile("html", outfile);
		if (opened) fclose(outfile);
		else fflush(outfile);
		return 0;	// succeed
	}
	if ( ! self->Split || ! c.entry->next) {
		// there is a title and 
		//	no Split required or only one section:
		//	write to only one file
		boolean opened;

		arbval rock;
		rock.obj = (pointer)self;
		htxt->EnumerateInsets((htmlefptr)ConvertInsets, rock);

		FILE *outfile = OpenOutput(self, &opened);
		if ( ! outfile)  return 1;
		AString s;
		FILE *temp = s.VfileOpen("w");
		htxt->Write(temp, 1, 0);
		s.VfileClose(temp);

		// use the first title we find.
		// xxx probably bogus.  check proximity to file start
		mark *m = c.entry->rem;
		self->wrapper->SetVarFromText("title", htxt, 
			m->GetPos(), m->GetLength());
		self->wrapper->SetVar("body", (char *)s);
		self->wrapper->Instantiate("");

		self->wrapper->WriteVarToFile("html", outfile);
		if (opened) fclose(outfile);
		else fflush(outfile);
		return 0;	// succeed
	}
	// Split has been asked for 
	// 	and there are at least two sections
	// write one file per section
	struct content_chapentry *e;
	enum {First, Middle, Last} which = First;
	char nmbuf[3][300];
	char *prevnm = nmbuf[0], *currnm = nmbuf[1], *nextnm = nmbuf[2], *oldprevnm;
	htmltext thtxt;
	AString s;
	int estartloc = 0, elen;		// portion of htxt defined by e
	for (e = c.entry; e; e = e->next) {
		if ( ! e->next) which = Last;
		switch(which) {
		case First:
			prevnm = nmbuf[0];
			currnm = nmbuf[1];
			nextnm = nmbuf[2];
			*prevnm = '\0';
			sprintf(currnm, "%s.%d.html",  self->outFileNm,
					++self->splitctr);
			sprintf(nextnm, "%s.%d.html",  self->outFileNm,
					++self->splitctr);
			which = Middle;
			break;
		case Middle:
			oldprevnm = prevnm;
			prevnm = currnm;
			currnm = nextnm;
			nextnm = oldprevnm;
			sprintf(nextnm, "%s.%d.html", self->outFileNm,
					++self->splitctr);
			break;
		case Last:
			oldprevnm = prevnm;
			prevnm = currnm;
			currnm = nextnm;
			nextnm = oldprevnm;
			*nextnm = '\0';
			break;
		}

		// open output file
		char filename[MAXPATHLEN];
		strcpy(filename, self->outPath);
		strcat(filename, currnm);
		FILE *outfile = 	fopen(filename, "w");
		if ( ! outfile) return 1;

		// set title from first chapentry
		self->wrapper->SetVarFromText("title", htxt, 
			e->rem->GetPos(), e->rem->GetLength());

		// determine length of section `e'
		// arbitrarily merge short sections
		while (TRUE) {
			if ( ! e->next) {
				// last section.  get rest of document
				elen = htxt->GetLength() - estartloc;
				*nextnm = '\0';
				break;
			}
			elen = e->next->rem->GetPos() - estartloc;
			if (elen > 1000) break;	// XXX arb constant
			e = e->next;	
		}

		// copy section `e' to thtxt
		thtxt.ClearCompletely();
		thtxt.AlwaysCopyTextExactly(0, htxt, estartloc, elen);
		estartloc += elen;

		arbval rock;
		rock.obj = (pointer)self;
		thtxt.EnumerateInsets((htmlefptr)ConvertInsets, rock);

		// convert thtxt, writing to `s'
		FILE *temp = s.VfileOpen("w");
		thtxt.Write(temp, 1, 0);
		s.VfileClose(temp);

		// set vars for wrapper and Instantiate
		self->wrapper->SetVar("body", (char *)s);
		self->wrapper->SetVar("prevfilename", prevnm);
		self->wrapper->SetVar("filename", currnm);
		self->wrapper->SetVar("nextfilename", nextnm);
		self->wrapper->Instantiate("");

		// write var `html' to currnm	
		self->wrapper->WriteVarToFile("html", outfile);
		fclose(outfile);
	}

	// write overall index file
	boolean opened;
	FILE *outfile = OpenOutput(self, &opened);
	if ( ! outfile)  return 1;
	self->wrapper->Instantiate("CleanUp");
	self->wrapper->WriteVarToFile("cleanuphtml", outfile);
	if (opened) fclose(outfile);
	else fflush(outfile);
	
	return 0;	// succeed
}


	int 
ez2htmlapp::Run() {
	text *txt;
	FILE *infile;
	boolean isplain, iscode;
	int retval = 0;

	infile = OpenInfile(this);
	if ( ! infile)  return 1;

	txt = ReadInfile(this, infile, &iscode, &isplain);
	fclose(infile);
	if ( ! txt)  return 1;

	// the input has been read into txt

	// output the best HTML we can muster 

	if (isplain && use_plaintext_tag) {
		// plain ASCII file; force to fixed-width, and skip scanning

		boolean opened;
		FILE *outfile = OpenOutput(this, &opened);
		if ( ! outfile)  return 1;
		fprintf(outfile, "<PLAINTEXT>\n"); 
				// HTML 2.0 spec says "this is obsolete", but ...
		fflush(outfile);
		txt->Write(outfile, 1, 0);

		if (opened) fclose(outfile);
		else fflush(outfile);
	} 
	else {

		/* file has styles; copy them into an htmltext object, 
			and clean them up so it can interpret them easily */
		htmltext *htxt= new htmltext;
		txt->SetCopyAsText(TRUE); 
		txt->SetExportEnvironments(TRUE); 
		htxt->AlwaysCopyTextExactly(0, txt, 0, txt->GetLength());

		if (iscode || isplain) { 
			/* source code; make sure the <PRE> tag gets put around it 
			   <CODE> would make more sense, but Mosaic likes to 
			   _flow_ <CODE>!     >:-[ */
			htxt->AddStyle(0,htxt->GetLength(), 
					findOrCreateStyle(htxt, "preformatted"));
		} 
		else /* arbitrary styles; clean them up as best we can */
			cleanup(htxt);

		// DO THE CONVERSION and WRITE FILE
		retval = ConvertAndWrite(this, htxt);
		htxt->Destroy();
	}

	txt->Destroy();
	return retval; /* successful completion */
}

/* textciStrEq returns TRUE if a case-insensitive comparison of str 
		and the text at pos are equal 
*/
	static boolean 
textciStrEq(text *txt, long pos, const char *str) {
	while (*str) {
		if (tolower(*str) != tolower(txt->GetChar(pos)))
			return FALSE;
		++str; ++pos;
	}
	return TRUE;
}

	static style *
findOrCreateStyle(text *txt, const char *stylename) {
	style *sty= (txt->styleSheet)->Find(stylename);
	if (!sty) { /* create one! no need to set any attributes, 
			since we won't be displaying it on the screen */
		sty = new style;
		sty->SetName(stylename);
		(txt->styleSheet)->Add(sty);
	}
	return sty;
}

	static void 
addtodoomedlist(htmlenv *env, enum style_actions action) {
	struct envlist *elist= (struct envlist *)malloc(sizeof(struct envlist));
	elist->env= env;
	elist->action= action;
	elist->next= doomedenvs;
	doomedenvs= elist;
}

/* markforcleanup remembers any environments that it knows 
	will have no visual effect anyway, 
	or that cannot be handled by the htmltext object 
   The list must be remembered because calling 
	environment_Remove DURING a
	text_EnumerateEnvironments will cause a core dump.  
	It's lame, but a lot speedier than what lookz does to 
	accomplish the same thing 
*/
	static boolean 
markforcleanup(long dummy, text *txt, long pos, environment *env) {
	const char *name;
	if (env->type != environment_Style)
		return FALSE; /* "Next!" */
	name= (env->data.style)->GetName();
	if ((*name=='f' && strcmp(name, "formatnote")==0) ||
		(*name=='F' && strcmp(name, "FormatNote")==0))
		if (textciStrEq(txt,pos, ".cp")) 
			/* BookMaster conditional page begin */
			addtodoomedlist((htmlenv *)env, 
				(style_actions)(action_REMOVE
						|action_REMOVETEXT
						|action_MAKEHR));
		else /* BookMaster .br tag, or troff .sp tag, 
					or something we have no clue about */
			addtodoomedlist((htmlenv *)env, 
				(style_actions)(action_REMOVE
						|action_REMOVETEXT));
	else if ((*name=='h' && strcmp(name, "helptopic")==0) ||
			 (*name=='t' && strcmp(name, "topic")==0))
		 /* leave text intact until after conversion */
		addtodoomedlist((htmlenv *)env, 
			action_CONVERTHELPTOPIC);
	else if (*name=='A' && strcmp(name, "Annotation")==0)
		addtodoomedlist((htmlenv *)env, 
				(style_actions)(action_REMOVE
					|action_REMOVETEXT));
	else if (*name=='L' && strcmp(name, "ListItem")==0)
		addtodoomedlist((htmlenv *)env, 
				(style_actions)(action_REMOVE
				|action_ENSUREDINGBAT));
	else if (*name=='P' && strcmp(name, "Plain")==0)
		addtodoomedlist((htmlenv *)env, 
				action_REMOVEPARENTS);
	else if (*name == 'I' && strcmp(name, "InvisibleIndex") == 0)
		addtodoomedlist((htmlenv *)env, 
				(style_actions)(action_REMOVE
						| action_REMOVETEXT));
	else if (*name == 'I' && strcmp(name, "Index") == 0)
		addtodoomedlist((htmlenv *)env, 
				(style_actions)(action_REMOVE));
	return FALSE; /* keep enumerating */
}

/* cleanupmarked blows away all the bogus environments that
	markforcleanup remembered during the
	htmltext_EnumerateEnvironments 
   XXX- jeepers, maybe htmltext oughta do all this itself.  
	After all, what's to stop somebody from Copy/Paste'ing 
	some bkmtext or .help file styled text into an HTML file?  
	This converter isn't USED in THAT situation :-( 
*/
	static void 
cleanupmarked(htmltext *txt) {
	struct envlist *elist= doomedenvs;
	while (elist) {
		htmlenv *env= elist->env;
		long pos = env->Eval(), len = env->GetLength();
		if (elist->action & (action_REMOVE 
					| action_REMOVEPARENTS))
			((htmlenv *)txt->rootEnvironment)->
				Remove(pos, len, environment_Style, 
				elist->action & action_REMOVEPARENTS);
		if (elist->action & action_REMOVETEXT)
			txt->ReplaceCharacters(pos, len, "\n", 1);
		if (elist->action & action_ENSUREDINGBAT) {
			/* make sure there's a dingbat there; 
				bkmtatk doesn't add one itself, and 
				htmltext wouldn't know to add one 
				just because **ListItem** was there */
			htmlenv *dingbatenv= (htmlenv *)
				(txt->rootEnvironment)->GetChild(pos);
			while (dingbatenv && 
					(dingbatenv->type!=environment_Style 
					|| strcmp((dingbatenv->data.style)
						->GetName(), "dingbat")!=0)) {
				dingbatenv= (htmlenv *)(dingbatenv)->
					GetChild(pos);
			}
			if (!dingbatenv) {
				/* add a pseudo bkmtext dingbat 
					(**DingBat** has already been renamed 
					**dingbat**) */
				htmlenv *list= (htmlenv *)(
						txt->text::rootEnvironment)->
						GetInnerMost(pos+1);
				while (list && list!=(htmlenv *)txt->
							rootEnvironment &&
						list->Eval()==pos) {
					if (list->type==environment_Style) {
						const char *styname=(list->data.style)
								->GetName();
						if (strcmp(styname,"ordered")==0 || 
						strcmp(styname,"unordered")==0 || 
						strcmp(styname,"definition")==0) {
							/* make the front flag TRUE 
							  so pseudo dingbat will be 
							  INSIDE list environment */
							list->SetStyle(TRUE,
									FALSE);
						}
					}
					list= (htmlenv *)(list)->GetParent();
				}
				txt->InsertCharacters(pos, "?\t",2);
				txt->AddStyle(pos, 2, 
					findOrCreateStyle(txt, "dingbat"));
			}
		}  // end EnsureDingbat

		if (elist->action & action_CONVERTHELPTOPIC) {
			/* expand contents of **helptopic** style 
					into an HREF= attribute */
			/* or in the case of the **(real)topic** style, 
				throw away previous expansion 
				and replace with ITS contents */
			htmlenv *htenv= env;
			while (htenv && (htenv->type!=environment_Style
					|| strcmp((htenv->data.style)
						->GetName(), "helptopic")!=0)) {
				htenv= (htmlenv *)(htenv)->GetParent();
			}
			if (htenv && !(htenv==env && htenv->
					GetAttribValue("href") != NULL)) { 
				/* DON'T do this if we're the helptopic itself 
					and this has already been done to us 
					by a nested **topic** style! */
				/* turn contents of env into a 
					HREF= attribute of htenv */
				char *realtopic= (char *)malloc(len+1);
				int i;
				for (i=0; i<len; ++i)
					*(realtopic+i)= txt->GetChar(pos+i);
				*(realtopic+len)= '\0';
				if (helptop_subst) {
					/* put realtopic into helptop_subst 
						string wherever %s was */
					char *substd= (char *)malloc(2048); 
						/*no real clue about size, yet */
					sprintf(substd, helptop_subst, realtopic, 
						realtopic, realtopic, realtopic, 
						realtopic, realtopic, realtopic, 
						realtopic); /* maximum of 8 %'s */
					free(realtopic);
					substd= (char *)realloc(substd, 
							strlen(substd)+1);

					(htenv->GetAttribs())->
							ClearAttributes();
					(htenv->GetAttribs())->
							AddAttribute("href", substd, 
									TRUE);
					free(substd);
				} 
				else {
					/* use the realtopic itself */
					(htenv->GetAttribs())->
						AddAttribute("href", realtopic, 
								TRUE);
					free(realtopic);
				}
			}
			if (strcmp((env->data.style)->GetName(), 
						"topic")==0)
				txt->DeleteCharacters(pos,len);
		}
		if (elist->action & action_MAKEHR)
			txt->InsertObject(pos, "hr","hrv");
		elist= elist->next;
	}
}

/* cleanup attempts to turn recognizable (but non-htmltext) styles 
	into something htmltext can easily convert into tags, 
	and destroy any known but non-translatable styles 
*/
	static void 
cleanup(htmltext *txt) {
	/* destroy no-op styles and their contents, if appropriate */
	doomedenvs = NULL;
	txt->EnumerateEnvironments(0, txt->GetLength(),
		markforcleanup,0);
	cleanupmarked(txt);
}
