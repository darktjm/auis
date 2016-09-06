
extern int weblisten(void);
extern void webaccept(FILE *f, void (*viewforclient)(char *));
extern int webclient(char *buf, int timeout, char *rbuf, int size);
