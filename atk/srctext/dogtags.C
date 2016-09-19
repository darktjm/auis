/* File dogtags.c created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   dogtags, an automatic 'stamper' for files as they're loaded in. */

#include <andrewos.h>

static UNUSED const char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";

#include <ctype.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/param.h> /* for MAXPATHLEN */
#include <attribs.h>
#include <filetype.H> /* for CanonicalizeFilename & ParseAttributes */
#include <buffer.H>
#include <text.H>

#include "dogtags.h"

static char *makeupper(char  *str)
{
    char *st;
    st=str;
    while(*str!=0) {
	if(islower(*str)) *str=toupper(*str);
	str++;
    }
    return st;
}

/* nextDogtagPos() returns the position of the next "<@" in the file, or returns "length" if not found */
static long nextDogtagPos(text *txt, long pos, long length)
{
    long p=pos;
    while (p<length) {
	p= (txt)->Index(p, '<',length-p);
	if (p<0) break;
	if ((txt)->GetChar(p+1)=='@') return p;
	p++;
    }
    return length;
}

/* dogtagSubstitution() returns a string that should replace whatever is in the file from dtpos of length dtlen.  If underscores are found within the delimiters, they will be counted, and the substitution string will try to fit itself into that number of characters, as well as being left- or right-justified, or centered if underscores were found on BOTH sides of the dogtag.  */
static char *dogtagSubstitution(text *self, long dtpos, long dtlen)
{
    static char subst[1024], padded[1024], firstletter;
    int leading=0, trailing=0, count=0;
    boolean justifiable=TRUE; /* set this to FALSE if it's not something that can be left- or right-justified */

    subst[0]= '\0';
    dtpos+= 2; dtlen-= 4; /* ignore the <@ @> delimiters */
    /* count up leading underscores */
    while (dtlen>0 && (self)->GetChar(dtpos) == '_') {
	++dtpos; --dtlen;
	++leading;
    }
    /* count up trailing underscores */
    while (dtlen>0 && (self)->GetChar(dtpos+dtlen-1) == '_') {
	--dtlen;
	++trailing;
    }
    if (dtlen<=1) {
	/* nothing there but a bunch of underscores, fill it in with whitespace (or the lone char) */
	char ch= (dtlen==1)?(self)->GetChar(dtpos):' ';
	while (count < leading+trailing)
	    subst[count++]= ch;
	subst[count]= '\0';
	return subst;
    }
    /* we've got a useful string; let's see if it matches any known dogtags */
    firstletter= (self)->GetChar(dtpos);
    if (!(self)->Strncmp(dtpos, "pathname",dtlen) || !(self)->Strncmp(dtpos, "filename",dtlen) || !(self)->Strncmp(dtpos, "name",dtlen) || !(self)->Strncmp(dtpos, "NAME",dtlen)) {
	buffer *buf= buffer::FindBufferByData(self);
	if (buf!=NULL) {
	    if (firstletter=='p') /* pathname */
		strcpy(subst,(buf)->GetFilename());
	    else {
		char fullpath[1024], *start, *find;
		long length;
		strcpy(fullpath,(buf)->GetFilename());
		find= strrchr(fullpath,'/');
		start= find ? (find+1) : fullpath;
		if (firstletter=='n' || firstletter=='N') { /* name or NAME */
		    find= strchr(start,'.');
		    length= find ? (find-start) : strlen(start);
		    strncpy(subst,start,length);
		    subst[length]='\0';
		    if (firstletter=='N') /* NAME */
			makeupper(subst);
		} else /* filename */
		    strcpy(subst,start);
	    }
	}
    } else if (!(self)->Strncmp(dtpos, "year",dtlen) || !(self)->Strncmp(dtpos, "yr",dtlen) || !(self)->Strncmp(dtpos, "date",dtlen) || !(self)->Strncmp(dtpos, "day",dtlen) || !(self)->Strncmp(dtpos, "day0",dtlen) || !(self)->Strncmp(dtpos, "time",dtlen) || !(self)->Strncmp(dtpos, "mon",dtlen) || !(self)->Strncmp(dtpos, "mon0",dtlen)) {
	char info[27];
	struct timeval tv[1];
	struct timezone tz[1];
	gettimeofday(tv,tz);
	strncpy(info,ctime((const time_t *)&(tv->tv_sec)),27);
	if (firstletter=='y') { /* year or yr */
	    if ((self)->GetChar( dtpos+1)=='r') { /* yr */
		strncpy(subst,info+22,2); subst[2]='\0';
	    } else { /* year */
		strncpy(subst,info+20,4); subst[4]='\0';
	    }
	} else if (firstletter=='d') { /* date or day or day0 */
	    if ((self)->GetChar( dtpos+2)=='y') { /* day or day0 */
		char *day=subst;
		if (isdigit(*(info+8)))
		    *(day++)= *(info+8);
		else if ((self)->GetChar(dtpos+3)=='0') /* day0 */
		    *(day++)= '0';
		*(day++)= *(info+9);
		*day= '\0';
	    } else { /* date */
		strncpy(subst,info,10); subst[10]=' ';
		strncpy(subst+11,info+20,4); subst[15]='\0';
	    }
	} else if (firstletter=='m') { /* mon or mon0 */
	    const char *mon;
	    boolean mon0=((self)->GetChar( dtpos+3)=='0');
	    switch (*(info+4)) {
		case 'A':
		    if (*(info+5)=='p') /* Apr */
			mon= mon0?"04":"4";
		    else /* Aug */
			mon= mon0?"08":"8";
		    break;
		case 'D': /* Dec */
		    mon= "12";
		    break;
		case 'F': /* Feb */
		    mon= mon0?"02":"2";
		    break;
		case 'J':
		    if (*(info+5)=='a') /* Jan */
			mon= mon0?"01":"1";
		    else if (*(info+6)=='n') /* Jun */
			mon= mon0?"06":"6";
		    else /* Jul */
			mon= mon0?"07":"7";
		    break;
		case 'M':
		    if (*(info+6)=='r') /* Mar */
			mon= mon0?"03":"3";
		    else /* May */
			mon= mon0?"05":"5";
		    break;
		case 'N': /* Nov */
		    mon= "11";
		    break;
		case 'O':
		    mon= "10";
		    break;
		case 'S':
		    mon= mon0?"09":"9";
		    break;
		default: /* unforeseen error */
		    mon= "??";
		    break;
	    }
	    strcpy(subst,mon);
	} else { /* time */
	    strncpy(subst,info+11,8); subst[8]='\0';
	}
    } else if (!(self)->Strncmp(dtpos, "programmer",dtlen) || !(self)->Strncmp(dtpos, "userid",dtlen)) {
	struct passwd *pw;
	pw= getpwuid(getuid());
	if (!pw) /* yoikes! the call failed! No /etc/passwd file? */
	    strcpy(subst,"Mystery User");
	else if (firstletter=='p') { /* programmer */
	    strcpy(subst,pw->pw_gecos);
#ifdef USERID_COMMA_SERIALNUM
	    /* define USERID_COMMA_SERIALNUM, if the /etc/passwd file at your site adds a comma and other gunk after your name, and the gunk isn't appropriate or useful enough to make it appear in your source code. */
	    if (strchr(subst,','))
		*(strchr(subst,','))= '\0';
#endif /*USERID_COMMA_SERIALNUM*/
	    if (strlen(subst)<=0) /* no name found */
		strcpy(subst,"Nameless User");
	} else /* userid */
	    strcpy(subst,pw->pw_name);
    } else if (!(self)->Strncmp(dtpos, "log",dtlen)) {
	justifiable= FALSE;
	strcpy(subst,"\074@log@>\n\n\074@date@>  \074@time@>  by \074@programmer@>\n<reason><version><Brief description and why change was made.>");
    } else if (!(self)->Strncmp(dtpos, "file:",5)) {
	int i=0;
	justifiable= FALSE;
	subst[0]= '\0'; /* make it look like an empty string. "hide" the filename right after it */
	while (i<dtlen) {
	    subst[i+1]= (self)->GetChar(dtpos+i);
	    ++i;
	}
	subst[i+1]= '\0';
    } else if (!self->Strncmp(dtpos, "attributes:",11)) {
	struct attributes *tempAttribute;
	char *buf;
	long actlen;
	justifiable= FALSE;
	subst[0]= '\0';
	dtpos+= 11;
	dtlen+= trailing-11;
	while (dtlen>0) {
	    buf= self->GetBuf(dtpos,dtlen, &actlen);
	    strncat(subst, buf, actlen);
	    dtpos+= actlen;
	    dtlen-= actlen;
	}
	tempAttribute= filetype::ParseAttributes(subst);
	if (tempAttribute) {
	    self->SetAttributes(tempAttribute);
	    filetype::FreeAttributes(tempAttribute);
	}
	return NULL; /* don't replace it */
    }
    /* pad it out if it needs justifying */
    if (justifiable && (leading || trailing)) {
	char *pad=padded;
	if (leading && trailing) {
	    /* center it */
	    int frontpad=(int)((leading+trailing-(int)strlen(subst)) / 2);
	    for (count=0; count<frontpad; count++)
		*pad++= ' ';
	    strcpy(pad, subst);
	    pad+= strlen(pad);
	    count= pad-padded;
	    while (count++ < leading+trailing)
		*pad++= ' ';
	    *pad= '\0';
	} else if (leading) {
	    /* right-justify it */
	    int frontpad=leading-strlen(subst);
	    for (count=0; count<frontpad; count++)
		*pad++= ' ';
	    strcpy(pad, subst);
	} else {
	    /* left-justify it */
	    count= strlen(subst);
	    strcpy(pad, subst);
	    pad+= count;
	    while (count++ < trailing)
		*pad++= ' ';
	    *pad= '\0';
	}
	return padded;
    }
    return subst;
}

