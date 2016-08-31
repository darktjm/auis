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

/*
  abbrpath.c

	ap_Shorten(pathname) tries to shorten pathname using the current home dir.
	ap_ShortenAlso(pathname, otherID, otherHD) does the same but uses otherID/otherHD also.
	ap_ShortenTo(pathname, maxlen) shortens to a maximum length, abbreviating the path prefix to hyphens.
	ap_ShortenAlsoTo(pathname, otherID, otherHD, maxlen) does it all.
      */

#include <andrewos.h>
#include <stdio.h>
#include <sys/param.h>
#include <ctype.h>
#include <util.h>

static int homeLen = -1;
static char *myHome = NULL;

static void initMyHome(void)
{
    const char *cp;
    if (homeLen == -1) {
	cp = getMyHome();  /* only use home to shorten if it's 2 chars or more */
	if (cp != NULL && cp[0] != '\0' && cp[1] != '\0') {
	    myHome = strdup(cp);
	    homeLen = strlen(cp);
	} else {
	    myHome = strdup("");
	    homeLen = 0;
	}
    }
}

static char shortenRes[MAXPATHLEN+1] = "";

const char *ap_Shorten(const char *pathname)
{/* Shorten it if we can. */
    initMyHome();
    if (homeLen > 0) {
	if (strncmp(pathname, myHome, homeLen) == 0) {
	    if (pathname[homeLen] == '\0') return "~";
	    else if (pathname[homeLen] == '/') {
		strcpy(shortenRes, "~/");
		strcpy(&shortenRes[2], &pathname[homeLen+1]);
		return shortenRes;
	    }
	}
    }
    return (char *)pathname;
}

const char *ap_ShortenAlso(const char *pathname, const char *auxI, const char *auxH)
{/* Shorten it if we can. */
    int auxHLen, auxAbbr;

    auxHLen = auxAbbr = 0;
    if (auxH != NULL && auxI != NULL) {
	auxHLen = strlen(auxH);
	if (auxHLen == 1) auxHLen = 0;	/* short ones don't shorten the abbr. */
	else {
	    auxAbbr = auxHLen - strlen(auxI);	/* how much it will shorten */
	    if (auxAbbr <= 1) auxAbbr = auxHLen = 0;
	}
    }
    initMyHome();
    if (homeLen > auxAbbr) {	/* first try the one with the greater shortening */
	if (strncmp(pathname, myHome, homeLen) == 0) {
	    if (pathname[homeLen] == '\0') return "~";
	    else if (pathname[homeLen] == '/') {
		strcpy(shortenRes, "~/");
		strcpy(&shortenRes[2], &pathname[homeLen+1]);
		return shortenRes;
	    }
	}
    }
    if (auxAbbr > 0) {
	if (strncmp(pathname, auxH, auxHLen) == 0) {
	    strcpy(shortenRes, "~");
	    strcat(shortenRes, auxI);
	    if (pathname[auxHLen] == '\0') return shortenRes;
	    else if (pathname[auxHLen] == '/') {
		strcat(shortenRes, &pathname[auxHLen]);
		return shortenRes;
	    }
	}
    }
    if (homeLen > 0 && homeLen <= auxAbbr) {
	if (strncmp(pathname, myHome, homeLen) == 0) {
	    if (pathname[homeLen] == '\0') return "~";
	    else if (pathname[homeLen] == '/') {
		strcpy(shortenRes, "~/");
		strcpy(&shortenRes[2], &pathname[homeLen+1]);
		return shortenRes;
	    }
	}
    }
    return (char *)pathname;
}

static const char pfx[] = "---";

const char *ap_ShortenTo(const char *pathname, int maxLen)
{/* Shorten it if we can. */
    const char *res, *cp; int len;

    res = ap_Shorten(pathname);
    len = strlen(res);
    if (len <= maxLen) return res;
    cp = strchr(&res[len - maxLen + sizeof(pfx)-2], '/');
    if (cp != NULL) {
	strcpy(shortenRes, pfx);
	memmove(shortenRes + sizeof(pfx), cp + 1, strlen(cp));
	return shortenRes;
    }
    return NULL;	/* Have to give up--can't shorten to spec. */
}

const char *ap_ShortenAlsoTo(const char *pathname, const char *auxI, const char *auxH, int maxLen)
{/* Shorten it if we can. */
    const char *res, *cp; int len;

    res = ap_ShortenAlso(pathname, auxI, auxH);
    len = strlen(res);
    if (len <= maxLen) return res;
    cp = strchr(&res[len - maxLen + sizeof(pfx)-2], '/');
    if (cp != NULL) {
	strcpy(shortenRes, pfx);
	memmove(shortenRes + sizeof(pfx), cp + 1, strlen(cp));
	return shortenRes;
    }
    return NULL;	/* Have to give up--can't shorten to spec. */
}

#ifdef TESTINGONLYTESTING
#include <stdio.h>
int main(void)
{
    char a[300];
    char *cp; int maxLen;
    char auxID[300], auxHome[1000];

    cp = getMyHome();
    printf("Using %s for my home directory.\n", cp == NULL ? "NULL" : cp);
    auxID[0] = auxHome[0] = '\0';
    printf("Aux ID: "); fflush(stdout);
    if (gets(auxID) != NULL && auxID[0] != '\0') {
	printf("and its home dir: "); fflush(stdout);
	if (gets(auxHome) == NULL) auxHome[0] = '\0';
    }
    if (auxID[0] == '\0') auxHome[0] = '\0';
    if (auxHome[0] == '\0') auxID[0] = '\0';
    maxLen = -1;
    printf("Max length? "); fflush(stdout);
    if (gets(a) == NULL) exit(0);
    if (a[0] >= '0' && a[0] <= '9') maxLen = atoi(a);
    for (;;) {
	printf("A path to abbreviate, please: "); fflush(stdout);
	if (gets(a) == NULL) exit(0);
	printf("ap_Shorten(``%s'') results in ``%s''.\n", a, ap_Shorten(a));
	if (auxID[0] != '\0') {
	    printf("ap_ShortenAlso(``%s'', ...) results in ``%s''.\n", a, ap_ShortenAlso(a, auxID, auxHome));
	}
	if (maxLen >= 0) {
	    cp = ap_ShortenTo(a, maxLen);
	    printf("ap_ShortenTo(``%s'', %d) results in ``%s''.\n", a, maxLen,
		   cp == NULL ? "NULL" : cp);
	}
	if (auxID[0] != '\0' && maxLen >= 0) {
	    cp = ap_ShortenAlsoTo(a, auxID, auxHome, maxLen);
	    printf("ap_ShortenAlsoTo(``%s'', ...) results in ``%s''.\n", a,
		   cp == NULL ? "NULL" : cp);
	}
    }
}
#endif /* TESTINGONLYTESTING */
