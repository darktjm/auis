#ifndef _runbutton_H_
#define _runbutton_H_
/** \addtogroup librunbutton Object runbutton
 *  A dynamically loadable ATK class file.  Contains runbutton, runbuttonview.
 * @{ */
/*
 * runbtn -- runbutton
 *
 * Dataobject for an executable button.
 *
 */

#include <sbutton.H>
class runbuttonview;
/** Data object for storing executable command button information.
 *  This stores the command and label for a run button inset.  It also
 *  actually executes the command.  The command must be in a path specified
 *  by the environment variable RUNBUTTONPATH.  Files found with an extension
 *  of \c .n need not be executable; these are run as Ness scripts.
 *  Its primary viewer is \ref runbuttonview.
 */
class runbutton : public  sbutton {
  public:
    runbutton();
    TRACED_CLASS(runbutton);
    static boolean InitializeClass();

    // overrides
    long Read(FILE *file, long id);
    long Write(FILE *file, long writeid, int level);
    const char *ViewName();
    boolean EnsureSize(int ind);

    virtual void SetCommandString(const char *cmd);
    inline char  *GetCommandString()	{ return (commandString); }
    virtual void SetLabelString(const char *lbl);
    inline const char *GetLabelString() { return GetLabel(0); }
    virtual void ExecuteCommand(runbuttonview *rbv);

  private:
    char *commandString;
    boolean has_label;
    void ExecuteRexx(runbuttonview *rbv, char *cmd, int argc, char **argv);
    void ExecuteNess(runbuttonview *rbv, char *cmd, int argc, char **argv);
    static void ChildDone(int pid, long rbvc, WAIT_STATUS_TYPE *status);	// zombie handler

    static const char *commandPath;
    static int numPaths;
    static char **runPath;
};
/** @} */
#endif /* _runbutton_H_ */
