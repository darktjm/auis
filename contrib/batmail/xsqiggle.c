#include <pwd.h>
#include <sys/param.h>

char *getenv(),*strchr();

/* returns a pointer to the expanded filename.  buf should point to a
 * buffer large enough room to store the expanded version.
 * returns NULL if the user is not valid.
 */
char *expandSquiggle(buf,buflen)
char *buf;
int buflen;
{
    if(*buf=='~'){
	char exp[MAXPATHLEN], *fn=buf+1;

	if(*fn=='/' || *fn=='\0')
	    strcpy(exp,getenv("HOME"));
	else{
	    struct passwd *pw;
	    char *name=fn;

	    fn=strchr(fn,'/');
	    if(fn==NULL)
		strcpy(exp,name);
	    else{
		strncpy(exp,name,fn-name);
		exp[fn-name]='\0';
	    }

	    pw=getpwnam(exp);
	    if(pw==NULL)
		return NULL;	/* abort, unknown user */
	    else
		strcpy(exp,pw->pw_dir);
	}

	strcat(exp,fn);
	strncpy(buf,exp,buflen);
    }

    /* now fn has the real filename */

    return buf;
}
