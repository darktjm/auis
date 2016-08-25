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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/preview/RCS/preview.C,v 1.9 1996/09/03 19:30:37 robr Exp $";
#endif


 

/* 
*
*	BE2 preview.
*		A program for previewing dvitroff input
*
 */
#include <andrewos.h> /* sys/file.h */
ATK_IMPL("preview.H")
#include <ctype.h>
#ifdef hpux
#include <signal.h>
#endif /* hpux */

#include <preview.H>
#include <menulist.H>
#include <scroll.H>
#include <im.H>
#include <graphic.H>
#include <keystate.H>
#include <environ.H>
#include <proctable.H>
#include <cursor.H>
#include <fontdesc.H>
#include <keymap.H>
#include <print.H>
#ifdef USEFRAME
#include <message.H>
#endif /* USEFRAME */

#define ResetOffsets(self) if(! self->DoScaling ) self->yoff = self->xoff = 0;

ATKdefineRegistry(preview, view, NULL);
#ifndef NORCSID
#endif
#ifdef hpux
#endif /* hpux */
#ifdef USEFRAME
#endif /* USEFRAME */
static void SetTitle(class preview  *self);
static void DisplayPage(class preview  *self,preview_pagetableindex  n);
static void SetScale(class preview  *self);
static void SetFullSize(class preview  *self);
static void LastPage(class preview  *self);
static void NextPage(class preview  *self);
static void SetPage(class preview  *self,long  pagenum);
static void PrintCmd(class preview  *self );
static void PrintPageCmd(class preview  *self );
static void DeleteWindowCmd(class preview  *self );
static void updatemenu(class preview  *self);
static void DeclarePage(class preview  *self,preview_pagenumber     n, long   FilePosition);
static void InitPageMap(class preview  *self);
void notdvifile(class preview  *self);
static void MakePageMap(FILE  *filein,class preview  *self);
static void MakePageMapWithoutCopying(FILE  *filein,class preview  *self);
static void DrawBorder(class preview  *self);
static void SetCursor(class preview  *self);
static void DoFindFirstPage(class preview  *self);
#ifdef hp9000s300
void preview_sigAlrm();
#endif /* hp9000s300 */
static void insert(char  *src,char  *c);
static void normalize(char  *s);
static void DoPrintCmd(class preview  *self ,int  page);
static void vgetinfo(class preview  *self, struct range  *total , struct range  *seen , struct range  *dot);
static void hgetinfo(class preview  *self, struct range  *total , struct range  *seen , struct range  *dot);
static long hwhatisat(class preview  *self, long  numerator , long  denominator);
static long vwhatisat(class preview  *self, long  numerator , long  denominator);
static void vsetframe(class preview  *self, long  position  , long  numerator , long  denominator);
static void hsetframe(class preview  *self, long  position  , long  numerator , long  denominator);


static void SetTitle(class preview  *self)
{
    if((self)->GetIM() == NULL) return;
    if(self->FindFirstPage || self->CurrentPageTableIndex == 0)
	sprintf(self->WindowTitle,"Initializing %s",self->DviBaseName);
    else if(self->PageTable[self->CurrentPageTableIndex].PageNumber < 1)
		sprintf(self->WindowTitle,"Processing %s",self->DviBaseName);
    else if (self->nowreading != -1) {
	sprintf(self->WindowTitle,"Processing %s/%d", 
		self->DviBaseName,
		self->PageTable[self->CurrentPageTableIndex].PageNumber);
    }
    else 
	sprintf(self->WindowTitle,"%s/%d of %d", 
		self->DviBaseName,
		self->PageTable[self->CurrentPageTableIndex].PageNumber,
		self->PageTable[self->NumberofPageTableEntries - 1].PageNumber
		);
    ((self)->GetIM())->SetTitle(self->WindowTitle);
}
static void DisplayPage(class preview  *self,preview_pagetableindex  n)
{
   if (self->debug)
      fprintf(stderr, "DisplayPage %d\n", n);

   /* Change the title to reflect the new page number */
   SetTitle(self);
   (self)->EraseVisualRect();

   if (self->WindowWidth < self->minWidth
   	 || self->WindowHeight < self->minHeight)
       return;

   DrawBorder(self);
   if (self->FindFirstPage){
      DoFindFirstPage(self);
        if(self->CurrentPageTableIndex <= 1) return; 
	if(! self->FindFirstPage){
/*	    preview_EraseVisualRect(self); */
	    if(n < self->CurrentPageTableIndex ) n = self->CurrentPageTableIndex;
	} 
   }
   if(self->FindFirstPage) return;
   if (0 <= n && n < self->NumberofPageTableEntries)
	    {
	       fseek(self->DviFileIn, self->PageTable[n].FileOffset, 0);
	       (self)->DviToDisplay();
	    }
}
void preview::WantUpdate(class view  *requestor)
        {
    if (this->RedrawRequested && (class view *)this == requestor) return;
    (this)->view::WantUpdate( requestor);
    if((class view *)this == requestor) this->RedrawRequested = TRUE;
}

