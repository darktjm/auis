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

#include <andrewos.h> /* sys/file.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/init.C,v 3.13 1995/11/29 17:45:30 robr Stab74 $";
#endif


 

/* init.c
 * Read a user's init file to bind keys and menus.
  */

ATK_IMPL("init.H")
#include <sys/file.h> /* MRT */

#include <sys/param.h>
#include <ctype.h>
#include <sys/stat.h>

#include <environ.H>
#include <filetype.H>
#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <path.H>
#include <init.H>
#include <atom.H>
#include <flex.H>
#include <util.h>
/* Format of a .bxinit file:
 * Blank lines and lines beginning with a # are ignored.
 * Other lines have a command and maybe some arguments.  The commands are:
 *
 * addkey function keysequence [class] [loadclass] [inheritp] -- Bind a key to a function.The key
 *     sequence is only honored if an object of type class is present. If
 *     class is omitted, the first part of the function name (upto a hyphen) is
 *     used. Loadclass is used to determine which module must be loaded to find
 *     function. If loadclass is omitted, the first part ot the function name
 *     (upto a hyphen) is used. The class parameter must be present for the
 *     loadclass parameter to be used. The inheritp parameter determines whether
 *     the class test is an inclusion test or an equality test.
 * addmenu function menustring [class] [loadclass] [inheritp] -- Add an extra menu item which invokes
 *     function. See description of addkey for information about class and loadclass.
 *     if function is "", this constitutes a menu deletion and will prevent the 
 *     named menu item from appearing.
 * addfiletype extension classname [attributes] -- Use indicated class as the
 *     data object for files with given extension. the optional attributes
 *     string is used to pass through class specific data.
 * load classname -- Loads the indicated class.
 * call function args -- call named function with given args.
 * include filename -- includes the given file.  Filename may begin with ~
 */

struct keys {
    struct keys *next;
    char *class_c;
    boolean inherit;
    class keymap *keymap;
    class keymap *orig;
};

struct keystateList {
    struct keystateList *next;
    class keystate *keystate;
};

struct mlList {
    struct mlList *next;
    class menulist *menulist;
    int initversion;
};

struct children {
    class init *child;
    struct children *next;
};

static struct keystateList *freeKeystates = NULL;
static struct mlList *freeMenulists = NULL;

/* Next three used for error handling. */
static const char *currentFile;
static int currentLine = 0;
static init_fptr currentErrorProc = NULL;
static long currentErrorRock;


ATKdefineRegistry(init, ATK, NULL);

static class keymap *GetKeymap(class init  *init, char  *className, boolean  inheritFlag);
static class menulist *GetMenulist(class init  *init, char  *className, boolean  inheritFlag);
enum init_bindingtype {init_KEY, init_MENU, init_BUTTON};
static void BindFunction(class init  *init, char  **args, boolean  forceLoad, enum init_bindingtype  type, char  *commandName);
static void AddFileType(char  **args, boolean  forceLoad);
static void Call(char  **args);
static void Load(char  **args);
static int IfDef (char  **args);
static int IfSys (char **args);
static void Include(class init  *init, char  **args, boolean  forceLoad);
static char *GetToken(char  **pp);
static int TranslateKeySequence(char  *from, char  *to);
static int parseBackslashed(char  **fromChars);
static struct keys *GetKeyFromKeystate(class init  *self, class keystate  *keystate);
static ATK  *CheckML(class menulist  *menulist, char  *class_c, boolean  inherit);
static char *MapFile(const char  *filename, long  *fileLength );
static int ReadFile(class init  *init, const char  *filename, boolean  executeImmediately);
static void ErrorMsg(char  *msg, ...);


init::init()
        {
    this->keys = NULL;
    this->menus = NULL;
    this->buttons = NULL;
    this->usedKeystates = NULL;
    this->usedMenus = NULL;
    this->parent=NULL;
    this->kids=NULL;
    this->version=0;
    THROWONFAILURE( TRUE);
}

