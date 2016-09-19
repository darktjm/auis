/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define class_StaticEntriesOnly

#include <andrewos.h>
#include <im.H>
#include <view.H>
#include <fontdesc.H>
#include <graphic.H>
#include <menulist.H>
#include <keystate.H>
#include <keymap.H>
#include <observable.H>
#include <proctable.H>
#include <updatelist.H>
#include <filetype.H>
#include <dataobject.H>
#include <msghandler.H>
#include <message.H>
#include <cursor.H>
#include <event.H>
#include <init.H>
#include <environ.H>
#include <bind.H>
#include <atom.H>
#include <atomlist.H>
#include <namespace.H>
#include <rm.H>
#include <windowsystem.H>

int basics_Initialize();


int basics_Initialize()
{
    im_StaticEntry;
    windowsystem_StaticEntry;
    view_StaticEntry;
    fontdesc_StaticEntry;
    graphic_StaticEntry;
    menulist_StaticEntry;
    keystate_StaticEntry;
    keymap_StaticEntry;
    observable_StaticEntry;
    proctable_StaticEntry;
    updatelist_StaticEntry;
    filetype_StaticEntry;
    dataobject_StaticEntry;
    msghandler_StaticEntry;
    message_StaticEntry;
    cursor_StaticEntry;
    event_StaticEntry;
    init_StaticEntry;
    environ_StaticEntry;
    bind_StaticEntry;
    atom_StaticEntry;
    atomlist_StaticEntry;
    namespace_StaticEntry;
    rm_StaticEntry;
    return 1;
}
