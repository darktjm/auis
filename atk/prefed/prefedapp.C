/* Copyright 1992 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
ATK_IMPL("prefedapp.H")

#include <prefedapp.H>
#include <prefs.H>
#include <pintv.H>
#include <ezapp.H>
#include <environ.H>
#include <filetype.H>

static const char *fakeargv[]=
{
    "prefed",
    NULL,
    NULL
};


ATKdefineRegistryNoInit(prefedapp, ezapp);


prefedapp::prefedapp()
{
    class ezapp *e=(class ezapp *)this;
    e->defaultObject="prefs";
    THROWONFAILURE( TRUE);
}

boolean prefedapp::ParseArgs(int  argc, const char  **argv)
{
    boolean result=(this)->ezapp::ParseArgs( argc, argv);
    if(((class ezapp *)this)->files==NULL) {
	fakeargv[1]=environ::GetProfile("PrefEdDefaultFile");
	if(fakeargv[1]==NULL) fakeargv[1]=environ::GetProfileFileName();
	return result && (this)->ezapp::ParseArgs( 2, fakeargv);
    } else return result;
}

prefedapp::~prefedapp()
{
}

boolean prefedapp::Start()
{
    /* XXX make sure the default object type is a prefs!
	this will override any settings in an initfile! */
    filetype::AddEntry("*", "prefs", "");
    return this->ezapp::Start();
}
