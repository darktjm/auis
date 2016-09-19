#ifndef _ATK_PROTO_H
#define _ATK_PROTO_H
/* Copyright 1993 Carnegie Mellon University All rights reserved. */

 /* Macros to make it easier to change when extern "C" is applied to .h files */

#ifdef __cplusplus
#define BEGINCPLUSPLUSPROTOS extern "C" {
#define ENDCPLUSPLUSPROTOS }
#else
#define BEGINCPLUSPLUSPROTOS
#define ENDCPLUSPLUSPROTOS
#endif
#endif /* _ATK_PROTO_H */
