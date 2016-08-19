#ifndef _runbutton_H_
#define _runbutton_H_
/*
 * runbtn -- runbutton
 *
 * Dataobject for an executable button.
 *
 */

#include <sbutton.H>
class runbuttonview;

class runbutton : public  sbutton {
  public:
    runbutton();
    virtual ~runbutton();
    virtual ATKregistryEntry *ATKregistry();
    static boolean InitializeClass();

    // overrides
    long Read(FILE *file, long id);
    long Write(FILE *file, long writeid, int level);
    char *ViewName();
    boolean EnsureSize(int ind);

    virtual void SetCommandString(const char *cmd);
    inline char  *GetCommandString()	{ return (commandString); }
    virtual void SetLabelString(const char *lbl);
    inline char *GetLabelString() { return GetLabel(0); }
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

#endif /* _runbutton_H_ */
