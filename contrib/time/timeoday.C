/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <stdio.h>
#include <timeoday.H>
#include <observable.H>
#include <im.H>
#include <util.h>
#include <environ.H>

/* Defined constants and macros */
#define MAX_LINE_LENGTH 70  /* can't be less than 6 */
#define DS_VERSION 1 /* datastream version */
#define FONTFAMILY "andysans"
#define FONTTYPE 0
#define FONTSIZE 12
#define SECMIN 60
#define MINHOUR 60
#define HOURDAY 12
#undef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))

/* External declarations */

/* Forward Declarations */

  

/* Global variables */
static const char * const months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December", NULL};
static const char * const weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", NULL};
static int maxdigraphlen;



ATKdefineRegistry(timeoday, dataobject, timeoday::InitializeClass);
static void UpdateTime(class timeoday  *self);
static void WriteLine(FILE  *f, const char  *l);
static char * GlomStrings(char  *s , char  *t);
static char * ReadLine(FILE  *f);
static char * EncodeFont(class timeoday  *self);


boolean
timeoday::InitializeClass()
{
/* 
  Initialize all the class data.
*/
  int i, t;

  maxdigraphlen = 2;
  for (i=0; months[i]; ++i) {
    t = strlen(months[i]);
    if (t>maxdigraphlen) maxdigraphlen = t;
  }
  for (i=0; weekdays[i]; ++i) {
    t = strlen(weekdays[i]);
    if (t>maxdigraphlen) maxdigraphlen = t;
  }
  return(TRUE);
}


void
timeoday::FormatTime()
{
/*     Field Descriptors:
          n    insert a new-line character
          t    insert a tab character
          m    month of year - 01 to 12
          O    month of year - 1 to 12
          d    day of month - 01 to 31
          A    day of month - 1 to 31
          Z    ordinal day of month - 1st to 31st
          y    last 2 digits of year - 00 to 99
          Y    year - 1900 on ...
          D    date as mm/dd/yy
          H    hour - 00 to 23
	  u    hour - 1 to 12
          M    minute - 00 to 59
          S    second - 00 to 59
          T    time as HH:MM:SS
          j    day of year - 001 to 366
          w    day of week - Sunday = 0
          W    weekday - Sunday to Saturday
          a    abbreviated weekday - Sun to Sat
          h    abbreviated month - Jan to Dec
          o    month - January to December
          r    time in AM/PM notation
	  P    AM or PM
*/
  int i, j;
  struct tm *the_time;
  
  if (this->tod == NULL) return;

  the_time = localtime(&(this->now));
  for(j = i = 0; this->format[i]; ++i) {
    if (this->format[i]=='%') {
      switch(this->format[++i]) {
      case '%':
	this->tod[j++] = '%';
	break;
      case 'n':
	this->tod[j++] = '\n';
	break;
      case 't':
	this->tod[j++] = '\t';
	break;
      case 'm':
	sprintf(this->tod+j, "%02d", the_time->tm_mon+1);
	j = strlen(this->tod);
	break;
      case 'O':
	sprintf(this->tod+j, "%d", the_time->tm_mon+1);
	j = strlen(this->tod);
	break;
      case 'd':
	sprintf(this->tod+j, "%02d", the_time->tm_mday);
	j = strlen(this->tod);
	break;
      case 'A':
	sprintf(this->tod+j, "%d", the_time->tm_mday);
	j = strlen(this->tod);
	break;
      case 'Z':
	sprintf(this->tod+j, "%d%s", the_time->tm_mday,
		ORDINALIZE(the_time->tm_mday));
	j = strlen(this->tod);
	break;
      case 'y':
	sprintf(this->tod+j, "%02d", (the_time->tm_year)%100);
	j = strlen(this->tod);
	break;
      case 'Y':
	sprintf(this->tod+j, "%4d", the_time->tm_year+1900);
	j = strlen(this->tod);
	break;
      case 'D':
	sprintf(this->tod+j, "%02d/%02d/%02d", the_time->tm_mon+1,
		the_time->tm_mday, (the_time->tm_year)%100);
	j = strlen(this->tod);
	break;
      case 'H':
	sprintf(this->tod+j, "%02d", the_time->tm_hour);
	j = strlen(this->tod);
	break;
      case 'M':
	sprintf(this->tod+j, "%02d", the_time->tm_min);
	j = strlen(this->tod);
	break;
      case 'S':
	sprintf(this->tod+j, "%02d", the_time->tm_sec);
	j = strlen(this->tod);
	break;
      case 'T':
	sprintf(this->tod+j, "%02d:%02d:%02d", the_time->tm_hour,
		the_time->tm_min, the_time->tm_sec);
	j = strlen(this->tod);
	break;
      case 'j':
	sprintf(this->tod+j, "%03d", the_time->tm_yday+1);
	j = strlen(this->tod);
	break;
      case 'w':
	sprintf(this->tod+j, "%01d", the_time->tm_wday);
	j = strlen(this->tod);
	break;
      case 'a':
	sprintf(this->tod+j, "%3s", weekdays[the_time->tm_wday]);
	j += 3;
	break;
      case 'W':
	sprintf(this->tod+j, "%s", weekdays[the_time->tm_wday]);
	j = strlen(this->tod);
	break;
      case 'h':
	sprintf(this->tod+j, "%3s", months[the_time->tm_mon]);
	j += 3;
	break;
      case 'o':
	sprintf(this->tod+j, "%s", months[the_time->tm_mon]);
	j = strlen(this->tod);
	break;
      case 'r':
	sprintf(this->tod+j, "%02d:%02d:%02d %s", 
		((the_time->tm_hour)%12)==0?12:
		((the_time->tm_hour)%12),
		the_time->tm_min, the_time->tm_sec,
		the_time->tm_hour > 11 ? "PM" : "AM");
	j = strlen(this->tod);
	break;
      case 'u':
	sprintf(this->tod+j, "%d", ((the_time->tm_hour)%12)==0?12:((the_time->tm_hour)%12));
	j = strlen(this->tod);
	break;
      case 'P':
	sprintf(this->tod+j, "%s", the_time->tm_hour > 11 ? "PM" : "AM");
	j = strlen(this->tod);
	break;
      }
    } else {
      this->tod[j++] = this->format[i];
    }
  }
  this->tod[j] = '\0';
  (this)->NotifyObservers(observable_OBJECTCHANGED);
}


