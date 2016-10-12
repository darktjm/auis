/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* xspell.c
 * Mega hack to get ispell capabilities in bx.
 * Perhaps will clean up later...
 */

#include <andrewos.h> /* sys/types.h sys/file.h */
ATK_IMPL("spell.H")

#include <signal.h>
#include <ctype.h>

#include <text.H>
#include <textview.H>
#include <mark.H>
#include <im.H>
#include <message.H>
#include <proctable.H>
#include <environ.H>

#include <spell.H>

#define Text(self) ((class text *) self->GetDataObject())

static FILE *inFile, *outFile;
static char *wordArray[10];
static boolean expertMode;
/* flags to account for variations in ispell behavior */
static boolean isvers_extranewlines = FALSE; /* in ispell 3, there were extra newlines on things. */
static boolean isvers_noaccept = FALSE; /* in ispell 4, there is no way to accept or add words in -a mode. */


/* Process handling stuff. Copied verbatim from compile.c */

#define READFD 0	/* The pipe index from which you can do a read. */
#define WRITEFD 1	/* The pipe index to which you can do a write. */

struct process {
    FILE *inFile, *outFile;
    short pid;
    boolean stopped;
};


ATKdefineRegistry(spell, ATK, spell::InitializeClass);
static struct process *StartProcess(const char  *command, FILE  **inputFile , FILE  **outputFile);
static int KillSpeller();
static int BuildQuestionLine(char  *buffer , char  *choices);
static void spell_CheckSpellingCmd(class textview  *self, long  rock);


static struct process *StartProcess(const char  *command, FILE  **inputFile , FILE  **outputFile)
        {

    int inpipe[2], outpipe[2], pid;
    struct process *thisProc;

/* Create pipe for parent reading from child. */
    if (inputFile != NULL) {
        if (pipe(inpipe) < 0)
            return 0;
        *inputFile = fdopen(inpipe[READFD], "r");
        fcntl(inpipe[READFD], F_SETFD, 1); /* Set close on exec flag. */
    }
    else
        if ((inpipe[WRITEFD] = open("/dev/null", O_WRONLY) < 0))
            return 0;

/* Create pipe for parent writing to child. */
    if (outputFile != NULL) {
        if (pipe(outpipe) < 0)
            return 0;
        *outputFile = fdopen(outpipe[WRITEFD], "w");
        fcntl(outpipe[WRITEFD], F_SETFD, 1); /* Set close on exec flag. */
    }
    else
        if ((outpipe[READFD] = open("/dev/null", O_RDONLY)) < 0)
            return 0;

    if ((pid = osi_vfork()) == 0) { /* In child */

        int numfds = FDTABLESIZE();
        int fd;

        dup2(outpipe[READFD], 0);
        dup2(inpipe[WRITEFD], 1);
        dup2(1, 2); /* Should have more control over this. */

        /* Don't leave any open file descriptors for child. */
        for (fd = 3; fd < numfds; fd++)
            close(fd);

	NEWPGRP();
        execlp("/bin/sh", "sh", "-c", command, (char *)NULL);
        exit(0);
    }

    close(outpipe[READFD]);
    close(inpipe[WRITEFD]);

    thisProc = (struct process *) malloc(sizeof(struct process));
    if (inputFile != NULL)
        thisProc->inFile = *inputFile;
    else
        thisProc->inFile = NULL;
    if (outputFile != NULL)
        thisProc->outFile = *outputFile;
    else
        thisProc->outFile = NULL;
    thisProc->pid = pid;
    thisProc->stopped = FALSE;
    return thisProc;
}

/* Kill the speller process. Depends on the interact loop to cleanup the
 * zombie.
 */
static int KillSpeller()
{

    fclose(inFile);
    fclose(outFile);
    return(0);
/* We assume the EOF will terminate the speller. The code used to send a
 * SIGTERM, but this resulted in a race condition that would trash the user's
 * private dictionary if the last word in a document was inserted in said
 * dict.
 */
}

/* Makes up a prompt line and fills in the global word array. This will be
 * replaced by a multiple choice question call shortly.
 * Really should have a bounds test on buffer...
 */
static int BuildQuestionLine(char  *buffer , char  *choices)
    {

    int i;

    if(isvers_extranewlines) {
	char *t;
	if ((t = strchr(choices, ':'))) {
	    choices = t + 2;
	}
    }
    else {
	while (*choices == ' ')
	    choices++;
    }

    for (i = 0; *choices != '\0' && (i < 10); i++) {
        sprintf(buffer, "%d) ", i);
        buffer += strlen(buffer);
        wordArray[i] = choices;
        while (*choices && *choices != ',' && *choices != ' ' && *choices != '\n')
            *buffer++ = *choices++;
	if (!*choices) break;
        *choices++ = '\0';
        *buffer++ = ' ';
        while (*choices && (*choices == ' ' || *choices == '\n'))
            choices++;
    }
    *buffer = '\0';
    return i + 1;
}