init::~init()
        {
    class init *parent=this->parent;

    if(parent) {
	struct children *kid=parent->kids;
	struct children *prevkid=NULL;
	struct children *nextkid;
	while(kid) {
	    nextkid = kid->next;
	    if (kid->child == this) {
		/* Remove me from my parent's kid list. */
		if (prevkid)
		    prevkid->next = nextkid;
		else
		    parent->kids = nextkid;
		free(kid);
		/* Leave prevkid as it was. */
	    } else
		prevkid = kid;
	    kid = nextkid;
	}
    }
    
    while(this->kids) {
	struct children *next=this->kids->next; this->kids->child->parent=NULL;
	free(this->kids);
	this->kids=next;
    }
    
    {
        struct keystateList *freeItem, *next;

        for (freeItem = this->usedKeystates; freeItem != NULL; freeItem = next) {
            next = freeItem->next;
	    freeItem->next = freeKeystates;
            freeKeystates = freeItem;
        }
    }

    {
        struct mlList *freeItem, *next;

        for (freeItem = this->usedMenus; freeItem != NULL; freeItem = next) {
            next = freeItem->next;
            freeItem->next = freeMenulists;
            freeMenulists = freeItem;
        }
    }
    
    struct keys *k=this->keys;
    while(k) {
	struct keys *n=k->next;
	if(k->orig==k->keymap) {
	 // doesn't work yet.... :-(   delete k->orig;
	}
	free(k);
	k=n;
    }
    struct menus *m=this->menus;
    while(m) {
	struct menus *n=m->next;
// doesn't work yet... :-(	if(m->orig==m->menulist) delete m->orig;
	free(m);
	m=n;
    }
}

static class keymap *GetKeymap(class init  *init, char  *className, boolean  inheritFlag)
            {

    struct keys *keys;

    for (keys = init->keys; keys != NULL && ((strcmp(className, keys->class_c) != 0) || (keys->inherit != inheritFlag)); keys = keys->next)
        ;
    if (keys == NULL) {
	atom *a=atom::Intern(className);
        keys = (struct keys *) malloc(sizeof(struct keys));
        keys->class_c = a->Name();
	keys->keymap = new keymap;
	keys->orig = keys->keymap;
        keys->inherit = inheritFlag;
        keys->next = init->keys;
        init->keys = keys;
    }
    return keys->keymap;
}

static class menulist *GetMenulist(class init  *init, char  *className, boolean  inheritFlag)
            {

    struct menus *menus;

    for (menus = init->menus; menus != NULL && ((strcmp(className, menus->class_c) != 0) || (menus->inherit != inheritFlag)); menus = menus->next)
        ;
    if (menus == NULL) {
	atom *a=atom::Intern(className);
        menus = (struct menus *) malloc(sizeof(struct menus));
        menus->class_c = a->Name();
	menus->menulist = new menulist;
	menus->orig=menus->menulist;
        menus->inherit = inheritFlag;
        menus->next = init->menus;
        init->menus = menus;
    }
    return menus->menulist;
}

static class menulist *GetButtonlist(class init  *init, char  *className, boolean  inheritFlag)
            {

    struct menus *menus;

    for (menus = init->buttons; menus != NULL && ((strcmp(className, menus->class_c) != 0) || (menus->inherit != inheritFlag)); menus = menus->next)
        ;
    if (menus == NULL) {
        menus = (struct menus *) malloc(sizeof(struct menus));
        menus->class_c = (char *) malloc(strlen(className) + 1);
        strcpy(menus->class_c, className);
        menus->menulist = new menulist;
        menus->inherit = inheritFlag;
        menus->next = init->buttons;
        init->buttons = menus;
    }
    return menus->menulist;
}



