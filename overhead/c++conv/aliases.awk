BEGIN {
    FS=":"
}
$1 == "Declaredin" {
    lastkey = substr($2, 1, length($2)-3);
}
$1 == "Class" {
    if($2 != lastkey) print lastkey"\t"$2;
    if(length($2)>10) {
	print $2"\t"lastkey;
    }
}
