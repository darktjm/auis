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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/amsutil.C,v 1.2 1993/06/03 00:33:17 Zarf Stab74 $";
#endif

#include <ctype.h>
#include <stdio.h>
#include <mailconf.h>

#include <environ.H>
#include <amsutil.H>
#include <proctable.H>
#include <ams.H>
#include <ams.h> /* Could have been cui.h, but not yet necessary here */

  
#include <msshrprotos.h>
#include <util.h>
#include <mail.h>

static struct OptionState MyOpts;
static char **KeyHeaders = NULL;


ATKdefineRegistry(amsutil, ATK, amsutil::InitializeClass);
#ifndef NORCSID
#endif
long hatol(const char  *s);
void GetBinaryOptions();


boolean amsutil::InitializeClass() 
{
    GetBinaryOptions();
    KeyHeaders = amsutil::ParseKeyHeaders();
    return(TRUE);
}

char **amsutil::GetKeyHeadsArray() 
{
	ATKinit;

    return(KeyHeaders);
}

int amsutil::GetOptBit(int  opt)
{
	ATKinit;

    return(GETOPTBIT(MyOpts.Opts, opt));
}

void amsutil::SetOptBit(int  opt , int  val)
{
	ATKinit;

    SETOPTBIT(MyOpts.Opts, opt, val);
}

int amsutil::GetPermOptBit(int  opt)
{
	ATKinit;

    return(GETOPTBIT(MyOpts.PermOpts, opt));
}

void amsutil::SetPermOptBit(int  opt , int  val)
{
	ATKinit;

    SETOPTBIT(MyOpts.PermOpts, opt, val);
}

int amsutil::GetOptMaskBit(int  opt)
{
	ATKinit;

    return(GETOPTBIT(MyOpts.OptMask, opt));
}

void amsutil::SetOptMaskBit(int  opt , int  val)
{
	ATKinit;

    SETOPTBIT(MyOpts.OptMask, opt, val);
}

void amsutil::BreakDownContentTypeField(char  *override , char  *fmttype , int  fmttypelen , char  *fmtvers , int  fmtverslen , char  *fmtresources, int  fmtresourceslen)
{
	ATKinit;

    ::BreakDownContentTypeField(override, fmttype, fmttypelen, fmtvers, fmtverslen, fmtresources, fmtresourceslen);
}

char **amsutil::BreakDownResourcesIntoArray(char  *res)
{
	ATKinit;

    return(::BreakDownResourcesIntoArray(res));
}

int amsutil::lc2strncmp(const char  *s1 , const char  *s2, int  len)
{
	ATKinit;

    return(::lc2strncmp(s1, s2, len));
}

char *amsutil::StripWhiteEnds(char  *s)
{
	ATKinit;

    return(::StripWhiteEnds(s));
}

const char *amsutil::StripWhiteEnds(const char *s, int *rlen)
{
	ATKinit;

    return(::SkipWhiteEnds(s, rlen));
}

char *amsutil::cvEng(int  num , int  min , int  max)
{
	ATKinit;

    return(::cvEng(num, min, max));
}

char *amsutil::convlongto64(long  t , long  p)
{
	ATKinit;

    return(::convlongto64(t, p));
}

long amsutil::conv64tolong(char  *s64)
{
	ATKinit;

    return(::conv64tolong(s64));
}

int amsutil::setprofilestring(char  *prog , char  *pref , char  *val)
{
	ATKinit;

    return(::setprofilestring(prog, pref, val));
}

char **amsutil::ParseKeyHeaders()
{
	ATKinit;

    int numkeys = 0;
    char *s, *t;
    char **KeyHeads;

    s = (char *) environ::GetProfile("messages.keyheaders");
    if (!s) {
	s = "From:Date:Subject:To:CC:ReSent-From:ReSent-To";
    }

    for (t=s; t=strchr(t, ':'); ++t, ++numkeys) {
	;
    }
    KeyHeads = (char **) malloc((2+numkeys) * sizeof(char *));
    t = (char *)malloc(1+strlen(s));
    strcpy(t, s); /* permanent copy */
    ::LowerStringInPlace(t, strlen(t));
    KeyHeads[0] = t;
    numkeys = 1;
    for (s=t; s=strchr(s, ':'); ++s, ++numkeys) {
	*s++ = '\0';
	if (*s) {
	    KeyHeads[numkeys] = s;
	} else {
	    --numkeys;
	}
    }
    KeyHeads[numkeys] = NULL;
    return(KeyHeads);
}

