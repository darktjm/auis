/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/content.C,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 

#include <andrewos.h>
ATK_IMPL("content.H")
#include "ctype.h"
#include <environment.H>
#include "text.H"
#include "style.H"
#include "buffer.H"
#include <environ.H>
#include <stylesheet.H>
#include "content.H"
#define INDENTSPACE 6
#ifndef TEXT_VIEWREFCHAR
#define TEXT_VIEWREFCHAR '\377'
#endif

#define ISPRINT(x) ((((unsigned char)x)>160)?TRUE:isprint(x))

static char defaultlist[] = 
/*    "majorheading,heading,subheading,chapter,section,subsection,paragraph,function" */
    "chapter,section,subsection,paragraph,function"
;

static char *contentstylenames[48];
static int contentstylecount = (-1);

static char *indexnames[] = {
    "index",
    "indexi",
    ""
};
#define indexnamecount 2


ATKdefineRegistry(content, text, NULL);

static findinlist(char  **lst ,int  cnt,char  *str);
static int appendlist(char  **lst,int  cnt,char  *ostr,int  TEST);
static char *chapnote(int  *ip);
static struct content_chapentry *findcp(class content  *self,long  pos,long  len);
static struct content_chapentry *findremcp(class content  *self,long  pos);
static struct content_chapentry *findindexcp(class content  *self,long  pos,long  len);
static long StoI(class content  *self,long  *ppos,int  *lev,class text  *src);
static denumber(class content  *self,struct content_chapentry  *cp);
static boolean skipnewlines(class text  *d,long  *pos,long  *len);
static boolean skipchapnumber(class text  *d,long  *pos,long  *len,boolean  update);
static number(class content  *self,char  *string,struct content_chapentry  *cp);
static void doindent(class content  *self);
static void unindent(class content  *self);
static struct content_chapentry *newentry(class content  *self,struct content_chapentry  *next,boolean  addmark);
static void freeentry(class content  *self,struct content_chapentry  *cp, struct content_chapentry  *lastcp);
static int ensure(class content  *self,struct content_chapentry  **base,long  pos,long  len);
static void copymark(class text  *desttext,class mark  *destmark,class text  *srctext,class mark  *srcmark,boolean  cap);
static void freeentrys(class content  *self);
static void clear(class content  *self);
static boolean ns(class content  *self,class text  *text,long  pos,class environment  *env);
static void initcontentstylenames(void);
static indexstyle(register char  *name);
static struct content_chapentry *addindexentry(class content  *self,long  pos,long  len,struct content_chapentry  **base);
static struct content_chapentry *insertentry(class content  *self,long  pos,long  len,struct content_chapentry  **base);
static void NoteStyle(class content  *self,long  pos,long  len,class style  *style);
static void doshuffle(class content  *self);
static void checknewline(class content  *self,struct content_chapentry  *cp);
static boolean updatemark(class text  *d,class mark  *m,boolean  nonum);
static void mod(class content  *self,struct content_chapentry  **base,boolean  nonum);
static void update(class content  *self);


