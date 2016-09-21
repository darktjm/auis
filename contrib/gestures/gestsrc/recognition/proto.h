#include "matrix.h"
#include "bitvector.h"
#include "fv.h"
/* fv.c */
extern double dist_sq_threshold;
extern double se_th_rolloff;
extern double invcos2(double m);
#include "gf.h"
/* matrix.c */
extern void ScalarTimesMatrix(double s, Matrix m, Matrix product);
extern int DebugInvertMatrix;
extern void OutputVector(FILE *f, Vector v);
extern void OutputMatrix(FILE *f, Matrix m);
#include "sc.h"
/* util.c */
extern int DebugFlag;
