The following variables may be set in your ~/.emacs file to affect the
behavior of BatMail.  Variables are set with the setq command.  String
values should be enclosed in double quotes (").  Boolean values are
't' for true or 'nil' for false.  For example, to set the
bat-folders-only variable to true, put the following in your .emacs

(setq bat-folders-only t)


--Name--		--Default-- --Description--
bat-headers		"from:resent-from:resent-to:date:subject:to:cc:
			 newsgroups:organization"
				    List of headers to show, seperated by
				    ':', OR a '!' followed by a list of
				    headers NOT to show.
bat-folders-only	nil	    When non-nil, checking more than one
				    folder for new messages will only list
				    the folders, not the captions (this is
				    much faster).  To see the captions, you
				    can use the 'N' command.
bat-body-active 	t	    If nil, certain commands pertain to
                                    the message under the cursor instead of
				    the currently displayed message
bat-send-bcc		nil	    If non-nil, all outgoing message will be
				    bcc'd to you.
bat-caption-percentage	30	    Percentage of screen height used by header
				    window.  If negative, windows are split
				    horizontally.
bat-default-folders	"*"	    Initial list of folders to read.
bat-show-unchanged	nil	    If non-nil, will put folders in the caption
				    area, even if there are no messages.
bat-low-density		nil	    If non-nil, will put a blank line between
				    message directories.
bat-exit-to-shell	t	    If non-nil, quitting from a batmail that
			 	    was started from the shell will exit
				    emacs as well.
bat-suspend-on-start	nil	    If non-nil, batmail will automatically
				    suspend itself when it starts up (so
				    you can do other things while it's
				    churning away).
bat-ispell-before-submit nil	    If non-nil, batmail will run ispell-buffer
				    on the bat-composition buffer before
				    submitting
bat-check-mail-regexp	"^mail"	    Batmail will try to read in new mail when
				    getting captions for folders matching
				    this regexp
bat-announce-new-mail	t	    If non-nil, announces new mail in the
				    modeline.
bat-composition-ckp-file "/tmp/batcomp.<userid>.CKP"
				    File name used to checkpoint the
				    bat-composition buffer or nil, meaning
				    don't checkpoint.
bat-obnoxious-logo	t	    If non-nil, displays huge obnoxious logo
				    when batmail is started from the shell.
bat-stingy-display	nil	    If non-nil, use less verbosity some places.
bat-sig-alist		'(("" "~/.signature"))
				    Appends the cadr of the first element
				    whose car (a regexp) matches the
				    destination.

These probably don't concern most people:

--Name--		--Default-- --Description--
bat-digest-separator	"^-----------------------------"
				    Regular expression of separator in
				    digests.
bat-default-append-directory
			"~/"	    Default directory for "append-to" commands.
bat-default-append-file "SavedMessages"
				    Default file for "append-to" commands
bat-trace		nil	    If non-nil, will record interactions with
				    the robin subprocess in the buffer
				    "*bat-log*".  If a string, will also write
				    the log to the named file.
robin			"robin"    The name of the executable for robin.
batcave			"/usr/misc/.batmail/lib/batcave"
				    The prefix used to find bat-files.
