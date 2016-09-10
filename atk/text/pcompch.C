/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved      *
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

#include <andrewos.h> /* sys/file.h and string(s).h */
ATK_IMPL("pcompch.H")

#include <ctype.h>

#include <environ.H>
#include <environment.H>
#include <text.H>
#include <textview.H>
#include <mark.H>

#include <pcompch.H>
#define MAXMACROLEN 15

static struct composites_s *composites[MAXCHAR+1];
static char *troffmagic[256];
static char *asciimagic[256];
static unsigned char order[256];

static const char hex[]="0123456789abcdef";
static const char octal[]="01234567";




ATKdefineRegistry(pcompch, ATK, pcompch::InitializeClass);
static void cleanmagic();
static int TranslateKeySequence(char  *from, char  *to);
static int parseBackslashed(char  **fromChars);
static  char ahotoi(char  *ptr,int  base2);
static  short parsecode(char  *ptr);
static void scanerr(const char  *msg,char  key,struct composites_s  *new_c,long  line);
static char *scanstring(char  **str);
static boolean scancomposites(char  *ptr,long  line);
static void fix_fgets(char  *buf);
static int lenorder(unsigned char *e1,unsigned char *e2);


static void cleanmagic()
{
    int i;
    for(i=0;i<=255;i++) {
	if(troffmagic[i]) free(troffmagic[i]);
	if(asciimagic[i]) free(asciimagic[i]);
    }
}

/* The following two function were taken from basics/common/init.c on June 12, 1990 */
/* Translate a key sequence that has ^A, \ddd, and \c conventions. */
static int TranslateKeySequence(char  *from, char  *to)
{
    while (*from != '\0') {
	if (*from == '\\') {

	    int temp = parseBackslashed(&from);

	    if (temp == -1)
		return -1;
	    else
		*to++ = temp;
	}
	else if (*from == '^') {
	    ++from;
	    if (*from == 0)
		return -1;
	    *to++ = (*from++) & 0x1f;
	}
	else
	    *to++ = *from++;
    }
    *to++ = 0;
    return 0;
}

static int parseBackslashed(char  **fromChars)
{

    int returnChar;
    char *from = *fromChars;
    static const char bsSource[] = "ebrnt";
    static const char bsDest[] = "\033\b\r\n\t";

    if (*from == '\\') {
	++from;
	if (*from == 0)
	    return -1;
	if (isdigit(*from)) {

	    int sum = 0, i;

	    for (i = 0; i < 3; ++i) {
		if (!isdigit(*from))
		    break;
		if (*from == '8' || *from == '9')
		    return -1;
		sum = sum * 8 + *from - '0';
		++from;
	    }
	    returnChar = sum;
	}
	else {

	    const char *p;

	    p = strchr(bsSource, *from);
	    if (p != NULL)
		returnChar = bsDest[p-bsSource];
	    else
		returnChar = *from;
	    ++from;
	}
    }
    else
	return -1;
    *fromChars = from;
    return returnChar;
}

/* ahotoi: used by pcompch_insert to parse a hexadecimal or octal number depending on if base is 3 or 4. */
static  char ahotoi(char  *ptr,int  base2)
{
    unsigned char result=0;
    unsigned char c;
    const char *i;
    const char *base=(base2==3)?octal:hex;
    while(*ptr && *ptr!=' ') {
	result<<=base2;
	c=(unsigned char)(*ptr);
	if(isupper((char)c)) c=(unsigned char)tolower((char)c);
	i=strchr(base,(char)c);
	if(!i) return '\0';
	result+=i-base;
	ptr++;
    }
    return (char)result;
}

/* parsecode: parse a code in decimal,hex, octal or simply set the high bit of an ASCII character. */
static  short parsecode(char  *ptr)
{
    switch(*ptr) {
	case '|':
	    return ((unsigned char )ptr[1])|0x80;
	case 'x':
	    return ahotoi(ptr+1,4);
	case 'o':
	    return ahotoi(ptr+1,3);
	case 'd':
	    ptr++;
	default:
	    return atoi(ptr);
    }
}

