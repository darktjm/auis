#ifndef _application_H_
#define _application_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */

#include <ATK.H>
#include <init.H>

/** Generic application.
 * In %ATK new applications don't usually define their own main() function.
 * Instead each %application (e.g. ez) has a class derived from
 * application, this class should have a name ending in "app".  runapp
 * or another base executable (such as created by genstatl) includes code in
 * main to perform a variety of setup operations and then creates an
 * application object of the appropriate type.  The appropriate
 * type is determined by the command line arguments to runapp if it is invoked
 * by that name, or by the name used to invoke runapp with "app" appended.
 *  For example ez is actually a symlink to runapp so when you run ez the
 * runapp executable is invoked, and creates an ezapp object.  If the
 * constructor for the application class didn't set the name of the
 * application with SetName, the name will be
 * taken from the name of the executable.  Then the
 * ParseArgs method is invoked to parse the command
 * line arguments.  If ParseArgs succeeds (returns
 * TRUE), the %ATK and application version numbers will be printed
 * (unless supressed, see SetPrintVersionFlag), the
 * init-files will be read, the Start method will then
 * be invoked to initialize the application, if the Start method indicates
 * sucesss (by returning TRUE), the Run
 * method will be invoked to actually begin the processing, when
 * Run exits Stop is invoked
 * to perform any cleanup needed, then finally the exit code returned by
 * Run is passed to exit.
 */
class application : public ATK  {
  public:
    virtual ATKregistryEntry *ATKregistry();

