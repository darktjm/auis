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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/tokpak.c,v 2.27 1995/03/18 17:31:01 rr2b Stab74 $";
#endif

/*
		tokpak.c -- Subroutines for getting and packing
			Venus tokens into datagrams.
*/

#include <util.h> 

#include <stdio.h>
#include <netinet/in.h>
#include <svcconf.h>

#ifdef AFS_ENV
#include <afs/param.h>
#include <rx/xdr.h>
#include <afs/afsint.h>
#include <afs/auth.h>
#include <tokens.h>
#include <afs/cellconfig.h>
#define KEYSIZE	(sizeof(auth_EncryptionKey))
#include <errno.h>
#include <ctype.h>
#define MAXPackedTicket_Len (11*sizeof(unsigned long) + sizeof(struct ktc_encryptionKey) + MAXKTCTICKETLEN + (2 * sizeof(struct ktc_principal)) + 8*sizeof(unsigned long))
#endif /* AFS_ENV */

#define NIL 0


#ifdef AFS_ENV
static int PackKTC(struct ktc_principal *aserv, struct ktc_token *atok, struct ktc_principal *acli, char *where, int debug, int IsPrim)
{
    register char *p;
    long int Dum;

#ifdef DEBUG
    if (debug) {
	int i;

	fprintf(stderr, "Server: ``%s''/``%s''/``%s''\n", aserv->name, aserv->instance, aserv->cell);
	fprintf(stderr, "Client: ``%s''/``%s''/``%s''\n", acli->name, acli->instance, acli->cell);
	fprintf(stderr, "Key vers num: %d\n", atok->kvno);
	fputs("Token: 0x", stderr);
	p = (char *) atok;
	for (i=0; i<(sizeof(struct ktc_token) - (MAXKTCTICKETLEN - atok->ticketLen)); ++i)
	    fprintf(stderr, "%02x", (unsigned char) *p++);
	fputc('\n', stderr);
    }
#endif /* DEBUG */

    /* Pack them */
    Dum = 0; Dum = htonl(Dum);
    p = where;
    * (long int *) p = Dum; /* 0/0 zero */
    p += sizeof(long int);
    * (long int *) p = htonl(atok->startTime); /* 1/4 start time */
    p += sizeof(long int);
    * (long int *) p = htonl(atok->endTime); /* 2/8 end time */
    p += sizeof(long int);
    * (long int *) p = htonl((long int) atok->kvno); /* 3/12 KVNo */
    p += sizeof(long int);
    * (long int *) p = htonl(atok->ticketLen); /* 4/16 tkt length */
    p += sizeof(long int);
    * (long int *) p = Dum; /* 5/20 zero */
    p += sizeof(long int);
    * (long int *) p = Dum; /* 6/24 zero */
    p += sizeof(long int);
    Dum = IsPrim; Dum = htonl(Dum);
    * (long int *) p = Dum; /* 7/28 prim/amshome flag */
    p += sizeof(long int);
    Dum = sizeof(atok->sessionKey.data); Dum = htonl(Dum);
    * (long int *) p = Dum; /* 8/32 size of session key */
    p += sizeof(long int);
    Dum = -1; Dum = htonl(Dum);
    * (long int *) p = Dum; /* 9/36 flag -1 */
    p += sizeof(long int);
    * (long int *) p = Dum; /* 10/40 flag -1*/
    p += sizeof(long int);
    memmove(p, atok->sessionKey.data, sizeof(atok->sessionKey.data)); /* 11/44 session key */
    p += sizeof(atok->sessionKey.data);
    memmove(p, atok->ticket, atok->ticketLen);
    p += atok->ticketLen;
    Dum = strlen(aserv->name) + 1;
    memmove(p, aserv->name, Dum);
    p += Dum;
    Dum = strlen(aserv->instance) + 1;
    memmove(p, aserv->instance, Dum);
    p += Dum;
    Dum = strlen(aserv->cell) + 1;
    memmove(p, aserv->cell, Dum);
    p += Dum;
    Dum = strlen(acli->name) + 1;
    memmove(p, acli->name, Dum);
    p += Dum;
    Dum = strlen(acli->instance) + 1;
    memmove(p, acli->instance, Dum);
    p += Dum;
    Dum = strlen(acli->cell) + 1;
    memmove(p, acli->cell, Dum);
    p += Dum;

#ifdef DEBUG
    if (debug) {
	char *T;

	fprintf(stderr, "Packed tokens (%d long): 0x", (p - where));
	for (T = where; T < p; ++T) fprintf(stderr, "%02x", (unsigned char) *T);
	fputc('\n', stderr);
    }
#endif /* DEBUG */
    while ( ((p - where) % sizeof(unsigned long)) != 0) *p++ = '\3';
    return (p - where);
}
#endif /* AFS_ENV */


int tok_AddStr(char **pOut, int *pOutL, int *pOutM, const char *StrToAdd)
{
    char C;
    const char *S;

    S = StrToAdd;
    for (C = *S++; C != '\0'; C = *S++) {
	if ((*pOutL + 20) > *pOutM) {
	    *pOutM = 2 * (*pOutL + 20);
	    *pOut = realloc(*pOut, *pOutM);
	    if (*pOut == NULL) return 0;
	}
	(*pOut)[*pOutL] = C;
	++(*pOutL);
    }
    do {
	(*pOut)[*pOutL] = '\0';
	++(*pOutL);
    } while (((*pOutL) % sizeof(unsigned long)) != 0);
    return 1;
}

