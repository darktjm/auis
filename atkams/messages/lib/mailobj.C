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
#include <util.h>
#include <mailobj.H>
#include <environment.H>
#include <amsutil.H>
#include <environ.H>
#include <ams.h>
#include <ams.H>
#include <message.H>
#include <im.H>
#include <text.H>
#include <completion.H>
#include <view.H>
#include <sys/param.h>
#include <fdphack.h>
#include <ctype.h>
#undef popen /* BOGUS -- should be handled by fdphack */
#undef pclose /* ditto */

ATKdefineRegistryNoInit(mailobj, sbutton);


// static int char64(char  c);
static void fputsquoting(char  *s, FILE  *fp);
static void WriteCtypeNicely(FILE  *fp, char  *ct);
static void output64chunk(unsigned int c1 , unsigned int c2 , unsigned int c3, int  pads, FILE  *outfile);
static void WriteEncoded(class mailobj  *self, FILE  *fp);

static void MetaOutput(FILE  *fp, char *rock)
{
    char buf[1000];
    int loc = 0;
    class text *mytext;
    class mailobj *self = (class mailobj  *)rock;

    if (!self->t) {
	class view *v[3], *vp;
	class dataobject *dob;
	int ct;
	ct = (self)->ListCurrentViews( v, 3);
	if (ct) {
	    vp = v[0];
	    while (vp && !self->t) {
		dob = (class dataobject *) (vp)->GetDataObject();
		if (dob && ATK::IsTypeByName((dob)->GetTypeName(), "text")) {
		    self->t = (class text *) dob;
		    self->env = NULL;
		}
		vp = vp->parent;
	    }
	}
    }
    if (self->t) {
	if (self->env) {
	    loc = (self->env)->Eval() + self->bytesadded;
	} else {
	    loc = (self->t)->GetLength();
	}
    }
    if (self->bytesadded == 0) {
	loc+=2; /* the extra one is to pass the button itself */
	++self->bytesadded;
	if (self->t) (self->t)->AlwaysInsertCharacters( loc, "\n", 1);
    }
    if (fgets(buf, sizeof(buf), fp) != NULL) {
	self->bytesadded += strlen(buf);
	if (self->t) {
	    (self->t)->AlwaysInsertCharacters( loc, buf, strlen(buf));
	    (self->t)->NotifyObservers( 0);
	} else {
	    fputs(buf, stdout);
	}
	return;
    }
    if (errno != EWOULDBLOCK) {
	int retcode;
	im::RemoveFileHandler(fp);
	self->fp = NULL;
	retcode = pclose(fp);
	if (retcode && (self->bytesadded <= 1)) {
	    char FBuf[1200], Msg[1500];
	    if ((ams::GetAMS())->GetBooleanFromUser( "This data cannot be viewed with 'metamail'.  Write it to a file", 1)) {
		FBuf[0] = '\0';
		if (completion::GetFilename(NULL, "Name of file to write data into: ", FBuf, FBuf, sizeof(FBuf), FALSE, FALSE) != -1) {
		    FILE *fp;

		    fp = (FILE *) fopen (FBuf, "w");
		    if (!fp) {
			message::DisplayString(NULL, 10, "ERROR: Could not open file for writing");
		    } else {
			if (self->EncodingCode == ENC_B64) {
			    mailobj::TranslateFrom64(self->RawData, self->RawBytes, fp);
			} else if (self->EncodingCode == ENC_QP) {
			    mailobj::TranslateFromQP(self->RawData, self->RawBytes, fp);
			} else {
			    fwrite(self->RawData, sizeof(char), self->RawBytes, fp);
			}
			fclose(fp);
			sprintf(Msg, "Wrote file %s as requested", FBuf);
			message::DisplayString(NULL, 10, Msg);
			im::ForceUpdate();
		    }
		}
	    }
	} else {
	    message::DisplayString(NULL, 10, "MetaMail command execution completed");
	}
    }
    if (self->t) (self->t)->NotifyObservers( 0);
}

