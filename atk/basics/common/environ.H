ATK_INTER
#ifndef _environ_H_
#define _environ_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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


 

/* environ.H
 * Class declarations for environment and preference maodule.
 *
 */

#include <ATK.H>
#define environ environclass
class environ : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

	static void PutPrinter(char *value);
	static char *GetPrinter();
	static void DeletePrinter();
        static void Put(char *variable, char *value);
        static void Delete(char *variable);
        static char * Get(char *variable)  ;
        static char * GetProfile(char *preference)  ;
        static boolean GetProfileSwitch(char *preference, boolean defaultValue)  ;
	static long GetProfileInt(char *preference, long defaultValue)  ;
	static boolean ProfileEntryExists(char *preference, boolean useDefault)  ;
	static char * GetConfiguration(char *key)  ;
	static char * AndrewDir(char *str)  ;
	static char * LocalDir(char *str)  ;
	static struct configurelist * ReadConfigureFile(char *filename)  ;
	static char * GetConfig(struct configurelist *cList, char *key, boolean usedefault)  ;
	static void FreeConfigureList(struct configurelist *cList);
	static char * GetHome(char *user)  ;  /* pass in NULL for current user */
	static void SetProgramName(char *string);	    /* for use by the GetProfile stuff */
	static char * GetFirstProfileFileName()  ;
	static char * GetProfileFileName()  ;
	static void AddStringProfile(const char *str);  /* Add a preference to the running environment. */
	static void ExpandEnvVars(char *to, const char *from, int maxsize);
};

#endif /* _environ_H_ */
