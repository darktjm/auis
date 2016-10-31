#/* ********************************************************************** *\
# *         Copyright IBM Corporation 1991 - All Rights Reserved           *
# *        For full copyright information see:'andrew/doc/COPYRITE'        *
#\* ********************************************************************** */

BEGIN{
      vcount = 0
      class= "! No Control Button Defined"
      classoverride = 0
      lastfunc = ""
      initing = 1
      acount = 0;	# number of objects
      bcount = 0;
      dclview = "";
      dclclass = "";
      funcdefined = 0;
      obcount = 0;
      childflag = "FALSE";
      ist = "nada";
      dclfunc = "InitApp";
   }
{
    if(initing == 1){
	if($1 == "DECLARE"){
	    if($3 == "long"){
		declarenm[acount] = "long "   $2   ";"
		  readnm[acount] = "adew_ReadLong(" "this->" $2 ",file,count);"
		  writenm[acount] =  "adew_WriteLong(" "this->" $2 ",file);"
		  initnm[acount] =  "adew_InitLong(" "this->" $2 ");"
		  bcount++
		  acount++
	    }
	    if($3 == "string"){
		declarenm[acount] = "char "   $2   "["  $4 "];"
		  readnm[acount] = "adew_ReadString(" "this->" $2 ",file,count);"
		  writenm[acount] =  "adew_WriteString(" "this->" $2 ",file);"
		  initnm[acount] =  "adew_InitString(" "this->" $2 ");"
		  bcount++
		  acount++
	    }
	    if($3 == "float"){
		declarenm[acount] = "float "   $2   ";"
		  readnm[acount] = "adew_ReadFloat(" "this->" $2 ",file,count);"
		  writenm[acount] =  "adew_WriteFloat(" "this->" $2 ",file);"
		  initnm[acount] =  "adew_InitFloat(" "this->" $2 ");"
		  bcount++
		  acount++
	    }
	    if($3 == "longarray"){
		declarenm[acount] = "long "   $2   "["  $4 "];"
		  readnm[acount] = "adew_ReadLongArray(" "this->" $2 ",file,count," $4 ");"
		  writenm[acount] =  "adew_WriteLongArray(" "this->" $2 ",file," $4 ");"
		  initnm[acount] =  "adew_InitLongArray(" "this->" $2 "," $4 ");"
		  bcount += $4
		  acount++
	    }
	    if($3 == "stringarray"){
		declarenm[acount] = "char  "   $2   "["  $4 "][" $5 "];"
		  readnm[acount] = "adew_ReadStringArray(" "this->" $2 ",file,count," $4 ");"
		  writenm[acount] =  "adew_WriteStringArray(" "this->" $2 ",file," $4 ");"
		  initnm[acount] =  "adew_InitStringArray(" "this->" $2 "," $4 ");"
		  bcount += $4
		  acount++
	    }
	    if($3 == "viewname")
		dclview = $2;
	    if($3 == "funcname")
		dclfunc = $2;
	    if($3 == "classname"){
		dclclass = $2 ;
		class = $2;
		classoverride = 1;
	    }
	    if($3 == "writechild")
		childflag = $2
	    next
	}
	else  {
	    initing = 2
	}
    }
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
	  obcount++
    }
}
    }
    if($2 == "<class>" && $1=="[string]" && NF==3 && view == "controlV"){
	newclass = substr($3,2,(length($3) - 2))
	  if(class == "! No Control Button Defined")
	      class = newclass
		if(newclass == dclview){
		    if(lastfunc != ""){
			funcs[lastfunc] = lastfunc
			  funcdefined = 1
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
	dclass = class
	  if(dclview == "")
	      class = class "v"
	  else class = dclview;
#write dataobj .H file
printf "%s.H\n",dclass
printf "/* user code begins here for HeaderInfo */\n"
printf "/* user code ends here for HeaderInfo */\n"
printf "#include <andrewos.h>\n"
printf "#include <arbiter.H>\n"
printf "class %s : public arbiter{ \n",dclass
printf "public:\n"
printf "TRACED_CLASS_NODEST(%s);\n",dclass
printf "/* user code begins here for methods*/\n"
printf "/* user code ends here for methods */\n"
printf "    const char *ViewName();\n"
printf "    long ReadSup (FILE *file, long id);\n"
printf "    long WriteSup (FILE *file, long writeid, int level);\n"
printf "     void ReadObjects() ;\n"
printf "/* user code begins here for overrides */\n"
printf "/* user code ends here for overrides */\n"
printf "    %s();\n",dclass
printf "/* user code begins here for classprocedures */\n"
printf "/* user code ends here for classprocedures */\n"
printf "/* user code begins here for macromethods */\n"
printf "/* user code ends here for macromethods */\n"
for(i = 0; i < acount; i++) print "    " declarenm[i];
printf "/* user code begins here for classdata */\n"
printf "/* user code ends here for classdata */\n"
printf "};\n"
printf "! THIS IS THE END OF THIS FILE !!!\n%s.C\n",dclass
#write dataobj .C file
printf "#include <andrewos.h>\n"
printf "/* user code begins here for HeaderInfo */\n"
printf "/* user code ends here for HeaderInfo */\n"
printf "#include <%s.H>\n",dclass
printf "#include <%s.fh>\n",dclass
printf "#include <dataobject.H>\n"
printf "#include <adew.h>\n"
printf "/* user code begins here for includes */\n"
printf "/* user code ends here for includes */\n"
printf "const char *%s::ViewName()\n",dclass
printf "{\n"
printf "    return \"%s\";\n",class
printf "}\n"
printf "void %s::ReadObjects()\n",dclass
printf "{\n"
printf "/* user code begins here for ReadObjects */\n"
printf "/* user code ends here for ReadObjects */\n"
printf "}\n"
printf "ATKdefineRegistryNoInit(%s,arbiter)\n", dclass
printf "%s::%s()\n",dclass, dclass
printf "{\n"
printf "this->SetDefaultStream(defaultstr);\n"
printf "this->SetWriteChild(%s);\n",childflag
# series of generated init calls are here
for(i = 0; i < acount; i++) print "    " initnm[i];
printf "/* user code begins here for InitializeObject */\n"
printf "/* user code ends here for InitializeObject */\n"
printf "THROWONFAILURE(TRUE);\n"
printf "}\n"
printf "long %s::ReadSup(FILE *file, long id)\n",dclass
printf "{\n"
printf "    long c,count,extradata;\n"
printf "    if((c = getc(file)) != '@'){\n"
printf "	ungetc(c,file);\n"
printf "	return dataobject::NOREADERROR;\n"
printf "    }\n"
# read the number of data elements to be read 
print "    fscanf(file,\"%ld\\n\",&count);"
# read known data elements
for(i = 0; i < acount; i++) print "    " readnm[i];
# read data saved by newer versions that this version doesn't know about
print "    if((extradata = count) > 0)" # may cause compiler warning 
print "        adew_ReadExtraData(file,count);"
printf "/* user code begins here for ReadSup */\n"
printf "/* user code ends here for ReadSup */\n"
printf "    return dataobject::NOREADERROR;\n"
printf "}\n"
printf "long %s::WriteSup(FILE *file ,long writeid,int level)\n",dclass
printf "{\n"
printf "/* user code begins here for Preparing_To_Write */\n"
printf "/* user code ends here for Preparing_To_Write */\n"
printf "    fprintf(file,\"@%d\\n\");/* the number of data elements */\n",bcount
for(i = 0; i < acount; i++) print "    " writenm[i];
printf "/* user code begins here for WriteSup */\n"
printf "/* user code ends here for WriteSup */\n"
printf "    return(TRUE);\n"
printf "}\n"
printf "/* user code begins here for %s */\n","Other Functions"
printf "/* user code ends here for %s */\n","Other Functions"
printf "! THIS IS THE END OF THIS FILE !!!\n"
# write view ch file
printf "%s.H\n",class
printf "/* user code begins here for %s */\n","HeaderInfo"
printf "/* user code ends here for %s */\n","HeaderInfo"
printf "#include <adew.h>\n"
printf "#include <arbiterview.H>\n"
printf "class %s : public arbiterview { \npublic:\n\tTRACED_CLASS(%s);\n",class,class
printf "	%s();\n",class
printf "/* user code begins here for %s */\n","classprocedures"
printf "/* user code ends here for %s */\n","classprocedures"
printf "\n\tvoid ObservedChanged( struct observable * observed, long status );\n"
printf "\tvoid InitCell(struct celview *cv);\n"
printf "\tclass view *GetApplicationLayer();\n"
printf "/* user code begins here for %s */\n","overrides"
printf "/* user code ends here for %s */\n","overrides"
printf "\n\tvoid %s();\n",dclfunc
printf "/* user code begins here for %s */\n","methods"
printf "/* user code ends here for %s */\n","methods"
printf "long InitCount;\n"
for(i in oblist){
    printf "	class %s *%s;\n",obtype[i],obfunc[i]
    printf "	class %s *%sView;\n",obview[i],obfunc[i]
}
printf "    struct adew_array AdewArray[%d];\n", obcount + 1
printf "/* user code begins here for %s */\n","classdata"
printf "/* user code ends here for %s */\n","classdata"
printf "	class view *v;\n\tclass arbiterview *arbv;\n\tclass %s *next;\n};\n\n",class
printf "! THIS IS THE END OF THIS FILE !!!\n%s.C\n",class
#write view .C file
printf "#include <andrewos.h>\n"
printf "/* user code begins here for %s */\n","HeaderInfo"
printf "/* user code ends here for %s */\n","HeaderInfo"
printf "#include <proctable.H>\n#include <cel.H>\n#include <celview.H>\n"
printf "#include <view.H>\n#include <arbiter.H>\n#include <arbiterview.H>\n#include <%s.H>\n#include <%s.H>\n",dclass,class
for(i in typelist){
    printf "#include <%s.H>\n",typelist[i]
}
printf "#define DataObject(A) ((A)->GetDataObject())\n"
printf "#define Parent(A) ((A)->GetParent())\n"
printf "#define Data(A) ((class %s *)DataObject(A) )\n",dclass
printf "#define CEL(A) ((struct cel  *)DataObject(A) )\n"
printf "/* user code begins here for %s */\n","includes"
printf "/* user code ends here for %s */\n","includes"
if(funcdefined == 1){
    printf "static struct  %s *FindSelf(class view *v)\n",class
    printf "{\n"
    printf " if(v->IsType(\"%s\")) return (class %s *) v;\n",class,class
    printf "else return (class %s *)arbiterview::FindArb(v);\n}\n",class
}
# Should probably make sure it is a 'class' type 
printf "class view *%s::GetApplicationLayer()\n",class
printf "{\n"
printf "    this->SetAppFlag(TRUE);\n",class
printf "    return this->arbiterview::GetApplicationLayer();\n"
printf "}\n"
if(vcount){
    for(i in uval){
	printf "static void %sCallBack(class ATK *s,class value *val,long r1,long r2)\n{\n", uval[i]
	printf "class %s *self=(class %s *)s;\n",class,class
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
#code for initapp
printf "void %s::%s()\n",class,dclfunc
printf "{\n/* user code begins here for %s_%s */\n",class,dclfunc
printf "/* user code ends here for %s_%s */\n}\n",class,dclfunc
#code for initcell
printf "void %s::InitCell(class celview *cv)\n{\n",class,class
printf "\tif(*(this->AdewArray[0].object) == NULL) adew_InitDataObjects(this,this->AdewArray);\n"
printf "\tadew_InitApplicationCel(this,NULL,cv,this->AdewArray,&(this->InitCount));\n"
printf "\tif(this->InitCount == 0){\n\tthis->InitCount--;\n"
printf "\t\tthis->%s();\n\t}\n",dclfunc
printf "/* user code begins here for InitCell */\n"
printf "/* user code ends here for InitCell */\n"
printf "}\n"
# add function definitions
if(funcdefined == 1){
    for(i in funcs){
	printf "static void %s_%s(class view *v,long dat)\n{\n",class,funcs[i]
	printf "class %s *self;\nif((self = FindSelf(v)) == NULL) return;\n",class 
	printf "/* user code begins here for %s_%s */\n",class,funcs[i]
	printf "/* user code ends here for %s_%s */\n}\n",class,funcs[i]
    }
}
printf "void %s::ObservedChanged(class observable *observed,long status)\n",class
printf "{\n",class
printf "/* user code begins here for %s */\n","ObservedChanged"
printf "/* user code ends here for %s */\n","ObservedChanged"
printf "if (status == observable::OBJECTDESTROYED) {\n"
printf "adew_NoteDestroyed(this,observed,this->AdewArray);\n";
printf "}\n"
printf "}\n"
printf "static boolean InitializeClass()\n{\n",class
if(funcdefined == 1){
    printf "ATKregistryEntry *viewtype = ATK::LoadClass(\"view\");\n"
      for(i in funcs){
	  printf  "proctable::DefineProc(\"%s-%s\",(proctable_fptr)%s_%s, viewtype,NULL,\"%s %s\");\n", class,funcs[i],class,funcs[i],class,funcs[i]
      }
}
printf "/* user code begins here for %s */\n","InitializeClass"
printf "/* user code ends here for %s */\n","InitializeClass"
print "return TRUE;\n}"
printf "%s::~%s()\n",class,class
printf "{\n",class
printf "adew_FinalizeApplication(this,this->AdewArray);\n";
printf "/* user code begins here for %s */\n","FinalizeObject"
printf "/* user code ends here for %s */\n","FinalizeObject"
print "}"
printf "ATKdefineRegistry(%s,arbiterview,InitializeClass)", class
printf "%s::%s()\n{\n",class,class
j = 0
printf "struct adew_array *aa = this->AdewArray;\n"
for(i in oblist){
    printf "aa->object = (class dataobject **) &(this->%s);\n",obfunc[i];
    printf "aa->view = (class view **) &(this->%sView);\n",obfunc[i];
    printf "aa->name = atom::Intern(\"%s\");\n",oblist[i];
    if(obtype[i] == "value"){
	printf "aa->callback = %sCallBack;\n",valuelist[i]
	printf "aa->rock = %d;\n",numlist[i]
    }
    else {
	printf "aa->callback = NULL;\n"
	printf "aa->rock = -1;\n"
    }
    printf "aa++;\n"
}
printf "aa->object = NULL;",j
printf "adew_InitializeApplication(this,this->AdewArray);\n";
printf "this->v = NULL;\tthis->next = NULL;\n"
printf "this->InitCount = %d;\n",obcount
printf "/* user code begins here for %s */\n","InitializeObject"
printf "/* user code ends here for %s */\n","InitializeObject"
printf "THROWONFAILURE(TRUE);}\n"
printf "/* user code begins here for %s */\n","Other Functions"
printf "/* user code ends here for %s */\n","Other Functions"
printf "! THIS IS THE END OF THIS FILE !!!\n"	
    }
}

