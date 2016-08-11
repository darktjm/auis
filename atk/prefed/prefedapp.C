/* Copyright 1992 Carnegie Mellon University All rights reserved.
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

#include <andrewos.h>
ATK_IMPL("prefedapp.H")

#include <prefedapp.H>
#include <prefs.H>
#include <pintv.H>
#include <ezapp.H>
#include <environ.H>
#include <filetype.H>

static char *fakeargv[]=
{
    "prefed",
    NULL,
    NULL
};


ATKdefineRegistry(prefedapp, ezapp, NULL);


prefedapp::prefedapp()
{
    class ezapp *e=(class ezapp *)this;
    e->defaultObject="prefs";
    THROWONFAILURE( TRUE);
}

boolean prefedapp::ParseArgs(int  argc, char  **argv)
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
