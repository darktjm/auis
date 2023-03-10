/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("deskey.H")
#include <sys/param.h>
#include <im.H>
#include <keystate.H>
#include <proctable.H>
#include <message.H>
#include <environ.H>
#include <fontdesc.H>
#include <frame.H>
#include <buffer.H>
#include <view.H>
#include <text.H>
#include <deskey.H>

static long NProcs;


#define KEYSORT 1
#define PROCSORT 2
#define KEYSORTSTRING "sort -t\001 -b -k1,2"
#define PROCSORTSTRING "sort -t\001 -k 2 -k 1"
#define TMPA "/tmp/,tmp1"
#define TMPB "/tmp/,tmp2"
#define TMPC "/tmp/,tmp3"


ATKdefineRegistry(deskey, ATK, deskey::InitializeClass);
static boolean bufferFill(class im  *im, class text  *text);
static boolean bufferFind(class frame  *f,class buffer  *b);
static boolean bufferSetup(FILE  **f, class im  *im, class text  **text);
static class view *bufferFindView(class buffer  *b);
static char *charToPrintable(long  c);
static void datastreamOpen(FILE  *f);
static void datastreamClose(FILE  *f);
static int transferLine(FILE  *f1, FILE  *f2, boolean  really);
static int removeOverridden(int  sort);
static void describeAllKeys(class im  *im, int  sort);
static long describeAllProcEntries(class im  *im);
static void describeBinding(char  *bind,int  len,int  runKey /* if == -1, not part of a run */,struct proctable_Entry  *pte,FILE  *f, long  mapno);
static void describeKeymap(class keymap  *map,char  *bind,int  len,FILE  *f, long  mapno);
static void describeKeys(class im  *im, FILE  *f);
static void describeProcEntry(struct proctable_Entry  *pe, FILE  *f);
static const char *getProcDoc(struct proctable_Entry  *pte);
static void makePrefix(char  *buf ,char  *keys,int  len);
static int parseFile(FILE  *f, FILE  *d);
static void parseBindFile(FILE  *f, FILE  *d);
static void parseProcFile(FILE  *f, FILE  *d);
static void sortByKey(class im  *im);
static void sortByProc(class im  *im);
static void describeAKey(class im  *im);
static void ListAProc(struct proctable_Entry  *pe, FILE  *f);
static long ListProctable(class im  *im);


boolean deskey::InitializeClass()
{
    struct ATKregistryEntry  *imc;
    imc = ATK::LoadClass("im");
    
    proctable::DefineProc("deskey-describe-key", (proctable_fptr) describeAKey, imc, NULL, "Show binding of a typed in key sequence.");
    proctable::DefineProc("deskey-describe-bound-procs", (proctable_fptr) sortByProc, imc, NULL, "Show bindings for all keys.");
    proctable::DefineProc("deskey-describe-bound-keys", (proctable_fptr) sortByKey, imc, NULL, "Show bindings for all keys.");
    proctable::DefineTypedProc("deskey-describe-proctable", (proctable_fptr) describeAllProcEntries, imc, NULL, "List all proctable entries.; Return count.", proctable_Long);
    proctable::DefineTypedProc("deskey-list-proctable", (proctable_fptr) ListProctable, imc, NULL, "List the entries in the proctable to /tmp/ProcList;   Return count.", proctable_Long);
    return TRUE;
}

/* empty and fill [scratch] buffer with text from a file.  Uses AlwaysDelete and AlwaysInsert to overcome the possibility that the scratch buffer might be read-only */
static boolean bufferFill(class im  *im, class text  *text)
{
    long len;
    
    len = (text)->GetLength();
    (text)->AlwaysDeleteCharacters( 0, len);
    if ((text)->AlwaysInsertFile( NULL, TMPC, 0) == 0){
	message::DisplayString(im, 0, "Could not insert file.");
	return(FALSE);
    }
    (text)->NotifyObservers( 0);
    return(TRUE);
}


static boolean bufferFind(class frame  *f,class buffer  *b)
{
  return((f)->GetBuffer() == b);
}

/* initiate file and buffer setup - the file will eventually be used to fill the buffer,
  the buffer is a scratch buffer (possibly read-only too) */
