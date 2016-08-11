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


################################################
set nonomatch
################################################
################################################
if ( $#argv < 2 || $#argv > 3) then
	echo 'usage: depends ${DEPEND} ${INCLUDES} [${DEFINES}]'
	exit(1)
endif
################################################
set DEPEND=$1
set INCLUDES="$2"
if ($#argv == 3) then
set DEFINES="$3"
else
set DEFINES=" "
endif

################################################
set CFILES=`/bin/ls *.c`
set CPPFILES=`/bin/ls *.C`

if ( "$#CFILES" == 0 && "$#CPPFILES"  == 0 ) then
	echo "No Dependencies to Check...continuing...."
	exit(0)
endif
################################################
set TMP_MKFILE=/tmp/Dep1_$$
set C_DEPENDS=/tmp/Dep2_$$
set DEPENDS=.depends
################################################
mv Makefile Makefile.BAK
sed -n -e '1,/##### DEPENDENCY LINE - DO NOT DELETE #####/p' Makefile.BAK > Makefile
################################################
echo >! ${C_DEPENDS}
if ( "$#CFILES" != 0 || "$#CPPFILES" != 0 ) then
	${DEPEND} -s "##### DEPENDENCY LINE - DO NOT DELETE #####" -- ${DEFINES} -- ${INCLUDES} -f ${C_DEPENDS} ${CFILES} ${CPPFILES}
endif

sed -e 's/##### DEPENDENCY LINE - DO NOT DELETE #####//g' ${C_DEPENDS} >> Makefile

foreach i ( ${C_DEPENDS} ${TMP_MKFILE} )
    rm -f $i $i.bak
end
rm -f ${DEPENDS}.bak