mailobj::mailobj()
{
    this->ContentType = NULL;
    this->RawData = (unsigned char *) NULL;
    this->t = NULL;
    this->env = NULL;
    this->bytesadded = 0;
    this->RawBytes = 0;
    this->fp = NULL;
    this->EncodingNeeded = this->EncodingCode = ENC_NONE;
    EncodingName=NULL;
    THROWONFAILURE((TRUE));
}

mailobj::~mailobj()
{
    if (this->ContentType) free(this->ContentType);
    if (this->RawData) free(this->RawData);
    if (this->fp) im::RemoveFileHandler(this->fp);
    if(EncodingName) free(EncodingName);
}

void
mailobj::SetTextInsertion(class text  *t, class environment  *env)
{
    this->t = t;
    this->env = env;
}

void
mailobj::ReadAlienMail(const char  *ContentType , const char  *ContentEncoding, FILE  *fp, int  StopAtEndData)
{
    int Alloced = 0, Used = 0, len, needsencoding=0;
#define CHUNKSIZE 5000
    char LineBuf[CHUNKSIZE+1], *tt;
    unsigned char *s;

    if (this->ContentType) free(this->ContentType);
    this->ContentType = (char *)malloc(1+strlen(ContentType));
    /* Strip leading white space */
    while (*ContentType && isspace(*ContentType)) ++ContentType;
    strcpy(this->ContentType, ContentType);
    /* strip trailing white space */
    for (tt=this->ContentType+strlen(this->ContentType) - 1;
	  tt>=this->ContentType && isspace(*tt); ++tt) {
	*tt = '\0';
    }
    /* strip leading white space */
    if (ContentEncoding) {
	while (*ContentEncoding && isspace(*ContentEncoding)) ++ContentEncoding;
    }
    if (!ContentEncoding) {
	this->EncodingCode = ENC_NONE;
    } else if (!amsutil::lc2strncmp("base64", ContentEncoding, 6)) {
	this->EncodingCode = ENC_B64;
    } else if (!amsutil::lc2strncmp("quoted-printable", ContentEncoding, 16)) {
	this->EncodingCode = ENC_QP;
    } else {
	if(EncodingName) free(EncodingName);
	EncodingName=NewString(ContentEncoding);
	this->EncodingCode = ENC_UNKNOWN;
    }
    Alloced = CHUNKSIZE;
    this->RawData = (unsigned char *) malloc(Alloced);
    s = this->RawData;
    *s = '\0';
    while (TRUE) {
	if (StopAtEndData) {
	    if (!fgets(LineBuf, sizeof(LineBuf), fp)) {
		break;
	    }
	    if (!strncmp(LineBuf, "\\enddata{mailobj", 16)) break;
	    len = strlen(LineBuf);
	} else {
	    len = fread(LineBuf, sizeof(char), sizeof(LineBuf), fp);
	    if (len <= 0) break;
	}
	if ((Used + len) >= Alloced) {
	    Alloced += CHUNKSIZE;
	    this->RawData = (unsigned char *) realloc(this->RawData, Alloced);
	    s = this->RawData + Used;
	}
	bcopy(LineBuf, s, len);
	if (!ContentEncoding) {
	    for (tt=LineBuf; tt<(LineBuf+len); ++tt) {
		if (!isprint(*tt) && !(isspace(*tt))) ++needsencoding;
	    }
	}
	s += len;
	Used += len;
    }
    this->RawBytes = Used;
    /* We always use at least qp encoding because it is easier than checking for long lines in data read with fread */
    this->EncodingNeeded = ((needsencoding == 0) || ((Used/needsencoding) >= 10)) ? ENC_QP : ENC_B64;
}



