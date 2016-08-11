/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/hyplink/RCS/pushbutton.C,v 1.5 1996/07/06 03:15:51 wjh Exp $";
#endif

#include <andrewos.h>
ATK_IMPL("pushbutton.H")
#include <stdio.h>
#include <sys/param.h>
#include <util.h>
#include <atom.H>
#include <cursor.H>
#include <environ.H>
#include <fontdesc.H>
#include <graphic.H>
#include <observable.H>
#include <pushbutton.H>

/* Defined constants and macros */
#define DS_VERSION 2 /* datastream version */

#define MAX_LINE_LENGTH 70  /* can't be less than 6 */


/* External declarations */

/* Forward Declarations */

  

/* Global variables */
static class atom *pushbutton_trigger;



ATKdefineRegistry(pushbutton, dataobject, pushbutton::InitializeClass);
#ifndef NORCSID
#endif
static void pushbutton__WriteDataPart(class pushbutton  *self, FILE  *fp);
static long pushbutton__ReadDataPart(class pushbutton  *self, FILE  *fp, int dsversion);
static long pushbutton_SanelyReturnReadError(class pushbutton  *self, FILE  *fp, long  id, long  code);
static int htoin(char  *s, int  n);
static void WriteLine(FILE  *f, char  *l);
static char * GlomStrings(char  *s , char  *t);
static char * ReadLine(FILE  *f);
static char * EncodeFont(class pushbutton  *self);


boolean
pushbutton::InitializeClass()
{
    /* 
      Initialize all the class data.
      */
    if ((pushbutton_trigger = atom::Intern("buttonpushed")) == NULL) return(FALSE);
    return(TRUE);
}


pushbutton::pushbutton()
{
	ATKinit;

/*
  Inititialize the object instance data.
*/

    this->text = NULL;
    observable::DefineTrigger(this->ATKregistry(), pushbutton_trigger);

    this->style = environ::GetProfileInt("pushbuttonstyle", 2);

    this->foreground_name = environ::GetProfile("foreground");
    if(this->foreground_name) this->foreground_name=NewString(this->foreground_name);
    if ((this->foreground_name == NULL) || (strcmp(this->foreground_name, "") == 0)) {
	if (this->foreground_name != NULL) free(this->foreground_name);
	this->foreground_name = NewString("black");
    } else {
	this->foreground_name = NewString(this->foreground_name);
    }
    this->foreground_color[0] = 0;
    this->foreground_color[1] = 0;
    this->foreground_color[2] = 0;

    this->background_name = environ::GetProfile("background");
    if(this->background_name) this->background_name=NewString(this->background_name);
    if ((this->background_name == NULL) || (strcmp(this->background_name, "") == 0)) {
	if (this->background_name != NULL) free(this->background_name);
	this->background_name = NewString("white");
    } else {
	this->background_name = NewString(this->background_name);
    }
    this->background_color[0] = 0;
    this->background_color[1] = 0;
    this->background_color[2] = 0;

    this->myfontdesc = NULL;
    THROWONFAILURE((TRUE));
}


pushbutton::~pushbutton()
{
	ATKinit;

/*
  Finalize the object instance data.
*/
  if (this->text != NULL) {
    free(this->text);
    this->text = NULL;
  }
  if (this->foreground_name != NULL) {
      free(this->foreground_name);
      this->foreground_name = NULL;
  }
  if (this->background_name != NULL) {
      free(this->background_name);
      this->background_name = NULL;
  }
  return;
}


static void
pushbutton__WriteDataPart(class pushbutton  *self, FILE  *fp)
{
/*
  Write the object data out onto the datastream.
*/
  char buf[100], *encfont;

  WriteLine(fp, self->text ? self->text : "");
  sprintf(buf,"%d", self->style); /* *BUG* how do we tell a defaulted 
				     style from a set style? */
  WriteLine(fp, buf);

  encfont = self->myfontdesc ? EncodeFont(self) : NULL;
  WriteLine(fp, encfont ? encfont : "");
  if (encfont) {
    free(encfont);
    encfont = NULL;
  }

#ifdef PL8
  if (!(self->new_DS)) return;
#endif /* PL8 */

  if (self->foreground_name != NULL) {
      sprintf(buf, "%s", self->foreground_name);
  } else {
      sprintf(buf, "0x%02x%02x%02x",
	      self->foreground_color[0],
	      self->foreground_color[1],
	      self->foreground_color[2]);
  }
  WriteLine(fp, buf);

  if (self->background_name != NULL) {
      sprintf(buf, "%s", self->background_name);
  } else {
      sprintf(buf, "0x%02x%02x%02x",
	      self->background_color[0],
	      self->background_color[1],
	      self->background_color[2]);
  }
  WriteLine(fp, buf);

}



