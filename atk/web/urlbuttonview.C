/*
 * urlbtnv -- urlbuttonview
 *
   (c) Copyright IBM Corp 1995.  All rights reserved. 
 */

#include <andrewos.h>
#include <bind.H>
#include <environ.H>
#include <keymap.H>
#include <keystate.H>
#include <im.H>
#include <menulist.H>
#include <message.H>
#include <sbutton.H>
#include <urlbutton.H>
#include <urlbuttonview.H>

// For printing...
#include <text.H>
#include <textview.H>
#include <style.H>
#include <fontdesc.H>
#include <fnote.H>
#include <fnotev.H>


ATKdefineRegistry(urlbuttonview, sbuttonv, urlbuttonview::InitializeClass);

#define PROMPTFONT "andysans12b"

menulist *urlbuttonview::s_ml;
keymap *urlbuttonview::s_km;

static struct bind_Description urlbtnBindings[] = {
    {"urlbutton-set-url", 0, 0, "UrlButton,Set URL~12", 0, 0,
     (proctable_fptr)urlbuttonview::SetURLProc, "Set the URL."},
    {"urlbutton-set-label", 0, 0, "UrlButton,Set Label~22", 0, 0,
     (proctable_fptr)urlbuttonview::SetLabelProc,"Set the label."},
    NULL
};


boolean urlbuttonview::InitializeClass()
{
    urlbuttonview::s_ml = new menulist;
    urlbuttonview::s_km = new keymap;
    bind::BindList(urlbtnBindings, urlbuttonview::s_km, urlbuttonview::s_ml,
		   &urlbuttonview_ATKregistry_);
    return TRUE;
}

urlbuttonview::urlbuttonview()
{
    ATKinit;
    ml = urlbuttonview::s_ml->DuplicateML(this);
    ks = keystate::Create(this, urlbuttonview::s_km);
    tidata = 0;
    THROWONFAILURE(ml && ks);
}

urlbuttonview::~urlbuttonview()
{
    DestroyTextPrintData();
}

void urlbuttonview::ReceiveInputFocus()
{
    PostMenus(ml);
    ks->next = NULL;
    PostKeyState(ks);
}

/*
 * Start the browser on the given url.
 */
void urlbuttonview::StartBrowser(const char  *url)
{
    const char *cmd = environ::GetProfile("WWWbrowser");
    char buf[4096];

    if (cmd == NULL)
	cmd = "mosaic";
    message::DisplayString(this, 0, "Started WWW browser.  Window should appear soon.");
    /* XXX we should quote the url if it needs it. */
    sprintf(buf, "%s %s &", cmd, url);
    /* XXX We should check status here. */
    system(buf);
}


/* Open a view on the given URL.
 * For now we try to "send" a message to mosaic using
 * the SIGUSR1 protocol.  Need to switch to CCI.
 */
void urlbuttonview::OpenURL(const char  *url)
{
    pid_t mosaic_pid;
    FILE *f;
    const char *mosaic_cmd;
    char *homedir;
    char filename[4096];
    char buf[50];

    if (url == NULL) {
	message::DisplayString(this, 0, "No URL is defined for this button.");
	return;
    }

    /* Find the process id for Mosaic.  This is located in ~/.mosaicpid. */
    homedir = getenv("HOME");
    if (homedir == NULL) {
	message::DisplayString(this, 0, "The HOME environment variable is not defined.");
	return;
    }
    sprintf(filename, "%s/.mosaicpid", homedir);
    f = fopen(filename, "r");
    if (f == NULL || (fgets(buf, sizeof(buf), f) == NULL)) {
	StartBrowser(url);
	return;
    }
    mosaic_pid = atoi(buf);
    if (mosaic_pid <= 1) {
	StartBrowser(url);
	return;
    }
    fclose(f);

    /* Write a control file in /tmp with the goto command in it. */
    sprintf(filename, "/tmp/Mosaic.%d", mosaic_pid);	/* protocol */
    f = fopen(filename, "w");
    if (f == NULL) {
	message::DisplayString(this, 0, "Cannot create /tmp/Mosaic.# temp file.");
	return;
    }
    if (environ::GetProfileSwitch("MosaicNewWindow", TRUE))
	mosaic_cmd = "newwin";
    else
	mosaic_cmd = "goto";
    fprintf(f, "%s\n%s\n", mosaic_cmd, url);
    fclose(f);

    /* Note there is a race condition here.  Sigh...bad protocol */
    if (kill(mosaic_pid, SIGUSR1) == 0) {
	message::DisplayString(this, 0, "Sent message to Mosaic.");
    } else {
	StartBrowser(url);
    }
}