long hatol(const char  *s)
{
    long n;
    char c;

    n = 0;
    while (c = *s) {
	if (c >= '0' && c <= '9') {
	    n = (16 * n) + c - '0';
	} else if (c >= 'a' && c <= 'f') {
	    n = (16 * n) + c - 'a' + 10;
	} /* ignore all other characters, including leading 0x and whitespace */
	++s;
    }
    return(n);
}

void GetBinaryOptions()
{
    int i;
    char *s, *t, *u;
    const char *o;

    for (i=0; i<=(EXP_MAXUSED/32); ++i) {
	MyOpts.DefaultOpts[i] = 0; /* All defaults to zero */
    }
    o = environ::GetProfile("messages.binaryoptions");
    s = o ? strdup(o) : NULL;
    if (s) {
	long dum, dum2;
	int offset;

	offset = 0;
	while (s && offset <= (EXP_MAXUSED/32)) {
	    /* This is a pain, but it is portable */ /* tjm -- not really */
	    t = strchr(s, ',');
	    if (t) *t++ = '\0';
	    u = strchr(s, '/');
	    if (u) *u++ = '\0';
	    dum = hatol(s);
	    dum2 = u ? hatol(u) : 0;
	    MyOpts.OptMask[offset] = dum2;
	    MyOpts.PermOpts[offset] = MyOpts.Opts[offset] = (dum & dum2) | (MyOpts.DefaultOpts[offset] & ~dum2);
	    s = t;
	    ++offset;
	}
	free(s);
    } else { /* first time for everything ... */
	char DumBuf[300];
	int bigmenus;

	/* First time with new option settings, get old preferences for
	   backwards compatibility; */

	for (i=0; i<=(EXP_MAXUSED/32); ++i) {
	    MyOpts.OptMask[i] = 0;
	    MyOpts.Opts[i] = MyOpts.DefaultOpts[i];
	}
	if (environ::GetProfileSwitch("messages.showclasses", 0)) {
	    SETOPTBIT(MyOpts.Opts, EXP_SHOWCLASSES, 1);
	    SETOPTBIT(MyOpts.Opts, EXP_FILEINTO, 1);
	    SETOPTBIT(MyOpts.Opts, EXP_FILEINTOMENU, 1);
	    SETOPTBIT(MyOpts.Opts, EXP_FILEICONCAPTIONS, 1);
	}
	if (environ::GetProfile("messages.crucialclasses") || environ::GetProfile("messages.maxclassmenu")) {
	    SETOPTBIT(MyOpts.Opts, EXP_FILEINTO, 1);
	    SETOPTBIT(MyOpts.Opts, EXP_FILEINTOMENU, 1);
	}
	o = environ::GetProfile("messages.density");
	if (o) {
	    int olen;
	    o = amsutil::StripWhiteEnds(o, &olen);
	    if (*o == 'l' || *o == 'l') {
		SETOPTBIT(MyOpts.Opts, EXP_WHITESPACE, 1);
	    } else if (*o == 'm' || *o == 'M') {
		SETOPTBIT(MyOpts.Opts, EXP_WHITESPACE, 1);
	    } else if (*o != 'H' && *o != 'h') {
		/* Ignoring the stupid obsolete density preference, shich should go away anyway */
	    }
	}
	if (!environ::GetProfileSwitch("messages.wastespaceandtime", 1)) {
	    SETOPTBIT(MyOpts.Opts, EXP_SHOWNOHEADS, 1);
	}
	if (environ::GetProfileSwitch("messages.fixcaptionfont", 0)) {
	    SETOPTBIT(MyOpts.Opts, EXP_FIXCAPTIONS, 1);
	}
	if (environ::GetProfileSwitch("messages.purgeonquitting", 0)) {
	    SETOPTBIT(MyOpts.Opts, EXP_PURGEONQUIT, 1);
	}
	if (environ::GetProfileSwitch("messages.subscriptionexpert", 0)) {
	    SETOPTBIT(MyOpts.Opts, EXP_SUBSEXPERT, 1);
	}	
	if (environ::GetProfileInt("messages.mailfontbloat", 0)) {
	    SETOPTBIT(MyOpts.Opts, EXP_WHITESPACE, 1);
	}
	if (environ::GetProfileSwitch("messages.clearaftersending", 0)) {
	    SETOPTBIT(MyOpts.Opts, EXP_CLEARAFTER, 1);
	}
	if (environ::GetProfileSwitch("messages.hideaftersending", 0)) {
	    SETOPTBIT(MyOpts.Opts, EXP_HIDEAFTER, 1);
	}
	if (environ::GetProfileSwitch("messages.bcc", 0)) {
	    SETOPTBIT(MyOpts.Opts, EXP_KEEPBLIND, 1);
	}
	bigmenus = environ::GetProfileInt("messages.menulevel", 0);
	if (!bigmenus) {
	    bigmenus = environ::GetProfileSwitch("messages.bigmenus", 0);
	    if (bigmenus) bigmenus = 127;
	}
	if (bigmenus) {
	    if (bigmenus & 1) SETOPTBIT(MyOpts.Opts, EXP_SHOWMORENEXT, 1);
	    if (bigmenus & 4) SETOPTBIT(MyOpts.Opts, EXP_MARKASUNREAD, 1);
	    if (bigmenus & 8) SETOPTBIT(MyOpts.Opts, EXP_APPENDBYNAME, 1);
	    if (bigmenus & 16) SETOPTBIT(MyOpts.Opts, EXP_MARKEDEXTRAS, 1);
	    if (bigmenus & 32) SETOPTBIT(MyOpts.Opts, EXP_INSERTHEADER, 1);
	    if (bigmenus & 64) SETOPTBIT(MyOpts.Opts, EXP_SETQUITHERE, 1);
	}
	for (i=0; i<=(EXP_MAXUSED/32); ++i) {
	    MyOpts.PermOpts[i] = MyOpts.Opts[i];
	    MyOpts.OptMask[i] |= MyOpts.Opts[i] ^ MyOpts.DefaultOpts[i];
	}
	amsutil::BuildOptionPreference(DumBuf);
	::setprofilestring("messages", "BinaryOptions", DumBuf);
    }
}


