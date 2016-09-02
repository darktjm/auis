/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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
#include <pwd.h>
#include <stdio.h>
#include <errprntf.h>
#include <wp.h>
#define boolean dumbdumbboolean
#include <wpi.h>
#define _INCLUDED_WPI_H_ 1
#undef boolean
#include <parseadd.h>
#include <mail.h>
#include <dropoff.h>
#include <svcconf.h>
#include <sys/param.h>
#include <string.h>

#include <atom.H>
#include <chlist.H>
#include <chlistview.H>
#include <fontdesc.H>
#include <frame.H>
#include <im.H>
#include <lpair.H>
#include <pushbutton.H>
#include <pshbttnv.H>
#include <scroll.H>
#include <style.H>
#include <text.H>
#include <textview.H>
#include <view.H>

#include <wpeditapp.H>

#include <util.h>

#ifndef _STD_C
#define remove(x) unlink(x)
#endif

struct wpedit_fieldrec {
  char *fieldindex;
  class wpeditapp *self;
};


static class frame *myframe;
static class pushbutton *commitpushbutton;
static class style *title_style, *field_style, *value_style;
static char *progname = "WPEdit";

/* TO DO:  
  1.  Create an Exit-hook function to try to save changes on quitting.
*/



ATKdefineRegistry(wpeditapp, application, wpeditapp::InitializeClass);
static char *GetWPEntry(WPI_entry_t  entry, char  *key);
static char * quote_wp_chg_field(char  *s);
static int CommitChanges(class wpeditapp  *self);
static int AlterSomething(struct wpedit_fieldrec  *frec, class chlist  *ch, enum view_MouseAction  action, long  nclicks);
static void CommitPushbuttonHit(class wpeditapp  *self, class pushbutton  *b, long  rock);
static boolean SetupList(class wpeditapp  *self);
static void ChangePushbuttonHit(class wpeditapp  *self, class pushbutton  *b, long  rock);
#if 0				/* exit handler code */
static boolean OnExit(class wpeditapp  *self, int  status);
#endif


static char *GetWPEntry(WPI_entry_t  entry, char  *key)
{
  char *result;

  return((result = WPI_Value(key, entry))?result:"");
}

/* The following should return zero on success, anything else on error.   It should actually be
      validating the entries in the hope that the daemon will later accept them this way */

static char *
quote_wp_chg_field(char  *s)
{				/* Quotes output for passwd.chg format. */
  char *q, *r;

  if ((r=(char *)malloc(2*(s?strlen(s)+1:3)))==NULL)
    return(NULL);
  q=r;
  if ((s==NULL)||(*s == '\0')) {
    strcpy(r,"+ ");
  } else {
    for(; *s; ++s) {
      switch(*s) {
      case ':' : *r++ = '+'; *r++ = '='; break;
      case '+' : *r++ = '+'; *r++ = '+'; break;
      default: *r++ = *s;
      }
    }
    *r = '\0';
  }
  return(q);
}

/* The following should return zero or greater on success, the number of changes sent off to the daemon.  If the changes could not be mailed to the daemon, a negative number should be returned. */

