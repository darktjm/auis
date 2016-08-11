#ifndef _txtvpse_H
#define _txtvpse_H
/* definitions used by txtvps.C that other text classes may want to use. */

#define locatag_Undef (-99999)
#define locatag_Content (1)
#define locatag_Index   (2)
struct textps_locatag {
    int type;
    char *name;
    long linenum;
    long pagenum;
    struct textps_locatag *next;
};

#endif   /* _txtvpse_H */