long
pushbutton::Write(FILE  *fp, long  id, int  level)
{
/*
  Write the object data out onto the datastream.

  Sample output from datastream version 1:
    \begindata{pushbutton, 1234567}
    Datastream version: 1
    This is my button label     -- label
    2                           -- style
    andy12b                     -- font
    \enddata{pushbutton, 1234567}

  Sample output from datastream version 2:
    \begindata{pushbutton, 1234567}
    Datastream version: 2
    This is my button label     -- label
    2                           -- style
    andy12b                     -- font
    black                       -- foreground color
    0xFFFFFF                    -- background color, RGB representation
    \enddata{pushbutton, 1234567}

*/

  long uniqueid = (this)->UniqueID();

  if (id != (this)->GetWriteID()) {
    /* New Write Operation */
    (this)->SetWriteID( id);
    fprintf(fp, "\\begindata{%s,%d}\nDatastream version: %d\n",
	    (this)->GetTypeName(), 
	    uniqueid, 
#ifndef PL8
	    DS_VERSION
#else /* PL8 */
            this->new_DS?DS_VERSION:1	/* lie, if we must*/
#endif /* PL8 */
);
    pushbutton__WriteDataPart(this, fp);

    fprintf(fp, "\\enddata{%s,%d}\n", (this)->GetTypeName(), uniqueid);
  }
  return(uniqueid);
}


static long
pushbutton__ReadDataPart(class pushbutton  *self, FILE  *fp, int dsversion)
{
/*
  Read in the object from the file.
*/
  char *buf;
  unsigned char rgb[3];
  
  if ((buf = ReadLine(fp)) == NULL)
    return(dataobject_PREMATUREEOF);
  if (strcmp(buf,"")!= 0 )
    (self)->SetText( buf);
  free(buf);

  if ((buf = ReadLine(fp)) == NULL)
    return(dataobject_PREMATUREEOF);
  if (strcmp(buf,"")!= 0 )
    (self)->SetStyle( atoi(buf));
  free(buf);

  if ((buf = ReadLine(fp)) == NULL)
    return(dataobject_PREMATUREEOF);
  if (strcmp(buf,"")!= 0) {
    char name[MAXPATHLEN];
    long style, size;
    if (fontdesc::ExplodeFontName(buf,name,sizeof(name), &style, &size)) {
      (self)->SetButtonFont(fontdesc::Create(name,style,size));
    }
  }
  free(buf);

  if (dsversion >= 2) {
      if ((buf = ReadLine(fp)) == NULL)
	return(dataobject_PREMATUREEOF);
      if (strcmp(buf,"")!= 0 ) {
	  if ((strncmp(buf, "0x", 2) != 0)
	      &&(strncmp(buf, "0X", 2) != 0)) {
	      (self)->SetFGColor( buf, 0, 0, 0);
	    } else {
		(self)->ParseRGB( buf, rgb);
		(self)->SetFGColor( NULL, rgb[0], rgb[1], rgb[2]);
	    }
      }
      free(buf);
	  
      if ((buf = ReadLine(fp)) == NULL)
	return(dataobject_PREMATUREEOF);
      if (strcmp(buf,"")!= 0 ) {
	  if (strncmp(buf, "0x", 2) != 0)  {
	      (self)->SetBGColor( buf, 0, 0, 0);
	    } else {
		(self)->ParseRGB( buf, rgb);
		(self)->SetBGColor( NULL, rgb[0], rgb[1], rgb[2]);
	    }
      }
      free(buf);
  }

  return(dataobject_NOREADERROR);
}



static long
pushbutton_SanelyReturnReadError(class pushbutton  *self, FILE  *fp, long  id, long  code)
                    {
    /*
      Suck up the file until our enddata, then return the error code.
      */
    char *buf, buf2[255];

    buf = NULL;
    sprintf(buf2, "\\enddata{%s,%ld}", (self)->GetTypeName(), id);
    do {
	if (buf != NULL) free(buf);
	if ((buf = ReadLine(fp)) == NULL)
	  return(dataobject_PREMATUREEOF);
    } while (strncmp(buf, "\\enddata{", 9) != 0); /* find an enddata */

    if (strcmp(buf, buf2) != 0) {
	free(buf);
	return(dataobject_MISSINGENDDATAMARKER); /* not ours! */
    }
    free(buf);

    return(code);
}