static boolean bufferSetup(FILE  **f, class im  *im, class text  **text)
{
    class im *newim;
    class buffer *buffer;
    class view *view;

    if ((*f = fopen(TMPC, "w")) == NULL){
	message::DisplayString(im, 0, "Could not create temp file.");
	return(FALSE);
    }

    message::DisplayString(im, 0, "Creating Bind List Window........");
    im::ForceUpdate();	/* cause the message to appear */
    if ((buffer = buffer::FindBufferByName("BindList")) == NULL) {
	if ((buffer = buffer::Create("BindList", NULL, "text", NULL)) == NULL) {
	    message::DisplayString(im, 50, "Could not create new buffer.");
	    return(FALSE);
	}
    }
    (buffer)->SetScratch( TRUE);
    if ((view = bufferFindView(buffer)) == NULL){
	message::DisplayString(im, 50, "Could not find view.");
	return(FALSE);
    }
    if ((newim = (view)->GetIM()) == NULL) {
	message::DisplayString(im, 50, "Could not find window.");
	return(FALSE);
    }
    if ((*text = (class text *)(buffer)->GetData()) == NULL){
	message::DisplayString(im, 50, "Could not find buffer data.");
	return(FALSE);
    }
    (newim)->ExposeWindow();
    (view)->WantInputFocus(view);
    /* im_SetWMFocus(newim);*/ /* questionable */
    return(TRUE);
}


static class view *bufferFindView(class buffer  *b)
{
  class frame *f;
  class im *im;

  if ((f = frame::Enumerate((frame_effptr)bufferFind, (long) b))==NULL) {
    /* No frame--need to map buffer to new window */

    if((f = new frame) == NULL) {
	fprintf(stderr,"Could not allocate enough memory.\n");
	return((class view*) NULL);
    }
    (f)->SetCommandEnable( TRUE);
    (f)->PostDefaultHandler( "message", (f)->WantHandler( "message"));
    (f)->SetBuffer( b, TRUE);

    if((im = im::Create(NULL)) == NULL) {
	fprintf(stderr,"Could not create new window.\n");
	if(f) (f)->Destroy();
	return((class view*) NULL);
    }
    (im)->SetView( f);
  }
  return((f)->GetView());
}


/* create a printable character from any [reasonable] [dec] value */
/* same routine as is used within 'im' for uniformity of appearance */
static char *charToPrintable(long  c)
{
    static char s[8];

    if (c==9)
	strcpy(s,"Tab");
    else if(c == 27)
	strcpy(s, "Esc");
    else if (c == 28)
	strcpy(s, "^\\\\");
    else if (c < 32)
	s[0] = '^', s[1] = c + '@', s[2] = '\0';
    else if(c==32)
	strcpy(s,"Spc");
    else if (c < 127)
	s[0] = c, s[1] = '\0';
    else if (c == 127)
	strcpy(s, "Del");
    else {
	s[0] = '\\';
	s[1] = '0' + (c > 127);
	s[2] = '0' + ((c & 56) >> 3);
	s[3] = '0' + (c & 3);
	s[4] = '\0';
    }

    return s;
}

/* set up the ATK datastream for the buffer - perhaps a bit of a hack, but.... */
static void datastreamOpen(FILE  *f)
{
    fprintf(f, "\\begindata{text,999666333}\n");
    fprintf(f, "\\textdsversion{12}\n");
    fprintf(f, "\\define{bindlist\n");
    fprintf(f, "attr:[Justification LeftJustified Point 0]\n");
    fprintf(f, "attr:[FontFace FixedFace Int Set]\n");
    fprintf(f, "attr:[FontFamily AndyType Int 0]\n");
    fprintf(f, "attr:[FontSize ConstantFontSize Point 10]}\n");
    fprintf(f, "\\define{binddesc\n");
    fprintf(f, "attr:[LeftMargin LeftMargin Cm 647350]\n");
    fprintf(f, "attr:[Indent LeftMargin Cm -32367]\n");
    fprintf(f, "attr:[Justification LeftJustified Point 0]\n");
    fprintf(f, "attr:[FontFace Italic Int Set]\n");
    fprintf(f, "attr:[FontFamily Andy Int 0]\n");
    fprintf(f, "attr:[FontSize ConstantFontSize Point 10]}\n");
    fprintf(f, "\\define{bold\n");
    fprintf(f, "attr:[FontFace Bold Int Set]}\n");
    fprintf(f, "\\define{majorheading\n");
    fprintf(f, "attr:[Justification Centered Point 0]\n");
    fprintf(f, "attr:[Flags KeepPriorNL Int Set]\n");
    fprintf(f, "attr:[Flags KeepNextNL Int Set]\n");
    fprintf(f, "attr:[FontFamily Andy Int 0]\n");
    fprintf(f, "attr:[FontSize ConstantFontSize Point 20]}\n");
}