static int CommitChanges(class wpeditapp  *self)
{				/* Format the mail into a file suitable 
				   for mailing to wpid,
				   returns the name of the file */
  int i, ct = 0;
  char filename[MAXPATHLEN];
  FILE *fd;
  time_t t;
  static char *WPAdministrators[2];

  CheckAMSConfiguration();
  if ((WPAdministrators[0] = CheckAMSWPIAddr(WPI_GetWorkingDomain()))==NULL)
    /* Don't have an address to address request to. */
    return(-4);
  WPAdministrators[1] = NULL;

  sprintf(filename,"/tmp/wpi.%d",getpid());
  if ((fd = fopen(filename, "w"))==NULL)
    return(-3);			/* Couldn't open file for mailing. */

  time(&t);
  fprintf(fd,"Date: %s",arpadate());
  fprintf(fd,"From: %s+@%s\n",self->requestor,self->reqdomain);
  fprintf(fd,"To: %s",WPAdministrators[0]);
  for(i=1; WPAdministrators[i];++i) {
    fprintf(fd, ", %s", WPAdministrators[i]);
  }
  fprintf(fd,"\n");

  fprintf(fd,"Subject:  Request for WP Update from '%s'. (from %s v%d)\n\n",
	  self->requestor, progname, WPI_DS_VERSION);
  fprintf(fd,"version:%d\n", WPI_DS_VERSION);
  fprintf(fd,"cell:%s\n", WPI_GetWorkingDomain());
  for(i=0; self->entry[i].fieldnum != -1; ++i)
    if (self->entry[i].changed) {
      fprintf(fd,"change:%s:%s:*:%s:%ld\n",
	      self->EditUser,
	      self->entry[i].fieldname,
	      quote_wp_chg_field(self->entry[i].value),
	      t);
      ++ct;
    }

  if(fclose(fd))
    return(-1);

  if ((ct > 0) 
      && (dropoff(WPAdministrators, filename, NULL,NULL,0) > D_LOCALQ))
    return(-2);

  (commitpushbutton)->SetText( "No unsaved changes have been made.");
  for(i=0; self->entry[i].fieldnum != -1; ++i)
      self->entry[i].changed = false;
  remove(filename);
  return(ct);
}


static int AlterSomething(struct wpedit_fieldrec  *frec, class chlist  *ch, enum view_MouseAction  action, long  nclicks)
{
    char Buf[2500], Buf2[2500], Buf3[2500], *wpentry;
    char *vmsg;
    char *fieldindex;
    class wpeditapp *self;
    class text *desc;

    fieldindex = frec->fieldindex;
    self = frec->self;
    desc = self->desc;
    if (action == view_LeftUp || action == view_RightUp) {
        sprintf(Buf, "Description of field ``%s'':\n%s\n\nExample: ``%s''.",
		WPI_Nice(fieldindex), WPI_Description(fieldindex),
		WPI_Example(fieldindex));

        (desc)->AlwaysDeleteCharacters( 0L, (desc)->GetLength());
	(desc)->AlwaysInsertCharacters( 0L, Buf, strlen(Buf));
	(desc)->NotifyObservers( 0L);

	if (!(self->adminflag) 
	    && ((WPI_CanIChange(fieldindex)!=ALLOW_MODIFY)
		|| strcmp(self->EditUser,self->requestor))) {
	    message::DisplayString(NULL, 10, "Sorry; this value may not be modified.");
	    return 0;
	}
	sprintf(Buf2, "Correct value for %s: ", WPI_Nice(fieldindex));
	wpentry = GetWPEntry(self->entry, fieldindex);
	if (message::AskForString(NULL, 25, Buf2, wpentry, Buf, sizeof(Buf)) < 0) {
	    message::DisplayString(NULL, 10, "Cancelled.");
	    return 0;
	}

	strcpy(WPI_error_msg, "No diagnostic.");
	WPI_error_code = 0;
	switch(WPI_Validate(fieldindex, Buf, self->entry)) {
	case cool:
	  vmsg = WPI_error_code ? WPI_error_msg : "Noted a change to your White Pages entry.";
	  break;
	case drag:
	  vmsg = WPI_error_code ? WPI_error_msg : "Requested change will require administrative approval.";
	  break;
	case uncool:
	default:
	  message::DisplayString(NULL, 10, WPI_error_code ? WPI_error_msg : "Sorry; could not validate white pages entry!");
	  return 0;
	}

	(commitpushbutton)->SetText( "Click here to save changes.");

	sprintf(Buf2, "%s: %s", WPI_Nice(fieldindex), wpentry);
	sprintf(Buf3, "%s: %s", WPI_Nice(fieldindex), GetWPEntry(self->entry,fieldindex));
	if (!(ch)->ChangeItem( Buf2, Buf3)) {
	    message::DisplayString(NULL, 10, "White pages change noted, but display could not be updated.");
	} else {
	    message::DisplayString(NULL, 10, vmsg);
	}
    }
    return 0;
}

