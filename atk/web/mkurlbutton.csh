#!/bin/csh -f

# Simple script to make a urlbutton datastream.
#
# USAGE:  mkurlbutton [-t] [url-spec]
#
# If url-spec is missing it will read a single line from stdin.
# This script does not yet attempt to check the validity of the url.
#
# -t generates a text inset with a title and a button.

set dotext=0
if ("$1" == "-t") then
	set dotext=1
	shift
endif

switch ("$#argv")
  case 0:
	set URL="$<"
	breaksw
  case 1:
	set URL="$1"
	breaksw
  default:
	echo "USAGE:  mkurlbutton [url-spec]"
	exit 1
endsw

if ($dotext) then
	echo "\begindata{text,1}"
	echo "\textdsversion{12}"
	echo "$URL"
	echo ""
	echo "\begindata{urlbutton,2}"
	echo "\url:$URL"
	echo "\enddata{urlbutton,2}"
	echo "\view{urlbuttonview,2,0,0,0}"
	echo "\enddata{text,1}"
else
	echo "\begindata{urlbutton,1}"
	echo "\url:$URL"
	echo "\enddata{urlbutton,1}"
endif