static findinlist(char  **lst ,int  cnt,char  *str)
{
    int i;
    for(i = 0; i < cnt; i++,lst++){
	if(*lst == NULL || str == NULL) return -1;
	if(**lst == *str && (*lst == str || strcmp(*lst,str) == 0)){
	    return i;
	}
    }
    return -1;
}
static int appendlist(char  **lst,int  cnt,char  *ostr,int  TEST)
{   /* BUG -- OVERFLOWS NOT DETECTED */

    char *str;
    long len;
    int next = 1;
    ;
    if(ostr == NULL || (len = strlen(ostr)) == 0 || ((str = (char *)malloc(len + 1)) == NULL)) return 0;
    strcpy(str,ostr);
    if(TEST){
	if(findinlist(lst,cnt,str) != -1) return cnt;
    }
    while(*str){
	if(*str == ',' || *str == '\n') {
	    *str = '\0';
	    next++;
	}
	else if(*str == '\0') break;
/*	else if(*str == ' ') ; */
	else if(next){
	    lst[cnt++] = str;
	    next = 0;
	}
	str++;
    }
    lst[cnt] = NULL;
    return cnt;
}
#define setchap(cp,remloc,remlen)   (cp->rem)->SetPos(remloc);(cp->rem)->SetLength(remlen);(cp->rem)->SetModified(1)
static char *chapnote(int  *ip)
{
	static char ret[128];
	char *c;
	c = ret;
	sprintf(ret,"%d",*ip++);
	while(*ip > 0 || ip[1] > 0 || ip[2] > 0){
		while(*c != '\0') c++;
		sprintf(c,".%d",*ip++);
	}
	while(*c != '\0') c++;
	*c++ = '\t';
	*c='\0';
	return(ret);
}
#define NUMLEV 24	
static struct content_chapentry *findcp(class content  *self,long  pos,long  len)
{
    struct content_chapentry *cp;
    if(pos < 0) return(self->entry);
    pos += len;
    for(cp = self->entry; cp != NULL;cp = cp->next){
	if((cp->loc)->GetPos() <= pos && (cp->loc)->GetPos() + (cp->loc)->GetLength() >= pos)
	    break;
	if(cp->next == NULL) return NULL;
    }
    return(cp);
}
static struct content_chapentry *findremcp(class content  *self,long  pos)
{
    struct content_chapentry *cp;
    if(pos < 0) return(self->entry);
    for(cp = self->entry; cp != NULL;cp = cp->next){
	if((cp->rem)->GetPos() <= pos && (cp->rem)->GetPos() + (cp->rem)->GetLength() >= pos)
	    break;
	if(cp->next == NULL) return NULL;
    }
    return(cp);
}
static struct content_chapentry *findindexcp(class content  *self,long  pos,long  len)
{
    struct content_chapentry *cp;
    if(pos < 0) return(self->entry);
    pos += len;
    for(cp = self->indexentry; cp != NULL;cp = cp->next){
	if((cp->loc)->GetPos() <= pos && (cp->loc)->GetPos() + (cp->loc)->GetLength() >= pos)
	    break;
	if(cp->next == NULL) return NULL;
    }
    return(cp);
}
static long StoI(class content  *self,long  *ppos,int  *lev,class text  *src)
{
    char *cp,c,buf[128];
    long pos = *ppos;
    cp = buf;
    if(self->srctext == NULL) return(0);
    for(; ((c = (src)->GetChar(pos)) == '\t' )|| c == ' ' || c == '\n'; pos++) ;
    *ppos = pos;
    for(; ((c = (src)->GetChar(pos)) == '\t') || c == '.' || isdigit(c) ; pos++){
	if(c == '.' || c == '\t'){
	    *cp = '\0';
	    if(lev){
		*lev= atoi(buf);
		lev++;
	    }
	    if(c == '\t') break;
	    cp = buf;
	}
	else *cp++ = c;
	
    }
    if(c == '\t' && cp != buf){
	if(lev) *lev = 0;
	return(pos + 1 - *ppos);
    }
    return(0);

}
 long content::StringToInts(long  pos,int  *lev)
{
return StoI(this,&pos,lev,(class text *)this);
}
    