    virtual boolean ParseArgs(int argc, const char **argv);
    /**< Handle the command-line arguments to the application.
     * If an application wishes to add new command line arguments, or
     * disable standard arguments it should override
     * ParseArgs.  If any of the standard arguments are to
     * be supported the first line in ParseArgs should be
     * a call to the base class version of ParseArgs.
     * (Typically application::ParseArgs(argc, argv)
     * since application classes are usually derived
     * directly from application.)  Note that after calling
     * application::ParseArgs the argc
     * value may be out of sync with the actual number of arguments left in
     * argv.  Since
     * application::ParseArgs deletes the
     * arguments it handles.)  The following flags are handled by
     * application::ParseArgs:
     *
     *  - \c -appname \e name
     *
     *    Overrides the application/program name.
     *
     *  - \c -bg \e backgroundcolor
     *
     *    Sets the default background color.
     *
     *  - \c -d
     *
     *    Causes the application to stay in the foreground instead
     *    of forking as is normal for ATK applications.
     *
     *  - \c -display \e host<tt>:0.0</tt>
     *
     *    Sets the default display for the applications windows.
     *    The syntax shown here (<em>host</em><tt>:0.0</tt>) is for X, other window
     *    systems may use other formats.
     *
     *  - \c -fl
     *
     *    Force loading of init files even if some errors occur.
     *
     *  - \c -fg \e foregroundcolor
     *
     *    Sets the default foreground color.
     *
     *  - \c -geometry \e widthxheight+left+top
     *
     *    Sets the default geometry for the initial windows.  The syntax
     *    shown here is for X, other window systems may use other formats.
     *
     *  - \c -host \e host<tt>:0.0</tt>
     *
     *    An alias for -display.
     *
     *  - \c -iconic
     *
     *    Specifies that the initial windows should be iconified on startup.
     *
     *  - \c -iconicoff
     *
     *    Specifies that the initial windows should not be iconified on
     *    startup.  (The default, unless the StartIconic preference is set
     *    to yes.)
     *
     *  - \c -ni
     *
     *    Disables all init-files so that only the built in bindings for
     *    keys, menus and filetypes are available.
     *
     *  - \c -pref \e preference<tt>:</tt>value
     *
     *    Sets or overrides a preference value; any value specified in a
     *    preference file will be ignored if there was a \c -pref switch for
     *    the preference on the command line.
     *
     *  \param argc The argument count; usually from main()'s argc.
     *  \param argv The argument list; usually from main()'s argv, but
     *              this may be modified to be a shorter (NULL-terminated)
     *              list.  Make sure and update argc if needed.
     *  \return TRUE if successful.  If not, some argument error occurred.
     */
    virtual void ReadInitFile();
    /**< Parse the appropriate init files.
     *
     * If application::ParseArgs found the \c -ni switch specified on the
     * command line, this does nothing.
     *
     * There are many possibilities for what init files will be loaded.  First
     * environ::GetConfiguration("SiteConfigName")
     * will be used to form the names:
     * \c $ANDREWDIR/lib/<em>sitename</em>.atkinit and
     * \c $ANDREWDIR/lib/<em>sitename</em>.<em>appname</em>init.  If either
     * of these files are found they will be loaded.  Then if there is a
     * .<em>appname</em>init file in the user's home directory that file
     * will be loaded and no others will be examined.  Otherwise
     * envrion::GetConfiguration("LocalLib") will
     * be consulted to find a colon separated list of path names to be
     * checked for a \c global.<em>appname</em>init file.  The first such
     * file found in that list of paths will be loaded.  If no such file is
     * found \c $ANDREWDIR/lib/global.<em>appname</em>init will be loaded if
     * it exists.  Then \c .atkinit or \c .be2init files (if any) in the
     * user's home directory will be loaded.  If no
     * \c global.<em>appname</em>init file has been found at this
     * point and the "LocalLib" configuration variable was not a path list,
     * but just a single directory that directory will be checked for a
     * \c global.atkinit.  Otherwise \c $ANDREWDIR/lib/global.atkinit is loaded
     * if it exists.
     */
    virtual boolean Start();
    /**< Perform initial actions of an application, before the main loop.
     * After the arguments have been parsed and any init files read,
     * Start() will be called to do any initialization
     * needed before Run() is invoked.
     * The default action is to set default foreground colors, background
     * colors and geometry from initialization preferences.
     * \return FALSE on errors.
     */
    virtual int Run();
    /**< Run main loop of application.
     * Run() will be called to perform the main loop or
     * other processing of the application.  The value returned from
     * Run() will typically be passed to exit() after the
     * Stop() method has been called.
     * The default action is to fork if desired, and then run the
     * interaction manager's main input processor (im::KeyboardProcessor()).
     * \return Return code from application to system.  The default is 0.
     */
    virtual void Stop();
    /**< Peform cleanup actions of an application, after the main loop.
     * Once the Run() has finished
     * Stop() will be called to perform any necessary cleanup.
     */
    virtual boolean Fork();
    /**< Fork if desired.
     * Unless \ref SetFork(\ref FALSE) has been called (e.g. via the default \c -d
     * command-line option) or Fork() has been called once already this
     * function will fork, giving the command prompt back to the user.
     * This is called by the default Run() method.
     * \return TRUE on success, FALSE otherwise.
     */
    virtual void PrintVersionNumber();
    /**< Print application version number.
     * Prints the version number of the application and the program,
     * unless the PrintVersionNumber preference is set to no.  The version
     * number will also not be printed if SetPrintVersionFlag(FALSE)
     * was called.
     * \pref{PrintVersionNumber,Boolean,When set to no\, the startup message
     *       with the application and %ATK version numbers is not printed.}
     */

    inline void SetFork(boolean torf) { (this->fork=(torf)); }
    /**< Set whether or not to fork the next time Fork() is called.
     * The default implementation of
     * Run() calls Fork() just
     * before entering the loop that updates windows and accepts input.
     * Fork() will set this to FALSE after forking so that it won't normally
     * fork more than once.
     * \param torf If TRUE, the application will fork when Fork() is called.
     *             Otherwise the application will not fork.
     */
    inline boolean GetFork() { return (this->fork); }
    /**< Check whether or not to fork the next time Fork() is called.
     * \return If TRUE, the application will fork when Fork() is called.
     *         Otherwise the application will not fork.
     */