void mailobj::RunMetamail()
{
    char TmpFileName[1+MAXPATHLEN], LineBuf[1000], Cmd[1+MAXPATHLEN],
    Msg[50+MAXPATHLEN];
    FILE *filp;

    sprintf(Msg, "Executing MetaMail to display %s", (this)->GetLabel( 0));
    message::DisplayString(NULL, 10, Msg);
    (ams::GetAMS())->MS_UpdateState(); /* unlock directories */    (ams::GetAMS())->CUI_GenLocalTmpFileName( TmpFileName);
    filp = (FILE *) fopen (TmpFileName, "w");
    if (filp) {
	/* Note that ContentType and ContentEncoding already have trailing newlines */
	WriteEncoded(this, filp);
	fclose(filp);
	sprintf(Cmd, "metamail -m messages -z -x -d -q %s 2>&1", TmpFileName); 
	filp = (FILE *) popen(Cmd, "r");
	im::AddFileHandler(filp, MetaOutput, (char *)this, 0);
	this->fp = filp;
    }
}

static const char basis_hex[] = "0123456789ABCDEF";
static const char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* static int hexchar(char  c)
{
    char *s;
    if (islower(c)) c = toupper(c);
    s = (char *) index(basis_hex, c);
    if (s) return(s-basis_hex);
    return(-1);
}
*/
void
mailobj::TranslateFrom64(unsigned char  *stuff, int  len, FILE  *fp)
{
    unsigned int c1, c2, c3, c4;
    int bytesleft;
    unsigned char *s, *end;

    for(s = stuff, end=stuff+len; c1 = *s++; s < end) {
        if (isspace(c1)) continue;
        do {
            c2 = *s++;
        } while (isspace(c2) && (s < end));
        do {
            c3 = *s++;
        } while (isspace(c3) && (s < end));
        do {
            c4 = *s++;
        } while (isspace(c4) && (s < end));
	if (s >= end) {
	    fprintf(stderr, "Bad parse of base64 data!\n");
            return;
        }
        c1 = char64(c1);
        c2 = char64(c2);
	fputc(((c1<<2) | ((c2&0x30)>>4)), fp);
        if (c3 != '=') {
            c3 = char64(c3);
            fputc((((c2&0XF) << 4) | ((c3&0x3C) >> 2)), fp);
            if (c4 != '=') {
                c4 = char64(c4);
		fputc((((c3&0x03) <<6) | c4), fp);
            }
        }
    }
}

/* static int char64(char  c)
{
    char *s = (char *) index(basis_64, c);
    if (s) return(s-basis_64);
    return(-1);
}
*/
void
mailobj::TranslateFromQP(unsigned char  *stuff, int  len, FILE  *fp)
{
    unsigned int c1, c2;
    int bytesleft;
    unsigned char *s, *end;

    for(s = stuff, end=stuff+len; c1 = *s++; s < end) {
	if (c1 == '=') {
	    c1 = *s++;
	    if (c1 == '\n') {
		/* ignore it */
	    } else {
		c2 = *s++;
		c1 = hexchar(c1);
		c2 = hexchar(c2);
		fputc(c1<<4 | c2, fp);
	    }
	} else {
	    fputc(c1, fp);
	}
    }
}

long mailobj::Write(FILE  *file, long  writeID, int  level)
{
    if ((this)->GetWriteID() != writeID)  {
	(this)->SetWriteID(writeID);
	fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
	WriteEncoded(this, file);
	fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
    }
    return (this)->GetID();
}

static void fputsquoting(char  *s, FILE  *fp)
{
    char *end = s + strlen(s) - 1;
    while (isspace(*end) && end > s) --end;
    if (*s == '\"') {
        putc(*s, fp);
        while (*++s) {
            if (*s == '\"') break; /* MAY TERMINATE EARLY! */
            if (*s == '\\') {
                putc(*s, fp);
                ++s; /* Don't check this next char */
                if (!*s) break;
            }
            putc(*s, fp);
        }
        putc('\"', fp);
    } else {
        putc('\"', fp);
        putc(*s, fp);
        while (*++s) {
            if (*s == '\"' || *s == '\\') {
                putc('\\', fp);
            }
            putc(*s, fp);
        }
        putc('\"', fp);
    }
}
        

