#ifndef _im_H_
#define _im_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#define XIM_GETLOC_FIXED 1
/* im.H
 * Class header file for the interface manager.
 *
 *
 */

#include <stdio.h>
#include <view.H>

#define im_PROGRAMMMERVERSION 2

#define im_AllUp 0L
#define im_LeftDown 1L
#define im_RightDown 2L

/* these bits are used in the default configuration function for transients and override windows the top 16 bits in the rock provided to that function are reserved for future use.  the lower bits may be used in any way a particular im might desire, particular applications should use the custom rock to hold any data they want to give to their config function. */
#define im_InMiddle (1<<31)
#define im_Centered (1<<30)
#define im_AtTop (1<<29)
#define im_AtBottom (1<<28)

enum im_EventType  {
    im_NoEvent,
    im_MouseEvent,
    im_KeyboardEvent,
    im_ProcEvent,
    im_MenuEvent,
    im_MacroEvent,
    im_SuspendEvent,
    im_AnswerEvent,
    im_ResumeEvent
};

union actionu {
    long key;
    struct actionu1 {
	struct proctable_Entry *procTableEntry;
	ATK  *object;
	long rock;
	struct action *keys, *keypos;
    } proc;
    struct actionu2 {
	enum view_MouseAction action;
	long x;
	long y;
	long newButtonState;
    } mouse;
    struct actionu3 {
	struct action *macro;
	struct action *nextaction;
	long remainingrepetitions;
    } macro;
    char *answer;
};

struct action {
	enum im_EventType type;
	struct action *next;
	class im *im;
	union actionu v;
};


struct im_ArgState {
    long argument;	/* the current argument */
    long tmpArgument;
    boolean argProvided;	/* true if argument is valid */
    boolean argNext;	/* if argProvided should be set on next command */
    boolean argDigit;	/* true if a digit has been seen on this argument */
    boolean argPending; /* true if we are currently trying to process a ^U as a command */
    boolean processCmd;
    long cmdpos;
    char argcmd[10];
};

#include <event.H>

enum im_keyecho_t {im_KeyEchoOff, im_KeyEchoPending, im_KeyEchoDisplayed};

	typedef void (*im_keyfptr)(class im  *self, long  key);
	typedef void (*im_mousefptr)(class im  *self, enum view_MouseAction  act, long  x, long  y, long  newButtonState);
	typedef void (*im_menufptr)(class im  *self, struct proctable_Entry  *procTableEntry, ATK   *object, long  rock);
	typedef void (*im_signalfptr)(int i, char *d);
	typedef void (*im_filefptr)(FILE *fp, char *d);

	typedef void (*im_zombiefptr)(int pid, long data, WAIT_STATUS_TYPE *zs);
	typedef void (*im_interactionfptr)(long data);
	typedef void (*im_deletefptr)(im *, long data);
	typedef void (*im_configurefptr)(class im  *self, long  rock , long  customrock, class im  *parent, int  *x , int  *y, unsigned int  *w , unsigned int  *h);

class im : public  view  {
public:
	virtual ATKregistryEntry *ATKregistry();
     void WantUpdate(class view *requestor);
     void WantInputFocus(class view *requestor);
     void WantColormap(class view *requestor, class colormap **cmap);
     void ReceiveColormap(class colormap *cmap);
     void WantNewSize(class view *requestor);
     ATK  * WantHandler(const char *handlerName)  ;
     const char * WantInformation(const char *key)  ;
     void PostKeyState(class keystate *keystate);
     void PostMenus(class menulist *menulist);
     void PostCursor(struct rectangle *rec,class cursor *cursor) ;
     void RetractCursor(class cursor *cursor) ;
     void RetractViewCursors(class view *requestor);
     void PostDefaultHandler(const char *handlerName, ATK  *handler);
     short GetResource( class atomlist * name, class atomlist * className,
		class atom * type, long * data )  ;
     void PostResource( class atomlist * path, class atom * type,
		 long data );
     void GetManyParameters( struct resourceList * resources,
		      class atomlist * name,
		      class atomlist * className );		      
     void UnlinkNotification(class view *unlinkedTree);
     class view * Hit (enum view_MouseAction action, long x, long y, long clicks)  ;
     void ObservedChanged(class observable *changed, long value);
     boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
     boolean RecSrchResume(struct SearchPattern *pat);
     boolean RecSrchReplace(class dataobject *text, long pos, long len);
     void RecSrchExpose(const struct rectangle &logical, struct rectangle &hit); 
  

