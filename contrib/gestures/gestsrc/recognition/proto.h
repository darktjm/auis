#include "matrix.h"
#include "bitvector.h"
#include "fv.h"
/* fv.c */
extern NO_DLL_EXPORT double dist_sq_threshold;
extern NO_DLL_EXPORT double se_th_rolloff;
extern NO_DLL_EXPORT double invcos2(double m);
#include "gf.h"
/* matrix.c */
extern NO_DLL_EXPORT void ScalarTimesMatrix(double s, Matrix m, Matrix product);
extern NO_DLL_EXPORT int DebugInvertMatrix;
extern NO_DLL_EXPORT void OutputVector(FILE *f, Vector v);
extern NO_DLL_EXPORT void OutputMatrix(FILE *f, Matrix m);
#include "sc.h"
/* util.c */
extern NO_DLL_EXPORT int DebugFlag;
