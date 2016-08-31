/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
class texttag :fnote {
overrides:
	ViewName() returns char *;
methods:
	GetTag(long size, char * buf) returns char *;
};