void preview::Update()
{
   if (this->debug)
      fprintf(stderr, "DoRedraw\n");
   this->RedrawRequested = FALSE;
   if(this->CurrentPageTableIndex == this->nowreading) return;
    if(this->quitpending && this->CursorChanged){
        SetCursor(this);
        return;
    }
/*    preview_EraseVisualRect(self);
 */   if (this->SizeChanged){
	 this->SizeChanged = FALSE;
	this->WindowWidth = (this)->GetLogicalRight();
	this->WindowHeight = (this)->GetLogicalBottom();

	 if (this->WindowWidth < this->minWidth 
		|| this->WindowHeight < this->minHeight)
	    return;

	 if (!this->DoScaling)
	     this->DisplayResolution = preview_DISPLAY_RESOLUTION;
	 else {
		this->DisplayResolution = this->WindowHeight/11;
		if (this->DisplayResolution > 2*this->WindowWidth/17)
			this->DisplayResolution = 2*this->WindowWidth/17;
	      }

   	this->xPixelsPerPage = this->DisplayResolution * 17 / 2;
   	this->yPixelsPerPage = this->DisplayResolution * 11;

	/*  RecomputeOffsets(self);  */
     }
   DisplayPage(this,this->CurrentPageTableIndex);
   SetCursor(this);
}
void preview::FullUpdate(enum view_UpdateType  type,long  left , long  top  ,  long  width  ,  long  right)
{
	this->SizeChanged = TRUE;
	(this)->Update();
}
void preview::ReceiveInputFocus()
    {
    this->hasInputFocus = TRUE;
    this->keystate->next = NULL;
    (this)->PostKeyState( this->keystate);
    (this)->PostMenus( this->menulist);
}