int GetAndPackAllTokens_Prim(char **pWhere, int *pWhereLen, int *pWhereMax, int debug, const char *PrimCell)
{/* Extend *pWhere with an array of all tokens in all cells.  Maybe override definition of ``primary'' token. */
#ifdef AFS_ENV
    struct ktc_principal serviceName, clientName;	/* service name for ticket */
    struct ktc_token token;			/* the token we're printing */
    int IsPrim, CellIx, NumPacked, RC;
    unsigned long int EndTime;
    char *theDom;
#ifdef DEBUG
    int OldWhereLen;
#endif /* DEBUG */

    CheckServiceConfiguration();
    EndTime = osi_GetSecs() + (3*60);	/* Don't pack tokens that will expire in less than 3 minutes. */
    if (*pWhere == NULL) {
	*pWhereMax = MAXPackedTicket_Len * 2;
	*pWhereLen = 0;
	*pWhere = malloc(*pWhereMax);
	if (*pWhere == NULL) return -1;
    }
#ifdef DEBUG
    OldWhereLen = *pWhereLen;
#endif /* DEBUG */
    NumPacked = 0;
    for (CellIx = 0; ;) {
	RC = ktc_ListTokens(CellIx, &CellIx, &serviceName);
	if (RC) break;
	/* get the ticket info itself */
	RC = ktc_GetToken(&serviceName, &token, sizeof(token), &clientName);
	if (RC) {
	    fprintf(stderr, "tokpak: failed to get token info for service %s.%s.%s (code %d)\n",
		    serviceName.name, serviceName.instance, serviceName.cell, RC);
	    return (-2);
	}
	if (clientName.instance[0] != '\0' || serviceName.instance[0] != '\0' || strcmp(serviceName.name, "afs") != 0 || strcmp(serviceName.cell, clientName.cell) != 0) continue;
	if ((unsigned long int) token.endTime < EndTime) continue;	/* Expired!  Don't keep it. */
	IsPrim = TokNotPrimary;
	if (PrimCell != NULL && ULstrcmp(PrimCell, serviceName.cell) == 0) IsPrim = TokIsPrimary;
	if ((*pWhereLen + MAXPackedTicket_Len) >= *pWhereMax) {
	    *pWhereMax = 2 * (*pWhereLen + MAXPackedTicket_Len);
	    *pWhere = realloc(*pWhere, *pWhereMax);
	    if (*pWhere == NULL) return -1;
	}
	(*pWhereLen) += PackKTC(&serviceName, &token, &clientName, &((*pWhere)[*pWhereLen]), debug, IsPrim);
	++NumPacked;
    }
/* now add ``local'' authentication */
    memset(&serviceName, 0, sizeof(serviceName));
    memset(&clientName, 0, sizeof(clientName));
    memset(&token, 0, sizeof(token));
    token.endTime = 0x7fffffff;
    sprintf(clientName.name, "Unix UID %d", getuid());
    strncpy(serviceName.name, "LOCAL", sizeof(serviceName.name));
    token.kvno = 998;
    token.ticketLen = 0;
    theDom = WorkstationName; /* subsequently, ThisDomain */
    for (CellIx = 0; CellIx < 2; ++CellIx) {
	IsPrim = TokLocalNotPrimary;
	if (PrimCell != NULL && ULstrcmp(PrimCell, theDom) == 0) IsPrim = TokLocalPrimary;
	if ((*pWhereLen + MAXPackedTicket_Len) >= *pWhereMax) {
	    *pWhereMax = 2 * (*pWhereLen + MAXPackedTicket_Len);
	    *pWhere = realloc(*pWhere, *pWhereMax);
	    if (*pWhere == NULL) return -1;
	}
	strncpy(serviceName.cell, theDom, sizeof(serviceName.cell));
	strncpy(clientName.cell, theDom, sizeof(clientName.cell));
	(*pWhereLen) += PackKTC(&serviceName, &token, &clientName, &((*pWhere)[*pWhereLen]), debug, IsPrim);
	if (!AMS_ThisDomainAuthFromWS || ULstrcmp(WorkstationName, ThisDomain) == 0) break;
	theDom = ThisDomain;	/* for next pass */
    }
#ifdef DEBUG
    if (debug) {
	char *p;
	int i;

	fputs("All tokens, packed: 0x", stderr);
	p = &((*pWhere)[OldWhereLen]);
	for (i = *pWhereLen - OldWhereLen; i > 0; --i) {
	    fprintf(stderr, "%02x", (unsigned char) *p++);
	}
	fputc('\n', stderr);
    }
#endif /* DEBUG */
    return NumPacked;	/* number of (AFS) tokens packed */
#else /* AFS_ENV */
    return 0;
#endif /* AFS_ENV */
}

int GetAndPackAllTokens(char **pWhere, int *pWhereLen, int *pWhereMax, int debug)
{/* Extend *pWhere with an array of all tokens in all cells. */
    return (GetAndPackAllTokens_Prim(pWhere, pWhereLen, pWhereMax, debug, NULL));
}

#ifdef TESTINGONLYTESTING
main()
{
    char *BigPacket = NULL;
    int BigLen, BigMax, RC;

    RC = GetAndPackAllTokens(&BigPacket, &BigLen, &BigMax, 1);
    fprintf(stderr, "GetAndPackAllTokens returns %d.\n", RC);
    exit(0);
}
#endif /* TESTINGONLYTESTING */