static void BindFunction(class init  *init, char  **args, boolean  forceLoad, enum init_bindingtype  type, char  *commandName)
                    {

    char *function, *tempString, *binding, *class_c, *loadClass, *inherit, *parameterString;
    char className[100], loadClassName[100], translatedKeys[32];
    struct proctable_Entry *proc;
    boolean inheritFlag = TRUE;
    keymap *kmap;
    menulist *mlist;

    function = GetToken(args);
    if (function == NULL) {
        ErrorMsg("Missing function name in %s command.\n", commandName);
        return;
    }
    binding = GetToken(args);
    if (binding == NULL) {
 	switch (type) {
	    case init_KEY:
		ErrorMsg("Missing key sequence in addkey command.\n");
		break;
	    case init_MENU:
		ErrorMsg("Missing menu string in addmenu command.\n");
		break;
	    case init_BUTTON:
		ErrorMsg("Missing button string in addbutton command.\n");
		break;
	}
        return;
    }
    class_c = GetToken(args);
    if (class_c == NULL) {

        char *hyphen;

        hyphen = strchr(function, '-');
        if (hyphen == NULL) {
            ErrorMsg("Missing class name in %s command.\n", commandName);
            return;
        }
        strncpy(className, function, hyphen - function);
        className[hyphen - function] = '\0';
        class_c = loadClass = className;
    }
    else {
        loadClass = GetToken(args);
        if (loadClass == NULL && *function != '\0') {

            char *hyphen;

            hyphen = strchr(function, '-');
            if (hyphen == NULL) {
                ErrorMsg("Missing loadclass name in %s command.\n", commandName);
                return;
            }
            strncpy(loadClassName, function, hyphen - function);
            loadClassName[hyphen - function] = '\0';
            loadClass = loadClassName;
        }
    }

    inherit = GetToken(args);
    if (inherit != NULL)
        if (strcmp(inherit, "noinherit") == 0)
            inheritFlag = FALSE;
        else if (strcmp(inherit, "inherit") != 0) {
            ErrorMsg("Bad inherit flag value %s; must be either \"inherit\" or \"noinherit\".\n", inherit);
            return;
        }            

    parameterString = GetToken(args);
    if(parameterString) {
	// there was an argument now lets check to see if it is an extended argument.
	// the extended syntax is:
	// ( [name=](atom|"string"|number)[:type] elem2 )  The spaces after the open and before the close paren are required.
	// elements may be quoted, within quotes \ and " must be preceded by a \.
       while(*args[0] && isspace(*args[0]) && *args[0]!='\n') (*args)++;
       if(parameterString[0]=='(' && parameterString[1]=='\0' && *args[0]!='\n'  && *args[0]!='\0') {
	   parameterString[1]=' ';
	   while(*args[0] && *args[0]!='\n') (*args)++;
	   *args[0]='\0';
	   char *res=(char *)malloc(strlen(parameterString)+6 /* 1 NUL, 5 list: */);
	   if(res) {
	       strcpy(res, "list:");
	       strcat(res, parameterString);
	       parameterString=NewString(res);
	   } else parameterString=NULL;
       } else parameterString=NewString(parameterString);
    }
    if (type == init_KEY)
        if (TranslateKeySequence(binding, translatedKeys) < 0) {
            ErrorMsg("Bad key sequence %s.\n", binding);
            return;
        }

/* This next if statement handles the special case of a meu item deletion. */
    if (*function != '\0') {
/* proctable_DefineProc does not allocate storage for its arguments... */
/* neither does it free them, so this is a leak */
        tempString = (char *) malloc(strlen(function) + 1);
        strcpy(tempString, function);
        function = tempString;
        tempString = (char *) malloc(strlen(loadClass) + 1);
        strcpy(tempString, loadClass);
        loadClass = tempString;
        proc = proctable::DefineProc(function, NULL, NULL, loadClass, NULL);

        if (forceLoad && !proctable::Defined(proc)) {
            proctable::ForceLoaded(proc);
            if (!proctable::Defined(proc)) { /* Should be better than this... */
                ErrorMsg("Either couldn't load class %s, or class didn't define function %s\n", loadClass, function);
                return;
            }
        }
    }
    else
        proc = NULL;
/*
    if (parameterString) {

        char *tempString;

        tempString = (char *) malloc(strlen(parameterString) + 1);
        strcpy(tempString, parameterString);
        parameterString = tempString;
    }
 */
    switch(type) {
	case init_KEY:

	    class keymap *keymap;

	    keymap = GetKeymap(init, class_c, inheritFlag);
	    (keymap)->BindToKey( translatedKeys, proc,
				 (parameterString != NULL) ? (long) parameterString :
				 (long) translatedKeys[strlen(translatedKeys) - 1] /* a useful rock */);
	    break;
	case init_MENU: 
	    class menulist *menulist;

	    menulist = GetMenulist(init, class_c, inheritFlag);
	    (menulist)->AddToML( binding, proc, (long) parameterString, 0);
	    break;
	case init_BUTTON:
	    // Do button init here.
	    mlist = GetButtonlist(init, class_c, inheritFlag);
	    mlist->AddToML(binding, proc, (long) parameterString, 0);
	    break;
    }
}

