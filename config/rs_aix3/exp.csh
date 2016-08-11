#!/bin/csh -f

if("$1" == "-d") then
	shift
	set dyn=1
else
	set dyn=0
endif
set outname = $1
set libname = $outname:r
shift
set basedir = $1
shift
if($dyn) then
echo "#\!$libname.+">$outname
else
echo "#\!lib$libname.a(shr.o)">$outname
endif
exec /bin/nm -C -h $*|awk -f $basedir/etc/exp.awk>>$outname
