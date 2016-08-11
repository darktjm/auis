#!/bin/sh
# Output a file defining a main function which registers each
# of the named classes.
# usage: mkdynmain class1 class2 etc....


echo "#include <andrewos.h>"
echo "#include <ATK.H>"
echo "#include <stdio.h>"

if [ "x$1" = "x-" ]; then
	shift
	main=$1
	shift
else
	main=main
fi

echo 'static void RegisterClasses() {'
for name in $*; do
	echo "ATKregister($name);"
done
echo '}'

echo 'extern "C" '"int $main(int argc, char **argv) {"
echo '  RegisterClasses();'
echo '  return 0;'
echo '}'


