BEGIN {FS = "|"}
$3 == "extern" && $7 != ""  && $1 ~ /^[^.][a-zA-Z0-9_.]*[ 	]*$/ {
    if(saw[$1]=="") print $1;
    saw[$1]=1;
}
