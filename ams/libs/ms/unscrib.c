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

#include <andrewos.h> /* sys/file.h */
#include <mail.h>
#include <ms.h>
#include <stdio.h>
#include <hdrparse.h>
#include <unscribe.h>

extern char *StripWhiteEnds();

MS_WriteUnscribedBodyFile(DirName, id, FileName)
char *DirName, /* IN */
     *id, /* IN */
     *FileName; /* OUT */
{
    struct MS_Directory *Dir;
    struct MS_Message *Msg;
    char SourceFileName[MAXPATHLEN+1];
    int code;

    if (ReadOrFindMSDir(DirName, &Dir, MD_OK) != 0) {
	return(mserrcode);
    }
    if ((Msg = (struct MS_Message *) malloc (sizeof (struct MS_Message))) == NULL) {
	AMS_RETURN_ERRCODE(ENOMEM, EIN_MALLOC, EVIA_WRITEUNSCRIBED);
    }
    bzero(Msg, sizeof(struct MS_Message));
    Msg->OpenFD = -1;
    QuickGetBodyFileName(Dir->UNIXDir, id, SourceFileName);
    ConsiderLoggingRead(SourceFileName);
    if (ReadRawFile(SourceFileName, Msg, FALSE)
    || ParseMessageFromRawBody(Msg)) {
	FreeMessage(Msg, TRUE);
	return(mserrcode);
    }
    code = WriteUnscribedBodyFile(Msg, FileName);
    FreeMessage(Msg, TRUE);
    return(code);
}

WriteUnscribedBodyFile(Msg, FileName)
struct MS_Message *Msg;
char *FileName;
{
    struct ScribeState ScribeState;
    int bytesleft, bytestoread, code = 0, IsBE2;
    FILE *fp;
    char ScribeVers[25], BigBuf[5000];

    GenTempName(FileName);
    fp = fopen(FileName, "w");
    if (!fp) {
	AMS_RETURN_ERRCODE(errno, EIN_FOPEN, EVIA_WRITEUNSCRIBED);
    }
    GetFormatFromMessage(Msg, ScribeVers, sizeof(ScribeVers), &IsBE2);
    if (ScribeVers[0]) {
	DeleteHeader(Msg, HP_SCRIBEFORMAT);
	DeleteHeader(Msg, HP_CONTENTTYPE);
	DeleteHeader(Msg, HP_UNSUPPORTEDTYPE);
    }
    if (fwriteallchars(Msg->RawBits, Msg->HeadSize, fp) != Msg->HeadSize) {
	fclose(fp);
	unlink(FileName);
	AMS_RETURN_ERRCODE(errno, EIN_FWRITE, EVIA_WRITEUNSCRIBED);
    }
    if (lseek(Msg->OpenFD, Msg->BodyOffsetInFD, SEEK_SET) < 0) {
	fclose(fp);
	unlink(FileName);
	AMS_RETURN_ERRCODE(errno, EIN_LSEEK, EVIA_WRITEUNSCRIBED);
    }
    bytesleft = Msg->FullSize - Msg->HeadSize;
    errno = 0;
    if (ScribeVers[0]) {
	code = UnScribeInit(ScribeVers, &ScribeState);
    }
    while (bytesleft > 0) {
	bytestoread = (bytesleft > (sizeof(BigBuf)-1)) ? (sizeof(BigBuf) - 1) : bytesleft;
	if (read(Msg->OpenFD, BigBuf, bytestoread) != bytestoread) {
	    fclose(fp);
	    unlink(FileName);
	    AMS_RETURN_ERRCODE(errno, EIN_READ, EVIA_WRITEUNSCRIBED);
	}
	if (ScribeVers[0]) {
	    if (UnScribe(code, &ScribeState, BigBuf, bytestoread, fp) < 0) {
		fclose(fp);
		unlink(FileName);
		AMS_RETURN_ERRCODE(errno, EIN_UNSCRIBE, EVIA_WRITEUNSCRIBED);
	    }
	} else {
	    if (fwriteallchars(BigBuf, bytestoread, fp) != bytestoread) {
		fclose(fp);
		unlink(FileName);
		AMS_RETURN_ERRCODE(errno, EIN_FWRITE, EVIA_WRITEUNSCRIBED);
	    }
	}
	bytesleft -= bytestoread;
    }
    if (ScribeVers[0] && UnScribeFlush(code, &ScribeState, fp)) {
	fclose(fp);
	unlink(FileName);
	AMS_RETURN_ERRCODE(errno, EIN_UNSCRIBE, EVIA_WRITEUNSCRIBED);
    }
    if (ferror(fp) || feof(fp)) {
	unlink(FileName);
	fclose(fp);
	AMS_RETURN_ERRCODE(errno, EIN_FERROR, EVIA_WRITEUNSCRIBED);
    }
    if (vfclose(fp)) {
	unlink(FileName);
	AMS_RETURN_ERRCODE(errno, EIN_VFCLOSE, EVIA_WRITEUNSCRIBED);
    }
    return(0);
}

UnformatMessage(Msg)
struct MS_Message *Msg;
{
    char FileName[1+MAXPATHLEN];

    if (WriteUnscribedBodyFile(Msg, FileName)) return(mserrcode);
    FreeMessageContents(Msg);
    if (Msg->OpenFD) close(Msg->OpenFD);
    if (ReadRawFile(FileName, Msg, FALSE)) {
	unlink(FileName);
	return(mserrcode);
    }
    unlink(FileName);  /* But an fd is still open, for now... */

    if (ParseMessageFromRawBody(Msg)) {
	return(mserrcode);
    }
    Msg->WeFiddled = TRUE;
    return(0);
}
