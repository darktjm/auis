ATK_INTER
#ifndef _environ_H_
#define _environ_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* environ.H
 * Class declarations for environment and preference maodule.
 *
 */

#include <ATK.H>
#define environ environclass
class environ : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

	static void PutPrinter(const char *value);
	static const char *GetPrinter();
	static void DeletePrinter();
        static void Put(const char *variable, const char *value);
        static void Delete(const char *variable);
        static const char * Get(const char *variable)  ;
        static const char * GetProfile(const char *preference)  ;
        static boolean GetProfileSwitch(const char *preference, boolean defaultValue)  ;
	static long GetProfileInt(const char *preference, long defaultValue)  ;
	static boolean ProfileEntryExists(const char *preference, boolean useDefault)  ;
	static const char * GetConfiguration(const char *key)  ;
	static const char * AndrewDir(const char *str)  ;
	static const char * LocalDir(const char *str)  ;
	static struct configurelist * ReadConfigureFile(char *filename)  ;
	static char * GetConfig(struct configurelist *cList, char *key, boolean usedefault)  ;
	static void FreeConfigureList(struct configurelist *cList);
	static const char * GetHome(const char *user)  ;  /* pass in NULL for current user */
	static void SetProgramName(const char *string);	    /* for use by the GetProfile stuff */
	static const char * GetFirstProfileFileName()  ;
	static const char * GetProfileFileName()  ;
	static void AddStringProfile(const char *str);  /* Add a preference to the running environment. */
	static void ExpandEnvVars(char *to, const char *from, int maxsize);
};

#endif /* _environ_H_ */
