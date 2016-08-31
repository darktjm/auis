/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

 /*
 * runbtn -- runbutton dataobject
 *
 */

#include <andrewos.h>


#include <util.h>
#include <pwd.h>
#include <environ.H>
#include <im.H>
#include <message.H>
#include <proctable.H>
#include <simpletext.H>
#include <sbutton.H>
#include <sbuttonv.H>
#include <rbttnv.H>
#include <runbutton.H>

#if defined(MK_AUTHORING) || defined(MK_NESS)
#include <nessmark.H>
#include <ness.H>
#endif


ATKdefineRegistry(runbutton, sbutton, runbutton::InitializeClass);

#define UNDEF_LABEL "[Undefined]"

const char *runbutton::commandPath = 0;
int runbutton::numPaths = 0;
char **runbutton::runPath = 0;

/*
 * Cleanpath strips leading and trailing blanks from
 * p, mallocs a new copy and returns a pointer to it.
 * It also expands a leading ~ or ~user.
 *
 * The pathname will always contain a trailing slash.
 */
static char *cleanpath(char  *p)
{
    char *q, *user, *ret;
    int len;
    struct passwd *pw;

    /* Advance past leading blanks and tabs. */
    while (*p != '\0' && (*p == ' ' || *p == '\t'))
	p++;
    len = strlen(p);
    if (len > 0) {
	q = p + len - 1;
	while (q >= p && (*q == ' ' || *q == '\t'))
	    q--;
	q[1] = '\0';	/* truncate trailing blanks and tabs. */
    }
    if (p[0] != '~') {
	/* Simple, just malloc and copy the name. */
	ret = (char *)malloc(strlen(p) + 2);
	if (ret != NULL)
	    sprintf(ret, "%s/", p);
    } else {
	/* Expand ~/ or ~user/ at front of name. */
	if (p[1] == '/' || p[1] == '\0') {
	    /* Want my home dir. */
	    pw = getpwuid(getuid());
	    if (pw == NULL) {
		fprintf(stderr, "RunButton:  could not find you in /etc/passwd!\n");
		return NULL;
	    }
	    p++;
	    if (*p == '/')
		p++;
	} else {
	    /* extract user name */
	    user = p+1;
	    q = user;
	    while (*q != '\0' && *q != '/')
		q++;
	    if (*q == '/') {
		*q = '\0';	/* null terminate username */
		p = q+1;
	    } else {
		p = q;		/* premature end of string. */
	    }
	    /* Lookup user's home directory. */
	    pw = getpwnam(user);
	    if (pw == NULL) {
		fprintf(stderr, "RunButton:  could not find user '%s' in /etc/passwd!\n", user);
		return NULL;
	    }
	}
	/* Return pw->pw_dir + '/' + p */
	ret = (char *)malloc(strlen(pw->pw_dir) + strlen(p) + 3);
	if (ret != NULL)
	    sprintf(ret, "%s/%s/", pw->pw_dir, p);
    }
    return ret;
}

