BEGIN {}
$2 ~ /^[A-TV-Z]$/  && $1  ~ /^[^.][a-zA-Z0-9_.]*[ 	]*$/ {
 s=$1
if(saw[s]=="") print s;
    saw[s]=1;
}
