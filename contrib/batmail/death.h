#define MAXKILLAGE (60*60*24*14) /* two weeks */

#define death_SUBJ 	0
#define death_FROM 	1

#define death_HASHED	0
#define death_REGEXP	1

struct killSpec {
    int field;
    int method;
    struct hshbuf *contents;	/* either this or regexp will be NULL */
    struct regexp *regexp;
    char *regexpSrc;		/* string the regexp was compiled from */
    struct killSpec *next;	/* actually in a list (shhhh!) */
    long timestamp;
    bool delete;		/* delete on next write */
    bool saved;			/* FALSE if created, but not written yet */
};

#define MAXCAPTION 100

#define death_Read(fn) death_Update(fn)

int death_Update(),death_Write(); /* return # specs discarded */
void death_SetFolder();
struct killSpec *death_KillBuf(),*death_KillMsg();

int death_MsgKilled(), death_MsgKilledBy();