static denumber(class content  *self,struct content_chapentry  *cp)
{
    long pos,len,opos,olen;
    opos = pos = (cp->rem)->GetPos();
    olen = (cp->rem)->GetLength();
    if((len = StoI(self,&pos,NULL,self->srctext)) > 0){
	(self->srctext)->AlwaysDeleteCharacters(pos,len);
}
    return(opos + olen <= pos + len);
}
 class mark *content::locate(long  pos)
{
    struct content_chapentry *cp;
    if(this->srctext == NULL) return NULL;
    cp = findcp(this,pos,0);
    if(cp == NULL && (cp = findindexcp(this,pos,0)) == NULL){
/*	for(cp = self->entry; cp != NULL;cp = cp->next)
	    if(cp->next == NULL) return(cp->rem); */
	return(NULL);
    }
    return cp->rem;
}
static boolean skipnewlines(class text  *d,long  *pos,long  *len)
{
    long i,end;
    int c;
    end = *pos + *len;
    for(i = *pos ; i < end; i++){
	c = (d)->GetChar(i);
	if(! isspace(c)) break;
    }
    *pos = i;
    for( ; i < end; i++){
	if((d)->GetChar(i) == '\n') {
	    end = i;
	    break;
	}
    }
    if(*pos >= end) return FALSE;
    *len = end - *pos;
    return TRUE;
	
}
static boolean skipchapnumber(class text  *d,long  *pos,long  *len,boolean  update)
{
    long i,end;
    int c;
    end = *pos + *len;
    for(i = *pos ; i < end; i++){
	c = (d)->GetChar(i);
	if(c == '.' || isdigit(c)) continue;
	if(isspace(c) && c != '\t') continue;
	break;
    }
    if(i >= end ) return FALSE;
    if(c == '\t') i++;
    if(i >= end ) return FALSE;
    if(update){
	*pos = i;
	*len = end - *pos;
    }
    return TRUE;	
}
struct content_chapentry *content::CopyEntry(long  pos,long  len,char  *buf,long  buflen)
{   /* copy out the index entry for the given pos in the source text */
    struct content_chapentry *cp;
    long llen;
    char *cc;
    if(this->srctext == NULL) return NULL;
    if(skipnewlines(this->srctext,&pos,&len) == FALSE) return NULL;
    cp = findremcp(this,pos);
    if(cp == NULL){
/* fprintf(stderr,"remcp failed, pos = %d\n",pos);fflush(stderr); */
	return(NULL);
    }
    if((llen = (cp->loc)->GetLength()) > buflen) llen = buflen;
    (this)->CopySubString((cp->loc)->GetPos(),llen,buf,FALSE);
/* fprintf(stderr,"CEbuf = |%s| \n",buf);fflush(stderr); */

    for(cc = buf; *cc != '\0'; cc++)
	if(!ISPRINT(*cc) && *cc != '\t') *cc = ' ';
    cc--;
    while(cc >= buf && isspace(*cc)) *cc-- = '\0';
    return cp;
}
   
 void content::Denumerate(long  pos,long  len)
{
    struct content_chapentry *cp,*endcp;
    if(this->srctext == NULL) return;
    if(pos < 0){
	endcp = NULL;
	this->enumerate = FALSE;
    }
    else endcp = findcp(this,pos,len);
    for(cp = findcp(this,pos,0);cp != NULL;cp = cp->next){
	if(cp == endcp) break;
	denumber(this,cp);
    }
    this->doindent = TRUE;
    if(!this->isindented) ::doindent(this);   
    (this->srctext)->NotifyObservers(0);
}
static number(class content  *self,char  *string,struct content_chapentry  *cp)
{
    char c;
    long pos,npos,end;
    class text *src = self->srctext;
    if(string != NULL && cp != NULL) {
	int len = strlen(string);
/*	denumber(self,cp); */
	pos = (cp->rem)->GetPos();
	end =  (cp->rem)->GetPos() + (cp->rem)->GetLength() ;
	for(npos = pos; npos <end && ((c = (src)->GetChar(npos)) == '\t') || c == ' ' || c == '\n'; npos++) ;
	if(npos == end && pos + 1 < npos){
	    npos = pos + 1;
	    c = (src)->GetChar(npos);
	}
	if(npos == end){
	    (self->srctext)->AlwaysInsertCharacters(npos,string,len);
	}
	else {
	    if(c == TEXT_VIEWREFCHAR)
		(self->srctext)->AlwaysCopyText(npos + 1,self->srctext,npos,1);
	    else{
		(self->srctext)->AlwaysInsertCharacters(npos + 1,&c,1);
	    }
	    (self->srctext)->AlwaysInsertCharacters(npos + 1,string,len);
	    (self->srctext)->AlwaysDeleteCharacters(npos,1);
	}
	return TRUE;
    }
    return FALSE;

}
 long content::Enumerate(long  opos,long  len,char  *start)
{   
    int tab[NUMLEV],lev[NUMLEV],i,pos,lastlev,curlev,initializing;
    long slen,maxlen;
    char *nn;
    struct content_chapentry *cp,*endcp;
    char buf[256];
    int res;

    if(this->srctext == NULL) return 0;
    ensure(this,&(this->entry),opos,len);
    ensure(this,&(this->indexentry),opos,len);
    
    if(this->isindented) unindent(this);
    if(start){
	slen = strlen(start);
	if(*start < '0' || *start > '9') start = NULL;
	else if(start[slen - 1] != '\t'){
	    strcpy(buf,start);
	    buf[slen - 1] = '\t';
	    buf[slen] = '\0';
	    start = buf;
	    slen++;
	}
    }
    initializing = FALSE;
    for(i = 0; i < NUMLEV; i++) lev[i] = tab[i] = 0;
    for(cp = this->entry; cp != NULL;cp = cp->next)
	tab[cp->which]++;
    lastlev = 0;curlev = 0;
    pos = 0;maxlen = 0;
    if(opos < 0) {
	endcp = NULL;
	this->enumerate = TRUE;
    }
    else endcp = findcp(this,opos,len);
    cp = findcp(this,opos,0);
    if(endcp == cp) return 0;
    if(start !=	NULL &&	cp != NULL) {
	res = denumber(this,cp);
	while (cp->loc == NULL || (cp->loc)->GetLength() == 0 || res){
	    if((cp = cp->next) == endcp || cp == NULL ) break;
	    res = denumber(this,cp);
	}
	if(cp && cp != endcp){
	    number(this,start,cp);
	    pos = (cp->loc)->GetPos();
	    if((this)->StringToInts(pos,lev) == 0){
		/* should never happen */
		for(i = 0; i < NUMLEV; i++) lev[i] = 0;
	    }
	    else initializing = TRUE;
	}
    }
    
    for(;cp != NULL && cp != endcp;cp = cp->next){
	if(!initializing) res = denumber(this,cp);
	if(res || cp->loc == NULL /* || mark_GetLength(cp->loc) == 0 */) continue;
	pos = (cp->loc)->GetPos();
	if(cp->which != lastlev){
	    for(curlev = 0, i = 0; i < cp->which; i++) 
		if(tab[i]) curlev++;
	    for(i = curlev+1; i < NUMLEV; i++)
		lev[i] = 0;
	    lastlev = cp->which;
	    if(initializing){
		initializing = FALSE;
		continue;
	    }
	}
	lev[curlev]++;
	nn = chapnote(lev);
	slen = strlen(nn);
	if(number(this,nn,cp) == FALSE)
	    lev[curlev]--;
	else {
	    if(slen > maxlen) maxlen = slen;
	}
    }
    (this)->NotifyObservers(0);
    (this->srctext)->NotifyObservers(0);
    return maxlen;
}