boolean runbutton::InitializeClass()
{
    char *p, *cmd;
    const char *q;
    int i;

    runbutton::commandPath = environ::Get("RUNBUTTONPATH");
    /* Parse the runbutton::commandPath into the path array. */
    if (runbutton::commandPath) {
	q = runbutton::commandPath;
	runbutton::numPaths = 1;
	while (q != NULL && *q != '\0') {
	    q = strchr(q, ':');
	    if (q) {
		runbutton::numPaths++;
		q++;	/* skip : */
	    }
	}
        /* Now we know how many paths exist. */
	runbutton::runPath = (char **)malloc(sizeof(char *) * runbutton::numPaths);
	cmd = (char *)malloc(strlen(runbutton::commandPath)+1);
	if (cmd == NULL || runbutton::runPath == NULL)
	    return FALSE;
	/* Split a copy of the string into paths. */
	strcpy(cmd, runbutton::commandPath);
	p = cmd;
	i = 0;
	runbutton::numPaths = 0;
	runbutton::runPath[i] = p;
	while (*p != '\0') {
	    if (*p == ':') {
		*p = '\0';
		/* Clean up previous path which is now null terminated. */
		runbutton::runPath[i] = cleanpath(runbutton::runPath[i]);
		if (runbutton::runPath[i] != NULL) {
		    if (strlen(runbutton::runPath[i]) == 0)
			fprintf(stderr, "RunButton: an empty element was found on your RUNBUTTONPATH.\n");
		    else if (*runbutton::runPath[i] != '/')
			fprintf(stderr, "RunButton: your RUNBUTTONPATH contains the relative pathname '%s'.\n", runbutton::runPath[i]);
		    else
			i++;
		}
		runbutton::runPath[i] = p+1;
	    }
	    p++;
	}
	/* Clean up final path which is null terminated. */
	runbutton::runPath[i] = cleanpath(runbutton::runPath[i]);
	if (runbutton::runPath[i] != NULL) {
	    if (strlen(runbutton::runPath[i]) == 0)
		fprintf(stderr, "RunButton: an empty element was found on your RUNBUTTONPATH.\n");
	    else if (*runbutton::runPath[i] != '/')
		fprintf(stderr, "RunButton: your RUNBUTTONPATH contains the relative pathname '%s'.\n", runbutton::runPath[i]);
	    else
		i++;
	}
	runbutton::numPaths = i;

	/* Can now free cmd because each path element was reallocated in cleanpath(). */
	free(cmd);

#ifdef DEBUG
	for (i=0; i< runbutton::numPaths; i++)
	    printf("runbutton::runPath[%d]=%s\n", i, runbutton::runPath[i] ? runbutton::runPath[i] : "NULL");
#endif
    }
    return TRUE;
}

runbutton::runbutton()
{
    ATKinit;
    commandString = NULL;
    has_label = FALSE;
    SetLabel(0, UNDEF_LABEL);
}

runbutton::~runbutton()
{
    if (commandString)
	free(commandString);
}

boolean runbutton::EnsureSize(int  ind)
{
    boolean noprefs = (this)->GetDefaultPrefs() == NULL;
    boolean ret;
    struct sbutton_prefs *prefs;

    ret = (this)->sbutton::EnsureSize( ind);
    if (noprefs) {
	prefs = (this)->GetDefaultPrefs();
	if (prefs) {
	    sbutton::InitPrefs(prefs, "runbutton");
	    prefs->bdepth = 1;
	}
    }
    return ret;
}

/*
 * Quote special chars in a string and return a copy
 * in a static buffer shared by both quote and unquote.
 *  Do not free the buffer.
 *
 * This function is limited to returning < 4096 bytes.
 */
static char *quote_buf = NULL;
static const char *quote(const char  *s)
{
    char *p;

    if (quote_buf == NULL) {
	quote_buf = (char *)malloc(4096);
	if (quote_buf == NULL)
	    return "";
    }
    for (p = quote_buf; *s; s++) {
	if (*s == '{' || *s == '}' || *s == '\\')
	    *p++ = '\\';	/* extra slash */
	if (*s >= 32 || *s < 127)
	    *p++ = *s;
	if (p - quote_buf > 4094)
	    break;
    }
    *p = '\0';
    return quote_buf;
}

/*
 * Unquote special chars.  The inverse of quote.
 */
static const char *unquote(const char  *s)
{
    char *p;

    if (quote_buf == NULL) {
	quote_buf = (char *)malloc(4096);
	if (quote_buf == NULL)
	    return "";
    }
    for (p = quote_buf; *s; s++) {
	if (*s == '\\') {
	    /* Skip first backslash */
	    s++;
	    if (*s == '\0')
		break;	/* premature end of string */
	}
	*p++ = *s;
	if (p - quote_buf > 4094)
	    break;
    }
    *p = '\0';
    return quote_buf;
}


