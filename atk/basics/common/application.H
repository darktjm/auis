ATK_INTER
#ifndef _application_H_
#define _application_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/* Generic application
 */

#include <ATK.H>
#include <init.H>

class application : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

        virtual boolean ParseArgs(int argc,const char **argv)  ;
        virtual void ReadInitFile();
        virtual boolean Start()  ;
        virtual int Run()  ;
        virtual void Stop();
        virtual boolean Fork()  ;
        virtual void PrintVersionNumber();
	virtual const char * const *SessionCheckpoint();
    
        inline void SetFork(boolean torf) { (this->fork=(torf)); }
        inline boolean GetFork() { return (this->fork); }

        inline void SetName(const char *str) { (this->name=(str)); }
        inline const char * GetName() { return (this->name); }

        inline void SetForceLoad(boolean f) { ((this)->forceload=(f)); }
        inline boolean GetForceLoad() { return ((this)->forceload); }
	inline void SetErrorProc(init_fptr p) { ((this)->errorProc=(p)); }
	inline init_fptr GetErrorProc() { return ((this)->errorProc); }
        inline void SetErrorRock(long r) { ((this)->errorRock=(r)); }
        inline long GetErrorRock() { return ((this)->errorRock); }
        inline void SetMajorVersion(long mv) { ((this)->majorversion=(mv)); }
        inline long GetMajorVersion() { return ((this)->majorversion); }
        inline void SetMinorVersion(long mv) { ((this)->minorversion=(mv)); }
        inline long GetMinorVersion() { return ((this)->minorversion); }
        inline void SetPrintVersionFlag(boolean TorF) { ((this)->printversioninfo=(TorF)); }
	inline boolean GetPrintVersionFlag() { return ((this)->printversioninfo); }

	inline void SetIconic(boolean val) { ((this)->iconic=(val)); }
	inline boolean   GetIconic() { return ((this)->iconic); }
	inline char **GetInitialArgv() { return init_argv; }

    
        static void DeleteArgs(const char **argv, int num);
        static char * GetATKVersion()  ;
        static application *GetStartupApplication();

	/* The following functions operate on the startup application. */
	static const char * const *AppSessionCheckpoint();
	static const char *AppGetName();
	static void AppSetName(char *s);
	static char **AppGetInitialArgv();

	application();
        
	virtual ~application();
    
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
private:
	void SaveInitArgv(int argc, const char **argv);
};
/** @} */
#endif /* _application_H_ */
