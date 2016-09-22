
extern NO_DLL_EXPORT int weblisten(void);
extern NO_DLL_EXPORT void webaccept(FILE *f, void (*viewforclient)(char *));
extern NO_DLL_EXPORT int webclient(char *buf, int timeout, char *rbuf, int size);