long runbutton::Read(FILE  *fp, long  id)
{
    long len;
    char *cmd, *p, *data;
    enum {NoneState, CommandState} state;
    char buffer[256];		/* line buffer for reading the datastream */
    char command_buffer[4096];	/* holds the command string */

    /* Read our data up to the \enddata marker. */
    state = NoneState;
    cmd = command_buffer;
    while (fgets(buffer, sizeof(buffer), fp)) {
	len = strlen(buffer);
	if (len > 0) {
	    /* Remove the trailing newline from the buffer.
	     * We return an error if the buffer doesn't end
	     * with a newline (i.e. the line was too long)
	     * because we never write lines that long.
	     */
	    len--;
	    if (buffer[len] != '\n')
		return dataobject_BADFORMAT;
	    buffer[len] = '\0';
	}
	/* Check for known keywords. */
	if (strcmp(buffer, "\\command") == 0) {
	    state = CommandState;
	    cmd = command_buffer;
	    continue;
	}
	if (strncmp(buffer, "\\label:", 7) == 0) {
	    SetLabelString(unquote(buffer+7));
	    continue;
	}
	if (strncmp(buffer, "\\enddata{", 9) == 0)
	    break;	/* End of our data. */
	/* Not a keyword.  It must be data which always
	 * begins and ends with an 'X' character (sanity check).
	 * The meaning of the data depends on our
	 * current state, which must not be NoneState.
	 */
	if (buffer[0] != 'X' || buffer[len-1] != 'X' || state == NoneState)
	    return dataobject_BADFORMAT;
	len--;
	buffer[len] = '\0';	/* Smash trailing X. */
	/* Data is the contents of the data string. */
	data = buffer+1;
	switch (state) {
	    case CommandState:
		/* Append rest of line to the current position
		 * in the command_buffer pointed to by cmd.
		 * A command can span many lines of text.
		 */
		if (cmd - command_buffer + len >= sizeof(command_buffer)) {
		    /* It won't fit in our buffer. */
		    return dataobject_BADFORMAT;
		}
		for (p = data; *p; )
		    *cmd++ = *p++;
		break;
	    default:
		/* What happened? */
		return dataobject_BADFORMAT;
	}
    }
    if (feof(fp))
	return dataobject_PREMATUREEOF;
    if (cmd != command_buffer) {
	/* Read a quoted command. */
	*cmd = '\0';
	(this)->SetCommandString( unquote(command_buffer));
    }
    return dataobject_NOREADERROR;
}

/*
 * Our datastream looks like this:
 *
 *	\begindata{runbutton,id}
 *	\command
 *	Xcommand line which may X
 *	Xspan many lines.X
 *	\enddata{runbutton,id}
 *
 * The data section contains a leading and trailing X on each
 * line.  The leading X guarantees that no data line starts with
 * a \, and so our commands are unambiguous.  The trailing X
 * prevents mail systems from chopping trailing blanks which may
 * appear within the data.
 */
long runbutton::Write(FILE  *fp, long  id, int  level)
{
    const char *cmd = GetCommandString();
    const char *lbl = GetLabelString();
    const char *quotecmd, *quotelabel, *p;
    char savechar;
    int len;
    long uniqueid = UniqueID();

    if (id != GetWriteID()) {
	/* New Write Operation */
	SetWriteID(id);

	fprintf(fp, "\\begindata{%s,%ld}\n",
		GetTypeName(), uniqueid);

	if (cmd) {
	    fprintf(fp, "\\command\n");
	    quotecmd = quote(cmd);
	    len = strlen(quotecmd);
	    /* Write out the command in chunks to fit ~70 char lines. */
	     p = quotecmd;
	     while (len > 70) {
		 fprintf(fp, "X%.70sX\n", p);
		 p += 70;
		 len -= 70;
	     }
	     if (len > 0)
		 fprintf(fp, "X%sX\n", p);
	}
	if (lbl && has_label) {
	    /* We assume the label won't be too long. */
	    quotelabel = quote(lbl);
	    fprintf(fp, "\\label:%s\n", quotelabel);
	}
	fprintf(fp, "\\enddata{%s,%ld}\n", GetTypeName(), uniqueid);
    }
    return(uniqueid);
}


const char *runbutton::ViewName()
{
    return "runbuttonview";
}


void runbutton::SetCommandString(const char *cmd)
{
    if (commandString)
	free(commandString);
    commandString = NewString((char *)cmd);
    if (!has_label) {
	// We do this as a convenience.
	SetLabel(0, commandString);
    }
}

