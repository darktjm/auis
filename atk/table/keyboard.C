/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* keyboard.c - keyboard input for table */

#include <andrewos.h>
#include <bind.H>
#include <proctable.H>
#include <keymap.H>
#include <view.H>
#include <message.H>
#include <table.H>

#define AUXMODULE
#include <spread.H>
#include <shared.h>
/* Cancel old input and set new message buffer state */

static boolean debug=FALSE;

void k_SetMessageState (class spread  * V, int  newstate);
void k_TellUser (class spread  * V, const char    *s);
boolean k_AskUser (class spread  * V, const char  prompt[], const char  def[], char  buff[], int  n);
boolean k_AreYouSure (class spread  * V, const char  *why);
boolean k_WantToDiscard (class spread  * V);
#ifdef NOTUSED
static void k_exit(class spread  * V, char  ch);
#endif /* NOTUSED */
void AddRows(class spread  * V, int  after , int  count);
void AddCols(class spread  * V, int  after , int  count);
static int k_CheckSelection (class spread  * V);
static int k_ReadFormula (class spread  * V, char  *startstring);
static void k_enterchar(class spread  * V, char  ch);
static void k_backspace(class spread  * V, char  ch);
static void k_tab(class spread  * V, char  ch);
static void k_newline(class spread  * V, char  ch);
static void k_killbuff (class spread  * V, char  ch);
static void k_rightarrow (class spread  * V, char  ch);
static void k_leftarrow (class spread  * V, char  ch);
static void k_downarrow (class spread  * V, char  ch);
static void k_uparrow (class spread  * V, char  ch);
static void k_home (class spread  * V, char  ch);
static void k_endline (class spread  * V, char  ch);
static void k_top (class spread  * V, char  ch);
static void k_bottom (class spread  * V, char  ch);
static void k_debug (class spread  * V, char  ch);
void k_DefineKeys (class keymap  * mainmap, struct ATKregistryEntry   *classinfo);
static void k_togglereadonly(class spread *V, char ch);


void k_SetMessageState (class spread  * V, int  newstate)
{
    if (V->bufferstatus != BUFFEREMPTY) {
	if (debug)
	    printf("Cancelled input request, status %d, new %d\n", V->bufferstatus, newstate);
	message::CancelQuestion (V);
    }
    if (V->bufferstatus != newstate) {
	V->bufferstatus = newstate;
    }
}

/* message to user */

void k_TellUser (class spread  * V, const char    *s)
{
    k_SetMessageState (V, BUFFERHASMESSAGE);
    message::DisplayString (V, 0, s);
}

/* ask for and read keyboard input */

boolean k_AskUser (class spread  * V, const char  prompt[], const char  def[], char  buff[], int  n)
{
    int notOK;

    k_SetMessageState (V, BUFFERHASPARAM);
    notOK = message::AskForString(V, 0, prompt, def, buff, n);
    V->bufferstatus = BUFFEREMPTY;
    if (debug)
	printf("k_Askuser got status %d, message %s\n", notOK, buff);
    if (notOK)
	k_TellUser (V, "Cancelled!");
    return !notOK;
}

/* Are you sure? */

boolean k_AreYouSure (class spread  * V, const char  *why)
{
    char buff[10];

    if (!k_AskUser (V, why, "", buff, sizeof buff))
	return 0;
    return (buff[0] == 'y' || buff[0] == 'Y');
}

/* Verify that user wants to discard changes */

boolean k_WantToDiscard (class spread  * V)
{
    if ((MyTable(V))->WriteTimestamp() < (MyTable(V))->CellsTimestamp()
     || (MyTable(V))->WriteTimestamp() < (MyTable(V))->EdgesTimestamp())
	return k_AreYouSure(V, "Table modified; exit anyway? [no]:  ");
    return TRUE;
}

#ifdef NOTUSED
static void k_exit(class spread  * V, char  ch)
{
    if (k_WantToDiscard (V))
	exit (0);
}
#endif /* NOTUSED */
/* add rows to table */

