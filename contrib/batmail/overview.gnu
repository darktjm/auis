			   BatMail Overview

Buffer Names and Movement Keys
	BatMail is organized into four main buffers, not all of which
are visible at any given time. Look at the separator bar for the name
of each buffer and other information.

Movement	Name		Function
Fore	Back
v	b	BatDisplay	displays body of the current message
				or help text
j	k	BatCaptions	displays current list of folders

	This means you would hit 'v' to scroll forward in this help
buffer.

	If you look at the bottom separator, you will see that the
cursor is currently in the BatCaptions buffer. The last command (v),
however, affected the appearance of another buffer. This is normal -
BatCaptions is the buffer that most of your commands are entered in
and several of those commands affect the BatDisplay buffer.

Exposing Folders
	To expose a list of changed folders, hit 'R' now. BatMail will
ask you for a list of folders to check, and a date to list messages
from.  The defaults will be to expose all subscribed folders, '*',
since 'updates' - or all changed folders.

	BatMail will now generate a list of captions for all messages
that were posted since you last read your messages and put them in the
BatCaptions buffer.  Notice that the separator for the BatCaptions
window changes to tell you which folder BatMail is checking at the
moment. You can give BatMail commands now, but they may be slow until
BatMail finishes checking for updated folders.

	Folders are separated by lines looking like:

	--- official.andrew - Updates (1 new message )

These lines are called folder separators. BatMail will let you move
around between folders with the following commands:

	.	move to next folder separator
	,	move to previous folder separator

	At this point you may want to try out the movement keys for
the BatCaptions buffer.

	Eventually you will probably want to read messages.  Don't
actually use the commands yet, or this Overview will go away and be
replaced by the BatBody buffer.

	SPC	expose more of message at cursor (or next if at end)
	n	read next message
	p	read previous message
	/	mark current folder as read (punt!) and move cursor to
			next folder separator
	^	make this message the first 'new' message

	The 'quit point' is the '_' character, and appears next to the
message number on the caption line. It marks, for each folder, where
BatMail will expose from if you choose to expose since 'updates'
again.

	Sending mail is done via the following commands :

	r	reply to sender of message at cursor
	w	reply to readers " "       "  " (wide reply)
	W	reply to both readers and senders (very wide reply)
	P	post message to the current folder
	s	send message

	These commands will 'pop' up the BatComposition buffer, and
move the cursor to it.  Inside of the BatComposition buffer, you can
type 'C-h m' (Control-h m) to see a help screen on commands for
composing messages. You will also see a list of basic commands on the
separator for the BatComposition buffer.  All normal Emacs editing
keys work in the BatComposition buffer.
