#ifndef _datacata_H_
#define _datacata_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libdatacata Object datacata
 *  A dynamically loadable ATK class file.  Contains datacata.
 * @{ */
#define datacata_IncludeNo (0)
#define datacata_IncludeYes (1)
#define datacata_IncludeRecursive (2)

#define datacata_TrackQuiet (0)
#define datacata_TrackNormal (1)
#define datacata_TrackVerbose (2)

#include <application.H>
class datacata : public  application {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	datacata();
      
	virtual ~datacata();

    
       boolean ParseArgs(int argc,const char **argv)  ;
       int Run()  ;
       void ReadInitFile();

    
      virtual void BeginRun();
      virtual void FinishRun();
      virtual boolean AddFile(const char *filename)  ;
      virtual void SetIncludeLevel(int val);
      virtual void SetVerboseLevel(int val);
      virtual void SetCleanMode(boolean val);
      virtual void SetOutputFile(const char *filename);
      virtual void SetLockAndDelete(boolean val);

    
      class text *tx;
      boolean pv, ipros, followlinks;
      boolean verbose, insertedtemplate, cleanmode;
      int LockAndDelete;
      char *ofilename;
};
/** @} */
#endif /* _datacata_H_ */