static void CommitPushbuttonHit(class wpeditapp  *self, class pushbutton  *b, long  rock)
{
  int ans = CommitChanges(self);
  if (ans < 0) {
    message::DisplayString(NULL, 10, "Could not save changes.");
  } else if (ans == 0) {
    message::DisplayString(NULL, 10, "There was nothing to change.");
  } else {
    char Buf[2500];
    sprintf(Buf, "Mailed changes for %d field%s to the White Pages daemon.", ans, (ans==1)?"":"s");
    message::DisplayString(NULL, 10, Buf);
  }
}

static boolean SetupList(class wpeditapp  *self)
{
    int i;
    long pos;
    char *fieldindex, *fieldname, *fieldvalue;
    char Buf[2500];
    char *welcome = "Welcome to WPEdit\n",
         *authors = "by M. McInerny and N. Borenstein.\n\n",
         *hint = "An ATK interface to the White Pages Interactive\n\nPlease select a field to change from the set of User-modifiable Fields.",
         *admin_text = "Administratively-modifiable Fields",
         *user_text = "User-modifiable Fields";
    class chlist *ach, *uch, *wch;
    class text *desc;
    struct wpedit_fieldrec *frec=NULL;

    ach = self->admin;
    uch = self->user;
    desc = self->desc;
    
    if (!(ach)->AddItemToEnd( admin_text, NULL, 0)) return(FALSE);
    if (!(uch)->AddItemToEnd( user_text, NULL, 0)) return(FALSE);

    (ach)->AlwaysAddStyle( 0L, strlen(admin_text), title_style);
    (uch)->AlwaysAddStyle( 0L, strlen(user_text), title_style);

    strcpy(Buf, welcome);
    strcat(Buf, authors);
    strcat(Buf, hint);
    (desc)->AlwaysReplaceCharacters( 0L, (desc)->GetLength(), Buf, strlen(Buf));
    (desc)->AlwaysAddStyle( 0L, strlen(welcome)+strlen(authors), title_style);
    (desc)->AlwaysAddStyle( strlen(welcome), strlen(authors), field_style);
    (desc)->NotifyObservers( 0L);

    for (i=0; self->entry[i].fieldnum != -1; ++i) {
        fieldindex = self->entry[i].fieldname;
	fieldname = WPI_Nice(fieldindex);
	fieldvalue = GetWPEntry(self->entry, fieldindex);

	wch = (WPI_CanIChange(fieldindex)==ALLOW_MODIFY)?uch:ach; 
	pos = (wch)->GetLength();

	frec = (struct wpedit_fieldrec *)malloc(sizeof(*frec));
	if (!frec) return(FALSE);
	frec->fieldindex = fieldindex;
	frec->self = self;

	sprintf(Buf, "%s: %s", fieldname, fieldvalue);
	if (!(wch)->AddItemToEnd( Buf, (chlist_itemfptr)AlterSomething, (long)frec)) {
	    return(FALSE);
	}
	(wch)->AlwaysAddStyle( pos, strlen(fieldname), field_style);
	(wch)->AlwaysAddStyle( pos+strlen(fieldname)+2, strlen(fieldvalue), value_style);
    }

    sprintf(Buf, "Editing White Pages entry %s@%s", self->EditUser,WPI_GetWorkingDomain());
    (myframe)->SetTitle( Buf);
    (commitpushbutton)->SetText( "No unsaved changes have been made.");
    return(TRUE);
}

