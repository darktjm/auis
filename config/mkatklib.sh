#!/bin/sh
archive=yes
shared=no
result=0
longnames=no
expprog=""
if [ "x$1" = "x-as" ]; then
   shift
   shared=yes
else
   if [ "x$1" = "x-s" ]; then
      shift
      archive=no
      shared=yes
   fi
fi

if [ "x$1" = "x-d" ]; then
   shift
   expprog="$1 $expprog"
   shift
fi

name="$1"
shift
basedir="$1"
shift
linkprefix="$1"
shift
if [ $archive = yes ]; then
	echo "making archive lib"$name.a	
	rm -f lib$name.a
	list=""
	for ename in $*; do
	      case "$ename" in
		*/*.o)	longnames=yes
			list="$ename $list"
			;;
		*.o)	list="$ename $list"
			;;
	      esac
	done
	# GNU ar is a bit too stupid to handle long names
	# that include directories.  If this is the case,
	# copy them to a tmp directory, then ar them from there.
	if [ $longnames = yes ]; then
		echo "mkatklib:  hack around GNU ar bug with long names..."
		tmpdir=mkatklib.$$
		mkdir $tmpdir
		cp $list $tmpdir
		(cd $tmpdir; ar rvs ../lib$name.a *.o)
		rm -rf $tmpdir
	else
		# Normal case.
		ar rvs lib$name.a $list
	fi
fi
if [ $shared = yes ]; then
	if [ ! -x $basedir/etc/mkatkshlib ]; then
		echo "Couldn't find $basedir/etc/mkatkshlib or it wasn't executable."
		echo "Perhaps ATK or the OS doesn't support shared libraries on this system?"
		exit 1
	fi
	$basedir/etc/mkatkshlib $name $basedir "$linkprefix" $*
	if [ $? -ne 0 ]; then
		echo  "Couldn't create shared library for $name."
		exit 2
	fi
fi
if [ "$expprog" != "" ]; then
	$expprog $name $basedir "$linkprefix" $*
	if [ $? != 0 ]; then
		echo "$expprog failed for $name."
		result="`expr $result - 4`"
	fi
fi
exit $result