/* Actually spell check an entire document starting at the current cursor
 * position.
 */
/* ARGUSED */
void spell::CheckDocument(class textview  *self, long  rock)
            {
	ATKinit;


    int pos = (self)->GetDotPosition(), savePos = pos, saveLength = (self)->GetDotLength();
    class text *text;
    class mark *tempMark;
    struct process *speller;
    char testBuf[100];
    long textlen;
    
/* Post a wait cursor. */
    message::DisplayString(self, 0, "Starting spell check.");
    im::ForceUpdate();
    text = Text(self);
    tempMark = (text)->CreateMark( 0, 0);
    
/* Start up process and free process structure. */
    if ((speller = StartProcess("ispell -a", &inFile, &outFile)) != NULL)
        free(speller); /* Not used for now. */
    else {
	message::DisplayString(self, 0, "Unable to execute ispell.");
	return;
    }

    *testBuf = (char) 0;
/* Do an initial rendezvous to ensure that process actually started. */
#define EatNewLine() fgets(testBuf, sizeof(testBuf), inFile)

    if (fputs("the\n", outFile) == EOF || fflush(outFile) == EOF ||
        fgets(testBuf, sizeof(testBuf), inFile) == NULL || *testBuf == '\0') {
        message::DisplayString(self, 0, "Could not start ispell. Spell checking may not be supported at this site.");
        return;
    }
    else {
	if(*testBuf != '*') { /* testBuf must contain the ispell3 version string */
	    message::DisplayString(self, 10, testBuf);
	    isvers_extranewlines = TRUE;
	    EatNewLine();   /* eat '*' result from 'the' query */
	    EatNewLine();   /* eat '\n' */
	}
	else {
	    /* I assume that the extranewlines problem and the noaccept problem do not coexist in any one version of ispell. */
	    fputs("@murglexxxnonexistentword\n", outFile);
	    fflush(outFile);
	    fgets(testBuf, sizeof(testBuf), inFile);
	    if (*testBuf != '*') {
		isvers_noaccept = TRUE;
	    }
	}
    }

    while (pos<(textlen=(text)->GetLength())) {

        int c;
        while (pos<textlen && !isalpha((unsigned char)(c = (text)->GetChar( pos)))) /* Skip to "word" */
	    pos++;

        if (pos<textlen) {

            int c, startpos = pos;
            char buffer[4096], thisWord[100], *temp = thisWord; /* Lots of space... */

	    for (c = (text)->GetChar( pos); isalpha((unsigned char)c); c = (text)->GetChar( ++pos)) {
                putc(c, outFile);
                *temp++ = c;
            }
            putc('\n', outFile);
            fflush(outFile);
            *temp = '\0';

            fgets(buffer, sizeof(buffer), inFile);

	    if (isvers_extranewlines) /* All queries in ispell3 end in a newline */
		EatNewLine();

            temp = buffer;
            while (*temp == ' ' || *temp == '\n')
                temp++;
            fflush(stdout);
	    int flag, maxWord = 0;
	    char messageBuf[200];

            switch (*temp) {

                case '*': /* Found it. */
                    break;
                case '+': /* Suffix removal. */
                    break;
                case '&': /* List of near misses given. */
                case '#': /* No near misses found. */
                    (self)->SetDotPosition( startpos);
                    (self)->SetDotLength( pos - startpos);
                    (self)->FrameDot( startpos);
                    (self)->WantUpdate( self);
                    im::ForceUpdate();

                    if (*temp == '&')
			maxWord = BuildQuestionLine(messageBuf, &temp[1]);
                    else
                        strcpy(messageBuf, "Word not found.");

                    sprintf(messageBuf + strlen(messageBuf), "   [<space>][A][R]%s[I][Q] ?", expertMode ? "[E]" : "");

                    do {

                        int response;
                        int i;
                        unsigned int len;
                        int max;
                        char wordBuf[200];
                        char *word;

                        i = (self)->GetDotPosition();
			len = (self)->GetDotLength();
                        if (len > sizeof(wordBuf) - 1)
                            max = i + sizeof(wordBuf) - 1;
			else
			    max = i + len;
                        for (word = wordBuf; i < max; ++i)
                            *word++ = (text)->GetChar( i);
                        *word = '\0';
                        flag = 0;
			message::DisplayString(self, 0, messageBuf);
			response = ((self)->GetIM())->GetCharacter();

                        switch (response) {
                            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                                if ((response - '0') <= maxWord) {
                                    (tempMark)->SetPos( pos);
                                    (tempMark)->SetLength( 0);
                                    (text)->ReplaceCharacters( (self)->GetDotPosition(), (self)->GetDotLength(), wordArray[response - '0'], strlen(wordArray[response - '0']));
                                    pos = (tempMark)->GetPos();
                                }
                                else {
                                    message::DisplayString(self, 0, "Number too large. Press any key to continue.");
                                    ((self)->GetIM())->GetCharacter();
                                    flag = 1;
                                }
                                break;
                            case ' ':
                                message::DisplayString(self, 0, "Accepted.");
                                break;
                            case 'a':
                            case 'A':
				if (isvers_noaccept) {
                                    message::DisplayString(self, 0, "Your ispell does not allow the 'accept' option. Press any key to continue.");
				    ((self)->GetIM())->GetCharacter();
				    flag = 1;
				    break;
				}
				fputs("@", outFile);
				fputs(thisWord, outFile);
                                fputc('\n', outFile);
                                fflush(outFile);
				if (! isvers_extranewlines) {
				    /* Snag garbage... */
				    fgets(buffer, sizeof(buffer), inFile);
				}
                                message::DisplayString(self, 0, "Accepted for rest of session.");
                                break;
                            case 'r':
                            case 'R':
                                if (message::AskForString(self, 0, "Replacement word: ", wordBuf, wordBuf, sizeof(wordBuf)) >= 0) {
                                    (tempMark)->SetPos( startpos);
                                    (text)->ReplaceCharacters( (self)->GetDotPosition(), (self)->GetDotLength(), wordBuf, strlen(wordBuf));
                                    pos = (tempMark)->GetPos();
                                }
                                else {
                                    message::DisplayString(self, 0, "Canceled. Press any key to continue.");
                                    ((self)->GetIM())->GetCharacter();
                                    flag = 1;
                                }
                                break;
                            case 'i':
                            case 'I':
				if (isvers_noaccept) {
				    message::DisplayString(self, 0, "Your ispell does not allow the 'insert' option. Press any key to continue.");
				    ((self)->GetIM())->GetCharacter();
				    flag = 1;
				    break;
				}
                                putc('*', outFile);
                                fputs(thisWord, outFile);
                                fputc('\n', outFile);
                                fflush(outFile);
				if (! isvers_extranewlines) {
				    /* Snag garbage... */
				    fgets(buffer, sizeof(buffer), inFile);
				}
                                message::DisplayString(self, 0, "Entered in private dict.");
                                break;
                            case 'q':
                            case 'Q':
                            case EOF:
                                KillSpeller();
                                message::DisplayString(self, 0, "Spell check terminated.");
                                (self)->SetDotPosition( savePos);
                                (self)->SetDotLength( saveLength);
				(self)->FrameDot( savePos);
                                (self)->WantUpdate( self);
                                (text)->RemoveMark( tempMark);
                                delete tempMark;
                                return; /* Get out of here. */
                            case 'e':
                            case 'E':
                                if (expertMode) {
				    (tempMark)->SetPos( startpos);
				    (self)->SetDotLength( 0);
				    (self)->WantUpdate( self);
				    message::DisplayString(self, 0, "Recursive editing, ^C exits.");
				    im::KeyboardProcessor();
				    pos = (tempMark)->GetPos(); /* Recheck... */
				    break;
				}
                            default:
				if (expertMode)
				    message::DisplayString(self, 0, "<space>: Accept once, [A]: Accept, [R]: Replace, [E] Edit, [I]: Insert in private dictionary, [Q]: Quit, <number>: replace with that word. Press any key to continue.");
				else
				    message::DisplayString(self, 0, "<space>: Accept once, [A]: Accept, [R]: Replace, [I]: Insert in private dictionary, [Q]: Quit, <number>: replace with that word. Press any key to continue.");
                                ((self)->GetIM())->GetCharacter();
                                flag = 1;
                                break;
			}
		    } while (flag);
                    (self)->WantUpdate( self);
                    im::ForceUpdate();
                    break;
              default: /* Ispell is broken. */
                    break;
	    }
	}
    }
    KillSpeller();
    (text)->RemoveMark( tempMark);
    delete tempMark;
    (self)->SetDotPosition( savePos);
    (self)->SetDotLength( saveLength);
    message::DisplayString(self, 0, "Spell check completed.");
    (self)->FrameDot( savePos);
    (self)->WantUpdate( self);
}

static void spell_CheckSpellingCmd(class textview  *self, long  rock)
        {
    spell::CheckDocument(self, rock);
}
 
boolean spell::InitializeClass()
    {
    proctable::DefineProc("spell-check-document",  (proctable_fptr)spell_CheckSpellingCmd, ATK::LoadClass("textview"), NULL, "Checks spelling from the caret on.");

    expertMode = environ::GetProfileSwitch("expertmode", FALSE);
    return TRUE;
}