void preview::LoseInputFocus()
    {
    this->hasInputFocus = FALSE;
}
static void SetScale(class preview  *self)
{

	/* change menu */
	if (self->DoScaling) return;
	self->DoScaling = TRUE;
	(self->menulist)->AddToML("Full Size~10",self->FullSizeProc,0,0);
	(self->menulist)->DeleteFromML("Scale~10");
        self->SizeChanged = TRUE;
 	if(self->hasInputFocus == TRUE) (self)->PostMenus( self->menulist);
	(self)->WantUpdate(self);
}
static void SetFullSize(class preview  *self)
{

	/* change menu */
	if (!self->DoScaling) return;
	self->DoScaling = FALSE;
	(self->menulist)->AddToML("Scale~10",self->ScaleProc,0,0);
	(self->menulist)->DeleteFromML("Full Size~10");
        self->SizeChanged = TRUE;
	if(self->hasInputFocus == TRUE)(self)->PostMenus( self->menulist);
	(self)->WantUpdate(self);

}
static void LastPage(class preview  *self)
{
/*	       if (self->CurrentPageTableIndex > self->LowestNonBlankPageIndex){ */
	       if (self->CurrentPageTableIndex > 1){
                   ResetOffsets(self);
                   self->CurrentPageTableIndex -= 1;
		   (self)->WantUpdate(self);
	}
}
static void NextPage(class preview  *self)
{
	if (!self->DviFileComplete 
	   || self->CurrentPageTableIndex < self->NumberofPageTableEntries-1){
               ResetOffsets(self);
		self->CurrentPageTableIndex += 1;
		(self)->WantUpdate(self);
	}
}
static void SetPage(class preview  *self,long  pagenum)
{
	if(pagenum < self->NumberofPageTableEntries &&
/*	    pagenum >= self->LowestNonBlankPageIndex && */
	    pagenum >= 1 &&
	    pagenum != self->CurrentPageTableIndex){
                ResetOffsets(self);
		self->CurrentPageTableIndex = pagenum;
		(self)->WantUpdate(self);
	}
}
static void PrintCmd(class preview  *self )	
{
    DoPrintCmd(self,-1);
}
static void PrintPageCmd(class preview  *self )
{
    DoPrintCmd(self,self->CurrentPageTableIndex);
}
static void DeleteWindowCmd(class preview  *self )	
{
    /* Since preview always runs in a single window and
      a seperate process, we just exit.
      previewapp will then call preview_ReadyToQuit */
    im::KeyboardExit();
}
int preview::ReadyToQuit()
{
#ifdef USEFRAME
    char answer[50];
    if(!this->printpending) return(TRUE);
    if((message::AskForString(this,0,"Print request won't be processed; exit anyway [n]? ", NULL, answer, sizeof(answer)) == -1) || *answer != 'y') return(FALSE);
    return(TRUE);
#else /* USEFRAME */
    if(this->printpending){
        this->quitpending = TRUE;
        this->CursorChanged = TRUE;
        (this)->WantUpdate(this);
        return(FALSE);
    }
    return(TRUE);
#endif /* USEFRAME */
}

static class keymap *fkeymap;

preview::preview()
{
    struct proctable_Entry *tempProc;
    this->NWMFonts = 0;
    this->DoScaling = 0;
    this->DviFileIn = this->DviFileOut = NULL;
    this->DviFileLength = 0;
    this->CharactersOnThisPage = 0;
    this->SizeChanged = 0;
    this->WindowWidth = this->WindowHeight = 0;
    this->minWidth = this->minHeight = 3;
    this->PollCount = this->peekc = 0;
    this->NWMFonts = 0;
    this->CurrentCursor = '\0';
    this->PhysicalX = this->PhysicalY = 0;
    this->LogicalX = this->LogicalY = 0;
    this->Centre = this->CentreY = 0;
    this->InputResolution = 0;
    this->curfont = 2;
    this->cursize = 10 ;
    this->slant = 0;
    this->hasInputFocus = FALSE;
    this->DviFileComplete = FALSE;
    this->CreatedTemp = FALSE;
    this->FindFirstPage = TRUE;
    this->LowestNonBlankPageIndex = 0;
    this->CurrentPageTableIndex = 0;
    this->nowreading = 0;
    this->lastc = '\n';
    this->menulist = menulist::Create(this);
    fkeymap = new keymap;
    this->DisplayResolution = preview_DISPLAY_RESOLUTION;
    this->xPixelsPerPage =this->DisplayResolution * 17 / 2;
    this->yPixelsPerPage = this->DisplayResolution * 11;
    this->RedrawRequested = TRUE;
    this->menupage = -1;
    this->printpending = FALSE;
    this->quitpending = FALSE;
    this->cursor = cursor::Create(this);
    this->CursorChanged = TRUE;
/*    self->scroll = textview_NoScroll;
    self->scrollLine = 0;
    self->scrollDist = -1; */
    this->debug = FALSE;
    this->yoff = this->xoff = 0;
    InitPageMap(this);
     this->keystate = keystate::Create(this, fkeymap);
    this->ScaleProc = proctable::DefineProc("preview-Scale", (proctable_fptr)SetScale, &preview_ATKregistry_ , NULL, "Scale preview display");
    this->FullSizeProc = proctable::DefineProc("preview-FullSize", (proctable_fptr)SetFullSize, &preview_ATKregistry_ , NULL, "Full size preview display");
    this->SetPageProc = proctable::DefineProc("preview-SetPage", (proctable_fptr)SetPage, &preview_ATKregistry_ , NULL, "Set Page");
    tempProc = proctable::DefineProc("preview-NextPage", (proctable_fptr)NextPage, &preview_ATKregistry_ , NULL, "Next Page");
    /* Bind Next Page to space, and ^v keys */
    (fkeymap)->BindToKey( " ", tempProc, 0);
    (fkeymap)->BindToKey( "\026", tempProc, 0);
    (this->menulist)->AddToML( "Next Page~20", tempProc, 0, 0);
    tempProc = proctable::DefineProc("preview-PreviousPage", (proctable_fptr)LastPage, &preview_ATKregistry_ , NULL, "Last Page");
    /* Bind Previous Page to b, backspace, del, and M-v keys */
    (fkeymap)->BindToKey( "b", tempProc, 0);
    (fkeymap)->BindToKey( "\010", tempProc, 0);
    (fkeymap)->BindToKey( "\177", tempProc, 0);
    (fkeymap)->BindToKey( "\033v", tempProc, 0);
    (this->menulist)->AddToML( "Previous Page~21", tempProc, 0, 0);
    tempProc = proctable::DefineProc("Preview-Print", (proctable_fptr)PrintCmd, &preview_ATKregistry_ , NULL, "Prints preview document");
    (this->menulist)->AddToML( "Print~30", tempProc, 0, 0);
    tempProc = proctable::DefineProc("Preview-PrintPage", (proctable_fptr)PrintPageCmd, &preview_ATKregistry_ , NULL, "Prints current page of preview document");
    (this->menulist)->AddToML( "Print Page~31", tempProc, 0, 0);
 
    tempProc = proctable::DefineProc("Preview-DeleteWindow", (proctable_fptr)DeleteWindowCmd, &preview_ATKregistry_ , NULL, "Delete Preview Window");
    (this->menulist)->AddToML( "Delete Window~89", tempProc, 0, 0);

    THROWONFAILURE( TRUE);
}

