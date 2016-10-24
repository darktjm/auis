#ifndef _runbuttonview_H_
#define _runbuttonview_H_
/** \addtogroup librunbutton
 * @{ */
/*
 * runbtnv -- runbuttonview
 *
 * View for an executable button.
 *
 */

#include <sbuttonv.H>
class menulist;
class keystate;
class keymap;
class runbutton;
/** Viewer for \ref runbutton, which executes commands on button presses.
 *  When printed, the button text is displayed within a dotted box, with
 *  a footnote indicating the command to be executed.
 */
class runbuttonview : public sbuttonv {
  public:
    runbuttonview();
    TRACED_CLASS(runbuttonview);
    static boolean InitializeClass();

    // overrides
    boolean Touch(int ind, enum view::MouseAction action);
    void ReceiveInputFocus();
    void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
    void *GetPSPrintInterface(const char *printtype);

    // This returns TRUE if rbv is still valid.  It is used by the dataobject
    // to report info when a command is finished executing (the view may be gone).
    static boolean IsValidRunButtonView(runbuttonview *ebv);
    inline runbutton *GetRunButton()	{ return (runbutton *)ButtonData(); }

    // procs
    static void SetCommandProc(runbuttonview *self, const char *arg); // runbutton-set-command 
    static void SetLabelProc(runbuttonview *self, const char *arg); // runbutton-set-label


  private:
    menulist *ml;
    keystate *ks;
    runbuttonview *next;	// for rbViewList
    struct textview_insetdata *tidata;	// for printing
    struct textview_insetdata *GetTextPrintData();
    void DestroyTextPrintData();

    static menulist *sMenuList;
    static keymap *sKeyMap;
    static runbuttonview *rbViewList;	// list of views.
};
/** @} */
#endif /* _runbuttonview_H_ */
