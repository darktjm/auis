/*********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved          *
 *        For full copyright information see:'andrew/doc/COPYRITE'       *
\* ********************************************************************* */

#include <andrewos.h> /* sys/file.h and string(s).h */
ATK_IMPL("compchar.H")

#include <ctype.h>

#include <proctable.H>
#include <compchar.H>
#include <textview.H>
#include <text.H>
#include <environ.H>
#include <message.H>
#include <observable.H>
#include <frame.H>
#include <framemessage.H>
#include <style.H>
#include <stylesheet.H>
#include <fontdesc.H>
#include <buffer.H>
#include <keymap.H>
#include <keystate.H>
#include <environment.H>
#include <pcompch.H>
#include <im.H>

static class style *boldulined=NULL,*fixed=NULL;

static struct proctable_Entry *nop=NULL,*insertchar=NULL;

static proctable_fptr textview_SelfInsertCmd=NULL;

static const char leftaccent[]="AEIOUYaeiouy";
static  const unsigned char leftaccentcodes[]={0xc1,0xc9,0xcd,0xd3,0xda,0xdd,0xe1,0xe9, 0xed,0xf3,0xfa,0xfd};

static const char rightaccent[]="AEIOUaeiou";
static  const unsigned char rightaccentcodes[]={0xc0,0xc8,0xcc,0xd2,0xd9,0xe0,0xe8,0xec, 0xf2,0xf9};

static const char hat[]="AEIOUaeiou";
static  const unsigned char hatcodes[]={0xc2,0xca,0xce,0xd4,0xdb,0xe2,0xea,0xee,0xf4,0xfb};

static const char tilde[]="ANOano";
static  const unsigned char tildecodes[]={0xc3,0xd1,0xd5,0xe3,0xf1,0xf5};

static const char umlaut[]="AEIOUaeiouy";
static  const unsigned char umlautcodes[]={0xc4,0xcb,0xcf,0xd6,0xdc,0xe4,0xeb,0xef,0xf6, 0xfc,0xff};

static const char hex[]="0123456789abcdef";
static const char octal[]="01234567";

/* ahotoi: used by compchar_insert to parse a hexadecimal or octal number depending on if base is 3 or 4. */

ATKdefineRegistry(compchar, ATK, compchar::InitializeClass);
static  char ahotoi(char  *ptr,int  base2);
static  char parsecode(char  *ptr);
static void SelfInsertCmd(class textview *tv, char code,char  *style);
static void compchar_insert(class textview  *tv,char  *ptr);
static void compchar_modifier(class textview  *tv,char  *ptr,const char  *list,const unsigned char *codes);
static enum keymap_Types doafter(struct arock  *rock,const char  key,struct proctable_Entry  **ppe,long  *prock);
static void after(class textview  *tv,const char  *list, const unsigned char *codes,char  *ch);
static void compchar_leftaccentafter(class textview  *tv,char  *rock);
static void compchar_rightaccentafter(class textview  *tv,char  *rock);
static void compchar_umlautafter(class textview  *tv,char  *rock);
static void compchar_tildeafter(class textview  *tv,char  *rock);
static void compchar_hatafter(class textview  *tv,char  *rock);
static void compchar_leftaccent(class textview  *tv,char  *ptr);
static void compchar_rightaccent(class textview  *tv,char  *ptr);
static void compchar_umlaut(class textview  *tv,char  *ptr);
static void compchar_tilde(class textview  *tv,char  *ptr);
static void compchar_hat(class textview  *tv,char  *ptr);
static struct composites_s *composework(char  key,struct composites_s  *c,char  *exts);
static long match(char  key,struct composites_s  *c,long  rock);
static void helpProc(char  *partial,struct helpRock  *myrock,message_workfptr  HelpWork,long  rock);
static void compchar_compose(class textview  *tv,char  *ptr);
static  char keywork(char  key,struct composites_s  *c,struct YARock  *rock);
static enum keymap_Types docomposing(struct YARock  *rock,long  key,struct proctable_Entry  **ppe,long  *prock);
static void compchar_nop(class textview  *tv, char  *possibilities);
static void compchar_compose2(class textview  *tv,char  *ch);
static long doatkreplacement(class text  *text,long  pos,char  *ascii,struct SARock  *r);
static void compchar_ATKToASCII(class textview  *tv,long  rock);
static long doasciireplacement(class text  *text,long  pos, long ch,char  *ascii,long lr);
static void compchar_ASCIIToATK(class textview  *tv,long  rock);