static void doindent(class content  *self)
{   
    int tab[NUMLEV],i,lastlev,curlev;
    struct content_chapentry *cp;
    static char buf[] = "                                                          ";

    if(self->srctext == NULL) return;
    for(i = 0; i < NUMLEV; i++) tab[i] = 0;
    for(cp = self->entry; cp != NULL;cp = cp->next)
	tab[cp->which]++;
    lastlev = 0;curlev = 0;
    for(cp = self->entry;cp != NULL;cp = cp->next){
	if(cp->which != lastlev){
	    for(curlev = 0, i = 0; i < cp->which; i++) 
		if(tab[i]) curlev++;
	    lastlev = cp->which;
	}
	cp->space = curlev;
	(self)->AlwaysInsertCharacters((cp->loc)->GetPos(),buf,cp->space *INDENTSPACE);
    }
    self->isindented = TRUE;
}
static void unindent(class content  *self)
{   
    struct content_chapentry *cp;
    long len,start;
    start = 0;
    for(cp = self->entry;cp != NULL;cp = cp->next){
	if((len =  (cp->loc)->GetPos() - start) > 0){
	    (self)->AlwaysDeleteCharacters(start,len);
	}
	start = (cp->loc)->GetPos() + (cp->loc)->GetLength() + 1;
	cp->space = -1;
   }
    self->isindented = FALSE;
}
static struct content_chapentry *newentry(class content  *self,struct content_chapentry  *next,boolean  addmark)
{
    struct content_chapentry *cp;
    cp = (struct content_chapentry *)malloc(sizeof(struct content_chapentry ));
    cp->rem = (self->srctext)->CreateMark(0,0);
    (cp->rem)->SetStyle(FALSE,TRUE);
    if(addmark){
	cp->loc = (self)->CreateMark(0,0);
	(cp->loc)->SetStyle(FALSE,FALSE);
    }
    else cp->loc = NULL;
    cp->next = next;
    cp->space = -1;
    return cp;
}
static void freeentry(class content  *self,struct content_chapentry  *cp, struct content_chapentry  *lastcp)
{
    if(cp->loc) (self)->RemoveMark(cp->loc);
    if(self->srctext && cp->rem ) (self->srctext)->RemoveMark(cp->rem);
    if(lastcp != NULL)
	lastcp->next = cp->next;
    free(cp);
}   
static int ensure(class content  *self,struct content_chapentry  **base,long  pos,long  len)
{
    struct content_chapentry *cp,*lastcp;
    long llen;
    int count;
    if(self->srctext == NULL || self->InUpdate) return 0;
    self->InUpdate = TRUE;
    count = 0;
    /* see if any marks have changed */
    lastcp = NULL;
    for(cp = *base; cp != NULL; lastcp = cp,cp = cp->next){
	if((cp->loc)->GetModified()){
	    if((cp->loc)->GetPos() < pos) continue;
	    llen = (cp->loc)->GetLength();
	    if(pos > 0 && (cp->loc)->GetPos() + llen > pos + len) break;
	    if((cp->loc)->GetLength() == 0) {
		if(lastcp == NULL) *base = cp->next;
		freeentry(self,cp,lastcp);
		count++;
		if(lastcp == NULL){
		    cp = *base;
		    if(cp == NULL) break;
		}
		else cp = lastcp;
		continue;
	    }
	}
    }
    self->InUpdate = FALSE;
    return count;
}
static void copymark(class text  *desttext,class mark  *destmark,class text  *srctext,class mark  *srcmark,boolean  cap)
{
    char *c,buf[256],*cp;
    long len = (srcmark)->GetLength();

    if(len > 250) c = (char *)malloc(len + 10);
    else c = buf;
    (srctext)->CopySubString((srcmark)->GetPos(),len,c,FALSE);
 /*   if(cap && islower(*c)) *c = toupper(*c); */
    for(cp = c; *cp != '\0'; cp++) if(!ISPRINT(*cp) && !isspace(*cp)) *cp = ' ';
    (desttext)->AlwaysReplaceCharacters((destmark)->GetPos(),(destmark)->GetLength(),
				  c,len);

    if(len > 250) free(c);
    (destmark)->SetLength(len);
    (destmark)->SetModified(0);
    (desttext)->NotifyObservers(0);
}
static void freeentrys(class content  *self)
{
    struct content_chapentry *cp,*lastcp;
    lastcp = NULL;
    for(cp = self->entry; cp != NULL;cp = cp->next){
	if(lastcp != NULL){
	    freeentry(self,lastcp,NULL);
	}
	lastcp = cp;
    }
    if(lastcp != NULL){
	freeentry(self,lastcp,NULL);
    }
    lastcp = NULL;
    for(cp = self->indexentry; cp != NULL;cp = cp->next){
	if(lastcp != NULL){
	    freeentry(self,lastcp,NULL);
	}
	lastcp = cp;
    }
    if(lastcp != NULL){
	freeentry(self,lastcp,NULL);
    }
    self->entry = self->indexentry = NULL;
}
static void clear(class content  *self)
{
    (self)->SetReadOnly(FALSE);
    freeentrys(self);
    (self)->Clear();
    (self)->SetReadOnly(TRUE);
}
static boolean ns(class content  *self,class text  *text,long  pos,class environment  *env)
{
    if (env->type == environment_Style){
	NoteStyle(self,pos,(env)->GetLength(),env->data.style);
    }
    return FALSE;
}
#define INDEXHEADER "------INDEX----\n"