void amsutil::BuildOptionPreference(char  *buf)
{
	ATKinit;

    int whichbyte;
    char MyBuf[50];

    *buf = '\0';
    for (whichbyte = 0; whichbyte <= (EXP_MAXUSED/32); ++whichbyte) {
	sprintf(MyBuf, "0x%x/0x%x", MyOpts.PermOpts[whichbyte], MyOpts.OptMask[whichbyte]);
	if (*buf) strcat(buf, ", ");
	strcat(buf, MyBuf);
    }
}

void amsutil::ReduceWhiteSpace(char  *s)
{
	ATKinit;

    ::ReduceWhiteSpace(s);
}

void amsutil::LowerStringInPlace(char  *s, int  slen)
{
	ATKinit;

    ::LowerStringInPlace(s, slen);
}

int amsutil::debug_open(char  *name, int  flags , int  mode)
{
	ATKinit;

    return(::dbg_open(name, flags, mode));
}

FILE *amsutil::debug_fopen(char  *name , char  *mode)
{
	ATKinit;

    return(::dbg_fopen(name, mode));
}

int amsutil::debug_close(int  fd)
{
	ATKinit;

    return(::dbg_close(fd));
}

int amsutil::debug_vclose(int  fd)
{
	ATKinit;

    return(vclose(fd));
}

int amsutil::debug_fclose(FILE  *fp)
{
	ATKinit;

    return(::dbg_fclose(fp));
}

int amsutil::debug_vfclose(FILE  *fp)
{
	ATKinit;

    return(::dbg_vfclose(fp));
}

