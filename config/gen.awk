# Awk script to compute the classes referenced/defined
# by one or more C++ object files.
# USE:
#	nm a.o b.o etc...|awk -f thisfile
# OUTPUT:
# definedclass1
# definedclass2
# etc...
# ===
# referencedclass1
# referencedclass2
# etc...
# ASSUMPTIONS:
#   SYSV or BSD style nm output.
#   Class names must start with an alphabetic character
#   and be composed of alpanumeric characters only.
#   Symbols must not be truncated.
#   Data symbol names must not be mangled.
#   The first construct of the form:
#     __[0-9][0-9]*[a-zA-Z][a-zA-Z0-9]* in any undefined
#   symbol is assumed to refer to the named class
#   immediately following the first number.  This number
#   is assumed to be the length of the class name.
#   A defined symbol of the form [a-zA-Z][a-ZA-Z0-9]*_ATKregistry_
#   is taken to indicate that the .o file defines the named
#   class.
#   The compiler does not add any characters besides _
#   to the front of data symbols, and no characters are
#   added to the end of a data symbol.

BEGIN {
    FS="|";
}

NF>=7 && $3 == "extern" && $1 ~ /^[_]*[a-zA-Z][a-zA-Z0-9]*_ATKregistry_[ 	]*$/ {
    n=split($1, foo, "_");
    for(i=1;i<=n && foo[i]=="" ;i++);
    if ($7 == "" && USED[foo[i]]!=2) USED[foo[i]]=1;
    else {
	PROVIDED[foo[i]]=1;
	USED[foo[i]]=2;
    }
    next
}

NF>=7 && $3 == "extern" && $1 ~ /[^_]*__[0-9][0-9]*[a-zA-Z][a-zA-Z0-9]*/ && $7 == "" {
    n=split($1, foo, "_");
    for(i=1;i<=n;i++) {
	if (foo[i]=="" && foo[i+1] ~ /[0-9][0-9]*[a-zA-Z][a-zA-Z0-9]*/) {
	    i++;
	    bar=foo[i];
	    num=0;
	    while (substr(bar, 1, 1) ~ /[0-9]/) {
		num=num*10+substr(bar, 1, 1);
		bar=substr(bar, 2, length(bar)-1);
	    }
	    bar=substr(bar, 1, num);
	    if(USED[bar]!=2) USED[bar]=1;
	    break;
	}
    }
    next
}

NF==1 {
    split($0, eek, " ")
      if (eek[1]=="U") {
	  if (eek[2] ~ /^[_]*[a-zA-Z][a-zA-Z0-9]*_ATKregistry_[ 	]*$/) {
	      n=split(eek[2], foo, "_");
	      for(i=1;i<=n && foo[i]=="";i++);
	      if(USED[foo[i]]!=2) USED[foo[i]]=1;
	  } else if (eek[2] ~ /[^_]*__[0-9][0-9]*[a-zA-Z][a-zA-Z0-9]*/) {
	      n=split(eek[2], foo, "_");
	      for(i=1;i<=n;i++) {
		  if (foo[i]=="" && foo[i+1] ~ /[0-9][0-9]*[a-zA-Z][a-zA-Z0-9]*/) {
		      i++;
		      bar=foo[i];
		      num=0;
		      while (substr(bar, 1, 1) ~ /[0-9]/) {
			  num=num*10+substr(bar, 1, 1);
			  bar=substr(bar, 2, length(bar)-1);
		      }
		      bar=substr(bar, 1, num);
		      if(USED[bar]!=2) USED[bar]=1;
		      break;
		  }
	      }
	  }
	      
      } else {
	  if (eek[2]=="D" && eek[3] ~ /^[_]*[a-zA-Z][a-zA-Z0-9]*_ATKregistry_[ 	]*$/) {
	      n=split(eek[3], foo, "_");
	      for(i=1;i<=n && foo[i]=="";i++);
	      PROVIDED[foo[i]]=1;
	      USED[foo[i]]=2;
	  }
      }
}

END {
    for (i in PROVIDED) {
	print i
    }
    print "==="
    for (i in USED) {
	if(USED[i]==1) print i;
    }
}
