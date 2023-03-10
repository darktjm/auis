#ifndef _frame_H_
#define _frame_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libframe Object frame
 *  A statically loadable ATK class file.  Contains frame, frameview,
 *  framecmds, framemessage, helptextview.  Note that while the object
 *  name is frame, which matches the primary class name, none of the others
 *  are linked, so this is really for static loading only.
 * @{ */
#define frame_VERSION 1

#define frame_MUSTANSWER -1

#define	frame_DefaultMenus	(1<<13)
#define	frame_BufferMenus	(1<<12)


#include <rect.h>

#include <message.H>

#include <lpair.H>

    	typedef boolean (*frame_effptr)(class frame *f, long d);

class frame: public  lpair {
public:
	TRACED_CLASS(frame);
        virtual void SetView(class view *view);
        virtual void SetBuffer(class buffer *buffer, boolean setInputFocus);
        virtual void SetCommandEnable(boolean enable);
        virtual class buffer * GetHelpBuffer()  ;
        virtual class buffer * SetHelpBuffer(class buffer *buf, boolean setInputFocus)  ;
        virtual void SetTitle(const char *title);
	virtual void Advice(enum message_Preference pp);
        virtual int DisplayString(int priority, const char *string)  ;
        virtual int AskForString(int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize)  ;
        virtual int AskForPasswd(int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize)  ;
        virtual int AskForStringCompleted(int priority, const char *prompt, const char *defaultString, char *buffer, int bufferSize, class keystate *keystate,
					  message_completionfptr completionProc, message_helpfptr helpProc, long functionData, int flags)  ;
	virtual int MultipleChoiceQuestion(int priority, const char *prompt, long defaultChoice, long *result, const char * const *choices, const char *abbrevKeys)  ;
	virtual void RemoveHelp();
	virtual void SetReturnFocus(class view *req);
	virtual void ReturnFocus();
	

        inline class view * GetView() { return ((this)->targetView); }
        inline class view * GetChildView() { return ((this)->childView); }
        inline class buffer * GetBuffer() { return ((this)->buffer); }
        inline boolean GetCommandEnable() { return ((this)->commandEnable); }
        inline const char * GetTitle() { return ((this)->title); }
        inline boolean GetQuitWindowFlag() { return ((this)->QuitWindowOnly); }
        inline void SetQuitWindowFlag(boolean VAL) { (((this)->QuitWindowOnly) = VAL); }

    
        
	frame();
        
	static class frame * Enumerate(frame_effptr function, long functionData)  ;
	static class frame * FindFrameForBuffer(class buffer *b)  ;
	static class frame * GetFrameInWindowForBuffer(class buffer *b)  ;
	static class frame * Create(class buffer *buffer)  ;

	static boolean InitializeClass();

	/* Make it hard for people to access our lpair functions. */
	void Update();
	void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
	const char * WantInformation(const char *key)  ;
	ATK  * WantHandler(const char *handlerName)  ;
	void PostKeyState(class keystate *keystate);
	void PostMenus(class menulist *menulist);
	void ObservedChanged(class observable *changed, long value);
	class view * Hit(enum view::MouseAction action, long x, long y, long nclicks)  ;
	void WantUpdate(class view *v);
	void WantNewSize(class view *requestor);
	void LinkTree(class view *parent);

	boolean RecSearch(class search *pat, boolean toplevel);
	boolean RecSrchResume(class search *pat);
	boolean RecSrchReplace(class dataobject *text, long pos, long len);

	virtual int VisitNamedFile(const char  *filename, 
			boolean  newWindow, boolean  rawMode);

	class frame *next;
	class buffer *buffer;
        class view *targetView;
        class view *childView;
        char *title;
        class text *messageText;
        class frameview *messageView;
        class framemessage *messageLine;
        long lineHeight; /* Height in pixels of the nominal one line message line. */
        class frameview *dialogView;
        class framemessage *dialogLine;
        class keystate *keystate;
        class menulist *menulist;
        char helpBuffer[16]; /* 16 is greater than sizeof("Help-Buffer-99") */
        boolean commandEnable;
	class buffer *DialogBuffer;
	class view *DialogTargetView;
	class view *DialogBufferView;

	class cursor *octcursor,*arrowcursor;
	class fontdesc *myfontdesc;
	class menulist *mymenus;
	class keystate *mykeystate;
	enum message_Preference PositionalPreference;
	const char **MultipleAnswers;
	struct rectangle AnswerBox;
	struct rectangle *HeightsOfAnswer;
	struct rectangle mesrec,bufferrec;
	struct pendingupdates *uplist;
	int IsAsking,
	     IsBlocking,
	     WildestAnswer, /* selected answer */
	     DefaultWildestAnswer, /* default answer */
	     StackPos, 
	     hasDialogMessage,
	     NumAnswerChoices,
	     AwaitingFocus,
	     UpdateRequested,
	     buttonmaxwid;
	class dataobject *object;
	boolean dataModified;
	boolean QuitWindowOnly;
	boolean drawn;
	boolean mono;
	int PotentialChoice;
	boolean UsingDialog, UseBuiltinDialogs;
	class dialogv *dv;
	class menulist *defaultmenulist;
	class buffer *realBuffer;
	class view *realView;
	boolean revertToReal;
	int height;
	class view *returnFocus;
	class buffer *deleteTarget;
};
/** @} */
#endif /* _frame_H_ */
