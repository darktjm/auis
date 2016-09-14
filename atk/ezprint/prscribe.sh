#!/bin/sh
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
# Backward compatibility -- a program that acts like the old printscribe,
#   but calls ezprint to do the real work.
# Ezprint understands most of the same arguments as printscribe, but the
#   -d changes to a -z, sigh...

DELETE=""
PASSTHRU=""

for x; do
    case "$x" in
	-d)
	    DELETE="-z"
	    ;;
	*)
	    PASSTHRU="$PASSTHRU $x"
	    ;;
    esac
done

exec ezprint $DELETE $PASSTHRU