void runbutton::SetLabelString(const char *lbl)
{
    const char *newlabel;

    if (lbl == NULL || *lbl == '\0') {
	has_label = FALSE;
	if (commandString)
	    newlabel = commandString;
	else
	    newlabel = UNDEF_LABEL;
    } else {
	has_label = TRUE;
	newlabel = lbl;
    }
    SetLabel(0, (char *)newlabel);
}

/*
 * Execute a rexx macro.  The cmd argument specifies
 * the absolute pathname of the file to execute.
 */
void runbutton::ExecuteRexx(runbuttonview *rbv, char *cmd, int argc, char **argv)
{
#ifdef RCH_ENV
    int cmdlen, i;
    char *command;
    static proctable_fptr rexx_run = NULL;

    if (rexx_run == NULL) {
	/* Lookup the rexx-run proc in the atkrexx package.
	 * Atkrexx should provide a classprocedure for this.
	 */
	struct ATKregistryEntry  *ci;
	struct proctable_Entry *pe;

	if (!(ci = ATK::LoadClass("atkrexx"))) {
	    message::DisplayString(rbv, 0, "Could not load the atkrexx macro package.");
	    return;
	}
	if (!(pe = proctable::Lookup("rexx-run")) || pe->proc == NULL) {
	    message::DisplayString(rbv, 0, "Could not find the rexx-run command in the atkrexx package.");
	    return;
	}
	rexx_run = pe->proc;
    }

    /* Put together a command line to pass to atkrexx. */
    cmdlen = strlen(cmd) + 1;
    for (i = 1; i < argc; i++)
	cmdlen = cmdlen + strlen(argv[i]) + 1;
    command = (char *)malloc(cmdlen+1);
    if (command == NULL)
	return;
    strcpy(command, cmd);
    for (i = 1; i < argc; i++) {
	strcat(command, " ");
	strcat(command, argv[i]);
    }
    /* We could write a message here, but instead leave it up to
     * the Rexx macro to write feedback messages to the user.
     */
    rexx_run(rbv, (long)command);
    free(command);
#endif
}


/*
 * Execute a ness macro.  The cmd argument specifies
 * the absolute pathname of the file to execute.
 */
void runbutton::ExecuteNess(runbuttonview *rbv, char *cmd, int argc, char **argv)
{
#if defined(MK_AUTHORING) || defined(MK_NESS)
    ness *nss;
    nessmark *arg, *args, *blank;
    simpletext *stxt;

    nss = new ness;
    args = new nessmark;
    stxt = new simpletext;
    blank = new nessmark;
    arg = new nessmark;
    if (!nss || !args || !stxt || !blank || !arg)
	return;

    /* concatenate args to pass to the ness */
    args->SetText(stxt);
    blank->MakeConst(" ");
    argv++;	/* Skip script name. */
    while (*argv != NULL) {
	arg->MakeConst(*argv);
	args->Next();
	args->Replace(arg);
	args->Next();
	args->Replace(blank);
	argv++;
    }
    delete arg;
    delete blank;

    args->Base();
    nss->SupplyMarkerArg(args);

    /* Read the ness macro file. */
    if (nss->ReadNamedFile(cmd) != dataobject_NOREADERROR) {
	char msg[4096];
	sprintf(msg, "%s is not valid ness.\n", cmd);
	message::DisplayString(rbv, 0, msg);
    } else {
	nss->SetAccessLevel(ness_codeUV);
	nss->EstablishViews(rbv);
	if (nss->Compile() != NULL) {
	    char msg[4096];
	    sprintf(msg, "Error compiling \"%s\".", cmd);
	    message::DisplayString(rbv, 0, msg);
	} else {
	    if (nss->Execute("main") != NULL) {
		char msg[4096];
		sprintf(msg, "Error executing \"%s\".", cmd);
		message::DisplayString(rbv, 0, msg);
		nss->Expose();  /* Should open a window over the ness function. */
	    }
	}
    }

#if 0
	/* cannot destroy the ness object because 'extend's may be in place.
		no need to destroy args because that is done by ness. */
    nss->Destroy();
    delete args;
#endif
#else
	message::DisplayString(rbv, 0, "Ness not installed.");
#endif
}


