/***********************************************************
                Copyright IBM Corporation 1991

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/writeone.C,v 1.4 1993/06/26 16:03:28 rr2b Stab74 $";
#endif


                                 

#include <andrewos.h>                  /* sys/file.h */
#include <stdio.h>
#include <amsutil.H>
#include <sys/param.h>
#include <util.h>
#include <pwd.h>
#include <ctype.h>


#include <text.H>
#include <environment.H>
#include <textview.H>
#include <message.H>
#include <im.H>
#include <mailobj.H>
#include <fnote.H>

#include <cui.h>
#include <fdphack.h>
#include <ams.H>
#define AUXMODULE 1
#include <sendmessage.H>


int WriteOneFile(class sendmessage  *sendmessage, char  *ViceFileName, Boolean  OnVice , Boolean  MayOverwrite , int  Version , Boolean  TrustDelivery , Boolean  UseMultipartFormat, int  *EightBitText);

extern int EnvViewCt(class environment  *env);
extern int ProduceUnscribedVersion(char  *FileName, FILE  *OutputFP);

int WriteOneFile(class sendmessage  *sendmessage, char  *ViceFileName, Boolean  OnVice , Boolean  MayOverwrite , int  Version , Boolean  TrustDelivery , Boolean  UseMultipartFormat, int  *EightBitText)
{
    FILE *fp;
    int i, lim, c = 0;
    class text *d;
    Boolean SeeingAt;
    char ErrorText[MAXPATHLEN+100], LocalName[1+MAXPATHLEN], MyViceFileName[1+MAXPATHLEN], boundary[60], boundary2[60];
    char TmpFname[1+MAXPATHLEN];
    FILE *fptmp;
    char *charset;

    if ((sendmessage->BodyText)->CheckHighBit()) {
	charset = (char *) getenv("MM_CHARSET");
	if (!charset) charset = "ISO-8859-1";
    } else charset = "US-ASCII";
    (ams::GetAMS())->TildeResolve( ViceFileName, MyViceFileName);
    if (OnVice && !(ams::GetAMS())->CUI_OnSameHost()) {
	(ams::GetAMS())->CUI_GenLocalTmpFileName( LocalName);
    } else {
	strcpy(LocalName, MyViceFileName);
    }
    if (!MayOverwrite && strncmp(MyViceFileName, "/tmp/", 5) 
	 && !access(LocalName, F_OK)
	 && ((ams::GetAMS())->GetBooleanFromUser( "That file exists; do you want to overwrite it", 2) != 1)) {
	return(-1);
    }

    if ((fp = fopen(LocalName, "w")) == NULL) {
	sprintf(ErrorText, "Error -- cannot open local file %s", LocalName);
	message::DisplayString(sendmessage, 10, ErrorText);
	return(-1);
    }
#ifdef M_UNIX
    chmod(LocalName, 0600);
#else
    fchmod(fileno(fp), 0600);
#endif
    if (UseMultipartFormat) {
	int kids = EnvViewCt(sendmessage->BodyText->rootEnvironment);
	fprintf(fp, "X-Andrew-Message-Size: %d+%d\n", (sendmessage->BodyText)->GetLength() - kids, kids);
	fprintf(fp, "MIME-Version: 1.0\n");
	if (kids) {
	    sprintf(boundary, "Alternative.Boundary.%s", (ams::GetAMS())->ams_genid( 1));
	    sprintf(boundary2, "Interpart.Boundary.%s", (ams::GetAMS())->ams_genid( 1));
	    fprintf(fp, "Content-Type: multipart/alternative; \n\tboundary=\"%s\"\n", boundary2);
	} else {
	    fprintf(fp, "Content-Type: text/richtext; charset=%s\nContent-Transfer-Encoding: quoted-printable\n", charset);
	}
	d = sendmessage->HeadText;
	for (i=0, lim = (d)->GetLength(); i<lim; ++i) {
	    putc((c=(d)->GetChar( i)), fp);
	}
	if (c != '\n') putc('\n', fp);
	putc('\n', fp);
	if (kids) {
	    fprintf(fp, "> THIS IS A MESSAGE IN 'MIME' FORMAT.  Your mail reader does not support MIME.\n> Please read the first section, which is plain text, and ignore the rest.\n\n--%s\nContent-type: text/plain; charset=US-ASCII\n\n", boundary2); /* 8-bit stuff will be stripped in this part */
	    /* Write plain text version  -- ugly hack */
	    (ams::GetAMS())->CUI_GenTmpFileName( TmpFname);
	    fptmp = fopen(TmpFname, "w");
	    if (!fptmp) return(-1);
	    (sendmessage->BodyText)->Write( fptmp, im::GetWriteID(), 1);
	    fclose(fptmp);
	    ProduceUnscribedVersion(TmpFname, fp);
	    fprintf(fp, "\n--%s\n", boundary2);
	    fprintf(fp, "Content-Type: multipart/mixed; \n\tboundary=\"%s\"\n", boundary);
	}
	fnote::CloseAll(sendmessage->BodyText); /* close any footnotes before converting to MIME datastream */
	/* write multipart / richtext version */
	(sendmessage->BodyText)->WriteOtherFormat( fp, im::GetWriteID(), 1, dataobject_OTHERFORMAT_MAIL, kids ? boundary : NULL);
	if (kids) {
	    fprintf(fp, "\n--%s--\n\n", boundary); /* NSB says that there should be two newlines at the end */
#ifdef THREEPART
	    /* Write Andrew version */
	    fprintf(fp, "\n--%s\nContent-type: application/andrew-inset\n\n", boundary2);
	    fptmp = fopen(TmpFname, "r");
	    if (!fptmp) return(-1);
	    while ((c = getc(fptmp)) != EOF) putc(c, fp);
	    fclose(fptmp);
#endif
	    fprintf(fp, "\n--%s--\n\n", boundary2); /* NSB says that there should be two newlines at the end */
	}
	unlink(TmpFname);
    } else {
	int NeedsEncoding=0;
	if (Version >= 10) {
	    int kids = EnvViewCt(sendmessage->BodyText->rootEnvironment);
	    fprintf(fp, "X-Andrew-Message-Size: %d+%d\n", (sendmessage->BodyText)->GetLength() - kids, kids);
	    fprintf(fp, "Content-Type: X-BE2; %d\n", Version);
	    fprintf(fp, "If-Type-Unsupported: %s\n", TrustDelivery ? "alter" : "send");
	} else {
          char *charset;
	  if ((sendmessage->BodyText)->CheckHighBit()) {
	      charset = (char *) getenv("MM_CHARSET");
	      if (!charset) charset = "iso-8859-1"; /* Just a guess! */
	      NeedsEncoding = 1;
	  } else charset = "US-ASCII";
	  if ( ! (strcmp(charset,"US-ASCII") == 0 && (! NeedsEncoding)) ) {
	    fprintf(fp, "MIME-Version: 1.0\nContent-type: text/plain; charset=%s\n", charset);
	    if (NeedsEncoding) fprintf(fp, "Content-Transfer-Encoding: quoted-printable\n");
	  }
	}
	/*    environ_Put("TextWriteVersion12", "X"); /* phased out 12/27/88, now default */
	 d = sendmessage->HeadText;
	 SeeingAt = FALSE;
	 for (i=0, lim = (d)->GetLength(); i<lim; ++i) {
	     c = (d)->GetChar( i);
	     /* this was a work around for a old BE1 bug.  it should not be needed anymore

		 if (c == '@') {
		     if (SeeingAt) continue;
		     SeeingAt = TRUE;
		 } else {
		     SeeingAt = FALSE;
		 } */
	     putc(c, fp);
	 }
	 if (c != '\n') putc('\n', fp);
	 putc('\n', fp);
	 d = sendmessage->BodyText;
	 if (NeedsEncoding) { /* grossly inefficient, would be better done in simpletext */
#include <simpletext.H> /* SLIGHTLY BOGUS -- simpletext needs to export a few things */
	     class simpletext *st = (class simpletext *) d;
	     if (EightBitText) *EightBitText = 1; /* did NOT write out in ATK format */
	     mailobj::ToQP((unsigned char *)st->string, st->lowSize, fp);
	     mailobj::ToQP((unsigned char *)&(st->string[st->lowSize + st->gapSize]), st->length - st->lowSize, fp);
	 } else {
	     (d)->Write( fp, im::GetWriteID(), 1);
	 }
	 fputs("\n", fp); /* Mail should end with a newline, I think */
    }
#ifdef M_UNIX
    chmod(LocalName,0600);
#else
    fchmod(fileno(fp), 0600);
#endif
    if (ferror(fp)) {
	sprintf(ErrorText, "Error in writing file %s", LocalName);
	message::DisplayString(sendmessage, 10, ErrorText);
	return(-1);
    }
    if(vfclose(fp)) {
	sprintf(ErrorText, "Error -- cannot close file %s", LocalName);
	message::DisplayString(sendmessage, 10, ErrorText);
	return(-1);
    }
    if (OnVice && !(ams::GetAMS())->CUI_OnSameHost()) {
	if ((ams::GetAMS())->CUI_StoreFileToVice( LocalName, MyViceFileName)) {
	    sprintf(ErrorText, "Error -- cannot write file %s", MyViceFileName);
	    message::DisplayString(sendmessage, 10, ErrorText);
	    unlink(LocalName);
	    return(-1);
	}
	unlink(LocalName);
    }
    sprintf(ErrorText, "Wrote file %s", (ams::GetAMS())->ap_Shorten( MyViceFileName));
    message::DisplayString(sendmessage, 10, ErrorText);
    return(0);
}