/* Establish a default dataobject for files with a given extension. */
static void AddFileType(char  **args, boolean  forceLoad)
        {

    char *extension, *type, *attributes, *existingAttributes;

    extension = GetToken(args);
    if (extension == NULL) {
	ErrorMsg("Missing extension in addfiletype command.\n");
	return;
    }
    type = GetToken(args);
    if (type == NULL) {
	ErrorMsg("Missing class name in addfiletype command.\n");
	return;
    }
    attributes = GetToken(args);

    existingAttributes = GetToken(args);

    if (forceLoad && (ATK::LoadClass(type) == NULL))
    	ErrorMsg("Could not load class %s in addfiletype command for extension %s.\n", type, extension);
    else {
	filetype::AddEntry(extension, type, attributes);
	filetype::AddExistingAttributes(extension, type, existingAttributes);
    }
}

/* Call the function associated with a proctable command with add-hoc
 * arguments. Not very useful since first argument is supposed to be a view.
 */
#define NARGS	6
static void Call(char  **args)
    {

    char *functionName;
    char *functionArgs[NARGS];
    struct proctable_Entry *entry;
    proctable_fptr function= NULL;
    int i;
    char *argument;

    functionName = GetToken(args);
    if (functionName == NULL) {
	ErrorMsg("Missing function name in call command.\n");
	return;
    }
    for (i = 0; i < NARGS && ((argument = GetToken(args)) != NULL); ++i) {
	if (isdigit(*argument))
	    functionArgs[i] = (char *) atoi(argument);
	else
	    functionArgs[i] = argument;
    }
    entry = proctable::Lookup(functionName);
    if (entry != NULL) {
        proctable::ForceLoaded(entry);
        function = entry->proc;
    }
    if (function == NULL) {
	ErrorMsg("Undefined function in call command: %s\n", functionName);
	return;
    }
    else
	(*function)((ATK *)functionArgs[0], (long)functionArgs[1]);
}

/* Load the named class. Useful for debugging and in certain applications where
 * key bindings (or menus) are inappropriate.
 */
static void Load(char  **args)
    {

    char *class_c = GetToken(args);

    if (class_c == NULL)
        ErrorMsg("Missing class name in load command.\n");
    else {
        if (ATK::LoadClass(class_c) == NULL)
            ErrorMsg("Could not load class %s in load command.\n", class_c);
    }
}

/*
Process an 'ifdef' entry, checking the specified environment variable.
 
   Returns :     1 :  If the statements after it should be executed
                     0 :  If the statements after it should NOT be executed
                   -1 :  If an error is encountered
*/
static int IfDef (char  **args)
{
    char *envvar, errmsg;

    if ((envvar = GetToken(args)) == NULL) {
	ErrorMsg("No environment variable named for ifdef.\n");
	return -1;
    }
    else if (environ::Get(envvar))
	return 1;
    else
	return 0;
}
/*
 Process an 'ifsys' entry, checking the system type

 Returns :     1 :  If the statements after it should be executed
 0 :  If the statements after it should NOT be executed
 -1 :  If an error is encountered
 */
static int IfSys (char  **args)
{
    char *systype;

    if ((systype = GetToken(args)) == NULL) {
	ErrorMsg("No system type named for ifsys.\n");
	return -1;
    }
    while (systype != NULL) {
	if (FoldedEQ(systype, SYS_NAME))
	    return 1;
	systype = GetToken(args);
    }
    return 0;
}


/* Include the named file as if its contents were inline within the current
    * file. If the file is not an absolute path, look for it in the
	* InitFilePath.
	*/
static void Include(class init  *init, char  **args, boolean  forceLoad)
{
    char fullName[MAXPATHLEN], tmpFullName[MAXPATHLEN];
    const char *file = GetToken(args);

    /* Expand environment variables first */
    environ::ExpandEnvVars(tmpFullName, file, MAXPATHLEN);
    /* Check to see if this is an absolute path */
    if ((tmpFullName[0] != '/')
#ifdef DOS_STYLE_PATHNAMES
	&& (tmpFullName[0]!='\\') && !(isalpha(tmpFullName[0]) && tmpFullName[1]==':')
#endif
	) {
	char *tmpfname = NewString(tmpFullName);
	const char *initfilepath = environ::GetConfiguration("InitFilePath");
	if (initfilepath != NULL) {
	    path::FindFileInPath(tmpFullName, initfilepath, tmpfname);
	    if (tmpFullName == NULL || tmpFullName[0] == '\0')
		strcpy(tmpFullName, file);
	}
	free(tmpfname);
    }
    file = path::UnfoldFileName(tmpFullName, fullName, 0);
    if (file == NULL)
	ErrorMsg("Missing filename in include command.\n");
    else {
	if (ReadFile(init, file, forceLoad) < 0)
	    ErrorMsg("Could not read included file %s.\n", file);
    }
}