static void WriteCtypeNicely(FILE  *fp, char  *ct)
{
    char *semi, *slash, *eq, *s;

    fprintf(fp, "Content-type: ");
    for (s = ct; *s; ++s) {
        if (*s == '\n') *s = ' ';
    }
    semi = (char *) index(ct, ';');
    if (semi) *semi = '\0';
    slash = (char *) index(ct, '/');
    fputs(ct, fp);
    if (!slash) fputs("/unknown", fp);
    while (semi) {
        ct = semi + 1;
        *semi = ';';
        semi = (char *) index(ct, ';');
        if (semi) *semi = '\0';
        eq = (char *) index(ct, '=');
        if (eq) *eq = '\0';
        fputs(";\n\t", fp);
        while (isspace(*ct)) ++ct;
        fputs(ct, fp);
	if (eq) {
	    s = eq;
            fputs("=", fp);
            ++s;
            while (isspace(*s)) ++s;
            fputsquoting(s, fp);
            *eq = '=';
        }
    }
    fputs("\n", fp);
}

static void output64chunk(unsigned int c1 , unsigned int c2 , unsigned int c3, int  pads, FILE  *outfile)
{
    putc(basis_64[c1>>2], outfile);
    putc(basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)], outfile);
    if (pads == 2) {
        putc('=', outfile);
        putc('=', outfile);
    } else if (pads) {
        putc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
        putc('=', outfile);
    } else {
        putc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
        putc(basis_64[c3 & 0x3F], outfile);
    }
}

static void WriteEncoded(class mailobj  *self, FILE  *fp)
{
    const char *s;
    int CodeToUse, needtoencode;
    if (self->ContentType && self->ContentType[0]) {
	WriteCtypeNicely(fp, self->ContentType);
    }
    s = (self)->GetLabel( 0);
    if (s) {
	const char *tt = index(s, '\n');
	fprintf(fp, "Content-Description: %.*s\n", (int)(tt ? tt - s : strlen(s)), s);
    }
    if (self->EncodingCode == ENC_NONE
	 && self->EncodingNeeded != ENC_NONE) {
	CodeToUse = self->EncodingNeeded;
	needtoencode = 1;
    } else {
	CodeToUse = self->EncodingCode;
	needtoencode = 0;
    }
    /* We MUST NOT write encodings for multipart or message, that is illegal MIME */
    if (self->ContentType && 
	 (!amsutil::lc2strncmp("multipart/", self->ContentType, 10) || 
	  !amsutil::lc2strncmp("message/", self->ContentType, 8))) {
	CodeToUse = ENC_NONE;
	needtoencode = 0;
    }
    if (CodeToUse != ENC_NONE) {
	fprintf(fp, "Content-Transfer-Encoding: ");
	switch(CodeToUse) {
	    case ENC_B64:
		fprintf(fp, "base64\n");
		break;
	    case ENC_QP:
		fprintf(fp, "quoted-printable\n");
		break;
	    case ENC_UNKNOWN:
		if(self->EncodingName) fprintf(fp, "%s", self->EncodingName);
		else fprintf(fp, "x-unknown\n");
		break;
	    default:
		fprintf(fp, "x-unknown\n");
		break;
	}
    }
    fputs("\n", fp);
    if (needtoencode) {
	/* Here we need to write it out encoded */
	if (CodeToUse == ENC_QP) {
	    /* NEED TO WRITE OUT IN QP */
         mailobj::ToQP(self->RawData, self->RawBytes, fp);
	} else { /* Use  base64 */
	    int i=0, c1, c2, c3, ct = 0;

	    while (i<self->RawBytes) {
		c1 = self->RawData[i++];
		if (i < self->RawBytes) {
		    c2 = self->RawData[i++];
		    if (i < self->RawBytes) {
			c3 = self->RawData[i++];
			output64chunk(c1, c2, c3, 0, fp);
		    } else {
			output64chunk(c1, c2, 0, 1, fp);
		    }
		} else {
		    output64chunk(c1, 0, 0, 2, fp);
		}
		if (++ct > 16) {
		    putc('\n', fp);
		    ct = 0;
		}
	    }
	}
    } else {
	fwrite(self->RawData, sizeof(char), self->RawBytes, fp);
    }
    fputs("\n", fp);
}