void content::reinit()
{
    struct content_chapentry *cp;
    if(this->srctext == NULL) return;
    clear(this);
    (this->srctext)->EnumerateEnvironments(0, (this->srctext)->GetLength(),(text_eefptr)ns,(long)this);
    if(this->indexentry){
	(this)->AlwaysInsertCharacters((this)->GetLength() ,INDEXHEADER,strlen(INDEXHEADER));
    }
    for(cp = this->indexentry; cp != NULL ; cp = cp->next){
	(cp->loc)->SetPos((this)->GetLength());
	copymark((class text *)(this),cp->loc,this->srctext,cp->rem,TRUE);
	(this)->AlwaysInsertCharacters((cp->loc)->GetPos() + (cp->loc)->GetLength() + 1,"\n",1);
    }
    this->isindented = FALSE;
    if(this->chapcount != -1){
	/* determine if new text is already enumerated */
	int i = 0;
	for(cp = this->entry; cp != NULL; cp = cp->next)
	    i++;
	if(this->chapcount > 0 && this->chapcount > i / 4)
	    this->doindent = FALSE;
	this->chapcount = -1;
    }
    if(this->enumerate) (this)->Enumerate(-1,0,NULL);
    else if(this->doindent) ::doindent(this);

}

int content::StyleNameContentLevel(register char  *name)
{
    register char **sp;
    register int which;

    if (contentstylecount < 0) {
	initcontentstylenames();
    }

    if(name == NULL) return 0;
    which = 0;
    for(sp = contentstylenames;sp &&  *sp && **sp; sp++){
	if(which++ == contentstylecount) break;
	if(**sp == *name && strcmp(*sp,name) == 0){
	    return which;
	}
    }
    return 0;
}
static indexstyle(register char  *name)
{
    register char **sp;
    register int which = 0;
    if(name == NULL) return 0;
    for(sp = indexnames;which < indexnamecount && sp &&  *sp && **sp; sp++){
	which++;
	if(**sp == *name && strcmp(*sp,name) == 0){
	    return which;
	}
    }
    return 0;
}