void AddRows(class spread  * V, int  after , int  count)
{
    if (after < 0) after = 0;
    (MyTable(V))->ChangeSize ( (MyTable(V))->NumberOfRows() + count, (MyTable(V))->NumberOfColumns());
    if (0 < count && after + count < (MyTable(V))->NumberOfRows()) {
	class table * Q;
	struct chunk remnant;

	remnant.LeftCol = -1;
	remnant.RightCol = (MyTable(V))->NumberOfColumns() - 1;
	remnant.TopRow = after;
	remnant.BotRow = (MyTable(V))->NumberOfRows() - count - 1;
	Q = (MyTable(V))->ExtractData ( &remnant);
	
	remnant.TopRow += count;
	remnant.BotRow += count;
	(MyTable(V))->InsertData ( Q, &remnant);
	(Q)->Destroy();
    }
    (V->GetParent())->WantNewSize( V);
}

/* add columns to table */

void AddCols(class spread  * V, int  after , int  count)
{
    if (after < 0) after = 0;
    (MyTable(V))->ChangeSize ( (MyTable(V))->NumberOfRows(), (MyTable(V))->NumberOfColumns() + count);
    if (0 < count && after + count < (MyTable(V))->NumberOfColumns()) {
	class table * Q;
	struct chunk remnant;

	remnant.TopRow = -1;
	remnant.BotRow = (MyTable(V))->NumberOfRows() - 1;
	remnant.LeftCol = after;
	remnant.RightCol = (MyTable(V))->NumberOfColumns() - count - 1;
	Q = (MyTable(V))->ExtractData ( &remnant);
	
	remnant.LeftCol += count;
	remnant.RightCol += count;
	(MyTable(V))->InsertData ( Q, &remnant);
	(Q)->Destroy();
    }
    (V->GetParent())->WantNewSize( V);
}

/* verify there is a selection to enter data into */

static int k_CheckSelection (class spread  * V)
{
    struct chunk newselection;

    if (V->selection.BotRow < 0 || V->selection.RightCol < 0) {
	k_TellUser (V, "Please select a cell");
	return 0;
    }

    if (V->selection.BotRow < V->selection.TopRow) {
	AddRows(V, V->selection.TopRow, 1);
	CopyChunk(&newselection, &(V->selection));
	newselection.BotRow = newselection.TopRow;
	SetCurrentCell (V, &newselection);
    }

    if (V->selection.RightCol < V->selection.LeftCol) {
	AddCols(V, V->selection.LeftCol, 1);
	CopyChunk(&newselection, &(V->selection));
	newselection.RightCol = newselection.LeftCol;
	SetCurrentCell (V, &newselection);
    }

    return 1;
}

/* Read new formula for cell */

static int k_ReadFormula (class spread  * V, char  *startstring)
{
    char   keybuff[1000];
    struct cell * cell;
    int r, c;
    int notOK;

    if (k_CheckSelection(V) == 0)
	return 0;

    for (r = max(0, V->selection.TopRow); r <= V->selection.BotRow; r++) {
	for (c = max(0, V->selection.LeftCol); c <= V->selection.RightCol; c++) {
	    if (!(MyTable(V))->IsJoinedToAnother( r, c)) {
		if ((MyTable(V))->GetCell ( r, c)->lock) {
		    k_TellUser (V, "A cell is locked.");
		    return 0;
		}
	    }
	}
    }

    k_SetMessageState (V, BUFFERHASINPUT);
    notOK = message::AskForString (V, 0, "", startstring, keybuff, sizeof keybuff);
    if (debug)
	printf("k_ReadFormula got status %d, string %s\n", notOK, keybuff);
    V->bufferstatus = BUFFEREMPTY;
    if (notOK) {
	k_TellUser(V, "Input canceled!");
	return 0;
    }

    for (r = max(0, V->selection.TopRow); r <= V->selection.BotRow; r++) {
	for (c = max(0, V->selection.LeftCol); c <= V->selection.RightCol; c++) {
	    if (!(MyTable(V))->IsJoinedToAnother( r, c)) {
		cell = (MyTable(V))->GetCell ( r, c);
		(MyTable(V))->ParseCell ( cell, keybuff);
	    }
	}
    }
    TellFormula (V);
    return 1;
}

