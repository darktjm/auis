/* thongs.c - manage and write out thong tables for gentlex.c */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
/*
	Maintains the list -Thongs-  sorted in order by ->thong
		by adding elements with ThongAdd
	At end, writes the thong tables with ThongOut
*/

#include <andrewos.h>
#include <stdio.h>

#include <global.h>
#include <gentlex.h>


static struct line *Thongs = NULL;	/* list of all thongs */
	/* each thong on the list has its ->action field pt
	   to a Hdr for the resulting action.  
	   These Hdrs are on the Classes list
	*/

/* compute the number of leading positions in which
	the two strings s and q are the same
*/
static int Similarity(const char  *s , const char  *q);
void ThongAdd(const char  *thong, struct line  *action, boolean  fromset);
void ThongReplaceNulls(struct line  *hdr);
void ThongOut(FILE  *f);
int ThongAction(struct line  *thong);


static int
Similarity(const char  *s , const char  *q)
	{
	int n;
	for (n=0; *s && *s++ == *q++; n++) {}
	return n;
}

/* ThongAdd(thong, action, fromset)
	adds the given thong with the given action
	Copies the text of the offered thong.
	If the thong is one character long and there is no
	conflict, the action is simply stored in Actions array.
	If lineno is zero in an action, it is assumed to be
		a replaceable default.
	However, a warning is given if the replacement
		has fromset TRUE.  
	Thus there will be a warning if a set overrides one
		of the entries made from the .tab.c file.
	Returns a pointer to a struct line representing the action;
	it may be the original Hdr or it may be a Thong.
*/
	void
ThongAdd(const char  *thong, struct line  *action, boolean  fromset)
			{
	struct line *tx, *prevx;
	struct line *tl;

	tl = Actions[UNSIGN(*thong)];
	if (tl == NULL && thong[1] == '\0') {
		/* just store as an action */
		Actions[UNSIGN(*thong)] = action;
		return;
	}
	
	if (tl == NULL || tl->type != Thong) {
		/* Create the thong entry for single character */
		tl = (struct line *)malloc(sizeof(struct line));
		tl->lineno = 0;
		tl->type = Thong;
		tl->u.g.thong = (char *)malloc(2);
		tl->u.g.thong[0] = *thong;
		tl->u.g.thong[1] = '\0';
		tl->u.g.hdr = Actions[UNSIGN(*thong)];
		tl->u.g.index = -1;	/* assigned later */
		tl->next = Thongs;
		Thongs = tl;
		Actions[UNSIGN(*thong)] = tl;
	}
	/* tl is the struct line of ->type==Thong 
			for the single character *thong */

	/* find insertion location in table */
	prevx = NULL;
	for (tx = tl; tx; tx = tx->next)
		if (*tx->u.g.thong != *thong)
			break;
		else if (strcmp(tx->u.g.thong, thong) >= 0)
			break;
		else prevx = tx;

	if (tx && strcmp(tx->u.g.thong, thong) == 0) {
		/* we have two actions for one string
			either may be an overridable default
			as indicated by having a ->lineno == 0
		    if the existing action is NULL, we are now 
			specifying an action for a single character
			not previously given an action  */
		if (tx->u.g.hdr == NULL)
			tx->u.g.hdr = action;
		else if (tx->u.g.hdr->lineno == 0) {
			/* the old one was overridable */
			tx->u.g.hdr = action;
			if (fromset)  ErrorA(WARNING, 
				"Set specifies character also used as token", 
				thong);
		}
		else if (action->lineno == 0) {
			/* the new one is overridable, leave the old */
		}
		else {
			int tlin = LineNo;
			LineNo = action->lineno;
			ErrorA(ERROR, "two actions for thong", tx->u.g.thong);
			LineNo = tlin;
		}
	}
	else {
		/* this string has never had an action defined:
			make a new entry in thongs list */
		tl = (struct line *)malloc(sizeof (struct line));
		tl->next = tx;
		tl->lineno = 0;
		tl->type = Thong;
		tl->u.g.thong = freeze(thong, NULL);
		tl->u.g.hdr = action;
		tl->u.g.index = -1;
		prevx->next = tl;
			/* if prevx == NULL, something is very wrong */
	}
}

/* ThongReplaceNulls(hdr)
	replace null actions in Thongs list with hdr
*/
	void
ThongReplaceNulls(struct line  *hdr)
	{
	struct line *tx;
	for (tx = Thongs; tx; tx = tx->next)
		if (tx->u.g.hdr == NULL)
			tx->u.g.hdr = hdr;
}


/* ThongOut - generate tables
	tlex_thongtbl,  tlex_thongsame,  tlex_thongact
with the form:
	static const char * const tlex_thongtbl[] = {
		"/", ".9", ... ""
	};
	static const char tlex_thongsame[] = {
		0, 0, ..., 0
	};
	static const short tlex_thongact[] = {
		13, N, ... 0
	};
    assign u.g.index values
*/

	void
ThongOut(FILE  *f)
	{
	int len = 0, tlen;
	int index;
	struct line *tx;
	char *es;
	const char *prev;

	/* thongtbl */
	fprintf(f, "static const char * const %s_thongtbl[] = {", Prefix);
	index = 0;
	for (tx = Thongs; tx; tx = tx->next) {
		tx->u.g.index = index++;	/* assign index value */
		es = Escapify(tx->u.g.thong, &tlen);

		if (tx->u.g.thong[1] == '\0') {
			/* one character thong; start a group */
			fprintf(f, "\n\t/* %d */ ",
					tlex_ACTTHONG | tx->u.g.index);
			len = 18;
		}
		if (len + tlen > 65) {
			fprintf(f, "\n\t");
			len = 8;
		}
		fprintf(f, "\"%s\"", es);
		fprintf(f, ", ");
		len += tlen + 4;
	}
	fprintf(f, "\"\"};\n");

	/* thongsame */
	prev = "";
	fprintf(f, "static const char %s_thongsame[] = {\n\t", Prefix);
	len = 8;	/* (8 for the \t in thongcon) */
	for (tx = Thongs; tx; tx = tx->next) {
		if (len + 3 > 75) {
			fprintf(f, "\n\t");
			len = 8;
		}
		fprintf(f, "%d", Similarity(prev, tx->u.g.thong));
		prev = tx->u.g.thong;
		fprintf(f, ", ");
		len += 3;
	}
	fprintf(f, "0};\n");

	/* thongact */
	fprintf(f, "static const short %s_thongact[] = {\n\t", Prefix);
	len = 8;	/* (8 for the \t in thongcon) */
	for (tx = Thongs; tx; tx = tx->next) {
		if (len + 5 > 64) {
			fprintf(f, "\n\t");
			len = 8;
		}
		fprintf(f, "%d", tx->u.g.hdr->u.h.action);
		fprintf(f, ", ");
		len += 5;
	}
	fprintf(f, "0};\n");
}

/* ThongAction -  compute action array entry for the given character
	value is   tlex_THONG | (index in thong table of entry)
*/
	int
ThongAction(struct line  *thong)
	{
	return tlex_ACTTHONG | thong->u.g.index;
}