/* dogtags_substituteregion() will replace dogtags with the appropriate information, but only in the region specified */
void dogtags_substituteregion(text *self, long *posa, long *lena /* *posa and *lena passed by reference; *lena will probably be changing! */)
{
    long pos;
    long srclen=(*posa)+(*lena), dtstart=nextDogtagPos(self, *posa, srclen);
    while (dtstart<srclen) {
	pos= (self)->Index(dtstart+2, '@',srclen-dtstart-2);
	if (pos>0 && pos<=srclen-2 && (self)->GetChar(pos+1)=='>') {
	    long substlen, dtlen=pos-dtstart+2;
	    char *subst;
	    /* figure out what to substitute it with */
	    subst= dogtagSubstitution(self, dtstart,dtlen);
	    substlen= subst?strlen(subst):0;
	    /* do the substitution and compensate for the change in length */
	    if (substlen>0 && (self)->ReplaceCharacters( dtstart,dtlen, subst,substlen)) {
		if (!(self)->Strncmp(dtstart, "\074@log@>",7))
		    dtstart+= 7; /* only skip over the log dogtag itself, we've still got work to do inside */
		else dtstart+= substlen;
		srclen+= substlen-dtlen;
	    } else {
		if (!subst) {
		    /* we don't want anything substituted; leave dogtag intact */
		    dtstart+= dtlen;
		} else if (substlen==0 && strncmp(subst+1,"file:",5)==0) {
		    /* it's a filename we "hid" there */
		    if ((self)->DeleteCharacters( dtstart,dtlen)) {
			char fname[MAXPATHLEN];
			filetype::CanonicalizeFilename(fname, subst+6, MAXPATHLEN);
			substlen= (self)->InsertFile( NULL, fname, dtstart);
			srclen+= substlen-dtlen;
			/* leave dtstart where it is, to catch dogtags WITHIN the file we inserted */
		    } else /* DeleteCharacters failed (readonly buffer?), just skip over it */
			dtstart+= dtlen;
		} else /* ReplaceCharacters failed (readonly buffer?), just skip over it */
		    dtstart+= dtlen;
	    }
	} else
	    ++dtstart; /* false alarm, not a dogtag, skip over the '<' */
	dtstart= nextDogtagPos(self, dtstart, srclen);
    }
    *lena= srclen-(*posa);
    return;
}

/* dogtags_substitute() will buzz through the entire text object looking for dogtags, which are delimited by <@Dogtagname@>, and substitute the appropriate actual information */
void dogtags_substitute(text *self)
{
    long pos=0, len=(self)->GetLength();
    dogtags_substituteregion(self, &pos,&len);
    return;
}
