/* Copyright 1995 Carnegie Mellon University -- All rights reserved */
#ifndef _webparms_h_
#define _webparms_h_

/* the www program to exec is controlled by configuration 
parameters set in system.mcr or system.h

MK_WWW (system.mcr) determines whether to build the
	www delivered with Andrew  (on by default)
ATKWWW is file name of Andrew version
	(default is (char *)environ::AndrewDir("/etc/awww")
*/

#define NCSAMETAINDEX  \
  "http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/MetaIndex.html"
#define NCSASTARTINGPOINTS \
 "http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/StartingPoints/NetworkStartingPoints.html"

#define LABELSTRING "Back:Forward:Home:Reload:Open:Save As:Clone:Close:Cancel"

#define DEFAULTHOME "http://www.cs.cmu.edu/~AUIS"

#define BROKENIMGFILE \
	((char *)environ::AndrewDir("/etc/Warning.gif"))
#define ADDRESS     "/tmp/AtkWeb"  /* addr to connect */
#define WEBPIDFILE "/tmp/web.pid"
#define WEBOPENFILE "/tmp/web.%d"
#define POLLINTERVAL 2
#define MAXCOUNT 300
#define HOTLIST ".mosaic-hotlist-default"
#define TMPHOTLIST "/tmp/hotlist.web"

#endif  /* _webparms_h_ */
