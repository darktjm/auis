/* Copyright 1995 Carnegie Mellon University -- All rights reserved */
#ifndef _weblink_H_
#define _weblink_H_
/** \addtogroup libhtmltext
 * @{ */
class text;

#include <link.H>
class weblink: public link{
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();

	const char * GetResolvedLink()  ;

	static weblink *GetNewWebLink(char *url)  ;
	static boolean isLinkString(char *s)  ; 
	static char *warpToURL(char *url, class view *v)  ;
	static void ReplaceTextWithLinks(class text *ptext, 
		long pos, long len) ;
	static void warpToURLinText(class text *ptext, 
		long pos, long len, class view *v) ;
};
/** @} */
#endif /* _weblink_H_ */