static void updatemenu(class preview  *self)
{
	if(self->menupage >= 0) {
		(self->menulist)->AddToML( self->menubuf, self->SetPageProc, self->menupage, 0);
		if(self->hasInputFocus == TRUE)(self)->PostMenus( self->menulist);
		self->menupage = -1;
	}
}

static void DeclarePage(class preview  *self,preview_pagenumber     n, long   FilePosition)
{
   preview_pagetableindex i;
/*     int RedrawRequested = FALSE;
 */
   if (self->debug)
      fprintf(stderr, "DeclarePage %d at %ld\n", n, FilePosition);

   /* finish the completion work for the last page */
/*    if (self->LowestNonBlankPageIndex == self->NumberofPageTableEntries)
      RedrawRequested = TRUE;
   if (self->CurrentPageTableIndex == self->NumberofPageTableEntries)
      RedrawRequested = TRUE;
 */
   /* and then start work for the next page */
   i = self->NumberofPageTableEntries + 1;
   if (i >= preview_MAXPageTable)
	   {
		fprintf(stderr,"Too many pages (> %d)\n",preview_MAXPageTable);
		exit(1);
	   }
   self->PageTable[i].PageNumber = n;
   self->PageTable[i].FileOffset = FilePosition;
   if (self->debug)
      fprintf(stderr, "Put page %d at %d\n", n, i);

   if (n != 0 || FilePosition != 0)
   {
	
	preview_pagenumber m;
	updatemenu(self); /* add the last page to the menu */
	m = n/10*10;
	/* set up menu info for this page */
   	sprintf(self->menubuf, "Pages %d-%d~%d,Page %d~%d", (m == 0)? 1:m, m+9,(m / 10) +10, n,n - m + 10);
	self->menupage = i;
   }
   self->NumberofPageTableEntries = i;

 /*   if (RedrawRequested) preview_WantUpdate(self,self); */
}

static void InitPageMap(class preview  *self)
{
   preview_pagetableindex    i;

   for (i = 1; i < preview_MAXPageTable; i++)
      self->PageTable[i].PageNumber = -1;
   /* page zero must be at offset zero; this allows us to pick
	up the troff dvi header information before the first
	page, like the resolution of the input */
   self->PageTable[0].PageNumber = 0;
   self->PageTable[0].FileOffset = 0;
   self->NumberofPageTableEntries = 0;
}
static char testdvi[] = "x T ";
#define TESTDVILEN 4

