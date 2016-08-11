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


# Note: This only works if more than one library is passed to nm
# Usage: nm -g lib1 lib2 lib3... | awk -f filename
# Ouput - similar to nm -og lib1 lib2 lib3
# Differences: hidden 15th character in objname shown
BEGIN {
	lib=""
	obj=""
}
/^[^ \t]*\.a:/ { # print "library", $1; 
		lib=$1 ;
		obj="" ;
		next}
/^[^ \t]*\:/ { # print "object", $1; 
		 obj=$1; 
		next}
(NF != 0) {print lib obj "\t" $0}
