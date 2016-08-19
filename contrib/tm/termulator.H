#ifndef _termulator_H_
#define _termulator_H_
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


 

/*
 * terminal emulator for text
 */

/* sunOS is the only place I know of that the "correct" way actually works */
#ifndef sunV3
#define INCORRECTSIGNALS
#endif /* sunV3 */

#define HACKEDNOECHO

struct cmd {
    struct cmd *next,*prev;
    char *string;
};

enum termulator_Mode {termulator_ModeCooked, termulator_ModeCbreak, termulator_ModeRaw};

#define INTRCHAR '\003'
#define QUITCHAR '\034'
#define STOPCHAR '\032'

#include <text.H>
class termulator;
typedef char *(*termulator_escfptr)(class termulator *self, char *buf, long len);
typedef char *(*termulator_statefptr)(class termulator *self, char *buf, long len);
class termulator: public  text {
public:
	virtual ATKregistryEntry *ATKregistry();

    
         void Clear();
         char * ViewName()  ;
	 void AlwaysInsertCharacters(long pos,const char *buf,long len);
	 long ReadSubString(long pos,FILE *file,boolean quoteCharacters)  ;
	 long Read(FILE *file, long id)  ;
	 long Write(FILE *file, long writeID, int level)  ;
    
        virtual int StartProcess(const char **args)  ;
	virtual char * GrabPrevCmd(char *str)  ;
	virtual char * GrabNextCmd(char *str)  ;
	virtual boolean EnterCmd(char *string)  ;
	virtual void RewindCmds();
	virtual void ProcessInput(char *text, long len);
	virtual void SendInput(char *text, long len);
	virtual void Submit();
	virtual void Signal(int signo);
	virtual void SetScreenSize(int width,int height);
	virtual void SetDispSize(int width,int height);
	virtual void ProcessOutput(char *str, long len);
	virtual char * GetTermcap()  ;
	virtual char * GetTerm()  ;
	virtual void Untermulate();
	/* the following are intended to be called from subclasses */
	virtual void GotoXY(int x,int y);
	virtual void WriteChars(char *chars, int numChars);
	virtual void DeleteChars(int numChars);
	virtual void ClearChars(int x,int numChars);
	virtual void InsertLines(int numLines);
	virtual void DeleteLines(int numLines);
	virtual void ClearLines(int y,int numLines);
	virtual void Scroll(int num);
	virtual void Newline(int num);
	virtual void Backspace(int num);
	virtual void CR();
	virtual void Tab();
	virtual void EOT();
    
	
	termulator();
        
	virtual ~termulator();
    
	inline int   GetX() { return (this->x); }
	inline int   GetY() { return (this->y); }
	inline int   GetWidth() { return (this->width); }
        inline int   GetHeight() { return (this->height); }
	inline void SetInsert(boolean b) { (this->insert=(b)); }
	inline boolean   GetInsert() { return (this->insert); }
	inline void SetStandout(boolean b) { (this->standout=(b)); }
        inline boolean   GetStandout() { return (this->standout); }
	inline void SetEscape(char c, termulator_escfptr action) { (this->escapes[c]=(action)); }
        inline termulator_escfptr GetEscape(char c) { return (this->escapes[c]); }
	inline void SetParseState(termulator_statefptr rtn) { (this->state=(rtn)); }
        inline termulator_statefptr GetParseState() { return (this->state); }
	inline char   *GetParseBuf(){ return (this->parseBuf); }
	inline int   GetParseBufLen() { return (this->parseLen); }
	inline void SetParseBufLen(int l) { (this->parseLen=(l)); }
        inline unsigned long GetParseBufMax() { return (sizeof(this->parseBuf)); }
    
        int screen,cursor,lastSubmit;
	int width,height;
	int dispWidth,dispHeight;
#ifdef HACKEDNOECHO
	char inpBuf[300];
	int inpLen;
#endif /* HACKEDNOECHO */
	class tmview *inpView; /* manipulated by views */
	char parseBuf[100];
	int parseLen;
	char *cwd;
        int x,y;
	int pty;
	FILE *ptyFile;
	int pid;
	struct cmd *lastCmd, *curCmd;
	char **args;
	boolean insert,standout;
	char *errstr;
	int errno;
	class style *cmdStyle,*noechoStyle,*soStyle,*screenStyle;
	class environment *cmdEnv,*screenEnv;
	int mode;
	boolean remote;
	long maxsize;
	long extraroom;
	termulator_escfptr escapes[256];
	termulator_statefptr state;
	boolean currentlyReadingTemplateKluge;
};

#endif /* _termulator_H_ */