void notdvifile(class preview  *self)
{
    fprintf(stderr,"The input file to preview is not a dvi file\n");
    fflush(stderr);
    self->printpending = FALSE;
    im::KeyboardExit();
}
static void MakePageMap(FILE  *filein,class preview  *self)
{
   register int   c;
   register long  FilePosition;
   register    FILE * f = self->DviFileOut;
   register int lastc = self->lastc;
    int lastread;
   FilePosition = self->DviFileLength;
   while ((c = getc(filein)) != EOF)
      {
	 putc(c, f);
          if(FilePosition < TESTDVILEN && c != testdvi[FilePosition]){
              notdvifile(self);
              return;
          }
	 if (c == 'p' && lastc == '\n')
	    {
	       preview_pagenumber   n = 0;
	       while ((c = getc(filein)) != EOF && isdigit(c))
		  {
		     n = n * 10 + c - '0';
		     FilePosition += 1;
		     putc(c, f);
		  }
	       putc(c, f);
	       FilePosition += 1;
	       fflush(f);
		lastread = self->nowreading;
		self->nowreading = n;
	       DeclarePage(self,n, FilePosition);
		if(self->CurrentPageTableIndex == lastread || self->FindFirstPage){
       			(self)->WantUpdate(self);
		}
	    }
	 lastc = c;
	 FilePosition += 1;
	 if (self->SizeChanged)
	    break;
	 if (FILE_HAS_IO(filein) <= 0 && !self->DviFileComplete)
	    break;
      }
   self->lastc = lastc;
   self->DviFileLength = FilePosition;

   if (c != EOF)
      return;

   /* have reached the end of standard input, and a DVI file has been
      created; set everything up as if it had been here all the time. 
   */
   fclose(self->DviFileOut);
   DeclarePage(self,0, 0);
   self->DviFileComplete = TRUE;
   if (self->debug)
      fprintf(stderr, "DviFile Completed\n");
   lastread = self->nowreading;
   self->nowreading = -1;
   updatemenu(self);
   SetTitle(self);
   if(self->NumberofPageTableEntries <= self->CurrentPageTableIndex){
       lastread = self->CurrentPageTableIndex = self->NumberofPageTableEntries - 1;
   }
   if(self->CurrentPageTableIndex == lastread || self->FindFirstPage) {
       			(self)->WantUpdate(self);
		}
    im::RemoveFileHandler(filein);
    fclose(filein);
    if(self->printpending) PrintCmd(self);
}

static void MakePageMapWithoutCopying(FILE  *filein,class preview  *self)
{
   register int   c;
   register int   lastc;
   register long  FilePosition;

   self->DviFileComplete = FALSE;

   FilePosition = 0;
   lastc = '\n';
   while ((c = getc(filein)) != EOF)
      {
          if(FilePosition < TESTDVILEN && c != testdvi[FilePosition]){
              notdvifile(self);
              return;
          }
	 if (c == 'p' && lastc == '\n')
	    {
	       /* get page number */
	       preview_pagenumber   n = 0;
	       while ((c = getc(filein)) != EOF && isdigit(c))
		  {
		     n = n * 10 + c - '0';
		     FilePosition += 1;
		  }
	       FilePosition += 1;
	       DeclarePage(self,n, FilePosition);
	    }
	 lastc = c;
	 FilePosition += 1;
      }
   self->DviFileLength = FilePosition;
   DeclarePage(self,0, 0);
   self->DviFileComplete = TRUE;
   self->nowreading = -1;

   if (self->FindFirstPage ){
       			(self)->WantUpdate(self);
		}
   updatemenu(self);
}