long
pushbutton::Read(FILE  *fp, long  id)
{
/*
  Read in the object from the file.
  (BUG) Doesn't set the font.
*/
  char *buf;
  long result;
  int dsversion;
  
  (this)->SetID( (this)->UniqueID());

  if ((buf = ReadLine(fp)) == NULL)
    return(pushbutton_SanelyReturnReadError(this, fp, id, dataobject_PREMATUREEOF));
  if (strncmp(buf,"Datastream version:",19))
    return(pushbutton_SanelyReturnReadError(this, fp, id, dataobject_BADFORMAT));
  if ((dsversion = atoi(buf+19)) > DS_VERSION)	/* datastream version */
    return(pushbutton_SanelyReturnReadError(this, fp, id, dataobject_BADFORMAT));
  if (dsversion < 1)
    return(pushbutton_SanelyReturnReadError(this, fp, id, dataobject_BADFORMAT));
  free(buf);
#ifdef PL8
  if (dsversion > 1) this->new_DS = TRUE; /* preserve information */
#endif /* PL8 */

  if ((result = pushbutton__ReadDataPart(this, fp, dsversion)) != dataobject_NOREADERROR)
    return(pushbutton_SanelyReturnReadError(this, fp, id, result));

  return(pushbutton_SanelyReturnReadError(this, fp, id, dataobject_NOREADERROR));
}


void
pushbutton::SetText(char  *txt)
{
/*
  Set the text label for this object.
	Do the NewString before the free so the cache will be 
	sure to see a change in the label.
*/
    char *oldtxt = text;
    text = (txt) ? NewString(txt) : NULL; 
    SetModified();
    NotifyObservers(observable_OBJECTCHANGED);
    if (oldtxt) 
      free(oldtxt);
}


void
pushbutton::SetStyle(int  stylecode)
{
/*
  Set the style code for this object.
*/

    this->style = stylecode;
    (this)->SetModified();
    (this)->NotifyObservers(observable_OBJECTCHANGED);
}


void
pushbutton::SetButtonFont(class fontdesc  *f)
{
/*
  Set the font descriptor for this object.
*/

    /* DON'T EVER FREE FONTS! */
    /* if (self->myfontdesc) free(self->myfontdesc); */

    this->myfontdesc = f;
    (this)->SetModified();
    (this)->NotifyObservers(observable_OBJECTCHANGED);
}



void
pushbutton::SetFGColor(char  *name, int  red , int  green , int  blue)
               {
    /*
      Set the foreground color for this object.
    */

    if (this->foreground_name != NULL) free(this->foreground_name);
    this->foreground_name = NULL;

    if (name != NULL) {
	this->foreground_name = NewString(name);
	this->foreground_color[0] = 0;
	this->foreground_color[1] = 0;
	this->foreground_color[2] = 0;
    } else {
	this->foreground_color[0] = red;
	this->foreground_color[1] = green;
	this->foreground_color[2] = blue;
    }
    
    (this)->SetModified();
    (this)->NotifyObservers(observable_OBJECTCHANGED);
#ifdef PL8
    this->new_DS = TRUE; /* preserve information */
#endif /* PL8 */
}


void
pushbutton::SetBGColor(char  *name, int  red , int  green , int  blue)
               {
    /*
      Set the background color for this object.
    */

    if (this->background_name != NULL) free(this->background_name);
    this->background_name = NULL;

    if (name != NULL) {
	this->background_name = NewString(name);
	this->background_color[0] = 0;
	this->background_color[1] = 0;
	this->background_color[2] = 0;
    } else {
	this->background_color[0] = red;
	this->background_color[1] = green;
	this->background_color[2] = blue;
    }
    
    (this)->SetModified();
    (this)->NotifyObservers(observable_OBJECTCHANGED);
#ifdef PL8
    this->new_DS = TRUE; /* preserve information */
#endif /* PL8 */
}


char *
pushbutton::GetFGColor(unsigned char rgb_vect[])
          {
    /*
      Return the foreground color for this object.
    */
    
    rgb_vect[0] = this->foreground_color[0];
    rgb_vect[1] = this->foreground_color[1];
    rgb_vect[2] = this->foreground_color[2];

    return(this->foreground_name);
}


char *
pushbutton::GetBGColor(unsigned char rgb_vect[])
          {
    /*
      Return the background color for this object.
    */
    
    rgb_vect[0] = this->background_color[0];
    rgb_vect[1] = this->background_color[1];
    rgb_vect[2] = this->background_color[2];

    return(this->background_name);
}


static int
htoin(char  *s, int  n)
          {
    int i, t;

    for(i = 0, t = 0; i < n; ++i) {
	t *= 16;
	t += ((s[i]>='a')
	      ?(s[i]-'a'+10)
	      :((s[i]>='A')
		?(s[i]-'A'+10)
		:(s[i]-'0')));
    }

    return(t);
}


void
pushbutton::ParseRGB(char  *rgb_string, unsigned char *rgb_vect)
               {
    /*
      Return the background color for this object.
    */
    
    if ((rgb_string != NULL)
	&& ((strncmp(rgb_string, "0x", 2) == 0)
	    || (strncmp(rgb_string, "0X", 2) == 0))
	&& (strlen(rgb_string) == 8)) {
	rgb_vect[0] = htoin(rgb_string+2,2);
	rgb_vect[1] = htoin(rgb_string+4,2);
	rgb_vect[2] = htoin(rgb_string+6,2);
	}
}


