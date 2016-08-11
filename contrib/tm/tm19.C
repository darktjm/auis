/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/tm/RCS/tm19.C,v 1.2 1993/05/31 20:35:18 rr2b Stab74 $";
#endif


 

/*
 * subclass of termulator implementing h19 escape sequences
 */

#include <andrewos.h>
#include <stdio.h>


#include <tm19.H>

/* this is slightly more complicated than necessary to handle an entire
 * escape sequence in one pass, but faster...
 */

ATKdefineRegistry(tm19, termulator, NULL);
#ifndef NORCSID
#endif
static char *escapeChar(class tm19  *self,char  *buf,int  len);


static char *escapeChar(class tm19  *self,char  *buf,int  len)
{
    boolean cont=TRUE;
    char *pbuf=(self)->GetParseBuf();
    int num,plen=(self)->GetParseBufLen();

#define Xpos (self)->GetX()
#define Ypos (self)->GetY()
#define Width (self)->GetWidth()
#define Height (self)->GetHeight()

    do{ /* process one or more escape sequences */

	/* fold some simple sequences if possible */
	if(plen==0){
	    num=1;
	    while(len>3 && buf[2]=='\033' && buf[3]==buf[1]){
		buf+=2;
		len-=2;
		num++;
	    }
	}else
	    num=self->foldCount;
/*
	if(num>1)
	    fprintf(stderr,"tm19: ESC-%c occured %d times.\n",buf[1],num);
*/
	do{ /* process one escape sequence */
	    cont=FALSE; /* punt unless told otherwise */
	    pbuf[plen++]= *buf++;
	    len--;
	    switch(plen){
		case 1:
		case 3:
		    cont=TRUE;
		    break;
		case 2:
		    switch(pbuf[1]){
			case 'H': /* home */
			    (self)->GotoXY(0,0);
			    break;
			case 'C': /* cursor forward */
			    (self)->GotoXY(Xpos+num,Ypos);
			    break;
			case 'D': /* cursor backward */
			    (self)->Backspace(num);
			    break;
			case 'B': /* cursor down */
			    (self)->GotoXY(Xpos,Ypos+num);
			    break;
			case 'A': /* cursor up */
			    (self)->GotoXY(Xpos,Ypos-num);
			    break;
			case 'I': /* reverse index */
			    num-=Ypos;
			    if(num<=0)
				(self)->GotoXY(Xpos,-num);
			    else{
				(self)->GotoXY(0,0);
				(self)->InsertLines(num);
			    }
			    break;
			case 'n': /* report cursor position */
			    {
			    char scratchBuf[80];
			    sprintf(scratchBuf,"\033Y%c%c",Ypos+' ',Xpos+' ');
			    (self)->ProcessInput(scratchBuf,4);
			    }
			    break;
			case 'j': /* save cursor position */
			    self->savedX=Xpos;
			    self->savedY=Ypos;
			    break;
			case 'k': /* restore cursor position */
			    (self)->GotoXY(self->savedX,self->savedY);
			    break;
			case 'Y': /* direct cursor positioning */
			    cont=TRUE;
			    break;
			case 'E': /* clear screen */
			    (self)->ClearLines(0,Height);
			    (self)->GotoXY(0,0);
			    break;
			case 'b': /* erase to beginning of screen */
			    (self)->ClearLines(0,Ypos-1);
			    (self)->ClearChars(0,Xpos+1);
			    break;
			case 'J': /* erase to end of screen */
			    (self)->ClearChars(Xpos,Width-Xpos);
			    (self)->ClearLines(Ypos,Height-Ypos);
			    break;
			case 'l': /* clear current line */
			    (self)->ClearLines(Ypos,1);
			    break;
			case 'K': /* clear to end of line */
			    (self)->ClearChars(Xpos,Width-Xpos);
			    break;
			case 'o': /* clear to beginning of line */
			    (self)->ClearChars(0,Xpos+1);
			    break;
			case 'L': /* insert line */
			    (self)->InsertLines(num);
			    break;
			case 'M': /* delete line */
			    (self)->DeleteLines(num);
			    break;
			case '@': /* enter insert mode */
			    (self)->SetInsert(TRUE);
			    break;
			case 'O': /* exit insert mode */
			    (self)->SetInsert(FALSE);
			    break;
			case 'N': /* delete character */
			    (self)->DeleteChars(num);
			    break;
			case 'p': /* standout mode */
			    (self)->SetStandout(TRUE);
			    break;
			case 'q': /* standout mode off */
			    (self)->SetStandout(FALSE);
			    break;
		    }
		    break;
		case 4:
		    /* this should happen only with the cm sequence */
		    (self)->GotoXY(pbuf[3]-' ',pbuf[2]-' ');
	    }
	}while(cont && len>0);

	if(len==0)
	    break;

	plen=0;
    }while(*buf=='\033');

    if(cont){
	self->foldCount=num;
	(self)->SetParseBufLen(plen);
	(self)->SetParseState((termulator_escfptr)escapeChar);
    }else{
	(self)->SetParseBufLen(0);
	(self)->SetParseState(NULL);
    }
	
    return buf;
}

tm19::tm19()
{
    (this)->SetEscape('\033',(termulator_escfptr)escapeChar);
    this->savedX=0;
    this->savedY=0;
    THROWONFAILURE( TRUE);
}

static char tcbuf[2000]; /* stupid HC can't handle statics inside functions */

char *tm19::GetTermcap()
{
    sprintf(tcbuf,
	    "tm19|termulator h19 emulation:\
cr=^M:nl=^J:al=5*\\EL:le=^H:bs:cd=\\EJ:ce=\\EK:cl=\\EE:\
cm=\\EY%%+ %%+ :co#%d:dc=\\EN:dl=5*\\EM:do=\\EB:ei=\\EO:\
ho=\\EH:im=\\E@:li#%d:mi:nd=\\EC:ms:ta=^I:pt:sr=\\EI:up=\\EA:\
so=\\Ep:se=\\Eq:",(this)->GetWidth(),(this)->GetHeight());
    return tcbuf;
}

char *tm19::GetTerm()
{
    return "tm19";
}
