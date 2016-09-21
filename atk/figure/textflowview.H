ATK_INTER
#ifndef _textflowview_H_
#define _textflowview_H_

/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
*/

/** \addtogroup libfigure
 * @{ */
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
/** @} */
#endif /* _textflowview_H_ */
