
# 		Copyright IBM Corporation 1991
# 
#                       All Rights Reserved
# 
BEGIN{
	vcount = 0
	class= "! No Control Button Defined"
	classoverride = 0
	lastfunc = ""
}
{
if ($1 == ">OBJ<" && NF ==2){
	typelist[$2] = $2
	 type=$2
}
if ($1 == ">VIEW<" && NF ==2){
	typelist[$2] = $2
	view = $2
}
if ($1 == ">REF<" ) {
	if(NF == 2) rname = $2
	else rname = substr($0,7,length - 6);
	if(rname != "" && rname != " "){
	    if(view == "controlV"){
		cvlist[rname] = rname
	    }
	    else {
		oblist[rname] = rname
		str = ""
		slen = 0
		for(i = 1; i <= length(rname); i++){
			cc = substr(rname,i,1)
			if(cc < "0" || (cc > "9" && cc < "A") || (cc > "Z" && cc < "a") || cc > "z") {
				str = str  "_"
				if(slen == 0) slen = i
			}
			else {
				if(slen > 0 && (cc < "0" || cc > "9")) slen = 0
				str = str  cc
		}
		}
		obfunc[rname] = str
		if(type == "value") {
			num = 0
			if(slen > 0){
				num = substr(str,slen + 1 ,10)
				 str = substr(str,1,slen - 1)
				}
			valuelist[rname] = str
			uval[str] = str
			numlist[rname] = num
			vcount++
			}
		obtype[rname] = type
		obview[rname]=view
	    }
	}
}
if(NR == 1 && $1=="CLASSNAME=" && $3== "FUNCTIONAME=" && NF=4){
	class = $2
	funcs[$4] = $4
	funcdefined++
	classoverride = 1
}
if($2 == "<class>" && $1=="[string]" && NF==3 && view == "controlV"){
	newclass = substr($3,2,(length($3) - 2))
	if(class == "! No Control Button Defined")
		class = newclass
	if(class == newclass){
		if(lastfunc != ""){
			funcs[lastfunc] = lastfunc
			funcdefined++
			lastfunc = ""
		}
	}
	else if(classoverride == 0){
		class = "! More Than one controller classes defined"
		classoverride = 2
		}
	}
if($2 == "<function>" && $1=="[string]" && NF==3 && view == "controlV"){
	lastfunc = substr($3,2,(length($3) - 2))
	}
}
END {
    if(substr(class,1,1) == "!" ) {
	print class
	}
    else {
	printf "%s.H\n",class
	printf "/* user code begins here for %s */\n","HeaderInfo"
	printf "/* user code ends here for %s */\n","HeaderInfo"
	printf "#include <observable.H>\n"
	printf "/* user code begins here for %s */\n","includes"
	printf "/* user code ends here for %s */\n","includes"
	printf "class %s : public observable { \npublic :\n",class
	printf "	virtual ATKregistryEntry *ATKregistry();\n"
	printf "	virtual ~%s();\n",class
	printf "	%s();\n",class
	printf "/* user code begins here for %s */\n","classprocedures"
	printf "/* user code ends here for %s */\n","classprocedures"
	printf "\n\tvoid ObservedChanged( class observable * observed, long status);\n"
	printf "/* user code begins here for %s */\n","overrides"
	printf "/* user code ends here for %s */\n","overrides"
	for(i in oblist){
		printf "	class %s *%s;\n",obtype[i],obfunc[i]
		printf "	class %s *%sView;\n",obview[i],obfunc[i]
	}
	printf "/* user code begins here for %s */\n","class data"
	printf "/* user code ends here for %s */\n","class data"
	printf "	class view *v;\n\tclass arbiterview *arbv;\n\tclass %s *next;\n};\n\n",class
	printf "! THIS IS THE END OF THIS FILE !!!\n%s.C\n",class
	printf "/* user code begins here for %s */\n","HeaderInfo"
	printf "/* user code ends here for %s */\n","HeaderInfo"
	printf "#include <andrewos.h>\n#include <proctable.H>\n#include <view.H>\n#include <arbiterview.H>\n#include <%s.H>\n",class
	for(i in typelist){
		printf "#include <%s.H>\n",typelist[i]
	}
	printf "/* user code begins here for %s */\n","includes"
	printf "/* user code ends here for %s */\n","includes"
	printf "static void initself(class %s *self,class view *v);\n",class
	printf "\nstatic class %s *first%s;\n",class,class
	printf "static class %s *FindSelf(class view *v)\n\t{\n	class %s *self,*last = NULL;\n",class,class
	printf "\tclass arbiterview *arbv =arbiterview::FindArb(v);\n"
	printf "\tfor(self= first%s; self != NULL; self = self->next){\n\t\tif(self->arbv == arbv) return self;\n\t\tlast = self;\n\t\t}\n",class
	printf "\tself = new class %s;\n\tself->arbv = arbv;\n\tinitself(self,v);\n\tif(last == NULL) first%s = self;\n",class,class
	printf "\telse last->next = self;\n"
	printf "\tself->arbv->AddObserver(self);\n\treturn self;\n}\n"
	if(vcount){
		for(i in uval){
			printf "static void %sCallBack(class %s *self,class value *val,long r1,long r2)\n{\n", uval[i],class
			printf "if(r2 == value_OBJECTDESTROYED) {\n"
			for(j in valuelist){
				if(valuelist[j] == uval[i]){
					printf "\tif(self->%s == val) ",obfunc[j]
					printf "self->%s = NULL;\n",obfunc[j]
				}
			}
			printf "}\n",uval[i]
			printf "{\n/* user code begins here for %sCallBack */\n",uval[i]
			printf "/* user code ends here for %sCallBack */\n}\n}\n",uval[i]
			}
		}
	printf "static void initself(class %s *self,class view *v)\n{\n",class
	printf "\tself->v = v;\n"
	for(i in oblist){
		printf "\tself->%sView = (class %s *)arbiterview::GetNamedView(v,\"%s\");\n",obfunc[i],obview[i],oblist[i]
		printf "\tself->%s = (class %s *)arbiterview::GetNamedObject(v,\"%s\");\n",obfunc[i],obtype[i],oblist[i]
		if(obtype[i] == "value"){
			printf "\tif(self->%s) self->%s->AddCallBackObserver(self,(value_fptr)%sCallBack,%d);\n",obfunc[i], obfunc[i], valuelist[i],numlist[i]
		}
		else {
			printf "\tif(self->%s) self->%s->AddObserver(self);\n", obfunc[i],obfunc[i]
		}
		printf "\tif(self->%sView) self->%sView->AddObserver(self);\n", obfunc[i],obfunc[i]

	}
	print "}"
	for(i in funcs){
		printf "void %s_%s(class ATK *av,long dat)\n{\n",class,funcs[i]
		printf "class view *v=(class view *)av;\nclass %s *self;\nif((self = FindSelf(v)) == NULL) return;\n",class 
		printf "/* user code begins here for %s_%s */\n",class,funcs[i]
		printf "/* user code ends here for %s_%s */\n}\n",class,funcs[i]
	}
	printf "void %s::ObservedChanged(class observable *observed,long status)\n",class
	printf "{\n"
	printf "/* user code begins here for %s */\n","ObservedChanged"
	printf "/* user code ends here for %s */\n","ObservedChanged"
	printf "if(observed == (class observable *) this->arbv){\n"
	printf "\tif (status == observable::OBJECTDESTROYED) this->arbv = NULL;\n"
	printf "\t else initself(this,this->v);\n}\n"
	printf "if (status == observable::OBJECTDESTROYED) {\n"
	for(i in oblist){
	     if(obtype[i] != "value"){
		printf "\tif (observed == (class observable *) this->%s) this->%s=NULL;\n", obfunc[i], obfunc[i];
		}
	     printf "\tif (observed == (class observable *) this->%sView) this->%sView=NULL;\n", obfunc[i], obfunc[i];
	}
	printf "}\n"
	printf "}\n"
	printf "boolean InitializeClass()\n{\n"
	printf "class ATKregistryEntry *viewtype = ATK::LoadClass(\"view\");\n"
	printf "first%s = NULL;\n",class
	for(i in funcs){
		printf  "proctable::DefineProc(\"%s-%s\",(proctable_fptr)%s_%s, viewtype,NULL,\"%s %s\");\n", class,funcs[i],class,funcs[i],class,funcs[i]
		}
	printf "/* user code begins here for %s */\n","InitializeClass"
	printf "/* user code ends here for %s */\n","InitializeClass"
	print "return TRUE;\n}"
	printf "\nATKdefineRegistry(%s,observable, InitializeClass);\n",class
	printf "%s::~%s()\n",class, class
	printf "{\n"
	for(i in oblist){
		if(obtype[i] == "value"){
			printf "\tif(this->%s) this->%s->RemoveCallBackObserver(this);\n",obfunc[i], obfunc[i]
		}
	}
	printf "/* user code begins here for %s */\n","FinalizeObject"
	printf "/* user code ends here for %s */\n","FinalizeObject"
	print "}"
	printf "%s::%s()\n{\n",class,class
	for(i in oblist){
		printf "this->%s = NULL;\n",obfunc[i]
		printf "this->%sView = NULL;\n",obfunc[i]
	}
	printf "this->v = NULL;\nthis->next = NULL;\n"
	printf "/* user code begins here for %s */\n","InitializeObject"
	printf "/* user code ends here for %s */\n","InitializeObject"
	printf "THROWONFAILURE(TRUE);}\n"
	printf "/* user code begins here for %s */\n","Other Functions"
	printf "/* user code ends here for %s */\n","Other Functions"
	printf "! THIS IS THE END OF THIS FILE !!!\n"	
    }
}

