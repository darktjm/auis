#!/bin/csh -f
foreach name ($argv:q)
if (-e $name.sym) then
	set files=($name.sym -)
else
	set files=(-)
endif
nm --no-cplus -pg lib$name.a|awk 'FILENAME=="-" && $2 != "U"  {\
if ($3 ~ /__[0-9][0-9]*[_A-Za-z][_A-Za-z0-9]*/ || $3 ~ /_[A-Za-z][A-Za-z0-9]*_ATKregistry_/ || $3 ~ /__$_[0-9][0-9]*[_A-Za-z][_A-Za-z0-9]*/)  print $3;\
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
end
exit 0



