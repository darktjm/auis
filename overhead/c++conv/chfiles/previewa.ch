/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
class previewapp[previewa] : application[app] {
    classprocedures:
      InitializeObject(struct ezprintapp *self) returns boolean;
    overrides:
	ParseArgs(int argc,char **argv) returns boolean;
        Start() returns boolean;
	Run() returns int;
};
