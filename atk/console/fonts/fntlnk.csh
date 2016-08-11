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

cd ${FONTDIR}
set nonomatch
rm -f console*.fwm

set NAMELIST=con*.fwm
foreach NAME ($NAMELIST)
	set LNAME=`echo $NAME | sed -e s+con+console+g`
	rm -f ${FONTDIR}/${LNAME}
	(set echo;${LN} ${NAME} ${FONTDIR}/${LNAME})
end