static void k_enterchar(class spread  * V, char  ch)
{
    char   *startstring=NULL;
    char *cp=NULL;

    if (ch != '$' && ch != '{') {
	GetFormula (V, &(V->anchor), &startstring);
	if(startstring==NULL) goto outofmemory;
	cp=startstring;
	if (*cp == '{') {
	    while (*cp && *cp++ != '}') ;
	}
	else if (*cp == '$')
	    cp++;
    }
    if(cp==NULL) {
	startstring=(char *)malloc(1000);
	if(startstring==NULL) goto outofmemory;
	cp=startstring;
    }
    *cp++ = ch;
    *cp++ = '\0';
    k_ReadFormula (V, startstring);
    free(startstring);
    return;
    outofmemory:
      message::DisplayString(V, 0, "Out of memory!");
    return;
      
}

static void k_backspace(class spread  * V, char  ch)
{
    char *keybuff=NULL;

    GetFormula (V, &(V->selection), &keybuff);
    k_ReadFormula (V, keybuff);
    free(keybuff);
}

static void k_tab(class spread  * V, char  ch)
{
    struct chunk newselection;

    k_CheckSelection(V);

    CopyChunk(&newselection, &(V->selection));
    newselection.LeftCol = ++newselection.RightCol;
    if (newselection.RightCol >= (MyTable(V))->NumberOfColumns())
	newselection.RightCol = (MyTable(V))->NumberOfColumns() - 1;
    SetCurrentCell (V, &newselection);
}

static void k_newline(class spread  * V, char  ch)
{
    struct chunk newselection;

    k_CheckSelection(V);

    CopyChunk(&newselection, &(V->selection));
    newselection.TopRow = ++newselection.BotRow;
    if (newselection.BotRow >= (MyTable(V))->NumberOfRows())
	newselection.BotRow = (MyTable(V))->NumberOfRows() - 1;
    newselection.LeftCol = newselection.RightCol = 0;
    SetCurrentCell (V, &newselection);
}

static void k_killbuff (class spread  * V, char  ch)
{
    k_SetMessageState (V, BUFFEREMPTY);
}

static void k_rightarrow (class spread  * V, char  ch)
{
    struct chunk chunk;

    k_killbuff (V, ch);
    CopyChunk(&chunk, &(V->anchor));
    if (chunk.RightCol < (MyTable(V))->NumberOfColumns() - 1) {
	if (chunk.TopRow < 0)
	    chunk.TopRow = 0;
	chunk.LeftCol = ++chunk.RightCol;
	chunk.BotRow = chunk.TopRow;
    }
    SetCurrentCell (V, &chunk);
}

static void k_leftarrow (class spread  * V, char  ch)
{
    struct chunk chunk;

    k_killbuff (V, ch);
    CopyChunk(&chunk, &(V->anchor));
    if (chunk.TopRow < 0)
	chunk.TopRow = 0;
    if (--chunk.LeftCol < 0)
	chunk.LeftCol = 0;
    chunk.RightCol = chunk.LeftCol;
    chunk.BotRow = chunk.TopRow;
    SetCurrentCell (V, &chunk);
}

static void k_downarrow (class spread  * V, char  ch)
{
    struct chunk chunk;

    k_killbuff (V, ch);
    CopyChunk(&chunk, &(V->anchor));
    if (chunk.BotRow < (MyTable(V))->NumberOfRows() - 1) {
	if (chunk.LeftCol < 0)
	    chunk.LeftCol = 0;
	chunk.TopRow = ++chunk.BotRow;
	chunk.RightCol = chunk.LeftCol;
    }
    SetCurrentCell (V, &chunk);
}