    virtual void SetView(class view *toplevel);
    virtual unsigned char * WhichWS()  ;  /* "X" or "wm" */
    virtual int GetCharacter()  ;
    virtual boolean WasMeta()  ;	/* Was the last ESC returned by
				 * GetCharacter a real escape,
				 * or part of a meta sequence? */
    virtual class im * DoKey(long key)  ; /* Simulates typing of key on window. */
    virtual void DoKeySequence(char *keys);  /* simulates typing string */
    virtual class im * HandleMenu(struct proctable_Entry *procTableEntry, ATK  *object, long rock)  ;
    virtual class im * HandleMouse(enum view_MouseAction action, long x, long y, long newButtonState)  ;
    virtual void DoMacro();
    virtual void DoMenu(char *itemname);

    /* These routines are used to handle argument passing with keystrokes */
    virtual struct im_ArgState * GetArgState()  ;
    virtual void ClearArg();
    virtual boolean ArgProvided()  ;
    virtual long Argument()  ;
    virtual void ProvideArg(long arg);
    /* These routine manipulate late the global last command value. */
    virtual long GetLastCmd()  ;	/* get it */
    virtual void SetLastCmd(long cmd);	/* set it */
    virtual long BumpArg(long digit)  ; /* shift one place and add in new digit */
    virtual void DisplayArg();	/* display it */
    virtual void SetWindowCursor(class cursor *cursor) ;
    virtual void ClearCursors(class cursor * C);
    virtual void ClearCursorList();
    virtual void UpdateCursors();
    virtual void SetTitle(const char *titleString);
    virtual char * GetTitle()  ; /* ugh... xim needs write perms */
    virtual FILE * FromCutBuffer()  ;
    virtual FILE * ToCutBuffer()  ;
    virtual void CloseFromCutBuffer(FILE *readFile);
    virtual void CloseToCutBuffer(FILE *writeFile);
    virtual void RotateCutBuffers(long count);
    virtual void AppendToCutBuffer(FILE *writeFile);
    virtual struct im_InteractionEvent * SetInteractionEvent(im_interactionfptr function, long functionData, long time)  ;
    virtual void CancelInteractionEvent(struct im_InteractionEvent *event);
    virtual void DispatchPendingInteractionEvents(); /* This is an internal only method. */
    /* maybe this should be done by HandleRedraw */
    virtual void RedrawWindow();

    /* These are window manager interface calls */
    virtual void SetWMFocus();
    virtual void ExposeWindow();
    virtual void HideWindow();	/* iconified */
    virtual void VanishWindow();	/* totally invisible */
    virtual void HandleRedraw();
    virtual boolean CreateWindow(char *host)  ;
#define im_TRANSIENTMENUS 1
#define im_OVERRIDEREDIRECT 2
    virtual boolean CreateTransientWindow(class im *other, int flags=im_TRANSIENTMENUS, im_configurefptr cfp=NULL, long crock=0)  ;
    virtual boolean CreateOverrideWindow(class im *other, im_configurefptr cfp=NULL, long crock=0);
    virtual boolean SupportsTransient()  ;
    virtual boolean SupportsOverride()  ;
    virtual boolean SupportsOffscreen()  ;
    virtual boolean CreateOffscreenWindow(class im *other, int width, int height)  ;
    virtual class colormap * CreateColormap()  ;
    virtual class color * CreateColor(char *name, unsigned int r, unsigned int g, unsigned int b)  ;
    virtual void SetBorderWidth(int n);
    virtual struct rectangle * GetLoc(class view *v, struct rectangle *r)  ;
    virtual void NormalConfiguration(long rock, long customrock, class im *parent, int *x, int *y, unsigned int *w, unsigned int *h);
    virtual char * GetKeyBinding(ATK  *object, struct proctable_Entry *pe, long rock)  ;   
    virtual boolean RequestSelectionOwnership(class view *requestor)  ;
    virtual void GiveUpSelectionOwnership(class view *requestor);
    /* A window callback is a procedure with the following prototype:
     *    void delwin(struct im *im, long rock)
     * It is called when the window manager wants to close the window.
     * It can ignore the request if it wishes.
     */
    virtual im_deletefptr GetDeleteWindowCallback()  ;
    virtual long GetDeleteWindowCallbackRock()  ;
    virtual void SetDeleteWindowCallback(im_deletefptr cb, long rock);
    virtual void CallDeleteWindowCallback();
    /* Drag/Drop support.  These calls return the host/files that
     * have been dropped on a view (when a hit mouse action is a
     * file drop.  They may only be called once.
     * 
     * The vector returned by GetDroppedFiles looks like this:
     *     retval[0] == "hostname"
     *     retval[1] == "pathname 1"
     *     retval[2] == "pathname 2"
     *            :
     *     retval[n] == "hostname"
     *     retval[n+1] == NULL
     *
     * THE CALLER MUST FREE EACH STRING AND RETVAL!!
     *
     * The DropFiles method is used for dropping a list of files
     * into another window.  The 'pathnames' list is the same as that
     * returned by GetDroppedFiles (see above), except that the
     * 0th element may be NULL (indicating localhost).  The cursor
     * is the cursor to use while the pointer is grabbed.
     *
     * The DropFile method is a convenience method that drops only
     * one file from the localhost.
     */
    virtual char ** GetDroppedFiles()  ; /* NULL terminated array of strings */
    virtual void DropFile(char *pathname, class cursor *cursor);
    virtual void DropFiles(char **pathnames, class cursor *cursor);