/* Find the first token on this line and update the pointer to the buffer to
 * point past it.  Also smashes the buffer with a null character.
 */
static char *GetToken(char  **pp)
    {
    char *from = *pp, *to = from;
    int quote = FALSE;
    char *s;

    while (*from != '\n' && isspace(*from)) /* Skip whitespace .*/
	++from;

    if (*from == '#') /* Skip comments and fall through to end-of-buffer code immediately below. */
        do {
	    ++from;
	} while (*from != '\n' && *from != '\0');

    if (*from == '\n' || *from == '\0') {
        *pp = from;
        return NULL;
    }

    if (*from == '"') {
        quote = TRUE;
        to = s = ++from;
        if (*from == '"') {
            *pp = from + 1;
            return "";
        }
    }
    else
        to = s = from;
    do {
        if (*from == '\\')
            *to++ = parseBackslashed(&from);
        else
            *to++ = *from++;
    } while (*from != 0 && (quote ? *from != '"' : !isspace(*from)));

    if (*from == '\n')
        *pp = from;
    else
        *pp = ++from;

    *to++ = '\0';
    return s;
}

/* Translate a key sequence that has ^A, \ddd, and \c conventions. */
static int TranslateKeySequence(char  *from, char  *to)
        {
    while (*from != '\0') {
        if (*from == '\\') {

            int temp = parseBackslashed(&from);

            if (temp == -1)
                return -1;
            else
                *to++ = temp;
        }
        else if (*from == '^') {
            ++from;
            if (*from == 0)
                return -1;
            *to++ = (*from++) & 0x1f;
        }
        else
            *to++ = *from++;
    }
    *to++ = 0;
    return 0;
}

static int parseBackslashed(char  **fromChars)
    {

    int returnChar;
    char *from = *fromChars;
    static char *bsSource = "ebrnt";
    static char *bsDest = "\033\b\r\n\t";

    if (*from == '\\') {
        ++from;
        if (*from == 0)
            return -1;
        if (isdigit(*from)) {

            int sum = 0, i;

            for (i = 0; i < 3; ++i) {
                if (!isdigit(*from))
                    break;
                if (*from == '8' || *from == '9')
                    return -1;
                sum = sum * 8 + *from - '0';
                ++from;
            }
            returnChar = sum;
        }
        else {

            char *p;

            p = strchr(bsSource, *from);
            if (p != NULL)
                returnChar = bsDest[p-bsSource];
            else
                returnChar = *from;
            ++from;
        }
    }
    else
        return -1;
    *fromChars = from;
    return returnChar;
}

static struct keys *GetKeyFromKeystate(class init  *self, class keystate  *keystate)
{
    class keymap *keymap = keystate->orgMap;
    struct keys *keys;

    for (keys = self->keys; keys != NULL && keys->keymap != keymap; keys = keys->next) {
    }

    return keys;
}

class keystate *init::ModifyKeystate(class keystate  *keystate)
        {

    struct keys *keys;
    class keystate *traverse, **previous;
    struct keystateList *freeItem, *next;
    boolean addKeymap;

    for (freeItem = this->usedKeystates; freeItem != NULL; freeItem = next) {
        next = freeItem->next;
	freeItem->next = freeKeystates;
        freeKeystates = freeItem;
    }
    this->usedKeystates = NULL;
    for (keys = this->keys; keys != NULL; keys = keys->next) {
        previous = &keystate;
	for (traverse = keystate; traverse != NULL; traverse = traverse->next) {
	    if (keys->inherit) {
		addKeymap = ATK::IsTypeByName((traverse->object)->GetTypeName(), keys->class_c);

		if (addKeymap) {
		    struct keys *k = GetKeyFromKeystate(this, traverse);

		    addKeymap = (k == NULL || ! ATK::IsTypeByName(k->class_c, keys->class_c));
		}
	    }
	    else {
		addKeymap = strcmp((traverse->object)->GetTypeName(), keys->class_c) == 0;
	    }

            if (addKeymap) {
                if (freeKeystates == NULL) {
                    freeItem = (struct keystateList *) malloc(sizeof(struct keystateList));
                    freeItem->keystate = keystate::Create(traverse->object, keys->keymap);
                }
                else {
                    freeItem = freeKeystates;
                    freeKeystates = freeItem->next;
                    freeItem->keystate->object = traverse->object;
                    freeItem->keystate->orgMap = keys->keymap;
		    freeItem->keystate->curMap = keys->keymap;
		    freeItem->keystate->next=NULL;
		}
                freeItem->next = this->usedKeystates;
                this->usedKeystates = freeItem;
                *previous = freeItem->keystate;
		freeItem->keystate->next = traverse;
		/* Don't need to attempt to add this keystate again */
		break;
            }
            previous = &traverse->next;
        }
    }
    return keystate;
}

