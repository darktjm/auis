#define GRIPEADDRESS	"postmaster"

#define FOLDERALIASFILE	"~/.folderaliases"
#define FAKEDOMFILE "~/.fakedomains"
#define KILLFILE "~/.graveyard"

#define MAXADDR		500
#define MAXFILENAME	1024
#define MAXFOLDERNAME	1024
#define MAXDEALIASES	25

#define MAXSUBJ         29
#define MAXFROM         29

#define FLAG_DUP	1
#define FLAG_KILLED     2

#define AMS_REPLY_GRIPE	(-12347)
#define AMS_REPLY_POST  (-12348)

#define syserr(msg,fn) error("%s %s: %s",msg,fn,sys_errlist[errno])