static void DrawBorder(class preview  *self)
{
   Boolean leftside = self->xoff > 0;
   Boolean topside = self->yoff > 0;
   preview_coordinate   right = self->xoff + self->xPixelsPerPage;
   preview_coordinate   bottom = self->yoff + self->yPixelsPerPage;
   Boolean rightside = right < self->WindowWidth;
   Boolean bottomside = bottom < self->WindowHeight;

   if (leftside)
      {
	 (self)->MoveTo(self->xoff,(topside) ? self->yoff : 0);
	 (self)->DrawLineTo(self->xoff,(bottomside) ? bottom : self->WindowHeight);
      }
   if (topside)
      {
	 (self)->MoveTo((leftside) ? self->xoff : 0, self->yoff);
	 (self)->DrawLineTo((rightside) ? right : self->WindowWidth,self->yoff);
      }
   if (rightside)
      {
	 (self)->MoveTo(right,(topside) ? self->yoff : 0);
	 (self)->DrawLineTo(right,(bottomside) ? bottom : self->WindowHeight);
      }
   if (bottomside)
      {
	 (self)->MoveTo((leftside) ? self->xoff : 0, bottom);
	 (self)->DrawLineTo((rightside) ? right : self->WindowWidth, bottom);
      }
}

static void SetCursor(class preview  *self)
{
    
    if(self->CursorChanged){
        self->CursorChanged = FALSE;
        if(self->quitpending){ 
/* Put up a wait sign it a print is pending and a quit is requested */
            class fontdesc *fd = fontdesc::Create("icon",fontdesc_Plain,12);
            (self->cursor)->SetGlyph(fd,'W');
            /* cursor_SetStandard(self->cursor,Cursor_DangerousBend); */
        }
        else if(self->FindFirstPage) 
/* Put up a clock until the first page is ready to display */
            (self->cursor)->SetStandard(Cursor_Wait);
	else {
	    if ((self->cursor)->IsPosted())
		(self)->RetractCursor( self->cursor);
	    return;
	}
	if(!(self->cursor)->IsPosted()){
	    struct rectangle tr;
	    (self)->GetVisualBounds(&tr);
	    (self)->PostCursor(&tr,self->cursor);
	}
    }
}

static void DoFindFirstPage(class preview  *self)
{
   preview_pagetableindex cp;

   /* if we are looking for the first real page and this one was
      blank, increment the page number until we find another candidate
      */

   cp = self->LowestNonBlankPageIndex;
   while (cp < self->NumberofPageTableEntries)
      {
	       if (self->debug) fprintf(stderr,"Try index %d\n",cp);
	       fseek(self->DviFileIn, self->PageTable[cp].FileOffset, 0);
	       (self)->DviToDisplay();
	       if (self->CharactersOnThisPage)
		  {
		     self->FindFirstPage = FALSE;
                      self->CursorChanged = TRUE;
		     if (self->debug) fprintf(stderr,"Found First %d\n",cp); 
		     self->LowestNonBlankPageIndex = cp;
		     if (self->CurrentPageTableIndex < self->LowestNonBlankPageIndex)
				 self->CurrentPageTableIndex = self->LowestNonBlankPageIndex;
   		    /* Change the title to the new page number */
		     SetTitle(self);
		     return;
		  }
	 cp += 1;
      }
   self->LowestNonBlankPageIndex = cp;
   if (self->DviFileComplete) self->FindFirstPage = FALSE;
}


#ifdef hp9000s300
void preview_sigAlrm()
{ }
#endif /* hp9000s300 */
  
static void insert(char  *src,char  *c)
{   /* inserts string src into the begining of string c , assumes enough space */
    char *p,*enddest;
    enddest = c + strlen(c);
    p = enddest + strlen(src);
    while(enddest >= c) *p-- = *enddest-- ;
    for(p = src; *p != '\0';p++)
	*c++ = *p;
}
static void normalize(char  *s)
{
    register char *c;
    for(c = s + strlen(s) - 1; c >= s; c--){
	if(!isalnum(*c)){
	    insert("\\",c);
	}
    }
}