static void scanerr(const char  *msg,char  key,struct composites_s  *new_c,long  line)
{
    fprintf(stderr,msg,line);
    printf(msg,line);
    if(key) fprintf(stderr,"Key: %c\n",key);
    if(new_c) free(new_c);
}

/* scanstring: returns a pointer to the first un-escaped terminator in a string */
static char *scanstring(char  **str)
{
    boolean flag=FALSE;
    char *end=(*str);
    if(*end=='\"') {
	(*str)++;
	end++;
	while(*end && (flag || *end!='\"')) {
	    if(flag) flag=FALSE;
	    else if(*end=='\\')
		flag=TRUE;
	    end++;
	}
    } else {
	/* find the terminator, a space or tab */
	while(*end && *end!=' ' && *end!='\t') end++;
    }
    return end;
}

/* scancomposites: reads in (from the line pointed to by PTR) the extensions which may be used with the character KEY and puts the entries into the composites table giving the extensions and corresponding character code.
 */
static boolean scancomposites(char  *ptr,long  line)
{
    char *end,*ptr2,*style=NULL;
     short code;
    struct composites_s *new_c=NULL;
    /* Skip leading spaces */
    while(*ptr==' ' || *ptr =='\t') ptr++;

    /* Handle  a quoted key sequence,  setting end to the ending
      quote if there is one */
    end=scanstring(&ptr);
    
    /* Terminate the keysequence string with a nul. */
    if(*end) *end='\0';
    else {
	/* if there is no code assume this is a request to delete
	    the given key sequence from the table */
	pcompch::DeleteComposite(*ptr,ptr+1);
	return TRUE;
    }
    ptr2=end;
    /* Skip any white space after the key sequence */
    while(*++ptr2==' ' || *ptr2=='\t');

    if(!*ptr2) {
	scanerr("No character code in line %ld\n",*ptr,new_c,line);
	return FALSE;
    }
    
    code=parsecode(ptr2);

    /* skip the character code and any whitespace following
      it */
    while(*ptr2 && *ptr2!=' ' && *ptr2!='\t') ptr2++;
    while(*ptr2==' ' || *ptr2=='\t') ptr2++;
    
    /* if there is anything after the trailing white space
	assume it is a troff macro for the character */
    if(*ptr2&&*ptr2!='!') {
	for(end=ptr2+1;*end && *end!=' ' && *end!='\t';end++);
	if(*end) *end++='\0';
	if(strlen(ptr2)>MAXMACROLEN) {
	    scanerr("troff macro too long in line %ld\n",*ptr,new_c,line);
	    return FALSE;
	}
	if(troffmagic[code]) free(troffmagic[code]);
	troffmagic[code]=strdup(ptr2);
	if(!troffmagic[code]) {
	    scanerr("Memory error in line %ld\n",*ptr,new_c,line);
	    return FALSE;
	}
	ptr2=end;
    } else if (*ptr2=='!') ptr2++;
    
    while(*ptr2==' ' || *ptr2=='\t') ptr2++;

    if(*ptr2) {
	end=scanstring(&ptr2);
	if(*end) *end++='\0';
	if(*ptr2) style=ptr2;
	ptr2=end;
	while(*ptr2==' ' || *ptr2=='\t') ptr2++;
	if(*ptr2 && *ptr2!='!') {
	    end=scanstring(&ptr2);
	    if(*end) *end++='\0';
	    if(*ptr2) {
		if(asciimagic[code]) free(asciimagic[code]);
		asciimagic[code]=strdup(ptr2);
		if(!asciimagic[code]) {
		    scanerr("Memory error in line %ld\n",*ptr,new_c,line);
		    return FALSE;
		}
	    }
	} else if(*ptr2=='!') ptr2++;
    }

    /* Add the keysequence to the compositions table */
    new_c=(struct composites_s *) malloc(sizeof(struct composites_s));
    
    if(!new_c) {
	scanerr("Memory error while reading composites file.\n", 0, new_c, line);
	return FALSE;
    }

    if(*ptr) {
	if(strlen(ptr)>sizeof(new_c->exts)) {
	    scanerr("Extension too long in line %ld.\n",*ptr, new_c, line);
	    return FALSE;
	}
	TranslateKeySequence(ptr,ptr);
	strcpy((char *) new_c->exts,ptr+1);
	if(style) strcpy((char *) new_c->style,style);
	else (new_c->style)[0]='\0';
	new_c->code=code;
	pcompch::DeleteComposite(*ptr,(char *)new_c->exts);
	new_c->next=composites[(unsigned char)*ptr];
	composites[(unsigned char)*ptr]=new_c;
    }
    return TRUE;
}

    
/* fix_fgets: just changes the first newline in BUF to a null.  This is just to make things easy after using fgets. */  
static void fix_fgets(char  *buf)
{
    char *p=strchr(buf,'\n');
    if(p) *p='\0';
}