static void k_uparrow (class spread  * V, char  ch)
{
    struct chunk chunk;

    k_killbuff (V, ch);
    CopyChunk(&chunk, &(V->anchor));
    if (--chunk.TopRow < 0)
	chunk.TopRow = 0;
    if (chunk.LeftCol < 0)
	chunk.LeftCol = 0;
    chunk.BotRow = chunk.TopRow;
    chunk.RightCol = chunk.LeftCol;
    SetCurrentCell (V, &chunk);
}

static void k_home (class spread  * V, char  ch)
{
    struct chunk chunk;

    k_killbuff (V, ch);
    CopyChunk (&chunk, &(V->anchor));
    chunk.LeftCol = chunk.RightCol = 0;
    if (chunk.BotRow < 0)
	chunk.BotRow = chunk.TopRow = 0;
    SetCurrentCell (V, &chunk);
}

static void k_endline (class spread  * V, char  ch)
{
    struct chunk chunk;

    k_killbuff (V, ch);
    CopyChunk (&chunk, &(V->anchor));
    chunk.LeftCol = chunk.RightCol = (MyTable(V))->NumberOfColumns() - 1;
    if (chunk.BotRow < 0)
	chunk.BotRow = chunk.TopRow = 0;
	SetCurrentCell (V, &chunk);
}

static void k_top (class spread  * V, char  ch)
{
    struct chunk chunk;

    k_killbuff (V, ch);
    CopyChunk (&chunk, &(V->anchor));
    chunk.TopRow = chunk.BotRow = 0;
    chunk.LeftCol = chunk.RightCol = 0;
    SetCurrentCell (V, &chunk);
}

static void k_bottom (class spread  * V, char  ch)
{
    struct chunk chunk;

    k_killbuff (V, ch);
    CopyChunk (&chunk, &(V->anchor));
    chunk.TopRow = chunk.BotRow = (MyTable(V))->NumberOfRows() - 1;
    chunk.LeftCol = chunk.RightCol = (MyTable(V))->NumberOfColumns() - 1;
    SetCurrentCell (V, &chunk);
}

/* toggle debug */

static void k_debug (class spread  * V, char  ch)
{

/*
    the rather peculiar if statement below is designed to work properly
    in either the case that table and spread are loaded separately or
    loaded together (ie two or one copies of 'debug'
*/
    if (debug) {
	(MyTable(V))->ToggleDebug();
	debug = 0;
    } else {
	(MyTable(V))->ToggleDebug();
	debug = 1;
    }
}

static void k_togglereadonly(class spread *V, char ch) {
    MyTable(V)->SetReadOnly(!MyTable(V)->GetReadOnly());
}

static struct bind_Description keytable[] = {

    {"table-begin-row", "\001", 0, 0, 0, 0, (proctable_fptr)k_home, "Beginning of row"},
    {"table-left", "\002", 0, 0, 0, 0, (proctable_fptr)k_leftarrow, "Left one column"},
    {"table-end-row", "\005", 0, 0, 0, 0, (proctable_fptr)k_endline, "End of row"},
    {"table-right", "\006", 0, 0, 0, 0, (proctable_fptr)k_rightarrow, "Right one column"},
    {"table-down", "\016", 0, 0, 0, 0, (proctable_fptr)k_downarrow, "Down one row"},
    {"table-up", "\020", 0, 0, 0, 0, (proctable_fptr)k_uparrow, "Up one row"},

    {"table-backspace", "\010", 0, 0, 0, 0, (proctable_fptr)k_backspace, "Backspace"},
    {"table-tab", "\011", 0, 0, 0, 0, (proctable_fptr)k_tab, "Tab"},
    {"table-newline", "\012", 0, 0, 0, 0, (proctable_fptr)k_newline, "New line"},
    {"table-return", "\015", 0, 0, 0, 0, (proctable_fptr)k_newline, "Carriage return"},
    {"table-DELchar", "\177", 0, 0, 0, 0, (proctable_fptr)k_backspace, "DEL"},

    {"table-erase", "\025", 0, 0, 0, 0, (proctable_fptr)k_killbuff, "Erase"},