static void DoPrintCmd(class preview  *self ,int  page)	
{
   int processid;
   Preview_Line PrintCommandFormat;
   Preview_Line PrintCommand;
   char *p;
   char BaseName[2048];
   strcpy(BaseName,self->DviBaseName);
   normalize(BaseName);
   p = print::GetPrintCmd(print_PRINTTROFF);
   strcpy(PrintCommandFormat,p);
   sprintf(PrintCommand,PrintCommandFormat,BaseName,BaseName,BaseName);
   if(page != -1){
       register FILE *fi,*fo;
       long n;
       register long c,diff,lastc;
       char buf[512];
       n = self->PageTable[page].FileOffset;
       if((fo = popen(PrintCommand,"w")) == NULL){
	   fprintf(stderr,"Can't execute %s",PrintCommand);
	   exit(1);
       }
       if((fi = fopen(self->DviFileName,"r")) == NULL){
	   fprintf(stderr,"Can't open %s",self->DviFileName);
	   exit(1);
       }
       diff = self->PageTable[1].FileOffset;
       while((c = getc(fi)) != EOF){
	   putc(c,fo);
	   if(--diff < 0) break;
       }
       if(page != 1){
	   fseek(fi, n , 0);
	   fgets(buf,512,fi);
       }
       lastc = '\n';
       while((c = getc(fi)) != EOF){
	   if(lastc == '\n' && c == 'p') break;
	   putc(c,fo);
	   lastc = c;
       }
       if(c != EOF)
	   fprintf(fo,"x trailer\nx stop\n");
       fflush(fo);
       pclose(fo);
       fclose(fi);
       return;
   }

   /* 
    Send the DviFile to be printed.
 */
 
   if (self->debug) fprintf(stderr, "Fork Print Process\n");
   
    if(!self->DviFileComplete){
        self->printpending = TRUE;
#ifdef USEFRAME
        message::DisplayString(self,0,"print pending");
#endif /* USEFRAME */
        return;
    }
#ifdef hp9000s300
  {
    int status;
    struct sigvec vecAlrm;
    struct itimerval timer;
    
    /** enable an interval timer so we can escape from wait(); **/
    vecAlrm.sv_handler = preview_sigAlrm;
    vecAlrm.sv_mask = 0;
    vecAlrm.sv_flags = 0;
    sigvector(SIGALRM, &vecAlrm, 0);
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 100000;
    setitimer(ITIMER_REAL, &timer, 0);
    
    while (wait(&status) > 0) ;

    /** disable the timer **/
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, 0);
  }
#else /* hp9000s300 */
     while (wait3(0, 1, 0) > 0);
#endif /* hp9000s300 */
   self->printpending = FALSE;
#ifdef USEFRAME
   message::DisplayString(self,0,"Initiating print process");
#endif /* USEFRAME */
   if (osi_vfork() != 0) {  /* if parent process */
       if(self->quitpending) {
           wait(0); /* insures that the child gets to open the tmp files before the parent deletes it */
           im::KeyboardExit();
       }
       return;
   }

   /* To avoid dieing when the parent quits; reset process
	group id; set it to processid to be unique */
   processid = getpid();
   NEWPGRP();
   /* form the print command */
   close(0);
   open(self->DviFileName, O_RDONLY, 0);

   /* now execute the printcommand */
   execlp("/bin/sh", "sh", "-c", PrintCommand, 0);
   exit(0);

}


/* Scroll stuff. */
  


class view *preview::GetApplicationLayer()
    {
   
     return (class view *) scroll::Create(this, scroll_LEFT | scroll_BOTTOM);
    
}

