/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <stdio.h>
#include <ctype.h>
#include <champ.h>
#include <sys/param.h>
#include <pwd.h>
#include <util.h>
#include <errno.h>

NO_DLL_EXPORT struct eventnode *RootEventNode = NULL;

static void ResolveTildes(char    *in , char    *out);
int ReadDatesFromChampPath(const char  *champpath);
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
	p = getpwuid(getuid());
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
    p = getpwnam(user);
    if (!p) {
	strcpy(out, in);
	return;
    }
    sprintf(out, "%s/%s", p->pw_dir, in);
}

int ReadDatesFromChampPath(const char  *champpath) 
{
    char *colon, FBuf[1+MAXPATHLEN];
    FILE *fp;
    char *pathbuf;

    if (!champpath) champpath = getprofile("champpath");
    if (!champpath) champpath = "~/events.champ:~/.events.champ";
    champpath=pathbuf=strdup(champpath);
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
    if(champpath) free((void *)champpath);
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
    const char *event;
    char *ev, *s;
    int rval;
    struct datespec ds;
    struct eventnode *newevent;

    if (Buf[0] == '#') return(NULL); /* a comment */
    ev = strchr(Buf, '#');
    if (ev) {
	*ev++ = '0';
	s = strchr(ev, '\n');
	if (s) *s = '\0';
	while (*ev && isspace(*ev)) ++ev;
	event = ev;
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
    newevent->event = strdup(event);
    if (!newevent->event) {
	free(newevent);
	printf("Out of Memory\n");
	return(NULL);
    }
    newevent->flagged = 0;
    return(newevent);
}