long mailobj::WriteOtherFormat(FILE  *file, long  writeID, int  level, int  usagetype, char  *boundary)
{
    FILE *tmpfp;
    char Fnam[1000];

    if (this->writeID == writeID)  return(this->id);
    this->writeID = writeID;
    
    fprintf(file, "\n--%s\n", boundary);
    WriteEncoded(this, file);
    return(this->id);
}

long mailobj::Read(FILE  *file, long  id)
{
    char LineBuf[200], Ctype[2000], Cenc[2000], Descrip[2000], c;
    const char *ctp;

    Ctype[0] = '\0';
    Cenc[0] = '\0';
    Descrip[0] = '\0';
    while (fgets(LineBuf, sizeof(LineBuf), file)) {
	if (LineBuf[0] == '\n') break;
	if (!amsutil::lc2strncmp("content-type:", LineBuf, 13)) {
	    strcpy(Ctype, LineBuf+13);
	    c = getc(file);
	    ungetc(c, file);
	    while ((c == ' ' || c == '\t') && fgets(LineBuf, sizeof(LineBuf), file)) {
		strcat(Ctype, LineBuf);
		c = getc(file);
		ungetc(c, file);
	    }
	} else if (!amsutil::lc2strncmp("content-transfer-encoding:", LineBuf, 26)) {
	    strcpy(Cenc, LineBuf+26);
	    c = getc(file);
	    ungetc(c, file);
	    while ((c == ' ' || c == '\t') && fgets(LineBuf, sizeof(LineBuf), file)) {
		strcat(Cenc, LineBuf);
                c = getc(file);
		ungetc(c, file);
	    }
	} else if (!amsutil::lc2strncmp("content-description:", LineBuf, 20)) {
	    strcpy(Descrip, LineBuf+20);
	}
    }
    ctp = Ctype[0] ? Ctype : "text/plain";
    while (*ctp && isspace(*ctp)) ++ctp;
    if (Descrip[0]) {
	sprintf(LineBuf, "%s ('%s' format)", Descrip, ctp);
    } else {
	sprintf(LineBuf, "Object of type '%s'", Ctype[0] ? Ctype : "text/plain");
    }
    (this)->SetLabel( 0, LineBuf);
    (this)->ReadAlienMail( Ctype, Cenc, file, TRUE);

    return dataobject_NOREADERROR;
} 		

const char *mailobj::ViewName()
{
    return "mailobjv";
}


void mailobj::ToQP(unsigned char *s /* to avoid high-bit problems */, int  len, FILE  *outfile)
{
    int ct=0, prevc=255;
    unsigned char *end=s+len;
    while (s < end) {
        if ((*s < 32 && (*s != '\n' && *s != '\t'))
             || (*s == '=')
             || ((*s >= 127))) {
            putc('=', outfile);
            putc(basis_hex[*s>>4], outfile);
            putc(basis_hex[*s&0xF], outfile);
            ct += 3;
            prevc = 'A'; /* close enough */
        } else if (*s == '\n') {
            if (prevc == ' ' || prevc == '\t') {
                putc('=', outfile); /* soft & hard lines */
                putc(*s, outfile);
            }
            putc(*s, outfile);
            ct = 0;
            prevc = *s;
        } else {
            putc(*s, outfile);
            ++ct;
            prevc = *s;
        }
        if (ct > 72) {
            putc('=', outfile);
            putc('\n', outfile);
            ct = 0;
            prevc = '\n';
	}
	++s;
    }
    if (ct) {
        putc('=', outfile);
        putc('\n', outfile);
    }
}


