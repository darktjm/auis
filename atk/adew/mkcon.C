/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* sys/file.h */
#include <util.h>
#include <stdio.h>
#include <sys/param.h>
#include <ctype.h>


char *cls,*viewname,*oldcls,*oldvw;
static const char defstring[] =
"usage: createcon <-C ClassName(:oldclassname)> <-F FunctionName> <-T Title> <-O ShellScriptName> <-M> <-I> <-V Viewname(:oldviewname)> <-W> <-Help> <Filename>\n"
"Explanation of Arguments\n"
"-C ClassName(:oldclassname) \n"
"	Sets the name of the class for createcon to create\n"
"	If the source file contains a control button view,\n"
"	it will default to the class given there.\n"
"	If an old classname is given, it will merge in the information from that file\n"
"-F FunctionName\n"
"	Sets the name of the initialzation function \n"
"	If the source file contains a control button view,\n"
"	it will default to the function given there.\n"
"	If used w/ -I, classname_FunctionName will be used as the initialization method,\n"
"	which will default to classname_InitApp.\n"
"-O ShellScriptName \n"
"	Tells createcon to create a shell script that will use runadew\n"
"	to run the application in stand-alone mode.\n"
"-T Title \n"
"	Sets the title to use when the application is run stand alone with\n"
"	the shell script created with -O.\n"
"-M \n"
"	Forces the creation of a new Imakefile, even if one already exists.\n"
"	The old Imakefile is moved to Imakefile.old\n"
"-I \n"
"	Creates an inset rather than a controller. The callbacks and pointers\n"
"	will be placed in the view. The -C argument will specify the name of\n"
"	the dataobject to create. Control buttons are supported but unnecessary.\n"
"-V Viewname(:oldviewname) \n"
"	If -I is specified, -V can be used to specify the name of the view to create.\n"
"	By default, a 'v' will be appended to the dataobject name.\n"
"	If an old viewname is given, it will merge in the information from that file\n"
"-W\n"
"	If -I is specified, -W will indicate that the datastream describing the object\n"
"	will be written when the file containing the object is saved. \n"
"-Help\n"
"	Display this help message.\n";

#define AWKPROG AndrewDir("/lib/arbiters/conpros.awk")
#define INSETAWKPROG AndrewDir("/lib/arbiters/instpros.awk")
#define START "* user code begins here for "
#define END "* user code ends here for "
#define STARTCHAR '/'
#define TRUE 1
#define FALSE 0
char *oldb,*newb;
struct stf {
    char *begin,*end,*key;
    struct stf *next,*bro;
};
static const char Imakebase[] =
"NormalObjectRule()\n"
"NormalATKRule()\n"
"InstallClassFiles($(HFILES))\n"
"LIBS =\n"
"SYSLIBS =\n"
"DynamicMultiObject(%s,%s,%s.o %s,$(LIBS),$(SYSLIBS),)\n"
"DependTarget()\n";

static const char shellbase[] =
"#!/bin/sh\n"
"if test -r Makefile \n"
"then \n"
"echo Using existing Makefile\n"
"else \n"
"genmake\n"
"echo making dependencies\n"
"make depend\n"
"fi\n"
"make\n";

static const char Insetbase[] =
"HFILES = %s.H %s.H\n";

static const char Controlbase[] =
"HFILES = %s.H\n";

static void ws(struct stf  *s,FILE  *f);
static boolean justcomments(struct stf  *s);
static void writemerge(struct stf  *o,struct stf  *n,FILE  *f);
static struct stf *newstf(struct stf  *old);
static void writestr(FILE  *fi,FILE  *fo,const char  *name);
static long createchfile(char  *src,char  *classn,char  *name);
static struct stf *makestf(char  *buf);
static char *getf(FILE  *f,const char  *s,char  *buf,char  *s1,char  *s2);
static boolean keycmp(char  *s1,char  *s2);
static void setbro(struct stf  *s1,struct stf  *s2);
static FILE *tryopen(char  *name,boolean  *renamed,char  **s1,char  **s2);
static boolean process(FILE  *oldf,char  *name);
static void usage(char  *s);
static char *ps(char  *s);