    virtual void InstallColormap(class colormap *cmap);
    virtual boolean ResizeWindow( int w, int h)  ;
    virtual boolean MoveWindow( int x, int y)  ;
    virtual FILE *OnlyFromCutBuffer();
    virtual FILE *OnlyFromSelection();
    virtual void Beep(int volume);	/* volume is 0..100% of user's bell */

    inline class view  * GetInputFocus() { return ((this)->inputFocus); }
/* these provide a way to modify the configure function's operation (the configure function controls size and placement) for a transient or override */
    inline im_configurefptr   &ConfigureFunction() { return ((this)->configfunc); }
    inline long   &ConfigureRock() { return ((this)->configrock); }
    inline long   &ConfigureCustomRock() { return ((this)->configcustomrock); }
    inline void SetAutoMap(boolean val) { ((this)->automap=(val)); }
    inline boolean   GetAutoMap() { return ((this)->automap); }
    inline void SetStartIconic(boolean val) { ((this)->starticonic=(val)); }
    inline boolean   GetStartIconic() { return ((this)->starticonic); }

    inline class colormap  * InstalledColormap() { return ((this)->installedColormap); }


    static long AllocLastCmd()  ;	/* allocate a value for it */
    
	static boolean InitializeClass();

    
	im();
    
	virtual ~im();
    static class im * Create(char *host)  ;
    static class im * CreateTransient(class im *other, int flasgs=im_TRANSIENTMENUS, im_configurefptr cfp=NULL, long crock=0)  ;
    static class im * CreateOverride(class im *other, im_configurefptr cfp=NULL, long crock=0)  ;
    static boolean AddFileHandler(FILE *file, im_filefptr proc, char *procdata, long priority)  ;
    static void RemoveFileHandler(FILE *file);
    static boolean AddCanOutHandler(FILE *file, im_filefptr proc, char *procdata, long priority)  ;
    static void RemoveCanOutHandler(FILE *file);
    static boolean Interact(boolean mayBlock)  ;
    static void SetCleanUpZombies(boolean value);
    static void AddZombieHandler(int pid, im_zombiefptr function, long functionData);
    static void RemoveZombieHandler(int pid);
    static void SignalHandler(long signalNumber, im_signalfptr proc, char *procdata);
    static long ChangeDirectory(const char *dirName)  ;
    static char * GetDirectory(char *outputString)  ;
    static void KeyboardExit();
    static long KeyboardLevel()  ;
    static void KeyboardProcessor();
    static class event * EnqueueEvent(event_fptr proc, const void *procdata, long timeIncrement)  ;
    static boolean IsPlaying()  ;
    static boolean CheckForInterrupt()  ;  /* scans ahead for control-G */
    static void ForceUpdate();
    static void SetProcessCursor(class cursor *cursor) ;
    static class cursor * GetProcessCursor()  ;
    static long GetWriteID()  ;
    static void SetProgramName(const char *nameString);
    static char * GetProgramName()  ;
    static void SetGlobalInit(class init *init);
    static class init * GetGlobalInit()  ;
    static void SetPreferedDimensions(long top, long left, long width, long height);
    static void GetPreferedDimensions(long *top, long *left, long *width, long *height);
    static void SetLastUsed(class im *used);
    static class im * GetLastUsed()  ;