/* make sure to close the ATK datastream */
static void datastreamClose(FILE  *f)
{
    fprintf(f, "\\enddata{text, 999666333}\n");
}

#define PARSE_ERROR (-2)

static int transferLine(FILE  *f1, FILE  *f2, boolean  really)
{
    int code;
    while ((code = fgetc(f1)) != '\n' && code != EOF)
	if (really)
	    fputc(code, f2);
    if (really)
	fputc('\n', f2);
    return((code == EOF) ? PARSE_ERROR : code);
}

/* read TMPA - write TMPB - mv TMPB to TMPA */
static int removeOverridden(int  sort)
{
    FILE *f1, *f2;
    int code;
    char key[50];
    static char lastKey[50];
    static boolean init = TRUE;
    if ((f1 = fopen(TMPA, "r")) == NULL) return(1); /* could be explicit */
    if ((f2 = fopen(TMPB, "w")) == NULL){
	fclose(f1);
	return(2); /* could be explicit */
    }
    if (init){
	init = FALSE;
	lastKey[0] = '\0';
    }
    code = 0;
    key[0] = '\0';
    while (code != EOF && code != PARSE_ERROR){
	int i = 0;
	while ((code = fgetc(f1)) != '\001' && code != EOF){
	    key[i++] = code;
	}
	key[i] = '\0';
	if (code != EOF)
	    while ((code = fgetc(f1)) != '\001' && code != EOF);
	if (code != EOF){
	    if (lastKey[0] == '\0'){/* first time through - use it */
		strcpy(lastKey, key);
		fprintf(f2, "%7s\001", key);
		code = transferLine(f1, f2, TRUE);
	    }
	    else{
		if (strcmp(lastKey, key) != 0){ /* new keybinding - grab it */
		    strcpy(lastKey, key);
		    fprintf(f2, "%7s\001", key);
		    code = transferLine(f1, f2, TRUE);
		}
		else {/* overridden keybinding  - skip line */
		    code = transferLine(f1, f2, FALSE);
		}
	    }
	}
	if (code != EOF && code < 0) code = PARSE_ERROR;
    }

    fclose(f1);
    fclose(f2);
    if (code == PARSE_ERROR) return(3); /* could be explicit */
    if (sort == PROCSORT){
	char buf[125];
	sprintf(buf, "%s %s > %s", PROCSORTSTRING, TMPB, TMPA);
	if (system(buf) != 0){
	    return(4); /* could be explicit */
	}
    }
    else 
	if (rename(TMPB, TMPA) != 0){
	    return(5); /* could be explicit */
	}
    unlink(TMPB); 
    return(0);
}



/* 'command control' for describing all keybindings - this basically
    initiates the temporary file, calling the functions needed to fill it
    calling the functions to setup the second file and buffer, calling
    the function to fill that, and getting rid of all the temporary files */
