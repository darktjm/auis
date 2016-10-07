/*
 * urlbtn -- urlbutton dataobject
 *
   (c) Copyright IBM Corp 1995.  All rights reserved. 
 */
#include <andrewos.h>
#include <stdio.h>
#include <util.h>

#include <sbutton.H>
#include <sbuttonv.H>
#include <urlbuttonview.H>
#include <urlbutton.H>

#define UNDEF_LABEL "[Undefined URL]"

ATKdefineRegistry(urlbutton, sbutton, urlbutton::InitializeClass);


boolean urlbutton::InitializeClass()
{
    return TRUE;
}

urlbutton::urlbutton()
{
    ATKinit;
    current_url = NULL;
    has_label = FALSE;
    SetLabel(0, UNDEF_LABEL);
}

urlbutton::~urlbutton()
{
    if (current_url)
	free(current_url);
}

boolean urlbutton::EnsureSize(int  ind)
{
    boolean noprefs = GetDefaultPrefs() == NULL;
    boolean ret;
    struct sbutton_prefs *prefs;

    ret = sbutton::EnsureSize(ind);
    if (noprefs) {
	prefs = GetDefaultPrefs();
	if (prefs) {
	    sbutton::InitPrefs(prefs, "urlbutton");
	    prefs->bdepth = 1;
	}
    }
    return ret;
}

long urlbutton::Read(FILE *fp, long id)
{
    long len;
    char buffer[256];		/* line buffer for reading the datastream */

    /* Read our data up to the \enddata marker. */
    while (fgets(buffer, sizeof(buffer), fp)) {
	len = strlen(buffer);
	if (len > 0) {
	    /* Remove the trailing newline from the buffer.
	     * We return an error if the buffer doesn't end
	     * with a newline (i.e. the line was too long)
	     * because we never write lines that long.
	     */
	    len--;
	    if (buffer[len] != '\n')
		return dataobject::BADFORMAT;
	    buffer[len] = '\0';
	}
	if (strncmp(buffer, "\\url:", 5) == 0) {
	    SetURL(buffer+5);
	    continue;
	}
	if (strncmp(buffer, "\\label:", 7) == 0) {
	    SetText(buffer+7);
	    continue;
	}
	if (strncmp(buffer, "\\enddata{", 9) == 0)
	    break;	/* End of our data. */
    }
    if (feof(fp))
	return dataobject::PREMATUREEOF;
    return dataobject::NOREADERROR;
}

/*
 * Our datastream looks like this:
 *
 *	\begindata{urlbutton,id}
 *	\url:<url here>
 *      \label:<label here>
 *	\enddata{urlbutton,id}
 *
 */
long urlbutton::Write(FILE  *fp, long  id, int  level)
{
    const char *url = GetURL();
    const char *lbl = GetURLLabel();
    long uniqueid = GetID();
    const char *tname = GetTypeName();

    if (id != GetWriteID()) {
	/* New Write Operation */
	SetWriteID(id);

	fprintf(fp, "\\begindata{%s,%ld}\n", tname, uniqueid);

	if (url) {
	    /* We assume the URL won't be too long. */
	    fprintf(fp, "\\url:%s\n", url);
	}
	if (has_label && lbl) {
	    /* We assume the label won't be too long. */
	    fprintf(fp, "\\label:%s\n", lbl);
	}
	fprintf(fp, "\\enddata{%s,%ld}\n", tname, uniqueid);
    }
    return(uniqueid);
}


const char *urlbutton::ViewName()
{
    return "urlbuttonview";
}


void urlbutton::SetURL(char  *url)
{
    if (current_url)
	free(current_url);
    current_url = strdup(url);
    if (!has_label) {
	/* We do this as a convenience. */
	SetLabel( 0, url);
    }
}

void urlbutton::SetText(char  *txt)
{
    const char *newlabel;

    if (txt == NULL || *txt == '\0') {
	has_label = FALSE;
	if (current_url)
	    newlabel = current_url;
	else
	    newlabel = UNDEF_LABEL;
    } else {
	has_label = TRUE;
	newlabel = txt;
    }
    SetLabel(0, newlabel);
}