static void
UpdateTime(class timeoday  *self)
{
  (self)->UpdateTime();
}


void
timeoday::UpdateTime()
{
  this->now = time(0);
  this->ev = im::EnqueueEvent((event_fptr)::UpdateTime, (char *)this, event_SECtoTU(this->epoch - (this->now % this->epoch)));
  (this)->FormatTime();

  return;
}



void
timeoday::SetFormat(char  *format)
{
  int i;
  char prof_namebuf[100];

  if (this->ev) {
    (this->ev)->Cancel();
    this->ev = NULL;
  }

  if (this->format) free(this->format);
  sprintf(prof_namebuf, "%sdefaultformat", (this)->GetTypeName());
  if (format != NULL) {
      this->format = NewString(format);
  } else {
      const char *format = environ::GetProfile(prof_namebuf);
      if (format == NULL) {
	  this->format = NewString("");
      } else {
	  this->format = NewString(format);
      }
  }
  if (strcmp(this->format, "")==0) {
    if (this->format) free(this->format);
    this->format = NewString("%o %A, %Y");
  }
  this->epoch = SECMIN*MINHOUR*HOURDAY;
  for(i=0; this->format[i]; ++i) {
    if (this->format[i] == '%') {
      switch(this->format[++i]) {
      case 'S': case 'T': case 'r':
	this->epoch = MIN(this->epoch, 1);
	break;
      case 'M':
	this->epoch = MIN(this->epoch, SECMIN);
	break;
      case 'H': case 'u':
	this->epoch = MIN(this->epoch, SECMIN*MINHOUR);
	break;
      }
    }
  }
  if (this->tod) free(this->tod);
  this->tod = (char *)malloc(strlen(this->format)/2*maxdigraphlen+1);
  ::UpdateTime(this);
}


boolean
timeoday::InitializeDefaults()
{
  const char *fontfamily;
  int fonttype, fontsize;
  char prof_namebuf[100];

  (this)->SetFormat( NULL);

  sprintf(prof_namebuf, "%sdefaultfontfamily", (this)->GetTypeName());
  fontfamily = environ::GetProfile(prof_namebuf);
  if ((fontfamily == NULL) || (strcmp(fontfamily, "") == 0)) fontfamily = FONTFAMILY;

  sprintf(prof_namebuf, "%sdefaultfonttype", (this)->GetTypeName());
  fonttype = environ::GetProfileInt(prof_namebuf, FONTTYPE);

  sprintf(prof_namebuf, "%sdefaultfontsize", (this)->GetTypeName());
  fontsize = environ::GetProfileInt(prof_namebuf, FONTSIZE);
  this->myfontdesc = fontdesc::Create(fontfamily, fonttype, fontsize);

  (this)->FormatTime();
  return(TRUE);
}