static void ChangePushbuttonHit(class wpeditapp  *self, class pushbutton  *b, long  rock)
{
    WPI_entry_t newentry;
    char Buf[2500], Buf2[2500];
    int i, ct = 0;

    if (message::AskForString(NULL, 25, "Edit white pages entry for what user: ", self->EditUser, Buf, sizeof(Buf)) < 0) {
      return;
    }
    
    if (!(newentry = WPI_Lookup(Buf, true))) {
      sprintf(Buf2, "Couldn't find entry for user ``%s''.\n(WPI error %d ``%s''.)", Buf, WPI_error_code, WPI_error_msg);
      message::DisplayString(NULL, 90, Buf2);
    } else {
      for (i=0; self->entry[i].fieldnum != -1; ++i) {
	if (self->entry[i].changed) ++ct;
      }
      if (ct > 0) {
	long result;
	static char *choices[3] = {"Yes, save changes", "No, ignore changes", NULL};
	if (message::MultipleChoiceQuestion(NULL, 25, "Save WP changes before changing user?", 0, &result, choices, NULL) < 0) return;
	if (result == 0) {
	  CommitPushbuttonHit(self, b, rock);
	}
      }

      while (self->user->numitems > 0) {
	(self->user)->DeleteItem( self->user->ItemList[0].str);
      }
      while (self->admin->numitems > 0) {
	(self->admin)->DeleteItem( self->admin->ItemList[0].str);
      }

      strcpy(self->EditUser, Buf);
      self->entry = newentry;
      if (SetupList(self)) {
	sprintf(Buf, "Now editing white pages entry for user %s.", self->EditUser);
	message::DisplayString(NULL, 10, Buf);
      } /* if (SetupList...) */
    } /* if (!(newentry...)) */
}

