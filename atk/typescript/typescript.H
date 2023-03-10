#ifndef _typescript_H_
#define _typescript_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtypescript Object typescript
 *  A dynamically loadable ATK class file.  Contains typescript,
 *  typescriptapp, pipescriptapp, typetext, fcomp.
 * @{ */
#define READONLYMAX 65

/* Typescript menu masks. */
#define typescript_SelectionMenus 1   /* Menus to be posted when there is a selection region. */
#define typescript_NoSelectionMenus 2 /* Menus to be posted only when there is no
                                     * selection. Handles the bogus case of the
                                     * paste item which should show up all the
                                     * time.
                                     */
#define typescript_AfterFenceMenus 4 /* Menus to be posted for writable textviews. */

#include <textview.H>
class typescript : public  textview {
public:
	TRACED_CLASS(typescript);


     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
     void Update();
     class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
     void ReceiveInputFocus();
     void ObservedChanged (class observable *changed, long value);
     void PostMenus(class menulist *menulistp);
     void GetClickPosition(long position, long numberOfClicks, enum view::MouseAction action, long startLeft, long startRight, long *leftPos, long *rightPos);
     void SetDataObject(class dataobject *dataObject);


    virtual void SetTitle(const char *title);
    virtual const char * GetTitle()  ;
    virtual void SetFrame(class frame *f);
    virtual class frame * GetFrame()  ;

    
	typescript();
    static class typescript * Create(const char **arglist,FILE *diskf,boolean filemenu)  ;
    static class typescript * CreatePipescript(FILE *indiskf, FILE *outdiskf, boolean filemenu)  ;
    
	static boolean InitializeClass();

    void CheckChild(void);


    class frame *frame;	/* First frame on the window (for setting the title) */
    class mark *cmdStart;	/* last place a new command was started */
    long lastPosition;		/* used for deciding to frame the dot when reading from the process */
    FILE *SCFile;               /* Sub Channel file * for BE 2file handler. */
    short SubChannel;           /* File descriptor of sub process i/o channel. */
    int SlaveChannel;          /* File descriptor of slave side of the pty. */
    short readOnlyLen;		/* Number of charaters in read only buffer. -1 if not in read only mode. */
    char readOnlyBuf[READONLYMAX]; /* Place to gather characters when in read only (no echo) mode. */
    char OutputWait;
    char ChopLines;
    class keystate *keystatep;
    class menulist *menulistp;
    char *progname;
    boolean pipescript;
    int	pgrpid;                /* process group id for signaling in no job-control systems */
    char *ptyname;             /* Name of the slave side pty device. */
    char *title;
    class text *cmdText;
    long lastCmdPos;
    int pid;
    WAIT_STATUS_TYPE child_status;
};
/** @} */
#endif /* _typescript_H_ */