static void describeAllKeys(class im  *im, int  sort)
{
    FILE *f, *tmpf;
    char buf[MAXPATHLEN];
    class text *text = NULL;
    int code;
    
    sprintf(buf, "%s > %s", KEYSORTSTRING, TMPA);
    if ((tmpf = popen(buf, "w")) == NULL){
	sprintf(buf, "Could not open %s", TMPA);
	message::DisplayString(im, 0, buf);
	return;
    }
    message::DisplayString(im, 0, "Parsing Binding List........");
    im::ForceUpdate();
    describeKeys(im, tmpf);
    pclose(tmpf);
    message::DisplayString(im, 0, "Sorting Binding List........");
    if ((code = removeOverridden(sort)) != 0) {
	sprintf(buf, "Something went wrong (%d)...sorry.", code);
	message::DisplayString(im, 0, buf);
	return;
    }
    if (!bufferSetup(&f, im, &text)) return;
    if ((tmpf = fopen(TMPA, "r")) == NULL){
	sprintf(buf, "Could not open %s", TMPA);
	message::DisplayString(im, 0, buf);
	return;
    }
    
    message::DisplayString(im, 0, "Formatting Binding List........");
    im::ForceUpdate();
    datastreamOpen(f);
    fprintf(f, "\\majorheading{Key Binding List\n\n");
    fprintf(f, "-------------------------}\n\n");
    parseBindFile(tmpf, f);
    datastreamClose(f);
    fclose(f);
    fclose(tmpf);
    if (!bufferFill(im, text)) return;
    unlink(TMPC);
    unlink(TMPA);  
    message::DisplayString(im, 0, "Done.");
}


/* 'command control' for describing all procentries - this basically
    initiates the temporary file, calling the functions needed to fill it
    calling the functions to setup the second file and buffer, calling
    the function to fill that, and getting rid of all the temporary files */
static long describeAllProcEntries(class im  *im)
{
    FILE *f, *tmpf;
    char buf[MAXPATHLEN];
    class text *text;

    NProcs = 0;
    sprintf(buf, "sort > %s", TMPA);
    if((tmpf = popen(buf, "w")) == NULL){
	sprintf(buf, "Could not open %s", TMPA);
	message::DisplayString(im, 0, buf);
	return(NProcs);
    }

    message::DisplayString(im, 0, "Parsing Proc List......");
    im::ForceUpdate();
    proctable::Enumerate((proctable_efptr) describeProcEntry, (char *) tmpf);
    pclose(tmpf);

    if (!bufferSetup(&f, im, &text)) return(NProcs);
    if((tmpf = fopen(TMPA, "r")) == NULL){
	sprintf(buf, "Could not open %s", TMPA);
	message::DisplayString(im, 0, buf);
	return(NProcs);
    }

    message::DisplayString(im, 0, "Formatting Proc List......");
    im::ForceUpdate();

    datastreamOpen(f);
    fprintf(f, "\\majorheading{Proctable Entry List\n\n");
    fprintf(f, "-------------------------}\n\n");
    parseProcFile(tmpf, f);
    datastreamClose(f);
    fclose(f);
    fclose(tmpf);
    if (!bufferFill(im, text)) return 0;
    unlink(TMPA);
    unlink(TMPC);
    message::DisplayString(im, 0, "Done.");
    return NProcs;
}

/* glorified printf - handles translating key-binding into a printable string */
/* mapno - is an incremented number representing which keymap the entry is from
  this is used on the other end of the processing to eliminate those bindings which
  were overridden */
static void describeBinding(char  *bind,int  len,int  runKey /* if == -1, not part of a run */,struct proctable_Entry  *pte,FILE  *f, long  mapno)
{
    char keys[50];

    keys[0] = '\0';
    if(runKey >= 0 && runKey != bind[len-1]){
	makePrefix(keys,bind,len - 1);
	strcat(keys,charToPrintable(runKey));
	strcat(keys," -> ");
    }

    makePrefix(keys + strlen(keys),bind,len - 1);
    strcat(keys,charToPrintable(bind[len - 1]));

    fprintf(f, "%7s\001%ld\001%-28s\001%s\001\n", keys, mapno, proctable::GetName(pte), getProcDoc(pte));
}