void pcompch::ATKToASCII(class text  *text,long  pos,long  len,pcompch_asciifptr  func,long  rock)
{
	ATKinit;

    class mark *area;
    if(len==0) {
	len=(text)->GetLength();
	pos=0;
    }
    
    area=(text)->CreateMark(pos,len);
    if(!area) return;

    while(pos<(area)->GetPos()+(area)->GetLength()) {
	long ch;
	ch=(text)->GetChar(pos);
	if(ch==EOF) return;
	ch &= 0xff;
	if(asciimagic[ch]) pos=func(text,pos,ch,asciimagic[ch],rock);
	else pos++;
    }
    (text)->RemoveMark(area);
    delete area;
}

void pcompch::ASCIIToATK(class text  *text,long  pos,long  len,pcompch_atkfptr  func,long  rock)
{
	ATKinit;

    long i,j;
    long end;
    class mark *area;
    if(len==0) {
	len=(text)->GetLength();
	pos=0;
    }
    area=(text)->CreateMark(pos,len);
    if(!area) return;
    for(i=0;i<256;i++) {
	pos=(area)->GetPos();
	j=order[i];
	if(asciimagic[j]) {
	    long len=strlen(asciimagic[j]);
	    end=(area)->GetPos()+(area)->GetLength();
	    do {
		pos=(text)->Index( pos, asciimagic[j][0], end-pos);
		if(pos==EOF) break;
		if(pos+len<=end && !(text)->Strncmp( pos, asciimagic[j], len)) {
		    pos=func(text,pos,j,asciimagic[j],rock);
		} else pos++;
		end=(area)->GetPos()+(area)->GetLength();
	    } while(pos!=EOF && pos+len <= end);
	}
    }
    (text)->RemoveMark(area);
    delete area;
}

static int lenorder(unsigned char *e1,unsigned char *e2)
{
    if(!asciimagic[*e2] || !asciimagic[*e1]) return 0;
    return strlen(asciimagic[*e2])-strlen(asciimagic[*e1]);
}
	

/* pcompch_ReadCompositesFile: reads COMPFILE and places the composites defined in it into the composites table. */
boolean pcompch::ReadCompositesFile(const char  *compfile)
{
	ATKinit;

    char buf[1024],*ptr=buf;
    boolean end=FALSE,err=FALSE;
    FILE *fp=fopen(compfile,"r");
    long line=0,i;
    if(!fp) return FALSE;
    while(!feof(fp)&!end) {
	*buf='\0';
	if(!fgets(buf,sizeof(buf),fp)) end=TRUE;
	fix_fgets(buf);
	line++;
	if(*buf=='#'||!*buf) continue;
	while(*ptr==' ' || *ptr=='\t') ptr++;
	if(!scancomposites(ptr,line)) err=end=TRUE;
    }
    for(i=0;i<=255;i++) order[i]=(unsigned char)i;
    qsort(order,256,1,QSORTFUNC(lenorder));
    return !err;
}