static void
WriteLine(FILE  *f, char  *l)
{
/* 
  Output a single line onto the data stream, quoting
  back slashes and staying within line length limits.
  Warning:  this routine wasn't meant to handle embedded
  newlines.
*/

  char buf[MAX_LINE_LENGTH];
  int i = 0;

  for (;*l != '\0'; ++l) {
    if (i > (MAX_LINE_LENGTH - 5)) {
      buf[i++] = '\\';  /* signal for line continuation */
      buf[i++] = '\n';
      buf[i++] = '\0';
      fputs(buf,f);
      i = 0;
    } /* if (i > ...) */
    switch (*l) {
    case '\\': 
      /* if a backslash, quote it. */
      buf[i++] = '\\';
      buf[i++] = *l;
      break;
    default:
      buf[i++] = *l;
    } /* switch (*l) */
  } /* for (; *l != ... ) */

  /* Need to empty buffer */
  if ((i > 0) && (buf[i-1]==' ')) {
    /* don't allow trailing whitespace */
    buf[i++] = '\\';
    buf[i++] = '\n';
    buf[i++] = '\0';
    fputs(buf,f);
    fputs("\n",f);
  } else {
    buf[i++] = '\n';
    buf[i++] = '\0';
    fputs(buf,f);
  }
}


static char *
GlomStrings(char  *s , char  *t)
{
/* 
  Safely (allocs more memory) concatenates the two strings, 
  freeing the first.  Meant to build a new string of unknown length.
*/

  char *r;

  if (r = (char *)malloc(strlen(s)+strlen(t)+1)) {
    *r = '\0';
    strcpy(r,s);
    free(s);
    strcat(r,t);
    return(r);
  } else {
    free(s);
    return(NULL);
  }
}


static char *
ReadLine(FILE  *f)
{
/* 
  Reads from the datastream, attempting to return a single string.
  Undoes quoting and broken lines.
  Warning:  this routine wasn't meant to handle embedded
  newlines.
  Warning:  possible source of memory leaks;  remember to 
  free the returned string when finished with it!
*/

  char buf[MAX_LINE_LENGTH], /* (BUG) What if the datastream is broken? */
       buf2[MAX_LINE_LENGTH],
      *result;
  int i,j;

  
  if (result = (char *)malloc(1)) {
    *result = '\0';

    while (fgets(buf,sizeof(buf),f)) {
      for (i = 0, j = 0; buf[i] != '\0'; ++i) {
	switch (buf[i]) {
	case '\\':
	  /* Unquote backslash or splice line */
	  switch (buf[++i]) {
	  case '\\':
	    /* Unquote the backslash */
	    buf2[j++] = buf[i];
	    break;
	  case '\n':
	    /* broke long line */
	    break;
	  default:
	    /* things like \enddata come through here */
	    buf2[j++] = '\\';
	    buf2[j++] = buf[i];
	    break;
	  } /* switch (buf[++i]) */
	  break;
	case '\n':
	  /* An unquoted newline means end of string */
	  buf2[j++] = '\0';
	  result = GlomStrings(result, buf2);
	  return(result);
	default:
	  buf2[j++] = buf[i];
	  break;
	} /* switch (buf[i]) */
      } /* for (i = 0, ...) */
      buf2[j++] = '\0';
      result = GlomStrings(result, buf2);
    } /* while (fgets...) */
    /* Should not get here... it means we went off the end
       of the data stream.  Ooops. */
  } /* if (result = ... ) */
  return(NULL);
}


static char *
EncodeFont(class pushbutton  *self)
{
/*
  Returns a string representing the name of the font for this object.
  (BUG) I shouldn't have to do this function, it should be a method
  of the fontdesc object.  In any case, I handle only Bold, Italic,
  and fixed styles.
*/

  char *buf, type[15];
  long myfonttype, myfontsize;
  char *myfontname;

  *type = '\0';
  myfontname = (self->myfontdesc)->GetFontFamily();
  myfontsize = (self->myfontdesc)->GetFontSize();
  myfonttype = (self->myfontdesc)->GetFontStyle();
  if (myfonttype & fontdesc_Bold) strcpy(type,"b");
  if (myfonttype & fontdesc_Italic) strcpy(type,"i");
  if (myfonttype & fontdesc_Fixed) strcpy(type,"f");
  if (buf = (char *)malloc(strlen(myfontname)+25)) {
    sprintf(buf,"%s%d%s", myfontname, myfontsize, type);
    return (buf);
  } else {
    return(NULL);
  }
}

