#!/bin/csh -f
set name = $argv[1]
if (-e $name.sym) then
	set files=($name.sym -)
else
	set files=(-)
endif
set list=()
foreach obj ($argv:q)
	        if ("$obj:e" == "o") then 
   	             set list = ($obj $list)
  	      endif
end
nm --no-cplus -pg $list|awk 'FILENAME=="-" && $2 != "U"  {\
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
exit 0