static ATK  *CheckML(class menulist  *menulist, char  *class_c, boolean  inherit)
            {

    class menulist *thisML;
    ATK  *thisObject;

    if (inherit ?
            ATK::IsTypeByName((menulist->object)->GetTypeName(), class_c) :
            (strcmp((menulist->object)->GetTypeName(), class_c) == 0))
        return menulist->object;
    menulist_RewindBeforeMC(menulist);
    while ((thisML = (menulist)->NextBeforeMC()) != NULL)
        if ((thisObject = CheckML(thisML, class_c, inherit)) != NULL)
            return thisObject;
    menulist_RewindAfterMC(menulist);
    while ((thisML = (menulist)->NextAfterMC()) != NULL)
        if ((thisObject = CheckML(thisML, class_c, inherit)) != NULL)
            return thisObject;
    return NULL;
}


class menulist *init::ModifyMenulist(class menulist  *menulist)
        {

    struct menus *menus;
    struct mlList *freeItem, *next;
    ATK  *thisObject;
    class menulist *topMenulist = menulist;

    for (freeItem = this->usedMenus; freeItem != NULL; freeItem = next) {
        next = freeItem->next;
        freeItem->next = freeMenulists;
	freeMenulists = freeItem;
	freeItem->menulist->ClearChain();
    }
    this->usedMenus = NULL;
    for	(menus = this->menus; menus != NULL; menus = menus->next) {
	if ((thisObject = CheckML(menulist, menus->class_c, menus->inherit)) != NULL) {
            if (freeMenulists == NULL) {
                freeItem = (struct mlList *) malloc(sizeof(struct mlList));
                freeItem->menulist = (menus->menulist)->DuplicateML( (view *)thisObject);
            }
            else {
                freeItem = freeMenulists;
                freeMenulists = freeMenulists->next;
                freeItem->menulist->object = thisObject;
		freeItem->menulist->menus = menus->menulist->menus;
		freeItem->menulist->ClearChain();
            }
            freeItem->next = this->usedMenus;
            this->usedMenus = freeItem;
            (freeItem->menulist)->ChainAfterML( topMenulist, (long) topMenulist);
	    topMenulist = freeItem->menulist;
	}
    }
    return topMenulist;
}

static struct keys *DuplicateKeys(struct keys *tk) {
    struct keys *res=NULL;
    while(tk) {
	struct keys *r2=(struct keys *)malloc(sizeof(struct keys));
	if(r2==NULL) return res;
	*r2=*tk;
	if(tk->orig==tk->keymap) {
	    r2->keymap=tk->keymap;
	    r2->orig=r2->keymap;
	} else r2->keymap=tk->keymap;
	r2->next=res;
	res=r2;
	tk=tk->next;
    }
    return res;
}

static struct menus *DuplicateMenus(struct menus *tm) {
    struct menus *res=NULL;
    while(tm) {
	struct menus *r2=(struct menus *)malloc(sizeof(struct menus));
	if(r2==NULL) return res;
	*r2=*tm;
	if(tm->orig==tm->menulist) {
	    //	    r2->menulist=tm-> menulist-> DuplicateML((view *)tm-> menulist-> object);
	    r2->menulist=tm->menulist;
	    r2->orig=r2->menulist;
	} else r2->menulist=tm->menulist;
	r2->next=res;
	res=r2;
	tm=tm->next;
    }
    return res;
}

