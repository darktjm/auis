#ifndef _mailobj_H_
#define _mailobj_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


#include <sbutton.H>
class mailobj : public  sbutton {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	mailobj();
      
	virtual ~mailobj();
      static void TranslateFrom64(unsigned char *data, int len, FILE *fp);
      static void TranslateFromQP(unsigned char *data, int len, FILE *fp);
      static void ToQP(unsigned char *data, int len, FILE *fp);
     
       long Read (FILE *file, long id)  ;
       long Write (FILE *file, long writeid, int level)  ;
       long WriteOtherFormat(FILE *file, long writeid, int level, int usagetype, char *boundary)  ;
       const char * ViewName()  ;
     
      virtual void ReadAlienMail(const char *ContentType, const char *ContentEncoding, FILE *fp, boolean StopAtEndData);
      virtual void RunMetamail();
      virtual void SetTextInsertion(class text *t, class environment *env);
    
      char *ContentType;
      unsigned char *RawData;
      class text *t;
      class environment *env;
      int bytesadded, RawBytes;
      int EncodingCode, EncodingNeeded;
      FILE *fp;
      char *EncodingName;
};

/*  Codes for EncodingCode variable */
#define ENC_NONE 0
#define ENC_B64 1
#define ENC_QP 2
#define ENC_UNKNOWN 3


#endif /* _mailobj_H_ */