static void ws(struct stf  *s,FILE  *f)
{
    char *c;
    for(c = s->begin; c < s->end; c++)
	putc(*c,f);
}
static boolean justcomments(struct stf  *s)
{
    char *c;
    int incomment = FALSE;
    for(c = s->begin; c < s->end; c++){
	if(*c == '/' && *(c+1) == '*') incomment = TRUE;
	if(incomment == FALSE){
	    return FALSE;
	}
	if(*c == '\n' && c - 1 > s->begin && *(c - 1) == '/' && *(c - 2) == '*')
	    incomment = FALSE;
    }
    return TRUE;
}
static void writemerge(struct stf  *o,struct stf  *n,FILE  *f)
{
    for(;n != NULL; n = n->next){
	if(n->bro) {
	    ws(n->bro,f);
	}
	else  {
	    ws(n,f);
	}
    }
    for(n = o; n != NULL; n = n->next){
	if(n->key && !n->bro && !justcomments(n)) break;
    }
    if(n != NULL){
	fprintf(f,"\n#ifdef UNUSED_USER_CODE\n");
	for(;o != NULL; o = o->next){
	    if(o->key && !o->bro && !justcomments(o)) ws(o,f);
	}
	fprintf(f,"\n#endif /* UNUSED_USER_CODE */\n");
    }
}
static struct stf *newstf(struct stf  *old)
{
    struct stf *new_c;
    new_c =  (struct stf *) (malloc(sizeof (struct stf)));
    if(old) old->next = new_c;
    new_c->begin = new_c->end = new_c->key = NULL;
    new_c->bro = new_c->next = NULL;
    return new_c;
}
static void writestr(FILE  *fi,FILE  *fo,const char  *name)
{
    int c;
    fprintf(fo,"static char %s[] = {\"\\\n",name);
    while((c = getc(fi))!= EOF){
	if(c == '\\' || c == '"' )putc('\\',fo);
	if(c == '\n') fprintf(fo,"\\n\\");
	putc(c,fo);
    }
    fprintf(fo,"\"};\n");
}
static long createchfile(char  *src,char  *classn,char  *name)
{
    FILE *oldf,*newf;
    sprintf(name,"%s.fh",classn);
    printf("creating %s\n",name);
    if((oldf =fopen(src,"r")) == NULL){
	fprintf(stderr,"Can't Read %s\n",name);
	return -1;
    }
    if((newf = fopen(name,"w")) == NULL){
	fprintf(stderr,"Can't Write %s\n",name);
	return -1;
    }
    writestr(oldf,newf,"defaultstr");
    fclose(oldf);
    fclose(newf);
    return 0;
}
static struct stf *makestf(char  *buf)
{
    struct stf *start, *new_c;
    char *c;
    int sl = strlen(START);
    int el = strlen(END);
    new_c = start = newstf(NULL);
    new_c->begin = buf;
    for(c = buf;*c != '\0';c++){
	if(*c == STARTCHAR ){
	    if(strncmp(c+1,START,sl) == 0){
		if(new_c->begin != c){
		    new_c->end = c;
		    new_c= newstf(new_c);
		    new_c->begin = c;
		}
		new_c->key = c + sl;
	    }
	    else if(strncmp(c+1,END,el) == 0){
		while(*c != '\n') c++;
		new_c->end = c + 1;
		new_c= newstf(new_c);
		new_c->begin = c + 1;
	    }
	}
    }
    new_c->end = c;
    return start;
}				
static char *getf(FILE  *f,const char  *s,char  *buf,char  *s1,char  *s2)
{
    static char ending[1024];
    char *c,*cp;
    int s2len = 0;
    if(s2 != NULL) s2len = strlen(s2);
    for(c = buf;fgets(c,2048,f) != NULL; ){
	if(*c && *c == *s && strncmp(c,s,strlen(s)) == 0){
	    *c = '\0';
	    return buf; 
	}
	/* The following code translates the object names into the
	 file names for the include files */
	if(*c == '#' && (strncmp(c,"#include <",10)) == 0){
	    c += 10;
	    cp = strchr(c,'.');
	    if(cp != NULL){
		*cp = '\0';
		strcpy(ending,cp +1);
		sprintf(c + strlen(c),".%s",ending);
	    }
	}
	if(s2len == 0){
	    while(*c != '\0') c++;
	}
	else{
	    while(*c != '\0'){
		if(*c == *s2 && 
		   (!isalnum((*(c + s2len)))) &&
		   (c == buf || !isalnum((*(c - 1)))) && 
		   strncmp(c,s2,s2len) == 0){
		    /* substitute the new name for the old */
		    strcpy(ending,c + s2len);
		    sprintf(c,"%s%s",s1,ending);
		    c += s2len;
		}
		c++;
	    }
	}
    }
    return(NULL);
}

