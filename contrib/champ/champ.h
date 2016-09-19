/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#ifndef CHAMP_DEFINED
struct gregoriandatespec {
    int year, month, day, hour, min, wkday, wkdayselector;
};

struct hebrewdatespec {
    int year, month, day;
};

struct ecclesiasticaldatespec {
    int year, landmark, offset, hour, min;
};

#define CALSYS_GREGORIAN 0
#define CALSYS_HEBREW 1
#define CALSYS_ECCLESIASTICAL 2

struct datespec {
    int calsys; /* one of CALSYS constants above */
    union {
	struct gregoriandatespec gd;
	struct hebrewdatespec hd;
	struct ecclesiasticaldatespec ed;
    } sys;
};

struct eventnode {
    struct datespec ds;
    char *event;
    int flagged;
    struct eventnode *next;
};

typedef int (*champ_ifefptr)(struct eventnode *e, long rock);
extern struct eventnode *RootEventNode;
extern void ClearAllFlaggedEvents();
extern void IterateFlaggedEvents(champ_ifefptr proc, long  rock);
extern int FlagEventsMatchingDate(struct tm  *date);
extern int matchdate(struct tm  *date, struct datespec  *spec);
extern void IncrementDate(struct tm  *d);
extern void TranslateTmToHebrew(struct tm  *date, struct hebrewdatespec  *hebdate);
extern int ReadDatesFromChampPath(const char  *champpath);
extern struct eventnode *readdateintoeventnode(char  *Buf);

#define CHAMPERR_NOERR 0
#define CHAMPERR_BADFORMAT 1
#define CHAMPERR_BADCALSYS 2
#define CHAMPERR_UNIMPLEMENTED 3
#define CHAMPERR_NOMATCH 4
#define CHAMP_DEFINED
#endif /* CHAMP_DEFINED */
