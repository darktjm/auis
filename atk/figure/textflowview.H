ATK_INTER
#ifndef _textflowview_H_
#define _textflowview_H_

/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <figview.H>

class textflowview : public figview {
    public:
      virtual ATKregistryEntry *ATKregistry();

      textflowview();
      virtual ~textflowview();

      void BuildToolList(struct figtoolview_tool_t *&list, int &listnum, int &listsize);
      void PrintPSDoc(FILE *outfile, long pagew, long pageh);
      void *GetPSPrintInterface(const char *printtype);

      virtual long BuildPlan(struct textps_layout_plan **planptr, long pagewidth, long pageheight);
};

#endif /* _textflowview_H_ */
