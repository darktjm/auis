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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/champ/RCS/read.C,v 1.4 1994/06/09 21:20:21 rr2b Stab74 $";
#endif

#include <andrewos.h>
#include <stdio.h>
#include <ctype.h>
#include <champ.h>
#include <sys/param.h>
#include <pwd.h>
#include <util.h>
#include <errno.h>

extern int errno;


struct eventnode *RootEventNode = NULL;

#ifndef NORCSID
#endif
static void ResolveTildes(char    *in , char    *out);
int ReadDatesFromChampPath(char  *champpath);
static int ReadDatesFromFile(FILE  *fp);
static int readgregoriandate(char  *db, struct gregoriandatespec  *ds);
static int readhebrewdate(char  *db, struct hebrewdatespec  *ds);
static int readecclesiasticaldate(char  *db, struct ecclesiasticaldatespec  *ds);
static int readdate(char  *datebuf, struct datespec  *datespec);
struct eventnode *readdateintoeventnode(char  *Buf);


static void ResolveTildes(char    *in , char    *out)
{
    char   *t, user[1000];
    struct passwd *p;

    while (in && *in && (*in == ' ' || *in == '\t' || *in == '\n')) {
	++in;
    }
    if (*in != '~') {
	strcpy(out, in);
	return;
    }
    if (*++in == '/') {
	p = getvpwuid(getvuid());
	if (p) {
	    sprintf(out, "%s/%s", p->pw_dir, ++in);
	} else {
	    strcpy(out, in);
	}
	return;
    }
    for (t = user; *in && *in != '/'; ++in, ++t) {
	*t = *in;
    }
    *t = '\0';
    if (*in) ++in;
    p = getvpwnam(user);
    if (!p) {
	strcpy(out, in);
	return;
    }
    sprintf(out, "%s/%s", p->pw_dir, in);
}

int ReadDatesFromChampPath(char  *champpath) 
{
    char *colon, FBuf[1+MAXPATHLEN];
    FILE *fp;
    char *pathbuf;

    if (!champpath) champpath = getprofile("champpath");
    if (!champpath) champpath = "~/events.champ:~/.events.champ";
    pathbuf=champpath=NewString(champpath);
    while(pathbuf) {
	colon = strchr(pathbuf, ':');
	if (colon) *colon++ = '\0';
	ResolveTildes(pathbuf, FBuf);
	pathbuf = colon;
	fp = fopen(FBuf, "r");
	if (!fp) {
	    if (errno != ENOENT) perror(FBuf);
	    continue;
	}
	ReadDatesFromFile(fp);
	fclose(fp);
    }
    if(champpath) free(champpath);
    return(0);
}

static int ReadDatesFromFile(FILE  *fp)
{
    char Buf[1000];
    struct eventnode *allocevent;

    while (1) {
	if (!fgets(Buf, sizeof(Buf)-1, fp)) {
	    return(0);
	}
	allocevent = readdateintoeventnode(Buf);
	if (allocevent) {
	    allocevent->next = RootEventNode;
	    RootEventNode = allocevent;
	}
    }
}

static int readgregoriandate(char  *db, struct gregoriandatespec  *ds)
{
    ds->year = -1;
    ds->month = -1;
    ds->day = -1;
    ds->hour = -1;
    ds->min = -1;
    ds->wkday = -1;
    ds->wkdayselector = -1,
    (void) sscanf(db, "%d %d %d %d %d %d %d", &ds->year, &ds->month, &ds->day, &ds->wkday, &ds->wkdayselector, &ds->hour, &ds->min);
    return(CHAMPERR_NOERR);
}

static int readhebrewdate(char  *db, struct hebrewdatespec  *ds)
{
    ds->year = -1;
    ds->month = -1;
    ds->day = -1;
    (void) sscanf(db, "%d %d %d", &ds->year, &ds->month, &ds->day);
    return(CHAMPERR_NOERR);
}

static int readecclesiasticaldate(char  *db, struct ecclesiasticaldatespec  *ds)
{
    ds->year = -1;
    ds->landmark = -1;
    ds->offset = 0;
    ds->hour = -1;
    ds->min = -1;
    (void) sscanf(db, "%d %d %d %d %d", &ds->year, &ds->landmark, &ds->offset, &ds->hour, &ds->min);
    return(CHAMPERR_NOERR);
}

static int readdate(char  *datebuf, struct datespec  *datespec)
{
    char *s;
    int calsys;

    for (s=datebuf; *s && !isspace(*s); ++s) {
	;
    }
    if (!*s) return(CHAMPERR_BADFORMAT);
    *s++ = '\0';
    calsys = atoi(datebuf);
    switch(calsys) {
	case CALSYS_GREGORIAN:
	    datespec->calsys = calsys;
	    return(readgregoriandate(s, &datespec->sys.gd));
	case CALSYS_HEBREW:
	    datespec->calsys = calsys;
	    return(readhebrewdate(s, &datespec->sys.hd));
	case CALSYS_ECCLESIASTICAL:
	    datespec->calsys = calsys;
	    return(readecclesiasticaldate(s, &datespec->sys.ed));
	default:
	    return(CHAMPERR_BADCALSYS);
    }
}


struct eventnode *readdateintoeventnode(char  *Buf)
{
    char *event, *s;
    int rval;
    struct datespec ds;
    struct eventnode *newevent;

    if (Buf[0] == '#') return(NULL); /* a comment */
    event = strchr(Buf, '#');
    if (event) {
	*event++ = '0';
	s = strchr(event, '\n');
	if (s) *s = '\0';
	while (*event && isspace(*event)) ++event;
    } else {
	event = "null event";
    }
    for (s=Buf; *s && isspace(*s); ++s) {
	;
    }
    if (*s == '\0') return(NULL); /* empty line */
    rval = readdate(s, &ds);
    if (rval != CHAMPERR_NOERR) {
	printf("readdate error %d on %s\n", rval, event);
	return(NULL);
    }
    newevent = (struct eventnode *) malloc(sizeof(struct eventnode));
    if (!newevent) {
	printf("Out of Memory\n");
	return(NULL);
    }
    bcopy(&ds, &newevent->ds, sizeof(struct datespec));
    newevent->event = (char *) malloc(1+strlen(event));
    if (!newevent->event) {
	free(newevent);
	printf("Out of Memory\n");
	return(NULL);
    }
    strcpy(newevent->event, event);
    newevent->flagged = 0;
    return(newevent);
}