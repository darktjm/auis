/* **************************************************** *\
Copyright 1989 Nathaniel S. Borenstein
Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of 
Nathaniel S. Borenstein not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission. 

Nathaniel S. Borenstein DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
Nathaniel S. Borenstein BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
\* ***************************************************** */
#include <andrewos.h>
#include <conapp.H>
#include <flexd.H>
#include <flexview.H>
#include <im.H>
#include <frame.H>
#include <buffer.H>
#include <sys/file.h> /* for access() call */
#include <errno.h>

ATKdefineRegistryNoInit(conapp, application);

conapp::conapp()
{
    this->desiredheight = 100;
    this->desiredwidth = 400;
    this->filenamesallocated = 5;
    this->filenames = (char **) malloc(this->filenamesallocated * sizeof (char *));
//    if (!this->filenames) return;  should raise an exception
    this->filenamesused = 0;
    this->SetMajorVersion(1);
    this->SetMinorVersion(0);
}

/* This routine could be easily modified to do a path search for the named console file */

static boolean FindFile(char *fname, FILE **fpp)
{
    if (fname == NULL) {
	*fpp = NULL;
	return(TRUE);
    }
    *fpp = fopen(fname, "r");
    if (*fpp) return(TRUE);
    if (errno != ENOENT) return(FALSE);
    *fpp = fopen(fname, "w");
    if (!*fpp) return(FALSE);
    fclose(*fpp);
    *fpp = NULL;
    return(TRUE);
}

boolean conapp::Start()
{
    struct im *im;
    struct flexd *cp;
    struct flexview *cpv;
    struct frame *f;
    struct buffer *b;
    int i;
    FILE *fp;
    char LineBuf[250], *s, *t, bufname[25];

    im::SetProgramName("console");
    im::SetPreferedDimensions(0,0, this->desiredwidth, this->desiredheight);
    if (this->filenamesused <= 0) {
	this->filenamesused = 1;
	this->filenames[0] = NULL;
    }
    for (i=0; i<this->filenamesused; ++i) {
	im = im::Create(NULL);
	cp = new flexd;
	if (!FindFile(this->filenames[i], &fp)) {
	    printf("There is no such file as %s.\n", this->filenames[i]);
	    return(FALSE);
	}
	if (fp) {
	    if (fgets(LineBuf, sizeof(LineBuf)-1, fp) == NULL) {
		printf("The file %s is empty.\n", this->filenames[i]);
		fclose(fp);
		return(FALSE);
	    }
	    if (strncmp(LineBuf, "\\begindata{flexd,", 19)) {
		printf("The file %s is not an ATK console file.\n", this->filenames[i]);
		fclose(fp);
		return(FALSE);
	    }
	    s = &LineBuf[19];
	    t = index(LineBuf, '}');
	    if (t) *t = '\0';
	    if (cp->Read(fp, atoi(s)) != dataobject::NOREADERROR) {
		printf("Error reading data from file %s\n", this->filenames[i]);
		fclose(fp);
		return(FALSE);
	    }
	    fclose(fp);
	}
	cpv = new flexview;
	sprintf(bufname, "console-%d", i);
	b = buffer::Create(bufname, this->filenames[i], "flexd", cp);
	f = frame::Create(b);
	cpv->SetDataObject(cp);
	f->SetCommandEnable(TRUE);
	f->SetView(cpv);
	im->SetView(f);
	/*    f->HideMessageLine(); */  /* There should be such a method */
	cpv->WantInputFocus(cpv);
    }
    return(TRUE);
}

boolean conapp::ParseArgs(int argc, char **argv)
{
    int i;
    for (i=1; i<argc; ++i) { /* skip argv[0] */
	if (argv[i][0] == '-') {
	    switch(argv[i][1]) {
		case 'H':
		    if (argv[i][2]) {
			this->desiredheight = atoi(&argv[i][2]);
		    } else if (++i < argc) {
			this->desiredheight = atoi(argv[i]);
		    } else {
			printf("No Height argument specified!\n");
			return(FALSE);
		    }
		    break;
		case 'W':
		    if (argv[i][2]) {
			this->desiredwidth = atoi(&argv[i][2]);
		    } else if (++i < argc) {
			this->desiredwidth = atoi(argv[i]);
		    } else {
			printf("No Width argument specified!\n");
			return(FALSE);
		    }
		    break;
		default:
		    printf("Unrecognized option switch %c\n", argv[i][1]);
		    return(FALSE);
	    }
	} else {
	    if (this->filenamesused >= this->filenamesallocated) {
		this->filenamesallocated += 5;
		this->filenames = (char **) realloc(this->filenames,(this->filenamesallocated * sizeof (char *)));
		if (!this->filenames) return(FALSE);
	    }
	    this->filenames[this->filenamesused] = strdup(argv[i]);
	    if (!this->filenames[this->filenamesused]) return(FALSE);
	    ++this->filenamesused;
	}
    }
    return(TRUE);
}