/* pcompch_ClearComposites: clears out all defined compositions,
  not tested yet but should be correct. */
void pcompch::ClearComposites()
{
	ATKinit;

    int i;
    for(i=1;i<=MAXCHAR;i++) {
	struct composites_s *next=composites[i];
	while(next) {
	    struct composites_s *this_c=next;
	    next=this_c->next;
	    free(this_c);
	}
	composites[i]=NULL;
    }
}

/* pcompch_DeleteComposite: finds and deletes one composite
  from the composites table.  */
void pcompch::DeleteComposite(char key,char *exts)
{
	ATKinit;

    struct composites_s *next=composites[(unsigned char)key],*last=NULL;
    while(next) {
	struct composites_s *this_c=next;
	next=this_c->next;
	if(strcmp((char *) exts, (char *) this_c->exts)) last=this_c;
	else {
	    if(last) last->next=this_c->next;
	    else composites[(unsigned char)key]=this_c->next;
	    free(this_c);
	}
    }
}

/* pcompch_EnumerateComposites: the function FUNC is called on each
  composite of key until it returns a non-zero value or there are no more
  composites.
  The call to func is made as below:
    func(key,composite,rock);
  where	composite is of type struct composites_s. */
long pcompch::EnumerateComposites(char  key,pcompch_ecfptr  func,long  rock)
{
	ATKinit;

    long result;
    struct composites_s *next=composites[(unsigned char)key];
    while(next) {
	struct composites_s *this_c=next;
	next=this_c->next;
	if((result=func(key,this_c,rock))) return result;
    }
    return 0;
}

char *pcompch::CharacterToTroff( char ch,class environment  *env,class textview  *tv)
{
	ATKinit;

    return troffmagic[(unsigned char)ch];
}

 char *pcompch::StringToTroff( char *str,char  *buf,long  bufsize,class environment  *env,class textview  *tv)
{
	ATKinit;

    char buf2[16],*bp=buf;
    memset(buf,0,bufsize);
    while(*str && buf-bp<bufsize-1) {
	if(*str=='\\'&& buf-bp<bufsize-2) {
	    *bp++='\\';
	    *bp++='\\';
	    str++;
	    continue;
	}    
	if(isprint(*str) || isascii(*str) || *str == '\t') *bp++=(*str++);
	else {
	     char *ccp=pcompch::CharacterToTroff(*str,env,tv);
	    if(ccp) {
		if((int)(buf-bp+strlen((char *) ccp))>=bufsize-1) return str;
		while(*ccp) *bp++=(*ccp++);
	    } else {
		sprintf(buf2,"\\\\%3.3o",*str);
		if((int)(buf-bp+strlen(buf2))>=bufsize-1) return str;
		strcat(bp,buf2);
		bp+=strlen(buf2);
	    }
	    str++;
	}
    }
    return str;
}
	    

boolean pcompch::InitializeClass()
{
    const char *compfile=environ::GetProfile("compositesfile");
    boolean override= environ::GetProfileSwitch("overridecomposites",FALSE);
    const char *libfile=environ::AndrewDir("/lib/compchar/comps");
    int i;

    for(i=0;i<=MAXCHAR;i++) composites[i]=NULL;
    
    for(i=0;i<=255;i++) {
	troffmagic[i]=NULL;
	asciimagic[i]=NULL;
    }
    
    if(override || access(libfile,F_OK)) {
	if(!compfile) {
	    fprintf(stderr,"No composites file given in preferences.\n");
	    return FALSE;
	}
    } else if(!override && !pcompch::ReadCompositesFile(libfile)) {
	fprintf(stderr,"Error reading default composites file:%s\n", libfile);
	pcompch::ClearComposites();
	cleanmagic();
	return FALSE;
    }
    if(compfile && !pcompch::ReadCompositesFile(compfile)) {
	fprintf(stderr,"Error reading composites file:%s.\n", compfile);
	pcompch::ClearComposites();
	cleanmagic();
	return FALSE;
    }
    return TRUE;
}
