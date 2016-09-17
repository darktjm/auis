ATK_INTER
#ifndef _datacata_H_
#define _datacata_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

#endif /* _datacata_H_ */