static boolean keycmp(char  *s1,char  *s2)
{
    while(*s1++ == *s2){
	if(*s2++ == '\n') return TRUE;
    }
    return FALSE;
}
static void setbro(struct stf  *s1,struct stf  *s2)
{
    struct stf *ss2,*ss1;
    for(ss1 = s1 ; ss1 != NULL; ss1 = ss1->next){
	for(ss2 = s2; ss2 != NULL ; ss2 = ss2->next){
	    if(ss1->key != NULL && ss2->key != NULL &&
	       *(ss1->key) == *(ss2->key) && (keycmp(ss1->key , ss2->key) == TRUE)){
		ss1->bro = ss2;
		ss2->bro = ss1;
		break;
	    }
	}
    }
}

static FILE *tryopen(char  *name,boolean  *renamed,char  **s1,char  **s2)
{
    char buf[1024],*p;
    FILE *f;
    unsigned int len;
    *renamed = FALSE;
    p = strrchr(name,'.');
    *s1 = *s2 = NULL;
    if(p != NULL){
	len =  p - name;
	*buf = '\0';
	if(oldcls != NULL && strlen(cls) == len && strncmp(name,cls,len) == 0){
	    sprintf(buf,"%s%s",oldcls,p);
	    *s1= cls;*s2 = oldcls;
	}
	else if(oldvw != NULL && strlen(viewname) == len && strncmp(name,viewname,len) == 0){
	    sprintf(buf,"%s%s",oldvw,p);
	    *s1 = viewname; *s2 = oldvw;
	}
	if(*buf != '\0'){
	    if((f = fopen(buf,"r")) == NULL){
		printf("Warning, can't open %s\n",buf);
		*s1 = *s2 = NULL;
		return NULL;
	    }
	    printf("Merging data from %s into %s\n",buf,name);
	    *renamed = TRUE;
	    return f;
	}
    }
    if((f =fopen(name,"r")) != NULL) return f;
#if 0
    strcpy(buf,name);
    p = strrchr(buf,'.');

    if(p != NULL && strcmp(p,".ch") == 0){
	strcpy(p,".H");
	if((f =fopen(buf,"r")) != NULL){
	    *renamed = TRUE;
	    printf("Merging old .H file and new info to create %s\n",name);
	    return f;
	}
    }
#endif
    return NULL;
}
	    
static boolean process(FILE  *oldf,char  *name)
{
    static char oldb[64000], newb[64000];
    FILE *newf;
    boolean renamed;
    struct stf *olds,*news;
    char *sub1,*sub2;
    sub1 = sub2 = NULL;
    getf(oldf,"! THIS IS THE END OF THIS FILE !!!\n",oldb,NULL,NULL);
    if((newf = tryopen(name,&renamed,&sub1,&sub2)) == NULL ){
	if((newf = fopen(name,"w")) != NULL){
	    printf("Creating %s\n",name);
	    fwrite(oldb,strlen(oldb),1,newf);
	    fclose(newf);
	    return TRUE;
	}
	fprintf(stderr,"Can't Write %s\n",name);
	return FALSE;
    }
    if(access(name,R_OK) != -1){
	sprintf(newb,"%s.old",name);
	rename(name,newb);	
	printf("Moving %s to %s.old\n",name,name);
    }
    getf(newf,"",newb,sub1,sub2);
    fclose(newf);
    if((newf = fopen(name,"w")) == NULL){
	fprintf(stderr,"Can't Write %s\n",name);
	return FALSE;
    }
    olds = makestf(oldb);
    news = makestf(newb);
    setbro(olds,news);
    if(!renamed) printf("Merging in new info to create %s\n",name);
    writemerge(news,olds,newf);
    fclose(newf);
    return TRUE;
}


static void usage(char  *s)
{
    fprintf(stderr,"usage: %s <-C ClassName(:oldclassname)> <-F FunctionName> <-T Title> <-O ShellScriptName> <-M> <-I> <-V Viewname(:oldviewname)> <-W> <-Help> <Filename>\n",s);
    exit(1);
}
static char *ps(char  *s)
{
    char *c;
    if((c = strchr(s,':')) != NULL) 
	*c++ = '\0';
    return c;
}

