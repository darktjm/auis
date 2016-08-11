#!/bin/csh -f
onintr error
/usr/ccs/bin/nm lib$1.so|awk -F'|' '$$8 ~ /_GLOBAL_.[ID]./ { print $$8}'>,dobj.exp||(rm -f ,dobj.exp;exit 1)
mv ,dobj.exp dobj.exp
exit 0
error:
	rm -f ,dobj.exp dobj.exp
exit 1