void preview::DeleteApplicationLayer(class view  *scrollbar)
        {

    (scrollbar)->Destroy();
}
#define RANGE(T,A) ((T->beg > A)? T->beg : ((T->end < A)? T->end : A))
static void vgetinfo(class preview  *self, struct range  *total , struct range  *seen , struct range  *dot)
        {

    total->beg = 0;
    total->end =self->yPixelsPerPage;
    seen->beg = RANGE(total,-self->yoff);
    seen->end = RANGE(total,self->WindowHeight - self->yoff);
    if(seen->end < 0) seen->end = 0;
    dot->beg = 0;
    dot->end = -1;
}
static void hgetinfo(class preview  *self, struct range  *total , struct range  *seen , struct range  *dot)
        {

    total->beg = 0;
    total->end =self->xPixelsPerPage;
    seen->beg = RANGE(total,-self->xoff);
    seen->end = RANGE(total,self->WindowWidth - self->xoff);
    if(seen->end < 0) seen->end = 0;
    dot->beg = 0;
    dot->end = -1;
}
static long hwhatisat(class preview  *self, long  numerator , long  denominator)
        {
return numerator - self->xoff;

}
static long vwhatisat(class preview  *self, long  numerator , long  denominator)
        {
return numerator - self->yoff;

}static void vsetframe(class preview  *self, long  position  , long  numerator , long  denominator)
        {
    self->yoff =   numerator-position ;
    (self)->WantUpdate(self);
}
static void hsetframe(class preview  *self, long  position  , long  numerator , long  denominator)
        {
    self->xoff =  numerator -position;
    (self)->WantUpdate(self);
}
static const struct scrollfns vscrollInterface = {(scroll_getinfofptr)vgetinfo, (scroll_setframefptr)vsetframe, NULL, (scroll_whatfptr)vwhatisat};
static const struct scrollfns hscrollInterface = {(scroll_getinfofptr)hgetinfo, (scroll_setframefptr)hsetframe, NULL, (scroll_whatfptr)hwhatisat};

const void *preview::GetInterface(const char  *interfaceName)
        {

    if (strcmp(interfaceName, "scroll,vertical") == 0)
        return &vscrollInterface;
    if (strcmp(interfaceName, "scroll,horizontal") == 0)
        return &hscrollInterface;
    return NULL;
}

class preview *preview::Create(FILE  *f,const char  *fname,const char  *fbase,boolean  compleated,boolean  scale)
{
class preview *self = new preview;
self->DviFileComplete = compleated;
if(fbase)strcpy(self->DviBaseName,fbase);
else *(self->DviBaseName) = '\0';
strcpy(self->DviFileName ,fname);
self->DoScaling = scale;
if(self->DoScaling){
	(self->menulist)->AddToML("Full Size~10",self->FullSizeProc,0,0);
    }
else {
	(self->menulist)->AddToML("Scale~10",self->ScaleProc,0,0);
    }
if(compleated == FALSE){
	int DviFD;
	 if (self->DviBaseName[0] == '\0')
	     sprintf(self->DviBaseName, "stdin_%d", getpid());
	 sprintf(self->DviFileName, "/tmp/%s.dvi", self->DviBaseName);
	 DviFD = open(self->DviFileName,
	       O_WRONLY | O_CREAT | O_EXCL,
	       0600);
	 if (DviFD < 0)
	    {
                int ii;
                for(ii = 0 ; ii < 30; ii++){
                    sprintf(self->DviFileName, "/tmp/%s.%d.dvi", self->DviBaseName,ii);
                    DviFD = open(self->DviFileName,O_WRONLY | O_CREAT | O_EXCL,0600);
                    if(DviFD >= 0)break;
                }
                if(DviFD < 0){
                    fprintf(stderr, "Can't create tmp file %s\n", self->DviFileName);
                    return(NULL);
                }
	    }
	 close(DviFD);
	 self->DviFileOut = fopen(self->DviFileName, "w");
	 if (self->DviFileOut == (FILE *) NULL)
	    {
	       fprintf(stderr, "Can't write dvi file %s\n", self->DviFileName);
	       return(NULL);
	    }
	 self->CreatedTemp = TRUE;
    }
   self->DviFileIn = fopen(self->DviFileName, "r");
   if (self->DviFileIn == (FILE *) NULL)
      {
	 fprintf(stderr, "Can't read dvi file %s\n", self->DviFileName);
	 return(NULL);
      }
   SetTitle(self);
if(compleated) MakePageMapWithoutCopying(f,self);
else {
	im::AddFileHandler(f,(im_filefptr)MakePageMap,(char *)self,6);
    }
    return(self);
}
class view *preview::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
                    {

 	if (! this->hasInputFocus)
	    (this)->WantInputFocus( this);
	return(this);
}
preview::~preview()
{
   if (this->CreatedTemp)
      unlink(this->DviFileName);
}
