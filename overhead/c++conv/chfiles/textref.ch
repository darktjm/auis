/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
class textref :fnote {
overrides:
	ViewName() returns char *;
methods:
	GetRef(long size, char * buf) returns char *;
};