/*
 * Getarg returns a malloced copy of the next argument
 * in the string pointed to by startpos.  It updates
 * nextpos to point to the character following the
 * next argument.
 *
 * Nextpos would typically be used as the startpos on
 * the next call to getarg.
 *
 * It returns NULL if there are no more arguments, or
 * if malloc fails.
 */
static char *getarg(char  *startpos, char  **nextpos)
{
    char inLiteral = 0;	/* Non-zero if in a literal.  If so, then inLiteral is either " or '. */
    char *src, *arg, *ret;
    char arg_buf[4096];

    /* skip blanks and tabs. */
    src = startpos;
    while (*src != '\0' && (*src == ' ' || *src == '\t'))
	src++;
    if (*src == '\0') {
	*nextpos = NULL;
	return NULL;
    }

    /* Collect up the next argument. */
    arg = arg_buf;
    while (*src != '\0' && (inLiteral || (*src != ' ' && *src != '\t'))) {
	if (!inLiteral && (*src == '\'' || *src == '"')) {
	    /* Start of new literal. */
	    inLiteral = *src;
	    src++;
	} else if (*src == inLiteral) {
	    /* End of a literal string. */
	    /* Not necessarily the end of this argument! */
	    src++;
	    inLiteral = 0;
	} else if (*src == '\\') {
	    /* Quoted character.  Copy it verbatim. */
	    src++;
	    if (src != '\0')
		*arg++ = *src++;
	} else
	    *arg++ = *src++;
	if (arg - arg_buf >= sizeof(arg_buf))
	    break;	/* out of space. Silently truncate! */
    }
    *arg = '\0';
    *nextpos = src;
    ret = (char *)malloc(arg - arg_buf + 1);
    if (ret)
	strcpy(ret, arg_buf);
    return ret;
}


#define RBMAXARGS 100
/*
 * Parse the command line, creating an argv and argc.
 * We only interpret ", ' and backslashes.
 *
 * Return a pointer to an argv array.  Free this with
 * freecommand (below).
 */
static void parsecommand(char  *commandline, char  ***argv, int  *argc)
{
    char *startpos, *nextpos, *cmd;
    char **av;
    int i;

    *argc = 0;	/* be paranoid */
    av = (char **)malloc(sizeof(char *) * RBMAXARGS);
    if (av == NULL)
	return;
    *argv = av;
    startpos = commandline;
    i = 0;
    while (i < RBMAXARGS && (cmd = getarg(startpos, &nextpos))) {
	av[i++] = cmd;
	startpos = nextpos;
    }
    av[i] = NULL;
    *argc = i;
}

/* This function frees the argv, argc created by parsecommand. */
static void freecommand(char  **argv, int  argc)
{
    int i;

    for (i=0; i<argc; i++)
	free(argv[i]);
    free(argv);
}

/*
 * This is a zombie handler for the child process.
 * It is called when the child process dies.
 */
void runbutton::ChildDone(int pid, long rbvc, WAIT_STATUS_TYPE *status)
{
    runbuttonview *rbv=(runbuttonview *)rbvc;
    runbutton *b;
    int code;
    const char *m = NULL;
    char msg[256];

    if (!runbuttonview::IsValidRunButtonView(rbv))
	return;	/* View is now gone.  Too bad. */

    b = (runbutton *)rbv->ButtonData();

    if (WIFEXITED(*status)) {
	/* Child exited with a status code. */
	code = WAIT_EXIT_STATUS(*status);
	if (code == 0)
	    m = "Program ran ok.";
	else if (code == 255) {
	    m = "Program failed to execute.";
	} else {
	    sprintf(msg, "Program terminated with exit code %d.", code);
	    m = msg;
	}
    } else if (WIFSIGNALED(*status)) {
	/* Child died from a signal. */
	code = WTERMSIG(*status);
	sprintf(msg, "Program terminated by signal %d.", code);
	m = msg;
    }
    if (m)
	message::DisplayString(rbv, 0, m);

    sbuttonv::UnHighlightButton(rbv, &(b)->GetButtons()[0], &rbv->info[0].rect);
    rbv->needredraw = TRUE;
    rbv->WantUpdate(rbv);
}


