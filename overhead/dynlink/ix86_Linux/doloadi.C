#include <andrewos.h>
#include <ATKDoLoad.H>
extern "C" {
#include "doload.h"
}

int (*ATKDoLoad(const char *path, int debug))(int argc, char **argv) {
    char *bp;
    long len;
    return (int (*)(int argc, char **argv))doload(-1, (char *)path, &bp, &len, (char *)path);
}
