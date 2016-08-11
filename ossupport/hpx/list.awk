NF<2 {
    tag=1
}


tag==1 && $3~/[DT]/ {
    print "-u "$4;
    tag=0;
}
