/* File ez2htmlapp.H created by R S Kemmetmueller
	(c) Copyright IBM Corp 1995.  All rights reserved. 
	Copyright 1995 Carnegie Mellon University  --All rights reserved

   ez2htmlapp: Converts datastream files to HTML+ tag format. 
*/

#ifndef _ez2htmlapp_H_
#define _ez2htmlapp_H_
ATK_INTER

enum gifmode {
	ez2html_NOGIFS, 	// use [ ... WAS HERE]
	ez2html_GIFIFY,		// create new gifs
	ez2html_NOOLDGIFS,	// delete existing gifs
	ez2html_REUSEGIFS	// reuse existing gifs (if # is right)
};

class gentext;

#include <application.H>
class ez2htmlapp : public application {
  public:
    virtual ATKregistryEntry *ATKregistry();

    boolean ParseArgs(int argc, const char *argv[]);
    boolean Start();
    int Run();
    void ReadInitFile();

    ez2htmlapp();
    virtual ~ez2htmlapp();

    char *inputFile;	// where is input
    char *objectType;	// what type is input
    char *outPath;	// output path (with trailing slash)
    char *gifDir;		// output dir for gifs (may be relative)
    char *outFileNm;	// output file name (no dir or extension)
    char *gifURL;	// URL prefix for references to gifs
	// html output to   outPath/outFileNm.html
	// gifs to   outPath/relGifDir/outFileNm.NNN.gif
    enum gifmode gifFlag;	// -N -G -F -R
    gentext *wrapper;
    boolean Split;		// T if split to sections
    int splitctr;  		// last used .NNN.html value for splitting
};

#endif /* _ez2htmlapp_H_ */