class init *init::Duplicate()
    {

    class init *newInit;
    struct children *newchild=(struct children *)malloc(sizeof(struct children));

    if(newchild==NULL) return NULL;
    
    newInit = new init;
    newInit->keys = DuplicateKeys(this->keys);
    newInit->menus = DuplicateMenus(this->menus);
    newInit->buttons = this->buttons;
    newInit->parent = this;
    newchild->next = this->kids;
    newchild->child = newInit;
    this->kids = newchild;
    
    return newInit;
}

#define INITIALSIZE 512

/* Hacked routine to rea a "whole file" into memory. */
static char *MapFile(const char  *filename, long  *fileLength /* OUT */)
        {

    int fd;
    char *buffer;
    long length = 0;

    if ((fd = open(filename, O_RDONLY, 0)) >= 0) {

        struct stat statBuf;

        if (fstat(fd, &statBuf) >= 0) {

            long bufferSize; /* Current size of malloced block. */
            long bytesRead = 0;

            /* Find the size. In the case of special files, use a suitable default. */
            if ((statBuf.st_mode & S_IFMT) == S_IFREG)
                bufferSize = statBuf.st_size ;
            else
                bufferSize = INITIALSIZE;

            buffer = (char *) malloc(bufferSize + 2); /* +2 for newline and NUL at end. */

            while (buffer != NULL && (bytesRead = read(fd, buffer + length, bufferSize - length )) > 0) {
                length += bytesRead;
                if (length >= bufferSize) {
                    bufferSize *= 2;
                    buffer = (char *) realloc(buffer, bufferSize + 2); /* +2 for NEWLINE and  NUL at end. */
                }
            }
            if (bytesRead < 0) {
                free(buffer);
                buffer = NULL;
            }
            else
	    {
		buffer[length]='\n'; // ensure that the last line has a newline
		buffer[length+1] = '\0';
	    }
        }
        else
            buffer = NULL;

        close(fd);
    }
    else
        buffer = NULL;

    if (fileLength != NULL)
        *fileLength = length;
    return buffer;
}

#define UnmapFile(mappedMemory) free(mappedMemory)

static int ReadFile(class init  *init, const char  *filename, boolean  executeImmediately)
{

    char *buffer;
    long length;
    boolean ignoring[8]; /* MRT */
    int level=0;

    ignoring[level]=FALSE;
    init->version++;
    
    if ((buffer = MapFile(filename, &length)) != NULL) {

        char *token, *p;
        const char *prevFile = currentFile;
        int prevLine = currentLine;
  
          currentFile = filename;
          currentLine = 0;
	
          p = buffer;
          while (p < buffer + length) {
              ++currentLine;
              token = GetToken(&p);
              if (token != NULL)
		if (strcmp(token, "endif") == 0) {
		    if (!ignoring[level])
			ignoring[level] = FALSE;
		    --level;
		}
		else if (strcmp(token, "else") == 0) {
		    if (level) {
			if (!ignoring[level-1])
			    ignoring[level]= !ignoring[level];
		    }
		    else
			ErrorMsg("Invalid syntax\n", token);
		}
  		else if (strcmp(token, "ifdef") == 0)
		    ++level,ignoring[level] = ignoring[level-1] ? TRUE:(IfDef(&p)==0);
		else if (strcmp(token, "ifndef") == 0)
		    ++level,ignoring[level]= ignoring[level-1] ? TRUE:IfDef(&p);
	        else if (strcmp(token, "ifsys") == 0)
		    ++level,ignoring[level]= ignoring[level-1] ? TRUE:(IfSys(&p) == 0);
	        else if (strcmp(token, "ifnsys") == 0)
		    ++level,ignoring[level]= ignoring[level-1] ? TRUE:IfSys(&p);
		else if (ignoring[level])
		    ;
		else if (strcmp(token, "addkey") == 0)
                      BindFunction(init, &p, executeImmediately, init_KEY, "addkey");
                  else if (strcmp(token, "addmenu") == 0)
                      BindFunction(init, &p, executeImmediately, init_MENU, "addmenu");
                else if (strcmp(token, "addbutton") == 0)
                    BindFunction(init, &p, executeImmediately, init_BUTTON, "addbutton");
                  else if (strcmp(token, "addfiletype") == 0)
                      AddFileType(&p, executeImmediately);
                  else if (strcmp(token, "call") == 0)
                    Call(&p);
                else if (strcmp(token, "load") == 0)
                    ::Load(&p);
                else if (strcmp(token, "include") == 0)
                    Include(init, &p, executeImmediately);
                else
                    ErrorMsg("Undefined command - %s\n", token);

            /* Skip to the end of line. */
            while (*p != '\n' && *p != '\0')
                ++p;

            /* Get to the next line. */
            ++p;
        }

        UnmapFile(buffer);

        currentFile = prevFile;
        currentLine = prevLine;

        return 0;
    }
    else
        return -1;
}    

