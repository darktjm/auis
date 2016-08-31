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
 * header.h - Article header format
 */

#define NUNREC 50

/* article header */
struct	hbuf {
	char	from[BUFLEN];		/* From:		*/
	char	path[PATHLEN];		/* Path:		*/
	char	nbuf[BUFLEN];		/* Newsgroups:		*/
	char	title[BUFLEN];		/* Subject:		*/
	char	ident[BUFLEN];		/* Message-ID:		*/
	char	replyto[BUFLEN];	/* Reply-To:		*/
	char	followid[BUFLEN];	/* References:		*/
	char	subdate[DATELEN];	/* Date: (submission)	*/
	time_t	subtime;		/* subdate in secs	*/
	char	recdate[DATELEN];	/* Date-Received:	*/
	time_t	rectime;		/* recdate in secs	*/
	char	expdate[DATELEN];	/* Expires:		*/
	time_t	exptime;		/* expdate in secs	*/
	char	ctlmsg[PATHLEN];	/* Control:		*/
	char	sender[BUFLEN];		/* Sender:		*/
	char	followto[BUFLEN];	/* Followup-to:		*/
	char	postversion[BUFLEN];	/* Post-Version:	*/
	char	relayversion[BUFLEN];	/* Relay-Version:	*/
	char	distribution[BUFLEN];	/* Distribution:	*/
	char	organization[BUFLEN];	/* organize:	*/
	char	numlines[8];		/* Lines:		*/
	int	intnumlines;		/* Integer version	*/
	char	keywords[BUFLEN];	/* Keywords:		*/
	char	summary[BUFLEN];	/* Summary:		*/
	char	approved[BUFLEN];	/* Approved:		*/
	char	nf_id[BUFLEN];		/* Nf-ID:		*/
	char	nf_from[BUFLEN];	/* Nf-From:		*/
#ifdef DOXREFS
	char 	xref[BUFLEN];		/* Xref:		*/
#endif /* DOXREFS */
	char	*unrec[NUNREC];		/* unrecognized lines	*/
};

#define hwrite(hp,fp)	ihwrite(hp,fp,0)
#define lhwrite(hp,fp)	ihwrite(hp,fp,1)

char *oident();