int main(int argc, char  *argv[])
{
    char *c;
    int cc;
    char bb[256],cmd[5200],tmpfile[256],bv[256];
    char *awkprog,*src,*program,defaultawk[256],parentname[1024];
    boolean iname = FALSE;
    boolean writechild = FALSE;
    int success = TRUE;
    int domake = FALSE;
    const char *func,*title,*sname;
    FILE *f;
    if(getprofileswitch("SecurityConscious", FALSE)) {
	fprintf(stderr, "SecurityConsciousness does not permit createcon to run.\n");
	exit(-1);
    }
    sprintf(defaultawk,"awk -f %s",AWKPROG);
    awkprog = defaultawk;
    title = sname = src = cls = viewname = NULL;
    func = "go";
    *tmpfile = '\0';
    program = *argv;
    while(*++argv != NULL){
	if(**argv=='-'){
	    switch((*argv)[1]){
		case 'c':   /* Classname for controller class */
		case 'C':   /* dataobject name when used w/ -i */
		    cls = (argv[0][2] == '\0')? *++argv : (*argv) + 2;
		    oldcls = ps(cls);
		    break;
		case 'f':   /*  function in controllers class to call */
		case 'F':   /* (default is 'go' ) */
		    func =(argv[0][2] == '\0')? *++argv : (*argv) + 2;
		    break;
		case 't':   /* Title for top of screen */
		case 'T':   /* requires -o */
		    title =(argv[0][2] == '\0')? *++argv : (*argv) + 2;
		    break;
		case 'a':   /* Name of alternate Awk program to run   */
		case 'A':   /* (debugging purposes only) */
		    awkprog =(argv[0][2] == '\0')? *++argv : (*argv) + 2;
		    break;
		case 'o':    /* name of shellscript to create for running application */
		case 'O':  
		    sname = (argv[0][2] == '\0')? *++argv : (*argv) + 2;
		    break;
		case 'm':   /* force creation of new Imakefile */
		case 'M':
		    domake = TRUE;
		    printf("Renaming Imakefile to Imakefile.old\n");
		    rename("Imakefile","Imakefile.old");
		    printf("Renaming Makefile to Makefile.old\n");
		    rename("Makefile","Makefile.old");
		    break;
		case 'i':   /* tells createcon to make an inset rather than a controller */
		case 'I':
		    iname = TRUE;
		    break;
		case 'v':   /* next arg indicates viewname */
		case 'V':   /* requires -i */
		    viewname = (argv[0][2] == '\0')? *++argv : (*argv) + 2;
		    oldvw = ps(viewname);
		    break;
		case 'w':   /* force inset to write childrens data-stream when finished */
		case 'W':   /* requires -i */
#ifdef WRITECHILDWORKS
		    writechild = TRUE;
#else
		    fprintf(stderr,"Warning, writechild option not currently supported \n");
		    fflush(stderr);
#endif /* WRITECHILDWORKS */
		    break;
		case 'h':
		case 'H':
		    fputs(defstring,stdout);
		    fflush(stdout);
		    exit(0);
		default:
		    break;
	    }
	    if(argv == NULL) usage(program);
	}
	else src = *argv;
    }
    if(iname){
	if( cls == NULL){
	    fprintf(stderr,"Need classname for inset, use -c flag\n");
	    exit(2);
	}
	if( src == NULL){
	    fprintf(stderr,"Need source file for inset, can't read stdin\n");
	    exit(3);
	}
	sprintf(defaultawk,"awk -f %s",INSETAWKPROG);
    }
    if(src){
	if(access(src,R_OK) == -1){
	    fprintf(stderr,"ERROR: Can't Open %s\n",src);
	    exit(1);
	}
	printf("Creating a controller for %s. Please wait...\n",src);
	if(iname){
	    if(createchfile(src,cls,parentname) == -1) exit(4);
	}

    }
    else {
	printf("Attempting to read file from standard input\n");
    }

    if(cls == NULL){
	if (src)
	    sprintf(cmd,"%s %s",awkprog,src);
	else 
	    sprintf(cmd,"%s",awkprog);
    }	
    else {
	FILE *ff,*fin;
	int i;
	for(i = 0; i < 100; i++){
	    sprintf(tmpfile,"/tmp/adew%d.tmp",i);
	    if(access(tmpfile,F_OK) == -1 && ((ff = fopen(tmpfile,"w")) != NULL))
		break;
	}
	if(ff == NULL) {
	    fprintf(stderr,"ERROR: Can't open file in /tmp");
	    exit(1);
	}
	if(iname){
	    /* look for and append .dcl file */
	    char fnm[128];
	    sprintf(fnm,"%s.dcl",cls);
	    if(viewname  != NULL && *viewname != '\0'){
		fprintf(ff,"DECLARE %s viewname\n",viewname);
	    }
	    if(cls && *cls){
		fprintf(ff,"DECLARE %s classname\n",cls);
	    }
	    if(func && *func){
		fprintf(ff,"DECLARE %s funcname\n",func);
	    }
	    if(writechild == TRUE)
		fprintf(ff,"DECLARE TRUE writechild\n");
	    if((fin = fopen(fnm,"r")) != NULL){
		int cc;
		while((cc = getc(fin)) != EOF)
		    putc(cc,ff);
		fclose(fin);
		putc('\n',ff);
	    }
	}
	fprintf(ff,"CLASSNAME= %s FUNCTIONAME= %s\n",cls,func);
	if(src == NULL) {
	    while((cc = getc(stdin)) != EOF){
		if(putc(cc,ff) == EOF){
		    fprintf(stderr,"ERROR: Error in writing %s",tmpfile);
		    exit(1);
		}
	    }
	    fclose(ff);
	    sprintf(cmd,"%s %s",awkprog,tmpfile);
	}
	else {
	    fclose(ff);
	    sprintf(cmd,"cat %s %s | %s",tmpfile,src,awkprog);
	}
    }
    *bb = '\0';
    printf("running %s\n",cmd);
    fflush(stdout);
    if((f = popen(cmd,"r")) != NULL){
	while(fgets(bb,256,f) != NULL){
	    if(*bb == '!'){
		fprintf(stderr,"%s",bb);
		pclose(f);
		if(*tmpfile) unlink(tmpfile);
		usage(program);
	    }
	    for(c = bb; *c != '\0'; c++) 
		if(*c == '\n') *c = '\0';
	    if(process(f,bb) == FALSE){
		success = FALSE;
		break;
	    }
	}
    }
    else success = FALSE;
    pclose(f);
    if(*tmpfile) unlink(tmpfile);
    if(success && strlen(bb) > 0){

	for(c = bb; *c != '\0'; c++){
	    if(*c == '.') {
		*c = '\0';
		break;
	    }
	}
	if(cls)strcpy(bb,cls);
	sprintf(cmd,"make%s",bb);
	if((domake || access("Imakefile",R_OK) == -1) &&
	   ((f = fopen("Imakefile","w")) != NULL)){
	    printf("Creating Imakefile\n");
	    if(iname){
		char bv2[256];
		if(viewname){
		    strcpy(bv,viewname);
		}
		else {
		    sprintf(bv,"%sv",bb);
		}
		fprintf(f,Insetbase,bb,bv);
		strcpy(bv2,bv);
		strcat(bv2, ".o");
		fprintf(f,Imakebase,bb,bv, bb, bv2);
	    }
	    else  {
		fprintf(f,Controlbase,bb);
		fprintf(f,Imakebase,bb,"",bb,"");
	    }
	    fclose(f);
	}
	sprintf(cmd,"make%s",bb);
	if(access(cmd,R_OK) == -1 &&
	   ((f = fopen(cmd,"w")) != NULL)){
	    printf("Creating %s shellscript\n",cmd);
	    fprintf(f,shellbase);
	    fclose(f);
	    chmod(cmd,0755);
	}
	if(sname){
	    if( src != NULL && 
	       ((f = fopen(sname,"w")) != NULL)){
		printf("Creating %s shellscript\n",sname);
		fputs("#!/bin/sh\nrunadew",f);
		if(title){
		    fprintf(f," -T\"%s\"",title);
		}
		if(iname){
		     fprintf(f," -I\"%s\" \"$@\"\n",cls);
		}
		else {
		    if(cls){
			fprintf(f," -C\"%s\" -F\"%s\"",cls,func);
		    }
		    fprintf(f," -S\"%s\" \"$@\"\n",src);
		}
		fclose(f);
		chmod(sname,0755);
	    }
	    else {
		if(src == NULL) 
		    fprintf(stderr,"ERROR: Need source file name to create %s shellscript\n",sname);
		else fprintf(stderr,"ERROR: Can't open %s for output",sname);
	    }
	}
	printf("Done\n");
    } 
}