/*
 * This method executes the command string.  The ebv argument
 * is the view which will be used for echoing to the message line.
 */
void runbutton::ExecuteCommand(runbuttonview  *rbv)
{
    char *commandline = GetCommandString();
    char **argv;
    char *ext;
    int argc, i;
    int pid;
    char cmd[4096], msg[4096];

    /* If there is no LastTriggerView, then we were somehow triggered
     * improperly.  For now we just skip the command.
     */
    if (rbv == NULL)
	return;

    if (commandline == NULL) {
	message::DisplayString(rbv, 0, "There is no command associated with this button.");
	return;
    }

    /* If no runbutton::commandPath, the command cannot possibly work. */
    if (runbutton::numPaths == 0) {
	message::DisplayString(rbv, 0, "The RUNBUTTONPATH environment variable must be set before this button can function.");
	return;
    }

    /* Split the command into an argv array. */
    parsecommand(commandline, &argv, &argc);
    if (argc == 0) {
	message::DisplayString(rbv, 0, "There is no command associated with this button.");
	return;
    }

    /* Ensure that the command does not contain any slashes. */
    if (strchr(argv[0], '/')) {
	message::DisplayString(rbv, 0, "The command contains illegal slash (/) characters.");
	return;
    }

#if DEBUG
    for (i = 0; i < argc; i++)
	printf("argv[%d]=%s\n", i, argv[i]);
#endif

    /* Find the command along the path.  It must be executable. */
    for (i=0; i<runbutton::numPaths; i++) {
	sprintf(cmd, "%s%s", runbutton::runPath[i], argv[0]);
	if (access(cmd, R_OK) == 0) {
	    break;
	}
    }
    if (i >= runbutton::numPaths) {
	sprintf(msg, "Could not find command \"%s\" on your RUNBUTTONPATH.", argv[0]);
	message::DisplayString(rbv, 0, msg);
	freecommand(argv, argc);
	return;
    }

    /* Check for special file extensions. */
    ext = strrchr(argv[0], '.');
    if (ext) {
#ifdef RCH_ENV
	if (strcmp(ext, ".rxm") == 0) {
	    /* It is a Rexx macro.  Call it. */
	    ExecuteRexx(rbv, cmd, argc, argv);
	    freecommand(argv, argc);
	    return;
	}
#endif
#if defined(MK_AUTHORING) || defined(MK_NESS)
	if (strcmp(ext, ".n") == 0) {
	    /* It is a Ness macro.  Call it. */
	    ExecuteNess(rbv, cmd, argc, argv);
	    freecommand(argv, argc);
	    return;
	}
#endif
    }

    /* If we get here the command must be a program or shell script.
     * Ensure that it is executable.
     */
    if (access(cmd, X_OK) != 0) {
	sprintf(msg, "Command \"%s\" does not have execute permission.", cmd);
	message::DisplayString(rbv, 0, msg);
	freecommand(argv, argc);
	return;
    }

    /* We found the command and it is executable.  Run it. */
    sprintf(msg, "Running %s ...", commandline);
    message::DisplayString(rbv, 0, msg);
    sbuttonv::HighlightButton(rbv, &GetButtons()[0], &rbv->info[0].rect);
    rbv->needredraw = TRUE;
    rbv->WantUpdate(rbv);
    im::ForceUpdate();

    pid = fork();
    if (pid == -1) {
	message::DisplayString(rbv, 0, "Cannot fork another process.");
	sbuttonv::UnHighlightButton(rbv, &GetButtons()[0], &rbv->info[0].rect);
	rbv->needredraw = TRUE;
	(rbv)->WantUpdate( rbv);
	freecommand(argv, argc);
	return;
    }
    if (pid == 0) {
	/* Child.  Exec the program. */
	close(0); close(1);
	for (i = 3; i < 128; i++)
	    close(i);
	open("/dev/null", O_RDWR, 0);	/* New stdin */
	dup(0);				/* New stdout */
	/* stderr remains as-is. */
	execv(cmd, argv);
	perror("exec failed");
	exit(255);
    }
    /* Parent. */
    im::AddZombieHandler(pid, ChildDone, (long)rbv);

    /* Clean up. */
    freecommand(argv, argc);
}