/* recursive function to go through entire active keymap */
static void describeKeymap(class keymap  *map,char  *bind,int  len,FILE  *f, long  mapno)
{
    int key, runKey = -1;
    enum keymap_Types lastKeyType=keymap_Empty;
    ATK  *lastObj = NULL;

    len++;

    for (key = 0; key < 128; key++) {
	ATK  *obj;
	long keyrock;
	enum keymap_Types keyType;

	keyType = (map)->Lookup( key, &obj, &keyrock);

	if (lastObj != NULL && (lastKeyType != keyType || lastObj != obj)) {
	    /* describe the previous run of similar bindings */
	    describeBinding(bind, len, runKey, (struct proctable_Entry *) lastObj, f, mapno);
	    lastObj = NULL;
	    runKey = -1;
	}
	bind[len - 1] = key;

	switch (keyType) {
	    case keymap_Empty:	/* do nothing */
		break;
	    case keymap_Keymap:	/* recurse on new keymap */
		describeKeymap((class keymap *) obj, bind, len, f, mapno);
		break;
	    case keymap_Proc:
		/*
		 * save parameters, since this might be part of a run
		 * of equivalent bindings
		 */
		if (lastObj == NULL) {
		    lastObj = obj;
		    lastKeyType = keyType;
		    runKey = key;
		}
		break;
	}
    }

    if (lastObj != NULL)
	/* describe the previous run of similar bindings */
	describeBinding(bind, len, runKey, (struct proctable_Entry *) lastObj, f, mapno);
}


static void describeKeys(class im  *im, FILE  *f)
{
    class keystate *ks;
    long mapno = 1; 
    for(ks = im->keystate; ks != NULL; ks = ks->next){
	char buf[20];
	describeKeymap(ks->curMap, buf, 0, f, mapno);
	mapno++;
    }
}


static void describeProcEntry(struct proctable_Entry  *pe, FILE  *f)
{
    NProcs++;

    fprintf(f, "%s\001", (pe->name != NULL) ? pe->name : "[unnamed]");
    if (pe->type != NULL)
	fprintf(f, " (%s)", (pe->type)->GetTypeName());
    else
	fprintf(f, " ");
    
    switch (pe->returntype) {
	case proctable_Void:
	    break;
	case proctable_Boolean:
	    fprintf(f, "=> Boolean");  break;
	case proctable_Char:
	    fprintf(f, "=> Char");  break;
	case proctable_Short:
	    fprintf(f, "=> Short");  break;
	case proctable_Long:
	    fprintf(f, "=> Long");  break;
	case proctable_Double:
	    fprintf(f, "=> Double");  break;
	case proctable_Object:
	    fprintf(f, "=> Object");  break;
	case proctable_NessMarker:
	    fprintf(f, "=> NessMarker");  break;
	case proctable_StaticString:
	    fprintf(f, "=> StaticString");  break;
	case proctable_DisposeString:
	    fprintf(f, "=> DisposableString");  break;
    }

    fprintf(f, "\001%s\001\n", getProcDoc(pe));
}


/* Sigh.  Should call proctable_GetDocumentation(pte) 
  but include file is broken. */
static const char *getProcDoc(struct proctable_Entry  *pte)
{
    if (!proctable::Defined(pte))
	return("<not loaded>");
    if (pte->doc == NULL)
	return("<no documentation.>");
    return(pte->doc);
}



static void makePrefix(char  *buf ,char  *keys,int  len)
{
    while(len-- > 0){
	sprintf(buf, "%s-", charToPrintable(*keys++));
	buf += strlen(buf);
    }
}


static int parseFile(FILE  *f, FILE  *d)
{
    int c;
    int count = 0;
    while ((c = fgetc(f)) != '\001' && c != EOF){
	if (c == '\n') continue;
	if (c != ' ') count++;
	fputc(c, d);
    }
    return((c == EOF) ? EOF : count);
}


static void parseBindFile(FILE  *f, FILE  *d)
{
    int c;
    for (;;){
	fprintf(d, "\\bindlist{");
	c = parseFile(f, d);
	fprintf(d, "  \\bold{");
	if (c != EOF){
	    c = parseFile(f, d);
	    fprintf(d, "}} ");
	    if (c > 28)
		fprintf(d, "\n\n");
	    if (c != EOF){
		fprintf(d, "\\binddesc{- ");
		c = parseFile(f, d);
		fprintf(d, "}\n\n");
	    }
	    else
		return;
	}
	else{
	    fprintf(d, "}}\n\n");
	    return;
	}
    }
}