boolean wpeditapp::Start()
{
    class chlist *ach, *uch;
    class chlistview *achv, *uchv;
    class text *desc;
    class textview *descv;
    class im *myim;
    class scroll *s1, *s2, *s3;
    class pushbutton *b, *b2;
    class pushbuttonview *bv, *bv2;
    class lpair *lp, *lp2, *lp3, *lp4;

    if (! (this)->application::Start()) return(FALSE);
    ach = new chlist;
    achv = new chlistview;
    uch = new chlist;
    uchv = new chlistview;
    desc = new text;
    descv = new textview;
    s1 = scroll::Create(achv, scroll_LEFT);
    s2 = scroll::Create(uchv, scroll_LEFT);
    s3 = scroll::Create(descv, scroll_LEFT);
    myframe = new frame; 
    myim = im::Create(NULL);
    b = new pushbutton;
    commitpushbutton = b;
    bv = new pushbuttonview;
    b2 = new pushbutton;
    bv2 = new pushbuttonview;
    lp = new lpair;
    lp2 = new lpair;
    lp3 = new lpair;
    lp4 = new lpair;
    if(!myim) {
	fprintf(stderr,"wpedit: Could not create new window; exiting.\n");
	return(FALSE);
    }
    if (!ach || !achv || !uch || !uchv || !desc || !descv || !myframe || !s1 || !s2 || !s3 || !b || !bv || !lp || !lp2 || !lp3 || !lp4 || !b || !b2) {
	fprintf(stderr,"wpedit: Could not allocate enough memory; exiting.\n");
	return(FALSE);
    }
    this->admin = ach;
    this->user = uch;
    this->desc = desc;
    (desc)->SetReadOnly( TRUE);
    (achv)->SetDataObject( ach);
    (uchv)->SetDataObject( uch);
    (descv)->SetDataObject( desc);
    (bv)->SetDataObject( b);
    (bv)->AddRecipient( atom::Intern("buttonpushed"), this, (observable_fptr)CommitPushbuttonHit, 0L);
    (bv2)->SetDataObject( b2);
    (b2)->SetText( "Switch User");
    (bv2)->AddRecipient( atom::Intern("buttonpushed"), this, (observable_fptr)ChangePushbuttonHit, 0L);
    (lp2)->SetUp( bv, bv2, 35, lpair_PERCENTAGE, lpair_VERTICAL, FALSE);
    (lp4)->SetUp( s3, s1, 50, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (lp3)->SetUp( lp4, s2, 33, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (lp)->SetUp( lp3, lp2, 25, lpair_BOTTOMFIXED, lpair_HORIZONTAL, FALSE);
    (myframe)->SetView( lp);
    (myim)->SetView( myframe);
    (myframe)->PostDefaultHandler( "message", (myframe)->WantHandler( "message"));
    (uchv)->WantInputFocus( uchv);

    if (!(this->entry = WPI_Lookup(this->EditUser, true))) {
      fprintf(stderr,"WPI Error: %s (%d).\n", WPI_error_msg, WPI_error_code);
      fflush(stderr);
      return(FALSE);
    }
#if 0				/* exit handler code */
    (void)exit_AddExitApprover(OnExit, this); /* won't need to remove handler */
#endif
    return(SetupList(this));
}
 
boolean wpeditapp::InitializeClass()
{
    if (!(title_style = new style)) return(FALSE);
    (title_style)->AddNewFontFace( fontdesc_Bold);
    (title_style)->SetJustification( style_Centered);

    if (!(field_style = new style)) return(FALSE);
    (field_style)->SetFontSize( style_PreviousFontSize,-2);
    (field_style)->AddNewFontFace( fontdesc_Italic);

    if (!(value_style = new style)) return(FALSE);
    (value_style)->AddNewFontFace( fontdesc_Plain);

    return(TRUE);
}

wpeditapp::wpeditapp()
{
	ATKinit;

    char *p;

    (this)->SetMajorVersion( 2);
    (this)->SetMinorVersion( 3);

    this->user = this->admin = NULL;
    this->desc = NULL;
    this->adminflag = FALSE;
    this->entry = NULL;

    CheckServiceConfiguration();
    strcpy(this->reqdomain, ThisDomain);
    WPI_SetWorkingDomain(ThisDomain);

    p = WPI_Self();
    if (!p) {
      fprintf(stderr,"WPI Error: %s (%d).\n", WPI_error_msg, WPI_error_code);
      fflush(stderr);
      THROWONFAILURE((FALSE));
    }
    strcpy(this->EditUser, p);
    strcpy(this->requestor, p);

    THROWONFAILURE((TRUE));
}

wpeditapp::~wpeditapp()
{
	ATKinit;

    return; /* Not really necessary, but class complains at its absence */
}

boolean wpeditapp::ParseArgs(int  argc, const char  **argv)  
{
    (this)->application::ParseArgs( argc, argv);
    ++argv;
    while (argv[0]) {
	if (argv[0][0] == '-') {
	  switch (argv[0][1]) {
	  case 'A':
	    this->adminflag = TRUE;
	    break;
	  case 'c':
	    if (argv[1]) {
	      WPI_SetWorkingDomain(argv[1]);
	      ++argv;
	    } else {
	      fprintf(stderr, "Must supply a cellname after '-c' switch.\n");
	      return(FALSE);
	    }
	    break;
	  case 'h':
	    fprintf(stderr, "Usage:  wpedit [-A] [-c cell] [username].\n");
	    break;
	  default:
	    fprintf(stderr, "Unrecognized switch: %s.\n", argv[0]);
	    return(FALSE);
	  }
	} else {
	  strcpy(this->EditUser, argv[0]);
	}
	++argv;
    }

    return(TRUE);
}

#if 0				/* exit handler code */
static boolean OnExit(class wpeditapp  *self, int  status)
{
  if (status==exit_Normal) {
    int i, ct = 0;

    for (i=0; self->entry[i].fieldnum != -1; ++i) {
      if (self->entry[i].changed) ++ct;
    }
    if (ct > 0) {
      int result;
      static const char * const choices[] = 
	{"Yes, save changes", "No, ignore changes", "Don't exit", NULL};

      if (message::MultipleChoiceQuestion(NULL, 90, 
					 "Save WP changes before exiting?", 
					 0, &result, choices, NULL) < 0)
	return(FALSE);

      switch(result) {
      case 0:
	CommitPushbuttonHit(self, NULL, 0);
	return(TRUE);
      case 1:
	return(TRUE);
      case 2:
      dafault:
	return(FALSE);
      } /* switch(result) */
    } /* if (ct > 0) */
  } /* if (status...) */
  return(TRUE);
}
#endif