    inline void SetName(const char *str) { (this->name=(str)); }
    /**< Set the application name.
     *   Note that the supplied name must be allocated for the lifetime of the
     *   application} object.  (Or longer of course.).  The name is used for
     *   preference matching, default titles, log messages, and more.
     *   \param str The name to set.
     */
    inline const char * GetName() { return (this->name); }
    /**< Get the current application name.
     *   \return The app name, or NULL if not yet set.
     */

    inline void SetForceLoad(boolean f) { ((this)->forceload=(f)); }
    /**< Set whether or not to load classes referenced by init files.
     *   By default any classes referenced in init files are only loaded
     *   as needed.  If \ref SetForceLoad(\ref TRUE) is called on the
     *   application ReadInitFile() will force all the
     *   referenced classes to be loaded immediately.
     *   \param f TRUE if referenced classes should be loaded; FALSE otherwise.
     */
    inline boolean GetForceLoad() { return ((this)->forceload); }
    /**< Check whether or not to load classes referenced by init files.
     *   See SetForceLoad() for details.
     *   \return TRUE if referenced classes should be loaded; FALSE otherwise.
     */
    inline void SetErrorProc(init_fptr p) { ((this)->errorProc=(p)); }
    /**< Set initialization error reporting function.
     *   Sets the function to be executed to report errors encountered while
     *   ReadInitFile() is reading the init files.  Use
     *   SetErrorRock() to set the \e rock to be passed in,
     *   The default routine prints the message to standard error.
     *   \param p The function to call.  May not be NULL.
     */
    inline init_fptr GetErrorProc() { return ((this)->errorProc); }
    /**< Get current initialization error reporting function.
     *   Returns the function which will be used to report errors while
     *   ReadInitFile is reading the init files.
     *   \return the current error reporting function.
     */
    inline void SetErrorRock(long r) { ((this)->errorRock=(r)); }
    /**< Set the generic parameter to be passed to the error proc.
     *   Set the rock to be passed to the error function when an
     *   error is encountered while ReadInitFile is reading the
     *   init files.
     *   \param r The generic parameter, passed in as \e rock.
     */
    inline long GetErrorRock() { return ((this)->errorRock); }
    /**< Get the current generic parameter to be passed to the error proc.
     *   Get the rock to be passed to the error function when an
     *   error is encountered while ReadInitFile is reading the
     *   init files.
     *   \return the generic parameter, passed in as \e rock.
     */
    inline void SetMajorVersion(long mv) { ((this)->majorversion=(mv)); }
    /**< Set application's reported major version number.
     *   Sets the major version number of the application which will be
     *   reported when PrintVersionNumber is called.  This should be
     *   called in the constructor for any class derived from application.
     *   \param mv Major version number.
     */
    inline long GetMajorVersion() { return ((this)->majorversion); }
    /**< Get the reported major version number of the application.
     *   \return the major version number. */
    inline void SetMinorVersion(long mv) { ((this)->minorversion=(mv)); }
    /**< Set application's reported minor version number.
     *   Sets the minor version number of the application which will be
     *   reported when PrintVersionNumber is called.  This should be
     *   called in the constructor for any class derived from application.
     *   \param mv Minor version number.
     */
    inline long GetMinorVersion() { return ((this)->minorversion); }
    /**< Get the reported minor version number of the application.
     *   \return the minor version number. */
    inline void SetPrintVersionFlag(boolean TorF) { ((this)->printversioninfo=(TorF)); }
    /**< Set whether or not to print the version by default.
     *   \param TorF If TRUE the application and ATK version numbers will
     *          be printed by default when the application is run.
     */
    inline boolean GetPrintVersionFlag() { return ((this)->printversioninfo); }
    /**< Check whether or not to print the version by default.
     *   \return TRUE iff the application and ATK version numbers will
     *          be printed by default when the application is run.
     */
    inline void SetIconic(boolean val) { ((this)->iconic=(val)); }
    /**< Set whether or not application will start out iconified.
     *   This is only supported under X.
     *   \param val TRUE if any application windows should start out
     *              iconified.
     */
    inline boolean   GetIconic() { return ((this)->iconic); }
    /**< Check whether or not application will start out iconified.
     *   \return TRUE if any application windows should start out iconified.
     */
    inline const char * const *GetInitialArgv() { return init_argv; }
    /**< Get the argument list last passed to ParseArgs.
     *   When ParseArgs is called, a copy is made of all arguments.
     *   This copy may be retrieved with this function.
     *   \return A NULL-terminated array of string arguments.
     */