    /* Private declarations for use by other interaction managers */
    static class windowsystem * GetWindowSystem()  ;
    static void DeliverSignals();
    static class cursor * GetCursor()  ;
    static class fontdesc * GetFontdesc()  ;
    static class graphic * GetGraphic()  ;
    static struct im_GlobalDataType * GetGlobalData()  ;
    static void IOMGRCancel(char * imPid);
    static boolean IOMGRSelect(long maxnum,long *rmask,long *wmask,long *emask,struct timeval *retTime)  ;
    static void IOMGRSoftSig(procedure aproc, char *arock);
    static long LWPCurrentProcess(char * curProcessID)  ;
    static FILE * vfileopen(const char * mode, struct expandstring *buffer)  ;
    static void vfileclose(FILE *f, struct expandstring *buffer);
    static void vfilecleanup();
    static void plumber(FILE * reportFile);

    static void RedrawChangedWindows();
    static void SetDefaultServerHost(const char *name);
    static void SetGeometrySpec(const char *value);
    static void CancelMacro();
    static long DefaultConfigureRock(long pos)  ;
    static im_configurefptr DefaultConfigureFunction(im_configurefptr func)  ;
    static long DefaultConfigureCustomRock(long rock)  ;
    static class im * CreateOffscreen(class im *other, int width, int height)  ;
    static struct action * GetMacro()  ;
    static void SetMacro(struct action *NewRecord);
    static void SuspendRecording();
    static void RecordAnswer(const char *answer);
    static boolean AnswerWasCancel()  ;
    static void RecordCancellation();
    static char * GetAnswer()  ;
    static void QueueAnswer(char *answer);
    static void ResumeRecording();
    static void QueueCancellation();
    static class view * GetSelectionOwner()  ;
    static void SetDefaultIconic(boolean val);
    static boolean GetDefaultIconic()  ;

    boolean WasMetaVal;		/* TRUE if GetCharacter returned
				 * an "invented" escape */
    class atom * programNameAtom;
    class im *next;			/* Next im allocated object */
    boolean doRedraw;			/* TRUE if redraw must be done */
    boolean inRedraw;			/* TRUE if in redraw routine */
    class view *topLevel;		/* The top level view connected to this object */
    class view *inputFocus;		/* Current view that is highlighted */
    class view *mouseFocus;		/* View that is receieving mouse events */
    long buttonState;		/* button state at output of queue */
    class keystate *imKeystate;	/* Key state for im object */
    class keystate *keystate;		/* Key state that is being used for input */
    enum im_EventType lastEvent;
    long lastX;
    long lastY;
    enum view_MouseAction lastMouseDown;
    long clickCount;
    struct im_ArgState argState;	/* structure for maintaining argument info */
    long lastCommand;
    class menulist *menus;
    struct im_InteractionEvent *interactionEvents;
    struct im_InteractionEvent *pendingInteractionEvents;
    class cursor *cursorlist;
    class cursor *WindowCursor;
    int CursorsAreActive;
    struct handler *handlers;
    char *title;
    char *programName;
    class init *init;
    long cursorPostsPending;
    FILE *LogFile;
    /* This function is called whenever the window manager requests
     * that the window be deleted.
     */
    im_deletefptr delete_window_cb;
    long delete_window_rock;

    /* these are for echoing keys in the message line */
    char keyEcho[40];
    enum im_keyecho_t keyEchoState;
    class event *keyEchoEvent;
    im_configurefptr configfunc;
    long configrock, configcustomrock;
    class im *moreRecentlyUsed, *lessRecentlyUsed; /* Links for GetLastUsed
						     * queue */
    int initversion;
    class menulist *postedml;
    boolean automap, starticonic;
    class colormap *installedColormap;

