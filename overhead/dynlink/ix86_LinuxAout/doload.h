#ifndef DOLOAD
#define DOLOAD 1
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* 
	doload.h - environment for dynamic loader

	Author:  John H Howard - April 9, 1987
 */



/* here is the state during a load */

typedef enum doload_mode_enum {
	Load,			/* .. loading .do file */
	List,			/* .. listing .do or .o file */
	Fix			/* .. converting .o to .do file */
} doload_mode;

extern int doload_trace;
typedef void *(*doload_entry)();

extern doload_entry ATKDoLoadI(int inFD, const char *name, char **bp, long *lenP, const char *path, ATKSymTab *ast,int debug);

struct setfixup {
    char *name;
    long *list;
    struct setfixup *next;
};

#ifdef NEED_DOLOAD_ENVIRONMENT
struct doload_environment {
    doload_mode mode;		/* operating mode */
    int fd;			/* input file descriptor */
    jmp_buf errorJump;		/* error exit */
    int problems;		/* count of problems encountered */
    struct exec header;		/* header at beginning of a.out file */
    char *text;			/* text segment */
    char *data;			/* data segment */
    struct relocation_info *rtab; /* relocation table */
    struct nlist *symtab;	/* symbol table */
    char *stringtab;		/* string table */
    struct nlist *newsym;	/* replacement symbol table */
    int newsymcount;		/* number of new symbols */
    struct setfixup *firstfixup;	/* fixups for constructors. */
};
#endif
#endif
