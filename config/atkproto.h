#ifndef _ATK_PROTO_H
#define _ATK_PROTO_H
/* Copyright 1993 Carnegie Mellon University All rights reserved. */

/** \addtogroup libatkos
 * @{ **/
 /* Macros to make it easier to change when extern "C" is applied to .h files */

#ifdef __cplusplus
#define BEGINCPLUSPLUSPROTOS extern "C" {
#define ENDCPLUSPLUSPROTOS }
#else
#define BEGINCPLUSPLUSPROTOS
#define ENDCPLUSPLUSPROTOS
#endif

/* Macros used in definitions and prototypes to add or remove warnings */
#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#define PRINTF_LIKE(f,a) __attribute__((__format__(printf,f,a)))
#define SCANF_LIKE(f,a) __attribute__((__format__(scanf,f,a)))
#define EXIT_LIKE __attribute__((__noreturn__))
#define NO_DLL_EXPORT __attribute__((visibility("hidden")))
#else
#define UNUSED /* UNUSED */
#define PRINTF_LIKE(f,a)
#define SCANF_LIKE(f,a)
#define EXIT_LIKE /* NORETURN */
#define NO_DLL_EXPORT
#endif

/** @} */
#endif /* _ATK_PROTO_H */