    int im_kind;	/* normal, transient, offscreen or override */

    // Buttonbar stuff
    long bbar_size;
    view *bbarv;
    int bbar_location;
};

/* Defines for im_kind above. */
#define im_NORMAL 0
#define im_TRANSIENT 1
#define im_OVERRIDE 2
#define im_OFFSCREEN 3

/* Defines for bbar_location */
#define im_BARTOP 0
#define im_BARBOTTOM 1
#define im_BARLEFT 2
#define im_BARRIGHT 3

struct im_InteractionEvent {
    struct im_InteractionEvent *next;
    class im *im;
    class event *event;
    im_interactionfptr function;
    long data;
};

/* I see no reason not to export the vfile interface so here it is. */

struct expandstring  {
    char *string;
    long pos;
    long length;
    long size;
};

/* Private declarations for use only by interaction managers -- to get access to these data, interaction managers must turn on the flag "INTERACTION_MANAGER" */

#ifdef INTERACTION_MANAGER

/* Global data for im and its children. This structure is used to pass global data back and forth among winodw manager dependent modules (too bad we don't have real dynamic loading) */

#define NUMFILE 16

struct FILEHandlers {
    FILE *file;
    im_filefptr proc;
    char *procdata;
    int priority;
};

/* Here is the actual state to be shared among everyone */

struct im_GlobalDataType {
    char *g_initialProgramName;
    boolean g_setDimensions;
    char *g_geometrySpec;
    long g_preferedTop, g_preferedLeft, g_preferedWidth, g_preferedHeight;

    struct timeval g_PollTime;/* static to allow FlagRedraw to change it */
    class im *g_imList;
    boolean g_doRedraw;
    class updatelist *g_globalUpdateList;
    class cursor *g_ProcessCursor;
    char *g_imPid;		/* lwp pid of im process currently in a select call. */
    class init *g_globalInit;
    boolean g_anyDelivered;
    struct FILEHandlers g_FILEHandlers[NUMFILE];   /* Input Handlers */
    long g_NFILEHandlers;
    struct FILEHandlers g_CanOutHandlers[NUMFILE];  /* Output Handlers */
    long g_NCanOutHandlers;
    struct expandstring g_readCutBuffer;
    struct expandstring g_writeCutBuffer;
    char *g_defaultServerHost;
    im_keyfptr g_enQuserKey;
    im_mousefptr g_enQuserMouse;
    im_menufptr g_enQuserMenu;
};


/* Mongo macros to promote ease of transition from old to new */

#define initialProgramName (gData->g_initialProgramName)
#define setDimensions (gData->g_setDimensions)
#define geometrySpec (gData->g_geometrySpec)
#define preferedTop (gData->g_preferedTop)
#define preferedLeft (gData->g_preferedLeft)
#define preferedWidth (gData->g_preferedWidth)
#define preferedHeight (gData->g_preferedHeight)

#define PollTime (gData->g_PollTime)
#define imList (gData->g_imList)
#define globalDoRedraw (gData->g_doRedraw)
#define globalUpdateList (gData->g_globalUpdateList)
#define ProcessCursor (gData->g_ProcessCursor)
#define imPid (gData->g_imPid)
#define globalInit (gData->g_globalInit)
#define anyDelivered (gData->g_anyDelivered)
#define globalFILEHandlers (gData->g_FILEHandlers)
#define NFILEHandlers (gData->g_NFILEHandlers)
#define CanOutHandlers (gData->g_CanOutHandlers)
#define NCanOutHandlers (gData->g_NCanOutHandlers)
#define readCutBuffer (gData->g_readCutBuffer)
#define writeCutBuffer (gData->g_writeCutBuffer)
#define defaultServerHost (gData->g_defaultServerHost)
#define enQuserKey (gData->g_enQuserKey)
#define enQuserMouse (gData->g_enQuserMouse)
#define enQuserMenu (gData->g_enQuserMenu)

#define IM_METAESC (-22)	/* This value is used to
				 * tell the event queue that a
				 * meta-foo keypress has been
				 * converted to esc, then foo */

#endif /* INTERACTION_MANAGER */
/** @} */
#endif /* _im_H_ */