static struct content_chapentry *addindexentry(class content  *self,long  pos,long  len,struct content_chapentry  **base)
{
    char buf[512],Buf[512];
    struct content_chapentry *cp,*lastcp;
    int rc;
    lastcp = NULL;
    /* should probably keep and end pointer around to save time here */
    if(len > 512) len = 512;
    (self->srctext)->CopySubString(pos,len,buf,FALSE);
    strcpy(Buf,buf);
    if(islower(*buf)) *Buf = toupper(*buf);
    else *buf = tolower(*Buf);
    for(cp = *base; cp != NULL;cp = cp->next){
	rc = (self->srctext)->GetChar((cp->rem)->GetPos());
	if(islower(rc)){
	    if(rc > *buf || (rc == *buf && (self->srctext)->Strncmp((cp->rem)->GetPos(),buf,len) > 0))
		break;
	    
	}
	else {
	    if(rc > *Buf || (rc == *Buf && (self->srctext)->Strncmp((cp->rem)->GetPos(),Buf,len) > 0))
		break;
	}
	lastcp = cp;
    }
    if(lastcp){
	cp = newentry(self,cp,TRUE);
	lastcp->next = cp;
    }
    else{
	cp = newentry(self,*base,TRUE);
	*base = cp;
    }
    return cp;
}
static struct content_chapentry *insertentry(class content  *self,long  pos,long  len,struct content_chapentry  **base)
{
    struct content_chapentry *cp,*lastcp;
    lastcp = NULL;
    /* should probably keep and end pointer around to save time here */
    for(cp = *base; cp != NULL; lastcp = cp,cp = cp->next){
	if((cp->rem)->GetPos() > pos) break;
    }
    if(lastcp){
	cp = newentry(self,lastcp->next,TRUE);
	lastcp->next = cp;
	(cp->loc)->SetPos((lastcp->loc)->GetPos() + (lastcp->loc)->GetLength() + 1);
    }
    else{
	cp = newentry(self,*base,TRUE);
	*base = cp;
    }
    return cp;

}

