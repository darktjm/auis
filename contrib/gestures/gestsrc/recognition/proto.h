#include "bitvector.h"
#include "fv.h"
/* fv.c */
extern double dist_sq_threshold;
extern double se_th_rolloff;
extern FV FvAlloc(void);
extern void FvFree(FV fv);
extern void FvInit(FV fv);
extern void FvAddPoint(FV fv, int x, int y, long t);
extern Vector FvCalc(FV fv);
extern double invcos2(double m);
#include "gf.h"
/* matrix.c */
extern Vector NewVector(int r);
extern Matrix NewMatrix(int r, int c);
extern void FreeVector(Vector v);
extern void FreeMatrix(Matrix m);
extern Vector VectorCopy(Vector v);
extern Matrix MatrixCopy(Matrix m);
extern void ZeroVector(Vector v);
extern void ZeroMatrix(Matrix m);
extern void FillMatrix(Matrix m, double fill);
extern double InnerProduct(Vector v1, Vector v2);
extern void MatrixMultiply(Matrix m1, Matrix m2, Matrix prod);
extern void VectorTimesMatrix(Vector v, Matrix m, Vector prod);
extern void ScalarTimesVector(double s, Vector v, Vector product);
extern void ScalarTimesMatrix(double s, Matrix m, Matrix product);
extern double QuadraticForm(Vector v, Matrix m);
extern int DebugInvertMatrix;
extern double InvertMatrix(Matrix ym, Matrix rm);
extern Vector SliceVector(Vector v, BitVector rowmask);
extern Matrix SliceMatrix(Matrix m, BitVector rowmask, BitVector colmask);
extern Matrix DeSliceMatrix(Matrix m, double fill, BitVector rowmask, BitVector colmask, Matrix r);
extern void OutputVector(FILE *f, Vector v);
extern Vector InputVector(FILE *f);
extern void OutputMatrix(FILE *f, Matrix m);
extern Matrix InputMatrix(FILE *f);
extern double InvertSingularMatrix(Matrix m, Matrix inv);
extern void PrintVector(Vector v, const char *fmt, ...);
extern void PrintMatrix(Matrix m, const char *fmt, ...);
#include "sc.h"
/* tt.c */
/* util.c */
extern char *recog_scopy(const char *s);
extern void recog_error(const char *a, ...);
extern int DebugFlag;
extern void recog_debug(const char *a, ...);
extern char *recog_tempstring(void);