    {"table-up", "\033A", 0, 0, 0, 0, (proctable_fptr)k_uparrow, "Up"},
    {"table-down", "\033B", 0, 0, 0, 0, (proctable_fptr)k_downarrow, "Down"},
    {"table-right", "\033C", 0, 0, 0, 0, (proctable_fptr)k_rightarrow, "Right"},
    {"table-left", "\033D", 0, 0, 0, 0, (proctable_fptr)k_leftarrow, "Left"},
    {"table-end-row", "\033J", 0, 0, 0, 0, (proctable_fptr)k_endline, "End"},
    {"table-begin-row", "\033H", 0, 0, 0, 0, (proctable_fptr)k_home, "home"},
    {"table-top", "\033<", 0, 0, 0, 0, (proctable_fptr)k_top, "Top"},
    {"table-bottom", "\033>", 0, 0, 0, 0, (proctable_fptr)k_bottom, "Bottom"},

    {"table-toggle-debug", "\033\033", 0, 0, 0, 0, (proctable_fptr)k_debug, "Toggle Debug"},
    {"table-toggle-readonly", "\033~", 0, NULL, 0, 0, (proctable_fptr)k_togglereadonly, "Toggle the read only status of the table."},
    {0, 0, 0, 0, 0, 0, 0}
};
static struct bind_Description keytable_readonly[] = {

    {"table-begin-row", "\001", 0, NULL, 0, 0, (proctable_fptr)k_home, "Beginning of row"},
    {"table-left", "\002", 0, NULL, 0, 0, (proctable_fptr)k_leftarrow, "Left one column"},
    {"table-end-row", "\005", 0, NULL, 0, 0, (proctable_fptr)k_endline, "End of row"},
    {"table-right", "\006", 0, NULL, 0, 0, (proctable_fptr)k_rightarrow, "Right one column"},
    {"table-down", "\016", 0, NULL, 0, 0, (proctable_fptr)k_downarrow, "Down one row"},
    {"table-up", "\020", 0, NULL, 0, 0, (proctable_fptr)k_uparrow, "Up one row"},

    {"table-tab", "\011", 0, NULL, 0, 0, (proctable_fptr)k_tab, "Tab"},
    {"table-up", "\033A", 0, NULL, 0, 0, (proctable_fptr)k_uparrow, "Up"},
    {"table-down", "\033B", 0, NULL, 0, 0, (proctable_fptr)k_downarrow, "Down"},
    {"table-right", "\033C", 0, NULL, 0, 0, (proctable_fptr)k_rightarrow, "Right"},
    {"table-left", "\033D", 0, NULL, 0, 0, (proctable_fptr)k_leftarrow, "Left"},
    {"table-end-row", "\033J", 0, NULL, 0, 0, (proctable_fptr)k_endline, "End"},
    {"table-begin-row", "\033H", 0, NULL, 0, 0, (proctable_fptr)k_home, "home"},
    {"table-top", "\033<", 0, NULL, 0, 0, (proctable_fptr)k_top, "Top"},
    {"table-bottom", "\033>", 0, NULL, 0, 0, (proctable_fptr)k_bottom, "Bottom"},
    {"table-toggle-readonly", "\033~", 0, NULL, 0, 0, (proctable_fptr)k_togglereadonly, "Toggle the read only status of the table."},
    {0, 0, 0, 0, 0, 0, 0}
};

void k_DefineKeys (class keymap  *mainmap, class keymap *readonlymap, struct ATKregistryEntry   *classinfo)
{
    char ch;
    struct proctable_Entry *tempProc;

    if(mainmap) {
	tempProc = proctable::DefineProc("table-self-insert", (proctable_fptr)k_enterchar, classinfo, NULL, "Enter character");
	for (ch = ' '; ch < 127; ch++) {		/* self-insert */
	    if (ch != '/') {
		char foo[2];
		foo[0] = ch;
		foo[1] = '\0';
		(mainmap)->BindToKey ( foo, tempProc, ch);
	    }
	}
	bind::BindList(keytable, mainmap, NULL, classinfo);
    }
    if(readonlymap) {
	bind::BindList(keytable_readonly, readonlymap, NULL, classinfo);
    }

}
