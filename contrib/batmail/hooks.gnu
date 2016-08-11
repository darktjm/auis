The following are variables, which you can setq to the name of
a function you define in your .emacs file.

--name--		    --description--
bat-init-message-hook	    Called in the outgoing message buffer, after 
			    the headers (and inital message contents, if any)
			    have been inserted, but before the user has had
			    a chance to type anything.  The dot is located
			    at the very end of the headers.
bat-submit-message-hook     Called in the outgoing message buffer just before
			    the message is sent.
bat-body-hook		    Called in the body buffer when a message body is
			    displayed, with the dot after the headers but
			    before the body of the message.
bat-captions-mode-hook	    Called in the headers window before any message
			    groups are read in, but after all keys are bound.
			    This can be use to add your own key bindings
			    (with local-set-key).
bat-composition-mode-hook   Called in the composition window when batmail
			    starts up.  This can be use to add your own key
			    bindings (with local-set-key).