    static void DeleteArgs(const char **argv, int num);
    /**< Delete arguments from an argument list.
     *   Given a NULL-terminated argument list, shift later arguments down
     *   in order to wipe some arguments from the beginning of the list.
     *   Pass in a pointer to the middle of the list to remove arguments
     *   from the middle.
     *   \param argv The start of the NULL-terminated argument list.
     *   \param num The number of arguments to wipe out.
     */
    static char * GetATKVersion();
    /**< Get ATK version string.
     *   \return a string describing the version of ATK being run.
     *           This string is valid for the duration of the process.
     */

    /* The following functions operate on the startup application. */
    static application *GetStartupApplication();
    /**< Returns the first application created for this process.
     *   The first application (usually the \e only application) created is
     *   known as the startup application.
     *   It is assumed that the startup application's lifetime is no shorter
     *   than that of any other created application.
     *   \return the first application structure created.
     *   \pref{ForeGroundColor,String,The default foreground color for the
     *         application.}
     *   \pref{BackGroundColor,String,The default background color for the
     *         application.}
     *   \pref{Geometry,String,The default geometry for application\, of
     *         the form
     *         [<em>Width</em>x<em>Height</em>][+/-<em>Left</em>/<em>Right</em>+/-<em>Top</em>/<em>Bottom</em>].} */
    virtual const char * const *SessionCheckpoint();
    /**< Provide checkpoint information for session manager.
     *   SessionCheckpoint is called when the window system wants the
     *   application to checkpoint itself.  The application should override
     *   this and return a NULL terminated argv that is the command line
     *   to reinvoke the app.
     *   \return a NULL-terminated argument list for re-invoking applicaiton.
     *   NOTE:  The caller is responsible for making a copy of the value
     *          returned here.
     */
    static const char * const *AppSessionCheckpoint();
    /**< Provide application checkpoint information for session manager.
     *   This is a shortcut to call SessionCheckpoint() for the startup
     *   application (which is the only one that can really be checkpointed,
     *   anyway).
     *   \return a NULL-terminated argument list for re-invoking applicaiton.
     *   NOTE:  The caller is responsible for making a copy of the value
     *          returned here.
     */
    static const char *AppGetName();
    /**< Return the startup application's name.
     *   Basically a shortcut for calling \ref GetName on the startup application.
     *   \return the program name, or NULL if not set.
     */
    static void AppSetName(const char *s);
    /**< Set the startup application's name.
     *   Basically a shortcut for calling \ref SetName on the startup application.
     *   \param s The program name.  The lifetime of s must match the
     *            lifetime of the startup application.
     */
    static const char * const *AppGetInitialArgv();
    /**< Get the startup application's initial argument list.
     *   Basically a shortcut for calling GetInitialArgv on the startup
     *   application.
     *   \return A NULL-terminated list of character arguments.
     */

    application();
    /**< The default constructor.
     *   This sets the startup application, if not already set.  It also
     *   reads the StartIconic preference to set the default for
     *   iconification.
     *   \pref{StartIconic,Boolean,If true\, create all windows of this
     *                             application iconified.  Otherwise
     *                             (default) create windows normally.} */
    virtual ~application();

  private:
    boolean fork;
    boolean forceload,readInitFile;
    const char *name;
    init_fptr errorProc;
    long errorRock;
    const char *fgcolor, *bgcolor;
    const char *geometry;
    long majorversion, minorversion;
    boolean printversioninfo;
    boolean iconic;
    char **init_argv;	// Initial command line (NULL terminated argv)
    void SaveInitArgv(int argc, const char **argv);
};
/** @} */
#endif /* _application_H_ */