static void NoteStyle(class content  *self,long  pos,long  len,class style  *style)
{
    int which;
    char *sn;
    struct content_chapentry *cp;
    if(style == NULL || ((sn = (style)->GetName()) == NULL)) return;
    if((which = content::StyleNameContentLevel(sn)) > 0){
	  struct content_chapentry *cp;
	  if(skipnewlines(self->srctext,&pos,&len)){
	      if(self->chapcount >= 0){
		  /* counting number of entries w/ chap numbers */
		  long npos,nlen;
		  npos = pos;
		  nlen = len;
		  skipchapnumber(self->srctext,&npos,&nlen,TRUE);
		  if(npos != pos) self->chapcount++;
	      }
	     cp = insertentry(self,pos,len,&(self->entry));
	     setchap(cp,pos,len);
	     copymark((class text *)(self),cp->loc,self->srctext,cp->rem,FALSE);
	     (self)->AlwaysInsertCharacters((cp->loc)->GetPos() + (cp->loc)->GetLength() + 1,"\n",1);
	     cp->which = which;
	     (cp->rem)->SetModified(0);
	     }
      }
    else if((which = indexstyle(sn))!= 0) {
	if(skipnewlines(self->srctext,&pos,&len) &&
	   skipchapnumber(self->srctext,&pos,&len,TRUE) ){
	   cp = addindexentry(self,pos,len,&(self->indexentry));
	   setchap(cp,pos,len);
	   }
	/* later the index will added to the end of the document 
	 */
    }
}
static void doshuffle(class content  *self)
{
    /* punt for now */
 /*   content_reinit(self);  */
}
static void checknewline(class content  *self,struct content_chapentry  *cp)
{
    /* punt for now */
}
static boolean updatemark(class text  *d,class mark  *m,boolean  nonum)
{
    long pos ,len;
    pos = (m)->GetPos();
    len = (m)->GetLength();
    if(skipnewlines(d,&pos,&len) &&
	(!nonum || skipchapnumber(d,&pos,&len,TRUE)) ){
	if(pos != (m)->GetPos()) (m)->SetPos(pos);
	if(len != (m)->GetLength()) (m)->SetLength(len);
	return TRUE;
    }
    else return FALSE;
}
static void mod(class content  *self,struct content_chapentry  **base,boolean  nonum)
{
    struct content_chapentry *cp,*lastcp;
/*    int shuffle = 0; */
    if(self->InUpdate) return;
    self->InUpdate = TRUE;
    /* see if any marks have changed */
    lastcp = NULL;
    for(cp = *base; cp != NULL; lastcp = cp,cp = cp->next){
	if((cp->rem)->GetModified()){
	    /* update local mark */
	    if(!updatemark(self->srctext,cp->rem,nonum )){
		if(lastcp == NULL) *base = cp->next;
		freeentry(self,cp,lastcp);
		if(lastcp == NULL){
		    cp = *base;
		    if(cp == NULL) break;
		}
		else cp = lastcp;
		continue;
	    }
	    copymark((class text *)(self),cp->loc,self->srctext,cp->rem,nonum);
/*
	    if(lastcp && mark_GetPos(cp->rem) < mark_GetPos(lastcp->rem))
		shuffle++;
*/
	    checknewline(self,cp);
	    (cp->loc)->SetModified( 0);
	    (cp->rem)->SetModified( 0);
	}
    }
/*    if(shuffle)  marks are out of order 
	doshuffle(self); */
    self->InUpdate = FALSE;
}
static void update(class content  *self)
{

    mod(self,&(self->entry),FALSE); 
    mod(self,&(self->indexentry),TRUE);
}
void content::UpdateSource(long  pos,long  len)
{
    /* unnecessary function */
}

static void initcontentstylenames()
{
    char *p;
    contentstylenames[0] = NULL;
    if((p = environ::Get("ContentsList")) == NULL){
	if((p = environ::GetProfile("ContentsList"))== NULL)
	    p = defaultlist;
    }
    contentstylecount = appendlist(contentstylenames,0,p,FALSE);
}

content::content()
{
    this->entry = this->indexentry = NULL;
    this->srctext = NULL;
    this->InUpdate = FALSE;
    this->enumerate = FALSE;
    this->isindented = FALSE;
    THROWONFAILURE( TRUE);
}
void content::SetSourceText(class text  *txt)
{
    if(this->srctext != txt){
	if(this->srctext){
	    clear(this);
	    (this->srctext)->RemoveObserver(this);
	}
	this->srctext = txt;
	this->chapcount = 0;
	this->doindent = TRUE;
	(this)->reinit();
	(txt)->AddObserver(this);
	(this)->SetReadOnly(TRUE);
    }
}

content::~content()
{
    if(this->srctext){
	freeentrys(this);
	(this->srctext)->RemoveObserver(this);
    }
}

void content::ObservedChanged(class observable  *changed,long  value)
{
    if(changed == (class observable *) this->srctext){
	if(value == observable_OBJECTDESTROYED){
	    this->srctext = NULL;
	    clear(this);
	}
	else update(this);
    }
    else (this)->text::ObservedChanged(changed,value);
}
char *content::ViewName()
{
return "contentv";
}
