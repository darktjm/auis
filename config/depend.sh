#!/bin/sh
if [ $# -lt 2 -o $# -gt 3 ]; then
	echo 'usage: depends ${DEPEND} ${INCLUDES} [${DEFINES}]'
	exit 1
fi
################################################
DEPEND=$1
INCLUDES="$2"
if [ $# -eq 3 ]; then
	DEFINES="$3"
else
	DEFINES=" "
fi

################################################
CFILES="`ls *.c 2> /dev/null`"
CPPFILES="`ls *.C 2> /dev/null`"

if [ "$CFILES" = "" -a "$CPPFILES"  = "" ]; then
	echo "No Dependencies to Check...continuing...."
	exit 0
fi
################################################
TMP_MKFILE=${TMPDIR:-/tmp}/Dep1_$$
C_DEPENDS=${TMPDIR:-/tmp}/Dep2_$$
DEPENDS=.depends
################################################
mv Makefile Makefile.BAK
sed -n -e '1,/##### DEPENDENCY LINE - DO NOT DELETE #####/p' Makefile.BAK > Makefile
################################################
echo > ${C_DEPENDS}
if [ "$CFILES" != "" -o "$CPPFILES" != "" ]; then
	${DEPEND} -s "##### DEPENDENCY LINE - DO NOT DELETE #####" -- ${DEFINES} -- ${INCLUDES} -f ${C_DEPENDS} ${CFILES} ${CPPFILES}
fi

sed -e 's/##### DEPENDENCY LINE - DO NOT DELETE #####//g' ${C_DEPENDS} >> Makefile

for i in ${C_DEPENDS} ${TMP_MKFILE}; do 
    rm -f $i $i.bak
done
rm -f ${DEPENDS}.bak