static void parseProcFile(FILE  *f, FILE  *d)
{
    int c1, c2, cd;
    for(;;){
	fprintf(d, "\\bindlist{\\bold{");
	c1 = parseFile(f, d);
	fprintf(d, "}");
	if (c1 != EOF){
	    c2 = parseFile(f, d);
	    cd = 36 - (c1 + c2);
	    if (cd > 0){
		int i;
		for (i = 0; i < cd; i++)
		    fprintf(d, " ");
	    }
	    fprintf(d, "}");
	    if ((c1 + c2) > 37)
		fprintf(d, "\n\n");
	    if (c2 != EOF){
		fprintf(d, "\\binddesc{- ");
		parseFile(f, d);
		fprintf(d, "}\n\n");
	    }
	    else
		return;
	}
	else{
	    fprintf(d, "}\n\n");
	    return;
	}
    }
}


static void sortByKey(class im  *im)
{
    describeAllKeys(im, KEYSORT);
}



static void sortByProc(class im  *im)
{
    describeAllKeys(im, PROCSORT);
}




/* Prompt for a key sequence and show the name and documentation of the function it's bound to. */
static void describeAKey(class im  *im)
{
    int done = FALSE;
    char buf[1000];
    int c;
    struct proctable_Entry *pte;
    long rock;
    ATK  *object;

    strcpy(buf, "Describe key: ");
    message::DisplayString(im, 0, buf);
    while (!done) {
	c = (im)->GetCharacter();
	if (c == EOF  ||  c == '\007') {
	    (im->keystate)->Reset();
	    message::DisplayString(im, 0, "Describe key aborted.");
	    return;
	}
	strcat(buf, charToPrintable(c));
	c &= 0177;
/*	AppendKey(buf, c); */
	switch ((im->keystate)->ApplyKey( c, &pte, &rock, &object)) {
	    case keystate_ProcFound:
		done = TRUE;
		strcat(buf, " -> ");
		strcat(buf, proctable::GetName(pte));
		strcat(buf, ": ");
		strcat(buf, getProcDoc(pte));
		break;
	    case keystate_NoBinding:
		done = TRUE;
		strcat(buf, " -> <Key not bound.>");
		break;
	    case keystate_Pending:
		strcat(buf, "-");
		break;
	}
	message::DisplayString(im, 0, buf);
    }
}


/* OLD CODE - kept around for some reason I do not completely understand (there is code above that does the same thing in a better user interface) --ghoti 12/22/89 */

/* ListAProc(pe, f)
	Format the entry 'pe' and print it to 'f'
*/
static void ListAProc(struct proctable_Entry  *pe, FILE  *f)
{
    NProcs++;

    if (pe->name != NULL)
	fprintf(f, "%s", pe->name);
    else fprintf(f, "[unnamed]");

    if(pe->type != NULL)
	fprintf(f, " ( %s )", (pe->type)->GetTypeName());
    else fprintf(f, " ( ??? )");

    switch (pe->returntype) {
	case proctable_Void:  break;
	case proctable_Boolean:   fprintf(f, "=> Boolean");  break;
    case proctable_Char:  fprintf(f, "=> Char");  break;
case proctable_Short:  fprintf(f, "=> Short");  break;
case proctable_Long:  fprintf(f, "=> Long");  break;
case proctable_Double:  fprintf(f, "=> Double");  break;
case proctable_Object:  fprintf(f, "=> Object");  break;
case proctable_NessMarker:  fprintf(f, "=> NessMarker");  break;
case proctable_StaticString:  fprintf(f, "=> StaticString");  break;
case proctable_DisposeString:  fprintf(f, "=> DisposableString");  break;
    }

    if (pe->doc != NULL)
	fprintf(f, "   %s\n", pe->doc);
    else fprintf(f, "\n");
}

/* ListProctable(im)
	write to /tmp/ProcList a list of all current proctable entries
	Return number of entries written.  
*/
static long ListProctable(class im  *im)
{
    FILE *f;
    NProcs = 0;
    f = popen("sort > /tmp/ProcList", "w");
    if (f == NULL) {
	message::DisplayString(im, 0, "Could not open /tmp/ProcList");
	return NProcs;
    }
    message::DisplayString(im, 0, "Writing /tmp/ProcList");
    im::ForceUpdate();	/* cause the message to appear */
    proctable::Enumerate((proctable_efptr) ListAProc, (char *) f);
    pclose(f);
    message::DisplayString(im, 0, "Wrote /tmp/ProcList");
    return NProcs;
}
