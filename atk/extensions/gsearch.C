/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
 \* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("gsearch.H")
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>



#include <bind.H>
#include <text.H>
#include <textview.H>
#include <mark.H>
#include <search.H>
#include <message.H>
#include <im.H>

#include <gsearch.H>

#define MAX(a,b) (((a)>(b))?(a):(b))

#define DYNSTR_GROWSIZE (64)
#define STATESTACK_GROWSIZE (32)

#define CTRL_G (7)
#define CTRL_H (8)
#define CTRL_L (12)
#define CTRL_Q (17)
#define CTRL_R (18)
#define CTRL_S (19)
#define CTRL_W (23)
#define CTRL_Y (25)
#define ESC (27)
#define DEL (127)

struct dynstr {
    int used, allocated;
    char *text;
};

struct statestacknode {
    int patternlen, wrappedp, failurep;
    long position, length, searchfrom;
    int forwardp;
};

struct statestack {
    int used, allocated;
    struct statestacknode *nodes;
};

static jmp_buf jmpenv;
static struct dynstr LastPattern;
static struct statestacknode *StackTop;


ATKdefineRegistry(gsearch, ATK, gsearch::InitializeClass);
static int statestack_init(struct statestack  *s);
static void statestack_destroy(struct statestack  *s);
static void statestack_pop(struct statestack  *s);
static void statestack_push(struct statestack  *s, int  pl , int  wp , int  fp, long  pos , long  len , long  sf, int  fwdp);
static int dynstr_init(struct dynstr  *d);
static void dynstr_shortento(struct dynstr  *d, int  size);
static void dynstr_ensuresize(struct dynstr  *d, int  size);
static void dynstr_put(struct dynstr  *d, const char  *str);
static void dynstr_append(struct dynstr  *d, const char  *str);
static void dynstr_destroy(struct dynstr  *d);
static int dynstr_empty(struct dynstr  *d);
static void dynstr_addchar(struct dynstr  *d, int  c);
static void dynstr_copy(struct dynstr  *dest , struct dynstr  *src);
static void dosearch(class textview  *tv, int  forwardp);
static void     fsearch(class textview  *tv, long             key);
static void     rsearch(class textview  *tv, long             key);


static int statestack_init(struct statestack  *s)
{
    s->used = 0;
    s->allocated = STATESTACK_GROWSIZE;
    if (!(s->nodes =
	  (struct statestacknode *) malloc(STATESTACK_GROWSIZE *
					    (sizeof (struct statestacknode)))))
	return (1);
    return (0);
}

static void statestack_destroy(struct statestack  *s)
{
    free(s->nodes);
}

static void statestack_pop(struct statestack  *s)
{
    --(s->used);
    StackTop = s->nodes + s->used - 1;
}

static void statestack_push(struct statestack  *s, int  pl , int  wp , int  fp, long  pos , long  len , long  sf, int  fwdp)
{
    if (s->used == s->allocated) {
	int newsize = s->allocated + STATESTACK_GROWSIZE;

	if (!(s->nodes =
	     (struct statestacknode *)
	      realloc(s->nodes,
		      newsize * (sizeof (struct statestacknode)))))
	    longjmp(jmpenv, 1);
	s->allocated = newsize;
    }
    s->nodes[s->used].patternlen = pl;
    s->nodes[s->used].wrappedp = wp;
    s->nodes[s->used].failurep = fp;
    s->nodes[s->used].position = pos;
    s->nodes[s->used].length = len;
    s->nodes[s->used].searchfrom = sf;
    s->nodes[(s->used)++].forwardp = fwdp;
    StackTop = s->nodes + s->used - 1;
}

static int dynstr_init(struct dynstr  *d)
{
    d->used = 1;
    d->allocated = DYNSTR_GROWSIZE;
    if (!(d->text = (char *)malloc(DYNSTR_GROWSIZE)))
	return (1);
    d->text[0] = '\0';
    return (0);
}

static void dynstr_shortento(struct dynstr  *d, int  size)
{
    d->used = size;
    d->text[d->used - 1] = '\0';
}

static void dynstr_ensuresize(struct dynstr  *d, int  size)
{
    int newsize;

    if (d->allocated >= size)
	return;
    if (!(d->text = (char *)realloc(d->text,
			    newsize = MAX(DYNSTR_GROWSIZE,
					  size + (DYNSTR_GROWSIZE >> 1)))))
	longjmp(jmpenv, 1);
    d->allocated = newsize;
}

