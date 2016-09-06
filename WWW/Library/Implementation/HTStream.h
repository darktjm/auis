/*                                                        The Generic Stream Class Definition
                                 STREAM OBJECT DEFINITION
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   A Stream object is something which accepts a stream of text. See also the Structured
   stream definition.
   
   The creation methods will vary on the type of Stream Object.  All creation methods
   return a pointer to the stream type below.
   
    As you can see, but the methods used to write to the stream and close it are pointed
   to be the object itself.
   
   This module is implemented by HTStream.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTSTREAM_H
#define HTSTREAM_H


typedef struct _HTStream HTStream;
/*

   These are the common methods of all streams.  They should be self-explanatory, except
   for end_document which must be called before free.  It should be merged with free in
   fact:  it should be dummy for new streams.
   
   NOTE: The put_block method was write, but this upset systems which had macros for
   write()
   
   NOTE: The methods _free and abort in the stream are now integers and not void anymore.
   The general return codes from the methods are:
   
      Error: EOF
      
      OK: >=0
      
   The positive return codes can be used freely by the streams.  An explanation on an
   error occured can be parsed using the Error Module.
   
 */
typedef struct _HTStreamClass {

        char*  name;                            /* Just for diagnostics */
                
        int (*_free) (
                HTStream*       me);

        int (*abort) (
                HTStream*       me,
                HTError         e);
                
        void (*put_character) (
                HTStream*       me,
                char            ch);
                                
        void (*put_string) (
                HTStream*       me,
                CONST char *    str);
                
        void (*put_block) (
                HTStream*       me,
                CONST char *    str,
                int             len);
                
                
} HTStreamClass;

#endif /* HTSTREAM_H */
/*

   end of HTStream.h */
