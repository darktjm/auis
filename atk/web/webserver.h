
extern int weblisten();
extern void webaccept(FILE *f, void (*viewforclient()));
extern int webclient(char *buf, int timeout, char *rbuf, int size);