static  char ahotoi(char  *ptr,int  base2)
{
     char result=0;
    char c;
    const char *i;
    const char *base=(base2==3)?octal:hex;
    while(*ptr && *ptr!=' ') {
	result<<=base2;
	c=(*ptr);
	if(isupper(c)) c=tolower(c);
	i=strchr(base,c);
	if(!i) return '\0';
	result+=i-base;
	ptr++;
    }
    return result;
}

/* parsecode: parse a code in decimal,hex, octal or simply set the high bit of an ASCII character. */
static  char parsecode(char  *ptr)
{
    switch(*ptr) {
	case '|':
	    return ptr[1]|0x80;
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

static void SelfInsertCmd(class textview *tv, char code,char  *style)
{
    class text *t=(class text *)(tv)->GetDataObject();
    class stylesheet *s=(t)->GetStyleSheet();
    class style *st;
    class environment *env;
    long pos=(tv)->GetDotPosition(),pos2;
    const char *tmpl2;
    char *tmpl;
    textview_SelfInsertCmd(tv,code);
    if(style[0]) {
	if(!s) {
	    message::DisplayString(tv,0,"No styles available.\n");
	    return;
	}
	pos2=(tv)->GetDotPosition();
	if(pos2<=pos) return;
	
	char *tstyle=strdup(style);
	if(!tstyle) return;
	tmpl=strchr(tstyle,',');
	if(tmpl) {
	    tmpl2=tmpl+1;
	    *tmpl='\0';
	    while(*tmpl2==' ') tmpl2++;
	} else tmpl2="symbol";

	st=(s)->Find(tstyle);
	if(!st) {
	    if((t)->ReadTemplate(tmpl2,FALSE)) {
		message::DisplayString(tv,99,"Warning: proper template could not be loaded.");
		return;
	    }
	    st=(s)->Find(tstyle);
	}
	free(tstyle);
	if(!st) {
	    message::DisplayString(tv,0,"Couldn't find style.\n");
	    return;
	}
	env=(t)->AddStyle(pos,pos2-pos,st);
	if(!env) {
	    message::DisplayString(tv,0,"Couldn't add style to document\n");
	    return;
	}
	(env)->SetStyle(FALSE,FALSE);
	(t)->NotifyObservers(0);
    }
}

/* compchar_insert: inserts an arbitrary character specified in an init file.  The following characters are treated specially when they occur as the first character of the argument:
      '|': inserts the character following it with the high bit set.
      'x': inserts the character whose code is given in hexadecimal.
      'o': inserts the character whose code is given in octal.
      'd': inserts the character whose code is given in decimal.
  If the first character of the argument isn't one of the above it is taken to be the decimal code for the character to be inserted. */
static void compchar_insert(class textview  *tv,char  *ptr)
{
    if((unsigned long)ptr<BADCHAR) {
	message::DisplayString(tv,0,"compchar-insert must be called with an argument from an initfile.");
	return;
    }
    textview_SelfInsertCmd(tv,parsecode(ptr));
}

/* compchar_modifier: if ptr isn't a valid character assumes it to be a pointer to an argument as provided by an .*init file.  In this case it inserts the character pointed to with the appropriate code as given by list and codes.  If ptr is a valid character then the character before the cursor is changed to the appropriate code as given by list and codes. */
static void compchar_modifier(class textview  *tv,char  *ptr,const char  *list,const unsigned char *codes)
{
    class text *t=(class text *)(tv)->GetDataObject();
    /* if given an arg in a  .*init insert it with an accent */
    if((unsigned long)ptr>MAXCHAR && ptr[0]!='\0') {
	const char *i=strchr(list,*ptr);
	if(!i) {
	    message::DisplayString(tv,0,"Character not available.");
	    return;
	}
	textview_SelfInsertCmd(tv,codes[i-list]);
    } else {
	long pos=(tv)->GetDotPosition()-1;
	const char *i;
	if(pos<0) {
	    message::DisplayString(tv,0,"No character before the cursor.");
	    return;
	}
	i=strchr(list,(t)->GetChar(pos));
	if(!i) {
	    message::DisplayString(tv,0,"Character not available.");
	    return;
	}
	(t)->ReplaceCharacters(pos, 1, (const char *)codes+(i-list),1);
    }
    (t)->NotifyObservers(observable::OBJECTCHANGED);
}

struct arock {
    class textview *tv;
    const char *list;
     const unsigned char *codes;
    keystate_fptr oldoverride;
    long oldrock;
};

/* doafter: the keystate override function for applying a umlaut, hat, tilde, etc to the next character typed;
  */
static enum keymap_Types doafter(struct arock  *rock,const char  key,struct proctable_Entry  **ppe,long  *prock)
{
    class textview *tv=rock->tv;
    const char *i=strchr(rock->list,key);
    if(!i) {
	(tv->keystate)->SetOverride((keystate_fptr)rock->oldoverride, rock->oldrock);
	free(rock);
	message::DisplayString(tv,0,"No such character.");
	*prock=0;
	*ppe=nop;
	return keymap_Proc;
    }
    (tv->keystate)->SetOverride((keystate_fptr)rock->oldoverride, rock->oldrock);
    free(rock);
    *prock=rock->codes[i-rock->list];
    *ppe=insertchar;
    return keymap_Proc;
}

/* after: the function which actually handles the work for all the compchar_*after functions, sets doafter as the override function on the textviews keystate, if the override function is already doafter it cancels the operation and if the .*init file so indicated will insert a character. */
static void after(class textview  *tv,const char  *list, const unsigned char *codes,char  *ch)
{
    struct arock *rock;
    rock=(struct arock *)malloc(sizeof(struct arock));
    if(!rock) return;
    rock->tv=tv;
    rock->list=list;
    rock->codes=codes;
    (tv->keystate)->GetOverride((keystate_fptr *)&rock->oldoverride, &rock->oldrock);
    if(rock->oldoverride==(keystate_fptr)doafter) {
	free(rock);
	free((struct arock *)rock->oldrock);
	if((unsigned long)ch>MAXCHAR && ch[0]!='\0') textview_SelfInsertCmd(tv,*ch);
	(tv->keystate)->SetOverride(NULL,0);
	return;
    }
    (tv->keystate)->SetOverride((keystate_fptr)doafter ,(long)rock);
}
/*
  compchar_leftaccentafter:
  compchar_rightaccentafter:
  compchar_umlautafter:
  compchar_tildeafter:
  compchar_hatafter:
      front-end functions to pass the appropriate list and codes
  arguments to after. */

static void compchar_leftaccentafter(class textview  *tv,char  *rock)
{
    after(tv,leftaccent,leftaccentcodes,rock);
}

static void compchar_rightaccentafter(class textview  *tv,char  *rock)
{
    after(tv,rightaccent,rightaccentcodes,rock);
}

static void compchar_umlautafter(class textview  *tv,char  *rock)
{
    after(tv,umlaut,umlautcodes,rock);
}

static void compchar_tildeafter(class textview  *tv,char  *rock)
{
    after(tv,tilde,tildecodes,rock);
}

static void compchar_hatafter(class textview  *tv,char  *rock)
{
    after(tv,hat,hatcodes,rock);
}

/*
  compchar_leftaccent:
  compchar_rightaccent:
  compchar_umlaut:
  compchar_tilde:
  compchar_hat:
      front-end functions to pass the appropriate list and codes
  arguments to compchar_modifier. */
 
static void compchar_leftaccent(class textview  *tv,char  *ptr)
{
    compchar_modifier(tv,ptr,leftaccent,leftaccentcodes);
}

static void compchar_rightaccent(class textview  *tv,char  *ptr)
{
    compchar_modifier(tv,ptr,rightaccent,rightaccentcodes);
}

static void compchar_umlaut(class textview  *tv,char  *ptr)
{
    compchar_modifier(tv,ptr,umlaut,umlautcodes);
}


static void compchar_tilde(class textview  *tv,char  *ptr)
{
    compchar_modifier(tv,ptr,tilde,tildecodes);
}

static void compchar_hat(class textview  *tv,char  *ptr)
{
    compchar_modifier(tv,ptr,hat,hatcodes);
}


/* composework: called from pcompch_EnumerateComposites with ROCK being the textview in which the compose operation began and C being the composite to be checked.  If the current composite is equal to the user's entry the appropriate character is inserted in the text and a value of one is returned to terminate the search.  Otherwise it simply returns 0. */ 
static struct composites_s *composework(char  key,struct composites_s  *c,char  *exts)
{
   if(strcmp((char *) c->exts,exts)) return NULL;
   else return c;
}

struct helpRock {
    message_workfptr HelpWork;
    class text *text;
    long rock;
    char *partial;
};

/* match: called from compchar_EnumerateComposites with ROCK being a struct helpRock as defined above.  If the user's entry up until this point matches one of the composites of KEY then that composite will be displayed in the help buffer. */
static long match(char  key,struct composites_s  *c,long  rock)
{
    struct helpRock *h=(struct helpRock *)rock;
    char buf[256];
    char ch[2];
    const char *t;
    if(*(h->partial)&&strncmp(h->partial+1,(char *) c->exts, strlen(h->partial+1))) return 0;
    ch[0]=(char)c->code;
    ch[1]='\0';
    buf[0]=key;
    strcpy(buf+1,(char *) c->exts);
    strcat(buf,"\t    ");
    strcat(buf, (char *) ch);
    strcat(buf,"\t\t");
    (*h->HelpWork)(h->rock,message_HelpGenericItem,buf,NULL);
    if(c->style[0]) {
	class style *st; 
	char *tmpl;
	strcpy((char *) buf,(char *) c->style);
	tmpl=strchr((char *) buf,',');
	if(tmpl) {
	    t=tmpl+1;
	    *tmpl='\0';
	    while(*t==' ') t++;
	} else t="symbol";
	(h->text)->ReadTemplate(t,FALSE);
	st=((h->text)->GetStyleSheet())->Find(buf);
	if(st) {
	    class environment *env=(h->text)->AddStyle( (h->text)->GetLength()-3, 1,st);
	    if(env) (env)->SetStyle(FALSE,FALSE);
	}
	if(tmpl) *tmpl=',';
    } else buf[0]='\0';
    strcat((char *) buf,"\n");
    (*h->HelpWork)(h->rock,message_HelpGenericItem,buf,NULL);
    return 0;
}

/* helpProc: called by frameview__Help after having been set as the help procedure for a message_AskForStringCompleted.  Initializes data needed by match for it to see if a composite matches the user's entry so far and procedes to use pcompch_EnumerateComposites to display a list of available composites in the help buffer. */
static void helpProc(char  *partial,struct helpRock  *myrock,message_workfptr  HelpWork,long  rock)
{
    int i;
    class text *t=myrock->text;
    (myrock->text)->ReadTemplate("default",FALSE);
    (*HelpWork)(rock,
		    message_HelpGenericItem,
		    "Entry\tCharacter\tStyle\n",
		    NULL);
    
    myrock->HelpWork=HelpWork;
    myrock->rock=rock;
    myrock->partial=partial;
    if(*partial) (void)pcompch::EnumerateComposites(*partial,
						    (pcompch_ecfptr)match,
					   (long)myrock);
    else for(i=1;i<=MAXCHAR;i++) {
	(void)pcompch::EnumerateComposites(i,
					   (pcompch_ecfptr)match,
					   (long)myrock);
    }
    (t)->SetGlobalStyle(fixed);
    (t)->AddStyle(0,5,boldulined);
    (t)->AddStyle(6,9,boldulined);
    (t)->AddStyle(16,5,boldulined);
}

/* compchar_compose: if an argument is given for this function in a .*init file then it will be taken as a string designating the composite character to be inserted. Otherwise this function prompts the user to specify a character to insert in the text.  Typing a '?' will cause a list of available characters to be displayed. */
static void compchar_compose(class textview  *tv,char  *ptr)
{
    char buf[1024];
    struct composites_s *c;
    struct helpRock myrock;
    int result;
    class framemessage *fmsg= (class framemessage *)(tv)->WantHandler("message");
    if((unsigned long)ptr>MAXCHAR && ptr[0]!='\0') {
	c=(struct composites_s *)pcompch::EnumerateComposites(*ptr, (pcompch_ecfptr)composework,(long)ptr+1);
	if(!c) {
	    message::DisplayString(tv,0,"No such composition found.\n");
	    return;
	}
	SelfInsertCmd(tv,c->code,(char *)c->style);
	return;
    }
    if(fmsg) {
	class buffer *b=(fmsg->frame)->GetHelpBuffer();
	myrock.text=(class text *)(b)->GetData();
    } else myrock.text=NULL;
    result=message::AskForStringCompleted(tv,
      					      0,
					      "Character: " ,
					      NULL, /* no default */
					      buf,
					      sizeof(buf),
					      NULL, /* no special keymap */
					      NULL,/* no completion */
					  (message_helpfptr)helpProc, /* will give help */
					      (long)&myrock,
					      (int)message_NoInitialString);
    if(result<0) {
	message::DisplayString(tv,0,"Character composition cancelled.");
	return;
    }
    c=(struct composites_s *)pcompch::EnumerateComposites(*buf, (pcompch_ecfptr)composework,(long)buf+1);
    if(!c) {
	message::DisplayString(tv,0,"No such composition found.\n");
	return;
    }
    SelfInsertCmd(tv,c->code,(char *)c->style);
}

/* YARock: Yet Another Rock for keeping track of information as a composition is being typed. */
struct YARock{
    class textview *tv;
    keystate_fptr oldoverride;
    long oldrock;
    struct composites_s *c;
     short key;
     char exts[9];
    int count;
     char possibilities[512];
};

/* keywork: used by handle key to determine what the possible matches for the current composition are. */
static  char keywork(char  key,struct composites_s  *c,struct YARock  *rock)
{
    char buf[7];
    if(strncmp((char *) c->exts,(char *) rock->exts,strlen((char *) rock->exts))) return 0;
    if(!strcmp((char *) c->exts,(char *) rock->exts)) {
	rock->c=c;
	return c->code;
    }
    if(strlen((char *) rock->possibilities)+4<sizeof(rock->possibilities)) {
	sprintf(buf,"%s%c-%c, ",c->style[0]?"*":"",c->code,c->exts[strlen((char *) rock->exts)]);
	strcat((char *) rock->possibilities,buf);
    }
    rock->count++;
    return 0;
}

/* handlekey: add a key to the composition in progress, if the composition is then completely specified return the appropriate code, otherwise list the current possibilities. */
static long handlekey(struct YARock  *rock,long  key)
{
    char buf[16];
    char c;
    buf[1]='\0';
    if(key==127||key==8) {
	if(!*rock->exts) rock->key=BADCHAR;
	else {
	    rock->exts[strlen((char *) rock->exts)-1]='\0';
	}
    } else {
	if(rock->key==BADCHAR) rock->key=key;
	else if(strlen((char *) rock->exts)<sizeof(rock->exts)-1) {
	    buf[0]=key;
	    strcat((char *) rock->exts,buf);
	}
    }
    rock->count=0;
    if(rock->key==BADCHAR) {
	strcpy((char *)rock->possibilities,"Initial character: ");
	return 0;
    } else {
	strcpy((char *) rock->possibilities,"Possible completions: "); 
	c=( char)pcompch::EnumerateComposites(rock->key, (pcompch_ecfptr)keywork,(long)rock);
	if(c) return (unsigned char)c;
	else {
	    int len=strlen((char *) rock->possibilities);
	    if(len>=2) {
		if(rock->possibilities[len-2]==',') rock->possibilities[len-2]='\0';
	    }
	    if(rock->count==0) return -1;
	    else return 0;
	}
    }
}

/* docomposing: intercepts lookups on the textview keystate to see if a valid composition is taking place, if not it removes the override on the textviews keystate and frees the rock.  Otherwise It checks to see if the composition has been completely specified yet.  If it has the appropriate character is inserted in the text. */
static enum keymap_Types docomposing(struct YARock  *rock,long  key,struct proctable_Entry  **ppe,long  *prock)
{
    enum keymap_Types code;
    class keystate  *ks=rock->tv->keystate;
    long handle=handlekey(rock,key);
    message::DisplayString(rock->tv,0,"");
    if(handle>0) {
	(ks)->SetOverride((keystate_fptr)rock->oldoverride,rock->oldrock);
	SelfInsertCmd(rock->tv,handle,(char *)rock->c->style);
	free(rock);
	*ppe=nop;
	*prock=0;
	return keymap_Proc;
    }
    if(handle>=0) {
	*ppe=nop;
	*prock=(long) rock->possibilities;
	return keymap_Proc;
    }
    if(rock->oldoverride)
	code=(enum keymap_Types) (*rock->oldoverride)(rock->oldrock,key,ppe,prock);
    else if(!ks->curMap) return keymap_Empty;
    else code=(ks->curMap)->Lookup(key,(class ATK **)ppe,prock);
    (ks)->SetOverride((keystate_fptr)rock->oldoverride,rock->oldrock);
    free(rock);
    return code;
}

/* compchar_nop: semi bogus proctable function so that the keys taken by docomposing don't have any other side-effects, except perhaps to display the list of possible completions for the composition */
static void compchar_nop(class textview  *tv, char  *possibilities)
{
    if((unsigned long)possibilities>MAXCHAR) {
	message::DisplayString(tv, 0, possibilities);
    }
}

/* compchar_compose2: basically the same as compchar_compose except that '?' help is not available and no return is needed at the end of the composed character. However, continuous help is given in the form of information in the message line.  Automatically notices the end of a composition. */
static void compchar_compose2(class textview  *tv,char  *ch)
{
    struct YARock *rock;
    rock=(struct YARock *)malloc(sizeof(struct YARock));
    rock->tv=tv;
    rock->key=MAXCHAR+1;
    rock->exts[0]='\0';
    rock->c=NULL;
    (tv->keystate)->GetOverride((keystate_fptr *)&rock->oldoverride, &rock->oldrock);
    if(rock->oldoverride==(keystate_fptr)docomposing) {
	free(rock);
	free((struct YARock *)rock->oldrock);
	if((unsigned long)ch>MAXCHAR && ch[0]!='\0') textview_SelfInsertCmd(tv,*ch);
	(tv->keystate)->SetOverride(NULL,0);
	message::DisplayString(tv,0,"");
	return;
    }
    (tv->keystate)->SetOverride((keystate_fptr)docomposing ,(long)rock);
    message::DisplayString(tv,0,"Initial character: ");
}


struct SARock {
    class textview *tv;
    boolean ask;
};

static long doatkreplacement(class text  *text,long  pos,char  *ascii,struct SARock  *r)
{
    int c,dpos;
    (r->tv)->SetDotPosition( pos);
    (r->tv)->SetDotLength( 1);
    (r->tv)->FrameDot( pos);
    do {
	if(r->ask) {
	    (r->tv)->Update();
	    c=((r->tv)->GetIM())->GetCharacter();
	} else c=' ';
	if(c=='?') {
	    message::DisplayString(r->tv,0,"One of 'q'-quit, 'n'-next, '!'-replace all, or ' '-replace and continue.");
	}
    } while (c=='?');
    switch(c) {
	case '\007':
	case '\003':
	case 'q':
	case EOF:
	    pos=EOF;
	    break;
	case 'n':
	    pos++;
	    break;
	case '!':
	    r->ask=FALSE;
	default:
	    (text)->AlwaysReplaceCharacters( pos, 1, ascii, strlen(ascii));
	    if(c=='.') return EOF;
	    pos+=strlen(ascii);
    }
    dpos=pos>=(text)->GetLength()?pos-1:pos;
    (r->tv)->SetDotPosition(dpos);
    (r->tv)->SetDotLength(0);
    (r->tv)->FrameDot(dpos);
    return pos;
}

static void compchar_ATKToASCII(class textview  *tv,long  rock)
{
    long pos,len;
    struct SARock r;
    r.tv=tv;
    if((unsigned long)rock<BADCHAR || *(char *)rock=='\0') r.ask=TRUE;
    else r.ask=FALSE;
    pos=(tv)->GetDotPosition();
    pcompch::ATKToASCII((class text *)(tv)->GetDataObject(), pos, (tv)->GetDotLength(), (pcompch_asciifptr)doatkreplacement, (long)&r);
    len=((class text *)(tv)->GetDataObject())->GetLength();
    if(pos>=len) pos=len-1;
    (tv)->SetDotPosition(pos);
    (tv)->SetDotLength(0);
    (tv)->FrameDot(pos);
    
}


static long doasciireplacement(class text  *text,long  pos, long ch,char  *ascii,long lr)
{
    struct SARock  *r=(struct SARock *)lr;
    int c;
    (r->tv)->SetDotPosition( pos);
    (r->tv)->SetDotLength( strlen(ascii));
    (r->tv)->FrameDot( pos);
    do {
	if(r->ask) {
	    (r->tv)->Update();
	    c=((r->tv)->GetIM())->GetCharacter();
	} else c=' ';
	if(c=='?') {
	    message::DisplayString(r->tv,0,"One of 'q'-quit, 'n'-next, '!'-replace all, or ' '-replace and continue.");
	}
    } while (c=='?');
    switch(c) {
	case '\007':
	case '\003':
	case 'q':
	case EOF:
	    pos=EOF;
	    break;
	case 'n':
	    pos+=strlen(ascii);
	    break;
	case '!':
	    r->ask=FALSE;
	default: {
	    char cch = ch;
	    (text)->AlwaysReplaceCharacters( pos, strlen(ascii), &cch, 1);
	    if(c=='.') return EOF;
	    pos++;
	}
    }
    if(pos>=(text)->GetLength()) pos=(text)->GetLength()-1;
    (r->tv)->SetDotPosition(pos);
    (r->tv)->SetDotLength(0);
    (r->tv)->FrameDot(pos);
    return pos;
}

static void compchar_ASCIIToATK(class textview  *tv,long  rock)
{
    long pos,len;
    struct SARock r;
    r.tv=tv;
    if((unsigned long)rock<BADCHAR || *(char *)rock=='\0') r.ask=TRUE;
    else r.ask=FALSE;
    pos=(tv)->GetDotPosition();
    pcompch::ASCIIToATK((class text *)(tv)->GetDataObject(), pos, (tv)->GetDotLength(), doasciireplacement, (long)&r);
    len=((class text *)(tv)->GetDataObject())->GetLength();
    if(pos>=len) pos=len-1;
    (tv)->SetDotPosition(pos);
    (tv)->SetDotLength(0);
    (tv)->FrameDot(pos);
    
}

boolean compchar::InitializeClass()
{
    struct ATKregistryEntry  *textviewtype = ATK::LoadClass("textview");
    
    if(!ATK::LoadClass("pcompch")) {
	fprintf(stderr,"Unable to load pcompch.\n");
	return FALSE;
    }
    /* Get the styles to use in the help buffer. */
    boldulined=new style;
    if(!boldulined) {
	fprintf(stderr,"Unable to get bold underlined text style.\n");
	return FALSE;
    }
    fixed=new style;
    if(!fixed) {
	delete boldulined;
	fprintf(stderr,"Unable to get fixed-width text style.\n");
	return FALSE;
    }
    
//    tjm - FIXME:  it would be more effective (and prettier) to set better tab stops.
//                  but that will have to wait until "em"s are implemented.
//    (fixed)->AddNewFontFace(fontdesc_Fixed);
    (fixed)->SetFontFamily("andytype");
    
    (boldulined)->AddNewFontFace(fontdesc_Bold);
    (boldulined)->AddUnderline();
    
    /* Code from swedish.c to get the textview function to insert a single character in the text. */
    if((insertchar = proctable::Lookup("textview-self-insert")) != NULL && proctable::Defined(insertchar) ){
	textview_SelfInsertCmd = proctable::GetFunction(insertchar) ;
    } else {
	fprintf(stderr,"Couldn't get textview-self-insert procedure.\n");
	return FALSE;
    }

    nop=proctable::DefineProc("compchar-nop",(proctable_fptr)compchar_nop, textviewtype,NULL,"nop for use with compchar.");


    proctable::DefineProc("compchar-ASCIIToATK",(proctable_fptr)compchar_ASCIIToATK, textviewtype,NULL,"map local ASCII conventions to ATK ISO characters"); 
    proctable::DefineProc("compchar-ATKToASCII",(proctable_fptr)compchar_ATKToASCII, textviewtype,NULL,"map ATK ISO characters to local ASCII");
    proctable::DefineProc("compchar-compose2", (proctable_fptr)compchar_compose2, textviewtype,NULL,"improved compchar-compose.");
    proctable::DefineProc("compchar-compose",(proctable_fptr)compchar_compose, textviewtype,NULL,"start the composition of a character");
    
    proctable::DefineProc("compchar-acuteaccent-after", (proctable_fptr)compchar_leftaccentafter,textviewtype,NULL,"put a left accent over the next character");
    proctable::DefineProc("compchar-graveaccent-after", (proctable_fptr)compchar_rightaccentafter,textviewtype,NULL,"put a right accent over the next character");
    proctable::DefineProc("compchar-circumflex-after", (proctable_fptr)compchar_hatafter,textviewtype,NULL,"put a hat over the next character");
    proctable::DefineProc("compchar-tilde-after", (proctable_fptr)compchar_tildeafter,textviewtype,NULL,"put a tilde over the next character");
    proctable::DefineProc("compchar-umlaut-after", (proctable_fptr)compchar_umlautafter,textviewtype,NULL,"put an umlaut over the next character");
   
    proctable::DefineProc("compchar-acuteaccent",(proctable_fptr)compchar_leftaccent, textviewtype,NULL,"add a left accent to the character to the left of the cursor.");
    proctable::DefineProc("compchar-graveaccent", (proctable_fptr)compchar_rightaccent, textviewtype,NULL,"add a right accent to the character to the left of the cursor.");
    proctable::DefineProc("compchar-circumflex",(proctable_fptr)compchar_hat, textviewtype,NULL,"add a hat to the character to the left of the cursor.");
    proctable::DefineProc("compchar-tilde",(proctable_fptr)compchar_tilde, textviewtype,NULL,"add a tilde to the character to the left of the cursor.");
    proctable::DefineProc("compchar-umlaut",(proctable_fptr)compchar_umlaut, textviewtype,NULL,"add a umlaut to the character to the left of the cursor.");
       
    proctable::DefineProc("compchar-insert",(proctable_fptr)compchar_insert, textviewtype,NULL,"inserts an arbitrary character specified in an initfile.");

    return TRUE;
}
