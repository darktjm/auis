/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define class_StaticEntriesOnly

#include <andrewos.h>
#include <buffer.H>
#include <completion.H>
#include <dictionary.H>
#include <environment.H>
#include <mark.H>
#include <nestedmark.H>
#include <rectlist.H>
#include <style.H>
#include <stylesheet.H>
#include <tree23int.H>
#include <viewref.H>

int support_Initialize();


int support_Initialize()
{
    buffer_StaticEntry;
    completion_StaticEntry;
    dictionary_StaticEntry;
    environment_StaticEntry;
    mark_StaticEntry;
    nestedmark_StaticEntry;
    rectlist_StaticEntry;
    style_StaticEntry;
    stylesheet_StaticEntry;
    tree23int_StaticEntry;
    viewref_StaticEntry;

    return 1;
}