/* Read the user's init file. */
int init::Load(char  *filename, init_fptr  errorProc, long  errorRock, boolean  executeImmediately /* True if modules should be loaded now. Useful for debugging init files. */)
                    {
    struct children *kids=this->kids;

    while(kids) {
	currentErrorProc = errorProc;
	currentErrorRock = errorRock;
	if(ReadFile(kids->child, filename, executeImmediately)<0) return -1;
	kids=kids->next;
    }

    currentErrorProc = errorProc;
    currentErrorRock = errorRock;
    return ReadFile(this, filename, executeImmediately);
}

static void ErrorMsg(char  *msg, ...)
    {

    char buffer[300], *bufferEnd;

    va_list va;
    va_start(va, msg);
    if (currentErrorProc != NULL) {
        sprintf(buffer, "File: %s Line: %d: ", currentFile, currentLine);
        bufferEnd = buffer + strlen(buffer);
        vsprintf(bufferEnd, msg, va);
        (*currentErrorProc)(currentErrorRock, buffer);
    }
    va_end(va);
}

	void
init::AddKeyBinding(char  *class_c, boolean  inherit, class keymap  *keymapp)
				{
	struct keys *keys;
	struct children *kids=this->kids;

	this->version++;
	while(kids) {
	    (kids->child)->AddKeyBinding( class_c, inherit, keymapp);
	    kids=kids->next;
	}
	atom *a=atom::Intern(class_c);
	keys = (struct keys *)malloc(sizeof(struct keys));
	keys->class_c = a->Name();
	keys->inherit = inherit;
	keys->keymap = keymapp;
	keys->orig=NULL;
	keys->next = this->keys;
	this->keys = keys;
}

	void
init::DeleteKeyBinding(char  *class_c, boolean  inherit, class keymap  *keymap)
				{
	struct keys *keys, **prev;

	struct children *kids=this->kids;

	this->version++;
	
	while(kids) {
	    (kids->child)->DeleteKeyBinding( class_c, inherit, keymap);
	    kids=kids->next;
	}

	for (keys = this->keys, prev = &this->keys; 
			keys != NULL 
			&& ((strcmp(class_c, keys->class_c) != 0)
			   || keys->keymap!= keymap) ; 
		    prev = &keys->next, keys = keys->next)
		{}
	if (keys != NULL) {
		/* found the one to delete */
		*prev = keys->next;
		free (keys);
	}
}

	void
init::AddMenuBinding(char  *class_c, boolean  inherit, class menulist  *menulist)
				{
	struct menus *menus;

	struct children *kids=this->kids;

	this->version++;
	
	while(kids) {
	    (kids->child)->AddMenuBinding( class_c, inherit, menulist);
	    kids=kids->next;
	}
	atom *a=atom::Intern(class_c);
	menus = (struct menus *)malloc(sizeof(struct menus));
	menus->class_c = a->Name();
	menus->inherit = inherit;
	menus->menulist = menulist;
	menus->orig=NULL;
	menus->next = this->menus;
	this->menus = menus;
}

	void
init::DeleteMenuBinding(char  *class_c, boolean  inherit, class menulist  *menulist)
				{
	struct menus *menus, **prev;
	struct children *kids=this->kids;

	this->version++;
	
	while(kids) {
	    (kids->child)->DeleteMenuBinding( class_c, inherit, menulist);
	    kids=kids->next;
	}
	for (menus = this->menus, prev = &this->menus; 
			menus != NULL 
			&& ((strcmp(class_c, menus->class_c) != 0)
			   || menus->menulist != menulist) ; 
		    prev = &menus->next, menus = menus->next)
		{} 
	if (menus != NULL) {
		/* found the one to delete */
		*prev = menus->next;
		free (menus);
	}
}


