#!/bin/csh -f
set name = $argv[1]
if (-e $name.sym) then
	set files=($name.sym -)
else
	set files=(-)
endif
nm -eg lib$name.a|nawk 'FILENAME=="-" && $2!="U" && $2!="N"  {\
if ($3 ~ /[_A-Za-z0-9]*[$][_A-Za-z0-9]*/ || $3 ~ /__C*[0-9][0-9]*[_A-Za-z][_A-Za-z0-9]*/ || $3 ~ /[A-Za-z][A-Za-z0-9]*_ATKregistry_/ || $3 ~ /_$_[0-9][0-9]*[_A-Za-z][_A-Za-z0-9]*/)  print $3;\
else later[$3]=$3;\
}\
FILENAME != "-" {\
extra[$1]=$1;\
}\
END {\
for (i in extra) {\
	for(j in later) {\
		if (later[j] ~ extra[i]) print later[j];\
	}\
}\
}' $files>,$name.exp
if $status then
    rm -f ,$name.exp
    exit $status
endif
mv ,$name.exp $name.exp
exit 0