static void dynstr_put(struct dynstr  *d, const char  *str)
{
    int need = 1 + strlen(str);

    dynstr_ensuresize(d, need);
    strcpy(d->text, str);
    d->used = need;
}

static void dynstr_append(struct dynstr  *d, const char  *str)
{
    int need = d->used + strlen(str);

    dynstr_ensuresize(d, need);
    strcat(d->text, str);
    d->used = need;
}

static void dynstr_destroy(struct dynstr  *d)
{
    free(d->text);
}

static int dynstr_empty(struct dynstr  *d)
{
    return ((d->used == 0) || (d->text[0] == '\0'));
}

static void dynstr_addchar(struct dynstr  *d, int  c)
{
    int need = d->used + 1;

    dynstr_ensuresize(d, need);
    d->text[d->used - 1] = c;
    d->text[(d->used)++] = '\0';
}

static void dynstr_copy(struct dynstr  *dest , struct dynstr  *src)
{
    dynstr_ensuresize(dest, src->used);
    strcpy(dest->text, src->text);
    dest->used = src->used;
}

static void dosearch(class textview  *tv, volatile int  forwardp)
{
    FILE *tmp_file;
    class text *txt = (class text *) (tv)->GetDataObject();
    long origpos = (tv)->GetDotPosition();
    long origlen = (tv)->GetDotLength();
    long foundloc, newsearchfrom=0;
    struct dynstr pattern, prompt;
    int wasmeta = 0;
    int c, dodokey = 0, oldforwardp;
    char *tmpbuf;
    class search compiled;
    const char *compileerr;
    struct statestack stack;

    if (dynstr_init(&pattern)) {
	message::DisplayString(tv, 0, "I-Search is out of memory; aborted.");
	im::ForceUpdate();
	return;
    }
    if (dynstr_init(&prompt)) {
	dynstr_destroy(&pattern);
	message::DisplayString(tv, 0, "I-Search is out of memory; aborted.");
	im::ForceUpdate();
	return;
    }
    if (statestack_init(&stack)) {
	dynstr_destroy(&pattern);
	dynstr_destroy(&prompt);
	message::DisplayString(tv, 0, "I-Search is out of memory; aborted.");
	im::ForceUpdate();
	return;
    }
    
    if (setjmp(jmpenv)) {
	dynstr_destroy(&pattern);
	dynstr_destroy(&prompt);
	statestack_destroy(&stack);
	(tv)->SetDotPosition( origpos);
	(tv)->SetDotLength( origlen);
	(tv)->FrameDot( origpos);
	message::DisplayString(tv, 0, "I-Search is out of memory; aborted.");
	im::ForceUpdate();
	return;
    }

    statestack_push(&stack, 1, 0, 0, origpos, origlen,
		    (tv)->GetDotPosition(), forwardp);

  initialstate:
    
    dynstr_put(&prompt, "I-Search");
    if (!forwardp)
	dynstr_append(&prompt, " backward");
    if (!dynstr_empty(&LastPattern)) {
	dynstr_append(&prompt, " [");
	dynstr_append(&prompt, LastPattern.text);
	dynstr_append(&prompt, "]");
    }
    dynstr_append(&prompt, ": ");
    message::DisplayString(tv, 0, prompt.text);
    im::ForceUpdate();
    while (1) {
	c = ((tv)->GetIM())->GetCharacter();
	switch (c) {
	  case CTRL_L:
	    ((tv)->GetIM())->RedrawWindow();
	    break;
	  case CTRL_G:
	    dynstr_destroy(&prompt);
	    dynstr_destroy(&pattern);
	    statestack_destroy(&stack);
	    message::DisplayString(tv, 0, "Cancelled.");
	    im::ForceUpdate();
	    return;
	  case CTRL_H:
	  case DEL:
	    break;
	  case CTRL_Q:
	    while ((c = ((tv)->GetIM())->GetCharacter()) == EOF)
		;
	    dynstr_addchar(&pattern, c);
	    goto compilestate;
	  case CTRL_R:
	  case CTRL_S:
	    forwardp = (c == CTRL_S);
	    if (!dynstr_empty(&LastPattern)) {
		dynstr_copy(&pattern, &LastPattern);
		goto compilestate;
	    }
	    goto initialstate;
	  case CTRL_W:
	    if ((tv)->GetDotLength() > 0)
		goto appendselectionstate;
	    break;
	  case CTRL_Y:
	    goto appendkillheadstate;
	  case ESC:
	    wasmeta = ((tv)->GetIM())->WasMeta();
	    /* Fall through */
	  case EOF:
	    goto exitstate;
	  default:
	    if (isascii(c) && (isprint(c) || isspace(c))) {
		dynstr_addchar(&pattern, c);
		goto compilestate;
	    }
	    else {
		dodokey = 1;
		goto exitstate;
	    }
	}
    }

  emptypatternstate:
    
    dynstr_put(&prompt, "I-Search");
    if (!forwardp)
	dynstr_append(&prompt, " backward");
    dynstr_append(&prompt, ": ");
    message::DisplayString(tv, 0, prompt.text);
    im::ForceUpdate();
    while (1) {
	c = ((tv)->GetIM())->GetCharacter();
	switch (c) {
	  case CTRL_L:
	    ((tv)->GetIM())->RedrawWindow();
	    break;
	  case CTRL_G:
	    dynstr_destroy(&prompt);
	    dynstr_destroy(&pattern);
	    statestack_destroy(&stack);
	    message::DisplayString(tv, 0, "Cancelled.");
	    im::ForceUpdate();
	    return;
	  case CTRL_H:
	  case DEL:
	    break;
	  case CTRL_Q:
	    while ((c = ((tv)->GetIM())->GetCharacter()) == EOF)
		;
	    dynstr_addchar(&pattern, c);
	    goto compilestate;
	  case CTRL_R:
	  case CTRL_S:
	    forwardp = (c == CTRL_S);
	    goto initialstate;
	  case CTRL_W:
	    if ((tv)->GetDotLength() > 0)
		goto appendselectionstate;
	    break;
	  case CTRL_Y:
	    goto appendkillheadstate;
	  case ESC:
	    wasmeta = ((tv)->GetIM())->WasMeta();
	    /* Fall through */
	  case EOF:
	    goto exitstate;
	  default:
	    if (isascii(c) && (isprint(c) || isspace(c))) {
		dynstr_addchar(&pattern, c);
		goto compilestate;
	    }
	    else {
		dodokey = 1;
		goto exitstate;
	    }
	}
    }

  compilestate:

  if ((compileerr = compiled.CompilePattern(pattern.text)) == NULL)
	goto searchstate;
    statestack_push(&stack, pattern.used, StackTop->wrappedp,
		    StackTop->failurep, StackTop->position, StackTop->length,
		    StackTop->searchfrom, forwardp);
    goto partialstate;

  searchstate:

    if (forwardp)
	foundloc = compiled.MatchPattern(txt, StackTop->searchfrom);
    else
	foundloc = compiled.MatchPatternReverse(txt, StackTop->searchfrom);
    if (foundloc >= 0) {
	statestack_push(&stack, pattern.used, StackTop->wrappedp,
			0, foundloc, compiled.GetMatchLength(),
			StackTop->searchfrom, forwardp);
	(tv)->SetDotPosition( foundloc);
	(tv)->SetDotLength( compiled.GetMatchLength());
	(tv)->FrameDot( foundloc);
	goto successstate;
    }
    statestack_push(&stack, pattern.used, StackTop->wrappedp,
		    1, StackTop->position, StackTop->length,
		    StackTop->searchfrom, forwardp);
    if (im::IsPlaying()) {
	im::CancelMacro();	/* This section should emulate exitstate */
	
	if (!dynstr_empty(&pattern))
	    dynstr_copy(&LastPattern, &pattern);
	dynstr_destroy(&prompt);
	dynstr_destroy(&pattern);
	statestack_destroy(&stack);
	(tv->atMarker)->SetPos( origpos);
	(tv->atMarker)->SetLength( origlen);
	message::DisplayString(tv, 0,
			      "Search failed, macro aborted, mark set.");
	im::ForceUpdate();
	return;
    }
    goto failurestate;

  newsearchstate:

    if (forwardp)
	foundloc = compiled.MatchPattern(txt, newsearchfrom);
    else
	foundloc = compiled.MatchPatternReverse(txt, newsearchfrom);
    if (foundloc >= 0) {
	statestack_push(&stack, pattern.used, StackTop->wrappedp,
			0, foundloc, compiled.GetMatchLength(),
			newsearchfrom, forwardp);
	(tv)->SetDotPosition( foundloc);
	(tv)->SetDotLength( compiled.GetMatchLength());
	(tv)->FrameDot( foundloc);
	goto successstate;
    }
    statestack_push(&stack, pattern.used, StackTop->wrappedp,
		    1, StackTop->position, StackTop->length,
		    StackTop->searchfrom, forwardp);
    if (im::IsPlaying()) {
	im::CancelMacro();	/* This section should emulate exitstate */
	
	if (!dynstr_empty(&pattern))
	    dynstr_copy(&LastPattern, &pattern);
	dynstr_destroy(&prompt);
	dynstr_destroy(&pattern);
	statestack_destroy(&stack);
	(tv->atMarker)->SetPos( origpos);
	(tv->atMarker)->SetLength( origlen);
	message::DisplayString(tv, 0,
			      "Search failed, macro aborted, mark set.");
	im::ForceUpdate();
	return;
    }
    goto failurestate;

  wrapsearchstate:

    if (forwardp)
	foundloc = compiled.MatchPattern(txt, (long) 0);
    else
	foundloc = compiled.MatchPatternReverse(txt, (txt)->GetLength());
    if (foundloc >= 0) {
	statestack_push(&stack, pattern.used, 1,
			0, foundloc, compiled.GetMatchLength(),
			(long) 0, forwardp);
	(tv)->SetDotPosition( foundloc);
	(tv)->SetDotLength( compiled.GetMatchLength());
	(tv)->FrameDot( foundloc);
	goto successstate;
    }
    statestack_push(&stack, pattern.used, 1,
		    1, StackTop->position, StackTop->length,
		    StackTop->searchfrom, forwardp);
    if (im::IsPlaying()) {
	im::CancelMacro();	/* This section should emulate exitstate */
	
	if (!dynstr_empty(&pattern))
	    dynstr_copy(&LastPattern, &pattern);
	dynstr_destroy(&prompt);
	dynstr_destroy(&pattern);
	statestack_destroy(&stack);
	(tv->atMarker)->SetPos( origpos);
	(tv->atMarker)->SetLength( origlen);
	message::DisplayString(tv, 0,
			      "Search failed, macro aborted, mark set.");
	im::ForceUpdate();
	return;
    }
    goto failurestate;

  partialstate:

    dynstr_put(&prompt, "");
    if (StackTop->failurep)
	dynstr_append(&prompt, "Failing ");
    if (StackTop->wrappedp) {
	if (StackTop->failurep)
	    dynstr_addchar(&prompt, 'w');
	else
	    dynstr_addchar(&prompt, 'W');
	dynstr_append(&prompt, "rapped ");
    }
    dynstr_append(&prompt, "I-Search");
    if (!forwardp)
	dynstr_append(&prompt, " backward");
    dynstr_append(&prompt, ": ");
    dynstr_append(&prompt, pattern.text);
    dynstr_append(&prompt, "  [incomplete input");
    if (compileerr) {
	dynstr_append(&prompt, " - ");
	dynstr_append(&prompt, compileerr);
    }
    dynstr_append(&prompt, "]");
    message::DisplayString(tv, 0, prompt.text);
    im::ForceUpdate();
    while (1) {
	c = ((tv)->GetIM())->GetCharacter();
	switch (c) {
	  case CTRL_L:
	    ((tv)->GetIM())->RedrawWindow();
	    break;
	  case CTRL_G:
	    dynstr_destroy(&prompt);
	    dynstr_destroy(&pattern);
	    statestack_destroy(&stack);
	    message::DisplayString(tv, 0, "Cancelled.");
	    im::ForceUpdate();
	    return;
	  case CTRL_H:
	  case DEL:
	    goto popstate;
	  case CTRL_Q:
	    while ((c = ((tv)->GetIM())->GetCharacter()) == EOF)
		;
	    dynstr_addchar(&pattern, c);
	    goto compilestate;
	  case CTRL_R:
	  case CTRL_S:
	    forwardp = (c == CTRL_S);
	    break;
	  case CTRL_W:
	    goto appendselectionstate;
	  case CTRL_Y:
	    goto appendkillheadstate;
	  case ESC:
	    wasmeta = ((tv)->GetIM())->WasMeta();
	    /* Fall through */
	  case EOF:
	    goto exitstate;
	  default:
	    if (isascii(c) && (isprint(c) || isspace(c))) {
		dynstr_addchar(&pattern, c);
		goto compilestate;
	    }
	    else {
		dodokey = 1;
		goto exitstate;
	    }
	}
    }

  exitstate:

    if (!dynstr_empty(&pattern))
	dynstr_copy(&LastPattern, &pattern);
    dynstr_destroy(&prompt);
    dynstr_destroy(&pattern);
    statestack_destroy(&stack);
    (tv->atMarker)->SetPos( origpos);
    (tv->atMarker)->SetLength( origlen);
    message::DisplayString(tv, 0, "Mark set.");
    im::ForceUpdate();
    if (dodokey)
	((tv)->GetIM())->DoKey( c);
    else {
	if (wasmeta) {
	    ((tv)->GetIM())->DoKey( ESC);
	    ((tv)->GetIM())->DoKey( ((tv)->GetIM())->GetCharacter());
	}
    }
    return;
    
  popstate:

    statestack_pop(&stack);
    goto poppedstate;

  popbeforeerrorstate:

    while (StackTop->failurep) {
	statestack_pop(&stack);
    }
    goto poppedstate;

  poppedstate:

    (tv)->SetDotPosition( StackTop->position);
    (tv)->SetDotLength( StackTop->length);
    (tv)->FrameDot( StackTop->position);
    dynstr_shortento(&pattern, StackTop->patternlen);
    forwardp = StackTop->forwardp;
    if (dynstr_empty(&pattern))
	goto emptypatternstate;
    if ((compileerr = compiled.CompilePattern(pattern.text)))
	goto partialstate;
    if (StackTop->failurep)
	goto failurestate;
    goto successstate;

  appendselectionstate:

    if (!(tmpbuf = (char *)malloc(1 + (tv)->GetDotLength()))) {
	longjmp(jmpenv, 1);
    }
    (txt)->CopySubString( (tv)->GetDotPosition(),
		       (tv)->GetDotLength(), tmpbuf, FALSE);
    dynstr_append(&pattern, tmpbuf);
    free(tmpbuf);
    goto compilestate;

  appendkillheadstate:

#define CLINELENGTH (80)
    tmp_file = ((tv)->GetIM())->FromCutBuffer();
    {
      char *res, inbuf[CLINELENGTH];
      boolean usual;

      res = fgets(inbuf, CLINELENGTH, tmp_file);
      usual = FALSE;
      if (res) {
	  if (strncmp(inbuf, "\\begindata{", 11))
	      usual = TRUE;
	  else {
	      long num;
	      long idnum;
	      char obuffer[296];
	      class text *tx;
	      num = sscanf(inbuf, "\\begindata{%[^,],%ld}", obuffer, &idnum);
	      if (num != 2)
		  usual = TRUE;
	      else {
		  if (ATK::IsTypeByName(obuffer, "text")) {
		      tx = (class text *)ATK::NewObject(obuffer);
		      num = tx->Read(tmp_file, idnum);
		      if (!num) {
			  idnum = tx->GetLength();
			  for (num=0; num < idnum; num++) {
			      c = tx->GetChar(num);
			      dynstr_addchar(&pattern, c);
			  }
		      }
		      else {
			  /* error reading inset -- do nothing */
		      }
		      tx->Destroy();
		  }
		  else {
		      /* not a text inset -- do nothing */
		  }
	      }
	  }

	  if (usual) {
	      for (res = inbuf; *res; res++)
		  dynstr_addchar(&pattern, *res);
	      while ((c = fgetc(tmp_file)) != EOF)
		  dynstr_addchar(&pattern, c);
	  }
      }
    }
    ((tv)->GetIM())->CloseFromCutBuffer(tmp_file);
    goto compilestate;

  successstate:

    dynstr_put(&prompt, "");
    if (StackTop->wrappedp)
	dynstr_append(&prompt, "Wrapped ");
    dynstr_append(&prompt, "I-Search");
    if (!forwardp)
	dynstr_append(&prompt, " backward");
    dynstr_append(&prompt, ": ");
    dynstr_append(&prompt, pattern.text);
    message::DisplayString(tv, 0, prompt.text);
    im::ForceUpdate();
    while (1) {
	c = ((tv)->GetIM())->GetCharacter();
	switch (c) {
	  case CTRL_L:
	    ((tv)->GetIM())->RedrawWindow();
	    break;
	  case CTRL_G:
	    dynstr_destroy(&prompt);
	    dynstr_destroy(&pattern);
	    statestack_destroy(&stack);
	    (tv)->SetDotPosition( origpos);
	    (tv)->SetDotLength( origlen);
	    (tv)->FrameDot( origpos);
	    message::DisplayString(tv, 0, "Cancelled.");
	    im::ForceUpdate();
	    return;
	  case CTRL_H:
	  case DEL:
	    goto popstate;
	  case CTRL_Q:
	    while ((c = ((tv)->GetIM())->GetCharacter()) == EOF)
		;
	    dynstr_addchar(&pattern, c);
	    goto compilestate;
	  case CTRL_R:
	  case CTRL_S:
	    oldforwardp = forwardp;
	    forwardp = (c == CTRL_S);
	    if (oldforwardp == forwardp) {
		if (forwardp)
		    newsearchfrom = StackTop->position + 1;
		else
		    newsearchfrom = StackTop->position - 1;
		goto newsearchstate;
	    }
	    goto successstate;
	  case CTRL_W:
	    if ((tv)->GetDotLength() > 0)
		goto appendselectionstate;
	    break;
	  case CTRL_Y:
	    goto appendkillheadstate;
	  case ESC:
	    wasmeta = ((tv)->GetIM())->WasMeta();
	    /* Fall through */
	  case EOF:
	    goto exitstate;
	  default:
	    if (isascii(c) && (isprint(c) || isspace(c))) {
		dynstr_addchar(&pattern, c);
		goto compilestate;
	    }
	    else {
		dodokey = 1;
		goto exitstate;
	    }
	}
    }

  failurestate:

    dynstr_put(&prompt, "Failing ");
    if (StackTop->wrappedp)
	dynstr_append(&prompt, "wrapped ");
    dynstr_append(&prompt, "I-search");
    if (!forwardp)
	dynstr_append(&prompt, " backward");
    dynstr_append(&prompt, ": ");
    dynstr_append(&prompt, pattern.text);
    message::DisplayString(tv, 0, prompt.text);
    im::ForceUpdate();
    while (1) {
	c = ((tv)->GetIM())->GetCharacter();
	switch (c) {
	  case CTRL_L:
	    ((tv)->GetIM())->RedrawWindow();
	    break;
	  case CTRL_G:
	    goto popbeforeerrorstate;
	  case CTRL_H:
	  case DEL:
	    goto popstate;
	  case CTRL_Q:
	    while ((c = ((tv)->GetIM())->GetCharacter()) == EOF)
		;
	    dynstr_addchar(&pattern, c);
	    goto compilestate;
	  case CTRL_R:
	  case CTRL_S:
	    oldforwardp = forwardp;
	    forwardp = (c == CTRL_S);
	    if (oldforwardp == forwardp)
		goto wrapsearchstate;
	    goto searchstate;
	  case CTRL_W:
	    if ((tv)->GetDotLength() > 0)
		goto appendselectionstate;
	    break;
	  case CTRL_Y:
	    goto appendkillheadstate;
	  case ESC:
	    wasmeta = ((tv)->GetIM())->WasMeta();
	    /* Fall through */
	  case EOF:
	    goto exitstate;
	  default:
	    if (isascii(c) && (isprint(c) || isspace(c))) {
		dynstr_addchar(&pattern, c);
		goto compilestate;
	    }
	    else {
		dodokey = 1;
		goto exitstate;
	    }
	}
    }
}

static void     fsearch(class textview  *tv, long             key)
{
    dosearch(tv, 1);
}

static void     rsearch(class textview  *tv, long             key)
{
    dosearch(tv, 0);
}

boolean         gsearch::InitializeClass()
{
    static struct bind_Description fns[] = {
	{"gsearch-forward", NULL, 0, NULL, 0, 0, (proctable_fptr)fsearch,
        "Search forward incrementally.", "gsearch"},
        {"gsearch-backward", NULL, 0, NULL, 0, 0, (proctable_fptr)rsearch,
	     "Search backward incrementally.", "gsearch"},
        {NULL},
    };
    struct ATKregistryEntry  *textviewClassinfo;

    if (dynstr_init(&LastPattern))
	return FALSE;
    textviewClassinfo = ATK::LoadClass("textview");
    if (textviewClassinfo != NULL) {
        bind::BindList(fns, NULL, NULL, textviewClassinfo);
        return TRUE;
    }
    else
        return FALSE;
}