timeoday::timeoday()
{
	ATKinit;

/*
  Inititialize the object instance data.
*/

  this->ev = NULL;
  this->tod = NULL;
  this->format = NULL;

  THROWONFAILURE(((this)->InitializeDefaults()));
}


timeoday::~timeoday()
{
	ATKinit;

/*
  Finalize the object instance data.
*/
  if (this->ev) (this->ev)->Cancel();
  if (this->tod) free(this->tod);
  if (this->format) free(this->format);
  return;
}


void
timeoday::WriteDataPart(FILE  *fp)
{
/*
  Write the object data out onto the datastream.
*/
  char *encfont;

  WriteLine(fp, this->format);
  encfont = this->myfontdesc ? EncodeFont(this) : NULL;
  WriteLine(fp, encfont ? encfont : "");
  if (encfont) {
    free(encfont);
    encfont = NULL;
  }
}


long
timeoday::Write(FILE  *fp, long  id, int  level)
{
/*
  Write the object data out onto the datastream.

  Sample output from datastream version 1:
    \begindata{timeoday, 1234567}
    Datastream version: 1
    format string
    font
    \enddata{timeoday, 1234567}

*/

  long uniqueid = (this)->UniqueID();

  if (id != (this)->GetWriteID()) {
    /* New Write Operation */
    (this)->SetWriteID( id);
    fprintf(fp, "\\begindata{%s,%ld}\nDatastream version: %d\n",
	    (this)->GetTypeName(), uniqueid, DS_VERSION);

    (this)->WriteDataPart( fp);

    fprintf(fp, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), uniqueid);
  }
  return(uniqueid);
}


long
timeoday::ReadDataPart(FILE  *fp)
{
/*
  Read in the object from the file.
*/
  char *buf;
  
  if ((buf = ReadLine(fp)) == NULL)
    return(dataobject_PREMATUREEOF);
  (this)->SetFormat( buf);
  free(buf);

  if ((buf = ReadLine(fp)) == NULL)
    return(dataobject_PREMATUREEOF);
  if (strcmp(buf,"")!= 0) {
    char name[MAXPATHLEN];
    long style, size;
    if (fontdesc::ExplodeFontName(buf,name,sizeof(name), &style, &size)) {
      (this)->SetFont(fontdesc::Create(name,style,size));
    }
  }
  free(buf);

  return(dataobject_NOREADERROR);
}



long
timeoday::Read(FILE  *fp, long  id)
{
/*
  Read in the object from the file.
*/
  char *buf, buf2[255];
  long result;
  
  (this)->SetID( (this)->UniqueID());

  if ((buf = ReadLine(fp)) == NULL)
    return(dataobject_PREMATUREEOF);
  if (strncmp(buf,"Datastream version:",19))
    return(dataobject_BADFORMAT);
  if (atoi(buf+19) != DS_VERSION)	/* datastream version */
    return(dataobject_BADFORMAT);
  free(buf);

  if ((result = (this)->ReadDataPart( fp)) != dataobject_NOREADERROR)
    return(result);

  if ((buf = ReadLine(fp)) == NULL)
    return(dataobject_PREMATUREEOF);
  sprintf(buf2, "\\enddata{%s,%ld}", (this)->GetTypeName(), id);
  if (strcmp(buf, buf2)) {
    free(buf);
    return(dataobject_MISSINGENDDATAMARKER);
  }
  free(buf);

  return(dataobject_NOREADERROR);
}



void
timeoday::SetFont(class fontdesc  *f)
{
/*
  Set the font descriptor for this object.
*/

  this->myfontdesc = f;
  (this)->NotifyObservers(observable_OBJECTCHANGED);
}



static void
WriteLine(FILE  *f, const char  *l)
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

  if ((r = (char *)malloc(strlen(s)+strlen(t)+1))) {
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

  
  if ((result = (char *)malloc(1))) {
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
EncodeFont(class timeoday  *self)
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
  if ((buf = (char *)malloc(strlen(myfontname)+25))) {
    sprintf(buf,"%s%ld%s", myfontname, myfontsize, type);
    return (buf);
  } else {
    return(NULL);
  }
}
