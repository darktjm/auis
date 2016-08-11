#!/bin/csh -f
## ###################################################################### ##
##         Copyright IBM Corporation 1988,1991 - All Rights Reserved      ##
##        For full copyright information see:'andrew/config/COPYRITE'     ##
## ###################################################################### ##
# $Disclaimer: 
# Permission to use, copy, modify, and distribute this software and its 
# documentation for any purpose and without fee is hereby granted, provided 
# that the above copyright notice appear in all copies and that both that 
# copyright notice and this permission notice appear in supporting 
# documentation, and that the name of IBM not be used in advertising or 
# publicity pertaining to distribution of the software without specific, 
# written prior permission. 
#                         
# THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
# TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
# HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
# 
#  $



#
# This script is only needed for files that are used by WM.  
# The fonts.alias file is used on X11 systems
#

set FONTDIR=$1
set LN="$2"

pushd ${FONTDIR}
set nonomatch
rm -f andy* courier* helvetica* symbol* times*

foreach REAL (cou hel symb tim)
	switch (${REAL})
		case "cou":
			set LINK1="andytype"
			set LINK2="courier"
			breaksw
		case "hel":
			set LINK1="andysans"
			set LINK2="helvetica"
			breaksw
		case "symb":
			set LINK1="andysymbol"
			set LINK2="symbol"
			breaksw
		case "tim":
			set LINK1="andy"
			set LINK2="times"
			breaksw
		default:
			echo "ERROR LINKING FONTS (${REAL})"
			exit(1)
	endsw
	set NAMELIST=$REAL*
	foreach NAME ($NAMELIST)
		set LNAME=`echo $NAME | sed -e s+$REAL+$LINK1+g`
		rm -f ${LNAME}
		(set echo;${LN} ${NAME} ${LNAME})
		set LNAME=`echo $NAME | sed -e s+$REAL+$LINK2+g`
		rm -f ${LNAME}
		(set echo;${LN} $NAME ${LNAME})
end
end
popd