/*
 * The user has clicked on the button.
 *
 * A right click means we should take the focus, post our
 *   menus, and print the command string on the message line.
 * A left click means we should execute the command.
 *
 * XXX we should perform these actions on the UP transition.
 */
boolean urlbuttonview::Touch(int ind, enum view::MouseAction action)
{
    urlbutton *b = GetURLButton();
    const char *m, *url;
    char msg[4096];

    switch (action) {
	case view::RightDown:
	    WantInputFocus(this);
	    url = b->GetURL();
	    if (url) {
		sprintf(msg, "URL is: %s", url);
		m = msg;
	    } else
		m = "No URL is defined for this button.";
	    message::DisplayString(this, 0, m);
	    break;
	case view::LeftDown:
	    OpenURL(b->GetURL());
	    break;
	default:
	    break;
    }
    return FALSE;
}

void urlbuttonview::SetURLProc(urlbuttonview *self, char *arg)
{
    urlbutton *b = self->GetURLButton();
    char *old_url;
    char buf[4096];

    old_url = b->GetURL();
    if (message::AskForString(self,50,"Enter new URL: ", old_url, buf, sizeof(buf)) >= 0) {
	b->SetURL(buf);
	b->SetModified();
	b->NotifyObservers(observable::OBJECTCHANGED);
	message::DisplayString(self, 10, "Changed URL.");
    }
}

void urlbuttonview::SetLabelProc(urlbuttonview *self, char *arg)
{
    urlbutton *b = self->GetURLButton();
    const char *oldtext;
    char buf[4096];

    oldtext = b->GetURLLabel();
    if (message::AskForString(self,50,"Enter new label for button: ", oldtext, buf, sizeof(buf)) >= 0) {
	b->SetText(buf);
	message::DisplayString(self, 10, "Changed button label.");
    }
}

void *urlbuttonview::GetPSPrintInterface(const char *printtype)
{
    if (strcmp(printtype, "text") == 0)
	return urlbuttonview::GetTextPrintData();
    // XXX Need to implement "generic" for insets like figure.
    // Probably the best thing to do is draw a button.
    return NULL;
}

struct textview_insetdata *urlbuttonview::GetTextPrintData()
{
    urlbutton *b = GetURLButton();
    const char *lbl = b->GetURLLabel();
    char *url = b->GetURL();
    int lbl_len = strlen(lbl);
    int url_len = strlen(url);

    if (tidata)
	DestroyTextPrintData();
    tidata = new textview_insetdata;
    if (!tidata)
	return NULL;
    tidata->type = textview_StitchText;
    tidata->u.StitchText = new textview;
    text *t = new text;
    t->InsertCharacters(0, lbl, lbl_len);
    style *s = new style;
    s->AddUnderline();
    s->AddNewFontFace(fontdesc_Italic);
    s->RemoveIncludeEnd();
    t->AddStyle(0, lbl_len, s);
    if (strcmp(lbl, url) != 0) {
	// The label is not the URL.  Insert as a footnote.
	fnote *fn = new fnote;
	fn->InsertCharacters(0, url, url_len);
	t->AddView(lbl_len, "fnotev", fn);
    }
    tidata->u.StitchText->SetDataObject(t);
    return tidata;
}

// This method cleans up stuff allocated in GetTextPrintData.
void urlbuttonview::DestroyTextPrintData()
{
    if (tidata) {
	class dataobject *d = tidata->u.StitchText->GetDataObject();
	if (d) d->Destroy();
	tidata->u.StitchText->Destroy();
	delete tidata;
	tidata = 0;
    }
}