void amsutil::fdplumber_SpillGutsToFile(FILE  *fp, boolean  doublenewlines)
{
	ATKinit;

#ifdef PLUMBFDLEAKS
    ::fdplumb_SpillGutsToFile(fp, doublenewlines);
#else /* #ifdef PLUMBFDLEAKS */
    fprintf(fp, "FD plumbing not compiled in!\n\n\n");
#endif /* #ifdef PLUMBFDLEAKS */
}

void amsutil::fdplumber_SpillGuts()
{
	ATKinit;

#ifdef PLUMBFDLEAKS
    ::fdplumb_SpillGuts();
#else /* #ifdef PLUMBFDLEAKS */
    printf("FD plumbing not compiled in!\n\n\n");
#endif /* #ifdef PLUMBFDLEAKS */
}

const char * amsutil::GetDefaultFontName()
{
	ATKinit;

    static const char *myfontname = NULL;

    if (!myfontname) {
	myfontname = environ::GetProfile("fontfamily");
	if (!myfontname || !*myfontname) {
	    myfontname = "andy";
	} else {
	    char *t;

	    t = (char *) malloc(1+strlen(myfontname));
	    if (t) {
		strcpy(t, myfontname);
		myfontname = t;
		amsutil::ReduceWhiteSpace(t);
		for (; *t; ++t) {
		    if (isupper(*t)) *t = tolower(*t);
		}
	    } else {
		myfontname = "andy";
	    }
	}
    }
    return(myfontname);
}

static char *SubsChooseVec[] = {
    "",
    "Yes (Normal subscription; shows new notices)",
    "No (no subscription)",
    "Ask subscription (shows new notices optionally)",
    "ShowAll subscription (shows all notices)",
    "Print subscription (prints new notices)",
    0,
};

static char *Ssubsvec[] = {
    "",
    "Yes", 
    "No",
    "Ask",
    "ShowAll",
    "Print",
    0,
};

static char *BabySubsVec[] = {
    "",
    "Yes",
    "No",
    "Other",
    0,
};

int amsutil::ChooseNewStatus(char  *nickname, int  GivenDefault, boolean  ShowAllChoices)
{
	ATKinit;

    int ans, defaultans;
    char QBuf[200];
    boolean IsExpert = amsutil::GetOptBit(EXP_SUBSEXPERT);

    sprintf(QBuf, "Subscribe to %s?", nickname);
    switch(GivenDefault) {
	case AMS_ALWAYSSUBSCRIBED:
	    defaultans = 1;
	    break;
	case AMS_UNSUBSCRIBED:
	    defaultans = 2;
	    break;
	case AMS_ASKSUBSCRIBED:
	    defaultans = 3;
	    break;
	case AMS_SHOWALLSUBSCRIBED:
	    defaultans = 4;
	    break;
	case AMS_PRINTSUBSCRIBED:
	    defaultans = 5;
	    break;
	default:
	    defaultans = 0;
	    break;
    }
    if (IsExpert) {
	Ssubsvec[0] = QBuf;
	ans = (ams::GetAMS())->ChooseFromList( Ssubsvec, defaultans);
    } else {
	BabySubsVec[0] = QBuf;
	if (ShowAllChoices) {
	    ans = 3;
	} else {
	    ans = (ams::GetAMS())->ChooseFromList( BabySubsVec, (defaultans > 2) ? 3 : defaultans);
	}
	if (ans > 2) {
	    SubsChooseVec[0] = QBuf;
	    ans = (ams::GetAMS())->ChooseFromList( SubsChooseVec, defaultans);
	}
    }
    switch (ans) {
	case 1:
	    return AMS_ALWAYSSUBSCRIBED;
	case 2:
	    return AMS_UNSUBSCRIBED;
	case 3:
	    return AMS_ASKSUBSCRIBED;
	case 4:
	    return AMS_SHOWALLSUBSCRIBED;
	case 5:
	    return AMS_PRINTSUBSCRIBED;
	default:
	    return AMS_ALWAYSSUBSCRIBED;
    }
}

