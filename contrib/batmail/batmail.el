;;; BatMail.el -- gnu-emacs interface to the Andrew Message System.
;;;	       designed to work the robin program

;;; ***************************************************************** 
;;;	Copyright 1988-1990 by Miles Bader
;;;	Copyright Carnegie Mellon Univ. 1994 - All Rights Reserved
;;; ***************************************************************** 
;;;
;;;	$Disclaimer: 
;;;Permission to use, copy, modify, and distribute this software and its 
;;;documentation for any purpose and without fee is hereby granted, provided 
;;;that the above copyright notice appear in all copies and that both that 
;;;copyright notice and this permission notice appear in supporting 
;;;documentation, and that the name of IBM not be used in advertising or 
;;;publicity pertaining to distribution of the software without specific, 
;;;written prior permission. 
;;;                        
;;;THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
;;;TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
;;;MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
;;;HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
;;;DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
;;;DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
;;;OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
;;;WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
;;;
;;; $

;;; 14 Dec 1988 - Miles Bader
;;; Copyright 1988,1989,1990 by Miles Bader

(defconst bat-version "2.15" "BatMail version number")

;;; ----------------------------------------------------------------
;;; variables that the user can set to effect BatMail behavior

(defvar bat-show-unchanged nil
  "*If non-nil, will put folders in the caption area, even if there are no
messages.")
  
(defvar bat-low-density nil
  "*If non-nil, will put an extra blank line between folders.")

(defvar bat-headers
  "from:resent-from:resent-to:date:subject:to:cc:newsgroups:organization"
  "*List of headers to show, seperated by ':' OR a '!' followed by a list
of headers NOT to show.")

(defvar bat-body-active t
  "*If nil, some commands use the current message under the
cursor instead of the message who's body is displayed.")

(defvar bat-folders-only nil
  "*If non-nil, don't look for captions, only folders, when doing a large
scan.")

(defvar bat-send-bcc nil
  "*If non-zero, send a blind carbon copy of outgoing mail to yourself.")

(defvar bat-caption-percentage 30
  "*Percentage of screen height used for the captions.  If negative, splits
windows horizontally.")

(defvar bat-composition-ckp-file (concat "/tmp/batcomp." (user-login-name) ".CKP")
  "*File name used to checkpoint the bat-composition buffer or nil,
meaning don't checkpoint.")

(defvar bat-default-folders "*"
  "*Default set folders to prompt with when starting batmail.")

(defvar bat-stingy-display nil "*If non-nil, use less verbosity some places.")

(defvar bat-exit-to-shell t
  "*If non-nil, quitting from a batmail that was started from the shell
will exit emacs as well.")

(defvar bat-suspend-on-start nil
  "*If non-nil, batmail will automatically suspend itself when it starts up
(so you can do other things while it's churning away).")

(defvar bat-obnoxious-logo t
  "*If non-nil, displays huge obnoxious logo when batmail is started from
the shell.")

(defvar bat-announce-new-mail t
  "*If non-nil, announces new mail in the modeline.")

(defvar bat-ispell-before-submit nil
  "*If non-nil, runs ispell on bat-composition buffer before sending.")

(defvar bat-check-mail-regexp "^mail"
  "*Batmail will try to read in new mail when getting captions for folders
matching this regexp")

(defvar bat-default-append-directory "~/"
  "*Default directory for file to append to")

(defvar bat-default-append-file "SavedMessages"
  "*Default file to append to")

(defvar bat-sig-alist '(("" "~/.signature"))
  "*Appends the cadr of the first element whose car (a regexp) matches
the destination")

(defvar bat-digest-separator "^-----------------------------"
  "*Regexp for separators in digests.")

(defvar bat-trace nil
  "*If non-zero, will record interactions with the robin subprocess in the
 buffer bat-log.  If a string, entries are appended to the file it names.")

;;; ----------------------------------------------------------------
;;; The following describe where various bat-parts are located...

(defvar sys (or (getenv "SYS") "@sys") "Andrew system type.")

(defvar bat-cave (or (getenv "BATCAVE") 
			(getenv "ANDREWDIR")
			"/usr/andrew")
  "*Place where batmail and all it's equipment live.")

(defvar robin (or (getenv "ROBIN") "robin")
  "*The name of the robin process to run.")

(defvar bat-bin-dir (concat bat-cave "/bin")
  "*Directory where batmail machine binaries are kept")
(defvar bat-help-dir (concat bat-cave "/help")
  "*Director where batmail help files are kept")
(defvar bat-etc-dir (concat bat-cave "/etc")
  "*Directory where miscellaneous bat files are kept")

;;; ----------------------------------------------------------------

(load-library (concat bat-cave "/lib/el/prompt"))

;;; ----------------------------------------------------------------

(defconst bat-display-buf "*bat-display*")
(defconst bat-metamail-buf "*bat-metamail*")
(defconst bat-captions-buf "*bat-captions*")
(defconst bat-composition-buf "*bat-composition*")
(defconst bat-scratch-buf " *bat-scratch*")
(defconst bat-folder-update-buf " *bat-folder-update*")

;;; ----------------------------------------------------------------

(defun bat-trace (str)
  "Append STRING to the bat log (and file), but only if tracing is turned on."
  (if bat-trace (bat-trace-always str)))

(defun bat-trace-always (str)
  "Append STRING to the bat log (and file is specified)."
  (save-excursion
    (set-buffer (get-buffer-create "*bat-log*"))
    (let
		((start (point-max)))
	(goto-char start)
	(insert str)
	(if (stringp bat-trace) 
		(condition-case nil 
			(append-to-file start (point-max) bat-trace)
			(error (setq bat-trace t))  ) )	;; write to file failed
    )
    t))


;;; ----------------------------------------------------------------

(defmacro bat-command (&rest commands)
  (` (bat-real-command (, (if (cdr commands)
			      (cons 'concat commands)
			    (car commands)))
		       nil)))

(put 'bat-command-nil-fail 'lisp-indent-hook 1)
(defmacro bat-command-nil-fail (nil-fail &rest commands)
  (` (bat-real-command (, (if (cdr commands)
			      (cons 'concat commands)
			    (car commands)))
		       nil-fail)))

;;; ----------------------------------------------------------------

(defvar ~bat-emacs-number 0)
(let*  (
		(ev  (emacs-version))
		(start (string-match   "[1-9][0-9]*" ev))  )
	(setq emacs-number (string-to-int (substring ev start)))
)

(defun ~bat-urgent (str &optional nolog)
  "Display STRING in the message line, and flush the display output.
If optional argument NOLOG is nil, put STRING in the bat log."
  (or nolog (bat-trace-always (concat str "\n")))
  (display-string str))

(defun ~bat-error (str &optional level)
  "Put STRING in an error message in the bat log, display it in the message
line, and flush the display output."
  (bat-trace-always (concat "Error: <<<" str ">>>\n"))
  (~bat-urgent str t))

(defun ~bat-info (str)
  "Display STRING in the message line if batmail is active
Also put STRING in the bat log."
  (bat-trace-always (concat str "\n"))
  (if (and bat-active (not bat-suspended))
      (display-string str)))

(defun ~bat-get-boolean-from-user (prompt ans)
  "Get a yes or no answer from the user, prompting with PROMPT, and send
the answer (default DEF) to robin."
  (unwind-protect (setq ans (get-tty-bool prompt ans))
    (bat-command "i" (if ans 1 0) "\n")))

(defun ~bat-get-string-from-user (prompt echo)
  "Get a string from the user, prompting with PROMPT, and send the result
to robin.  ECHO is ignored."
  (let ((ans ""))
    (unwind-protect (setq ans (get-tty-str prompt))
      (bat-command "i" ans "\n"))))

(defun ~bat-choose-from-list (def quest &rest args)
  "Get a user answer (default DEF) to the string QUESTION.  Each additional
(string) argument is one choice."
  (setq count 1)
  (set-buffer bat-scratch-buf)
  (erase-buffer)
  (insert (concat quest "\n"))
  (mapcar '(lambda (str)
	     (insert (concat (if (< count 10)
				 count
			       (char-to-string (+ (- count 10) ?a)))
			     "\t" str "\n"))
	     (setq count (1+ count)))
	  args)
  (setq choice (if (< def 10) (+ def ?0) (+ (- count 10) ?a)))
  (unwind-protect 
      (setq choice
	    (get-prompted-key bat-scratch-buf
			      "BatChoice"
			      choice
			      "---- BatChoices"
			      t))
    (bat-command "i"
                 (if (and (>= choice ?0) (<= choice ?9))
                     (char-to-string choice)
                   (+ (- choice ?a) 10))
		 "\n")))

;;; ----------------------------------------------------------------
;;; useful functions

;;; this is derived from perform-replace
(defun replace (from to &optional regexp delimited)
  "Replace FROM with TO.  Optional flags are REGEXP & DELIMITED."
  (let ((nocasify (not (and case-fold-search case-replace
			    (string-equal from (downcase from)))))
	(literal (not regexp))
	(search-function (if regexp 're-search-forward 'search-forward))
	(search-string from)
	(lastrepl nil))			; Position after last match considered.
    (if delimited
	(setq search-function 're-search-forward
	      search-string (concat "\\b"
				    (if regexp from (regexp-quote from))
				    "\\b")))
    (while (and (not (eobp)) (funcall search-function search-string nil t))
      ;; Don't replace the null string 
      ;; right after end of previous replacement.
      (if (eq lastrepl (point))
	  (forward-char 1)
	(undo-boundary)
	(replace-match to nocasify literal)
	(setq lastrepl (point))))))

(defun refresh-modeline (&optional buf)
  "Make sure the modelines in any windows associated with BUF (default
is the current buffer) are refreshed during the next redisplay."
  (save-excursion
    (and buf (set-buffer buf))
    (set-buffer-modified-p (buffer-modified-p))))

(defun write-current-buffer (file)
  "Write the contents of the current buffer to FILE."
  (write-region (point-min) (point-max) file))

(defun buf-in-win-v-p (buf win stop)
  (cond ((equal (window-buffer win) buf) t)
	((equal win stop) nil)
	(t (buf-in-win-v-p buf (next-window win) (next-window win)))))

(defun buffer-visible-p (&optional bufname)
  "Returns t if BUFFER, defaulting to (current-buffer), is on the screen."
  (let ((win (selected-window)))
    (buf-in-win-v-p
     (if bufname (get-buffer bufname) (current-buffer)) win win)))

(defun goto-marker (mark)
  (set-buffer (marker-buffer mark))
  (goto-char (marker-position mark)))

(require 'backquote)
(require 'comint)

(put 'when 'lisp-indent-hook 1)

(defmacro when (condition &rest body)
  "When CONDITION is non-nil, execute any following arguments."
  (` (and (, condition) (progn (,@ body)))))

(put 'temp-pop-to-buffer 'lisp-indent-hook 1)

(defmacro temp-pop-to-buffer (bufname &rest body)
  "Make sure BUFFER is visible on the screen, then evaluate any remaining
arguments.  The original screen layout is restored afterwards."
  (` (let ((buf (get-buffer-create (, bufname)))
	   (oldbuf (current-buffer)))
       (pop-to-buffer buf)
       (unwind-protect (progn (,@ body))
	 (and buf (pop-to-buffer oldbuf))))))

(put 'temp-set-or-pop--to-buffer 'lisp-indent-hook 1)

(defmacro temp-set-or-pop-to-buffer (bufname pop &rest body)
  "Make BUFFER the current buffer, and if POP is non-nil make sure it's
visible on the screen, then evaluate any remaining arguments.  The original
buffer is restored afterwards."
  (` (let ((buf (get-buffer-create (, bufname)))
	   (oldbuf (current-buffer)))
       (if (, pop)
	   (pop-to-buffer buf)
	 (set-buffer buf))
       (unwind-protect (progn (,@ body))
	 (pop-to-buffer oldbuf)))))

(put 'temp-set-buffer 'lisp-indent-hook 1)

(defmacro temp-set-buffer (bufname &rest body)
  "Make BUFFER the current buffer, then evaluate any remaining
arguments.  The original current buffer is restored afterwards."
  (` (let ((buf (get-buffer-create (, bufname)))
	   (oldbuf (current-buffer)))
       (set-buffer buf)
       (unwind-protect (progn (,@ body))
	 (set-buffer oldbuf)))))

(put 'ignoring-read-only 'lisp-indent-hook 0)

(defmacro ignoring-read-only (&rest body)
  "Evaluates it's args while the current is writeable, restoring read-only
afterwards."
  (` (let ((bro buffer-read-only))
       (setq buffer-read-only nil)
       (unwind-protect (progn (,@ body))
	 (setq buffer-read-only bro)))))

(defun buffer-match (num)
  "Returns the string matched by the NUMth pair of parens, or the entire
match if NUM is 0."
  (buffer-substring (match-beginning num) (match-end num)))

(defun delete-current-buffer-contents ()
  (delete-region (point-min) (point-max)))

(defun next-line-no-insert ()
  "Just like next-line, but doesn't insert a newline at the end of the buffer"
  (interactive)
  (and (save-excursion (= (forward-line 1) 0))
       (call-interactively 'next-line)))

(defun buffer-bindings-string (buf)
  "Return a string containing all but the first line of the documentation
for the major mode in BUFFER; this is assumed to contain keybinding info."  
  ;; assumes that the documenation string for func has keybindings defined
  ;; in all but the first line
  (save-excursion
    (if buf (set-buffer buf))
    (let ((str (documentation major-mode)))
      (cond ((null str) "")
	    (t
	     (let ((nl (string-match "\n" str)))
	       (while nl
		 (setq str (substring str (1+ nl)))
		 (setq nl (string-match "[ \n\t]" (substring str 0 1))))
	       str))))))

(defun regexp-assoc (str re-alist)
  "Matches STRING agains the car of each sublist of ALIST, treated as a regexp.
Returns the first sublist that matches."
  (cond ((null re-alist) nil)
	((string-match (car (car re-alist)) str)
	 (car re-alist))
	(t (regexp-assoc str (cdr re-alist)))))

(defun markp ()
	"Returns nil if no mark is active otherwise returns (mark)."
	(if  (>= ~bat-emacs-number 19)
		(if mark-active (mark) nil)
		(mark)
	))

;;; ----------------------------------------------------------------

(defvar bat-robin nil "Batmail robin process") ; robin process

(defvar bat-command nil )
(defvar bat-output "")

(defvar bat-kill-robin-installed nil)
(defvar bat-old-kill-emacs-hook nil)

(defun bat-call-robin ()
  "Makes sure robin is running, starting him up if necessary."
  (if (null bat-robin)
      (progn
	(~bat-info "Atomic batteries to power...")
	(setq bat-robin
	      (start-process "robin" nil
			     (if (eq (aref robin 0) 47)	;'/'
				 robin
			       (concat bat-bin-dir "/" robin))))
	(cond (bat-robin
	       (set-process-filter bat-robin 'bat-filter)
	       (set-process-sentinel bat-robin 'bat-sentinel)
	       (process-kill-without-query bat-robin)

	       ;; the following is a hack for a pty bug in Ultrix,
	       ;; which DEC has screwed up yet again
	       (when (not bat-kill-robin-installed)
		 (setq bat-old-kill-emacs-hook kill-emacs-hook)
		 (if (>= ~bat-emacs-number 19)
			(add-hook 'kill-emacs-hook  'bat-kill-robin)
			(setq kill-emacs-hook 'bat-kill-robin)
		)
		 (setq bat-kill-robin-installed t)))
	      (t
	       (error "Couldn't start robin process.")))
	(~bat-info "Turbines to speed..."))
    (~bat-info "Robin's already here...")))

(defun bat-kill-robin ()
  (and bat-robin (kill-process bat-robin))
  (and  (not (>= ~bat-emacs-number 19)) (and bat-old-kill-emacs-hook (funcall bat-old-kill-emacs-hook)))
)

;;; clean up after ourselves...
(defun bat-flush-robin (&optional nil-fail)
  "Flushes both batmail & robin state; it's safe to kill both after this."
  (bat-command-nil-fail nil-fail
    "[\n"
    "C(~bat-begin-state \"Updating state\")\n"
    "pa\nU\nR\n"
    "C(~bat-end-state)\n"
    "|\n"
    "C(~bat-fail-state)\n"
    "]\n"))

(defun bat-real-command (str &optional nil-fail)
  "Send STRING to robin; this is interpreted as one or more commands,
each terminated by a newline.  For huge numbers of commands, use
bat-exec-current-buffer.  If optional arg NIL-FAIL is t, it will return
nil instead of signalling an error if robin is dead."
  (cond (bat-robin
	 (bat-trace str)
	 (process-send-string bat-robin (concat str "\n"))
	 t)
	(nil-fail
	 nil)
	(t
	 (error "Robin is dead!"))))

;;; temporary
(defvar old-bat-output "")

(defun bat-sentinel (process state)
  (when (not (equal state 'run))
    (setq bat-robin nil)
    (~bat-urgent "Robin died!")))

(defun bat-filter (proc output)
  "Process output from robin, invoking callbacks or displaying messages."

  ;; temporary
  (setq old-bat-output bat-output)

  (setq bat-output (concat bat-output output))

  (let (newline)
    (while (setq newline (string-match "\n" bat-output))
      (let ((cmd (substring bat-output 0 newline)))

	(setq bat-output (substring bat-output (1+ newline)))

	(bat-trace (concat cmd "\n"))

	(if (equal (substring cmd 0 1) "(")
;	    (let ((old-match (match-data))) ;
;	      (unwind-protect  ;
	          (while (> (length cmd) 0)
		    (let* ((rfs
			    ;; error handling code for spurious error
			    (condition-case err
				(read-from-string cmd)
			      (error
			       (bat-trace-always
				(concat "Error in read-from-string(" cmd "): " (prin1-to-string err) "\n"
					"   output: <" output ">\n"
					"   bat-output: <" bat-output ">\n"
					"   newline: " (prin1-to-string newline) ">\n"
					"   old-bat-output: <" old-bat-output ">\n"
			       (~bat-error
				"Error in bat-filter, please check the *bat-log* buffer")
			       (list nil newline)))))))
		      (condition-case err
			  (eval (car rfs))
			(error (~bat-error
				(concat "Callback error: <<<"
					(substring cmd 0 (cdr rfs))
					">>>"))))
		      (setq cmd (substring cmd (cdr rfs)))))
;		(store-match-data old-match))) ;
	  (if (> (length cmd) 1)
	      (progn
		(while (string= (substring cmd (1- (length cmd))) "\r")
		  (setq cmd (substring cmd 0 (1- (length cmd)))))
		(~bat-info (concat "robin: " cmd)))))))))

(defun bat-exec-current-buffer ()
  "Send the contents of the current buffer to robin as a series of commands.
(This is avoids problems with using large strings with bat-command)"
  (let ((fn (bat-temp-file)))
    (goto-char (point-max))
    (insert "D" fn "\n")		; delete file after its done
    (write-current-buffer fn)		; write-file screws things up
    (bat-command "I" fn "\n")))

(defvar bat-temp-file-counter 0)

(defun bat-temp-file ()
  "Return a string useable as the name of a temporary file."
  (setq bat-temp-file-counter (1+ bat-temp-file-counter))
  (concat (make-temp-name "/tmp/BatMail.temp.") "." bat-temp-file-counter))

;;; ----------------------------------------------------------------

(defconst bat-cuid-cols 4)
(defconst bat-cuid-pat "^[0-9]+")
(defconst bat-cuid-exact-pat "^[0-9][0-9 ][0-9 ][0-9 ]")

(defvar bat-old-captions-width 0)

(defun bat-set-caption-width (w)
  "Tell robin to adjust it's output for a window WIDTH columns wide."
  (when (/= w bat-old-captions-width)
    (bat-command "w" (- w (1+ bat-cuid-cols) 2) "\n")
    (setq bat-old-captions-width w)))

;;; callback from robin
;;; Rewrites an existing caption
(defun ~bat-alter-caption (cuid caption)
  "Change the existing caption for CUID to CAPTION."
  (save-excursion
    (set-buffer bat-captions-buf)
    (and (bat-find-caption cuid)
	 (ignoring-read-only 
	   (delete-region (+ (point) bat-cuid-cols 1)
			  (progn (end-of-line) (point)))
	   (insert caption)))))

;;; callback from robin
(defun ~bat-kill-caption (cuid)
  "Delete the existing caption for CUID altogether."
  (save-excursion
    (when (bat-find-caption cuid)
      (bat-bump-update-mark)
      (ignoring-read-only (kill-line 1)))))

;;; puts point at beginning, returns end
(defun bat-find-folder (folder)
  "Find the extent of FOLDER, putting the point at the beginning and
returning the position of the end."
  (goto-char (point-max))
  (cond ((search-backward (concat "--- " folder " ") nil t)
	 (beginning-of-line)
	 (let ((beg (point))
	       (end
		(cond ((search-forward "\n---" nil t)
		       (beginning-of-line)
		       (point))
		      (t
		       (point-max)))))
	   (goto-char beg)
	   end))
	(t
	 (point))))

(defvar bat-folder-name "")
(defvar bat-folder-mark (make-marker))
(defvar bat-last-caption-mark (make-marker))
(defvar bat-first-new-msg (make-marker))
(defvar bat-folder-did-exist nil)
(defvar bat-saved-new-msgs 0)
(defvar bat-saved-point nil)
(defvar bat-saved-mark nil)
(defvar bat-first-new-msg nil)

;;; callback from robin
;;; First of the three part protocol from the robin process; 
;;; If the give directory exists, it is erased and its new message count
;;; is subtracted from the total.  It then records the directory name
;;; and the location of the directory (or the end of the buffer if it
;;; didn't exist)
(defun ~bat-start-folder-update (folder)
  "Set up state for updating FOLDER."
  (setq bat-folder-name folder)
  (~bat-begin-state (concat "Checking " bat-folder-name))
  (temp-set-buffer bat-folder-update-buf
    (delete-region (point-min) (point-max))
    (set-marker bat-last-caption-mark (point))
    (setq bat-first-new-msg nil)))

;;; callback from robin
(defun ~bat-add-caption (cuid caption isnew)
  "Add an entry for CUID and CAPTION to the folder currently being updated.
ISNEW should be non-0 if this is a new message."
  (temp-set-buffer bat-folder-update-buf
    (goto-marker bat-last-caption-mark)
    (if (and (/= isnew 0) (null bat-first-new-msg))
	(setq bat-first-new-msg cuid))
    (insert (concat cuid))
    (indent-to-column (1+ bat-cuid-cols)) ; +1 for update-mark column
    (insert caption "\n")
    (set-marker bat-last-caption-mark (point))))

;;; callback from robin
(defun ~bat-end-folder-update (since num new &optional killed)
  "Put the folder currently being updated into the captions buffer.
SINCE is the date used for updating, NUM is the total number of captions
received, NEW is the number of those that are new, and KILLED is the number
not sent due to being killed."
  (temp-set-buffer bat-captions-buf
    (let ((folder-beg nil)
	  (folder-end nil)
	  (within-folder nil))
      (save-excursion
	(setq folder-end (bat-find-folder bat-folder-name)
	      folder-beg (point)))

      (setq within-folder (and (>= (point) folder-beg) (<= (point) folder-end)))

      (when (or (> num 0) bat-show-unchanged (/= folder-beg folder-end))
	(let ((point-cuid (bat-maybe-save-pos (point) folder-beg folder-end))
	      (mark-cuid
	       (bat-maybe-save-pos (markp) folder-beg folder-end)))
	  (save-excursion
	    (ignoring-read-only
	      (delete-region folder-beg folder-end)

	      (if (and bat-low-density (eobp) (not (bobp)))
		  (newline))

	      (goto-char folder-beg)

	      (insert "--- " bat-folder-name " - ")
	      (if (equal since "")
		  (insert "Updates")
		(insert "Since " since))
	      (insert
	       (concat " ("
		       (if (> num 0) num "No")
		       (if (and (= new num) (or (> new 0) (equal since "")))
			   " new"
			 "")
		       " message" (if (= num 1) "" "s")
		       (if (and (> new 0) (> num new))
			   (concat ", " new " new")
			 "")
		       (if (and killed (> killed 0))
			   (concat ", " killed " killed")
			 "")
		       ")\n"))

	      (insert-buffer-substring bat-folder-update-buf)
	      (setq folder-end (point))

	      (if bat-low-density (newline))

	      (bat-invalidate-update-mark-for bat-folder-name)

	      (when (/= new num)	; need a quit line
		(if (null bat-first-new-msg)
		    ;; all old, quit line at end
		    (goto-char folder-end)
		  ;; else, folder line before 1st new message
		  (bat-find-caption bat-first-new-msg))
		
		(forward-line -1)
		(bat-set-orig-update-mark))))

	  (cond (within-folder
		 (if mark-cuid
		     (set-marker (mark-marker)
				 (bat-maybe-restore-pos mark-cuid
							(markp))))
		 (cond (point-cuid
			(goto-char
			 (bat-maybe-restore-pos point-cuid (point))))
		       (t
			(goto-char folder-beg)
			(forward-line 1)))) ; special case for browsing

		(bat-goto-new-message
		 (goto-char folder-beg)
		 (if (> new bat-saved-new-msgs)
		     (forward-line (- num (- new bat-saved-new-msgs)))
		   (forward-line 1))))

	  (setq bat-goto-new-message nil)))))
  (~bat-end-state))

(defun ~bat-fail-folder-update ()
  "Matches a ~bat-start-folder-update if something went wrong."
  (save-excursion
    (set-buffer bat-captions-buf)
    (let ((end (bat-find-folder bat-folder-name)))
      (ignoring-read-only
	(delete-region (point) end)
	(if (and bat-low-density (eobp) (not (bobp)))
	    (newline))
	(insert "--- " bat-folder-name " - Error getting captions\n")
	(if bat-low-density (newline)))))
  (~bat-fail-state))

(defun ~bat-folders-only-folder-update (folder since)
  "Do a caption-less folders-only update on FOLDER, since DATE."
  (save-excursion
    (set-buffer bat-captions-buf)
    (let ((end (bat-find-folder folder)))
      (ignoring-read-only
	(delete-region (point) end)
	(if (and bat-low-density (eobp) (not (bobp))) (newline))
	(insert "--- " folder " - "
		(if (equal since "") "Updates" (concat "Since " since))
		" (New messages)\n")
	(if bat-low-density (newline))))))

(defun ~bat-list-folder-status (folder status)
  "Do a caption-less list-folder update on FOLDER, with subscription
status STATUS."
  (save-excursion
    (set-buffer bat-captions-buf)
    (let ((end (bat-find-folder folder)))
      (ignoring-read-only
	(delete-region (point) end)
	(if (and bat-low-density (eobp) (not (bobp))) (newline))
	(insert "--- " folder " - " status "\n")
	(if bat-low-density (newline))))))

(defun bat-current-folder ()
  "Returns the folder that the point is in, or nil if none"
  (save-excursion
    (set-buffer bat-captions-buf)
    (forward-line 1)
    (cond ((re-search-backward "^--- \\([^ ]*\\)" nil t)
	   (buffer-match 1))
	  (t nil))))

(defun bat-first-cuid (&optional folder)
  "Returns the cuid of the first message displayed in FOLDER, default
(bat-current-folder)."
  (save-excursion
    (set-buffer bat-captions-buf)
    (and (cond (folder
		(/= (bat-find-folder folder) (point))) ; order of evaluation
	       (t
		(forward-line 1)
		(re-search-backward "^--- \\([^ ]*\\)" nil t)))
	 (= (forward-line 1) 0)
	 (looking-at bat-cuid-pat)
	 (string-to-int (buffer-match 0)))))

;;; ----------------------------------------------------------------

(defvar bat-update-mark (make-marker))
(defvar bat-orig-update-mark (make-marker))
(defvar bat-cur-folder-name nil)
(defvar bat-cur-folder-beg (make-marker))
(defvar bat-cur-folder-end (make-marker))

(defun bat-invalidate-update-mark-for (folder)
  "If the current update mark is in FOLDER, invalidate any update state."
  (when (equal folder bat-cur-folder-name)
    (setq bat-cur-folder-name nil)
    (set-marker bat-cur-folder-beg nil)
    (set-marker bat-cur-folder-end nil)
    (set-marker bat-update-mark nil)
    (set-marker bat-orig-update-mark nil nil)))

;;; update robins idea of the current groups last-update
;;; If there is no current group, update-mark should be nil
(defun bat-save-last-update (&optional nil-fail)
  "Flush any update state to robin."
  (and (not (equal (marker-position bat-update-mark)
		   (marker-position bat-orig-update-mark)))
       (and (bat-command-nil-fail nil-fail
	      "d" bat-cur-folder-name "\n"
	      "u"
	      (if (marker-position bat-update-mark)
		  (save-excursion
		    (goto-marker bat-update-mark)
		    (if (looking-at bat-cuid-pat) (buffer-match 0) 0))
		0)
	      "\n")
	    (set-marker bat-orig-update-mark
			(marker-position bat-update-mark)
			(marker-buffer bat-update-mark)))))

(defconst bat-quit-pat (concat "^---\\|" bat-cuid-exact-pat "_"))

(defun bat-get-update-mark ()
  "Returns the position of the current update mark for the group the point is in."
  (if (or (not (marker-position bat-update-mark))
	  (< (point) (marker-position bat-cur-folder-beg))
	  (> (point) (marker-position bat-cur-folder-end)))
      ;; reading a new group, set up state info for it
      (save-excursion
	;; first, try and save last-update info for prev group we read...
	(bat-save-last-update)
	;; now look for the beginning, name, update-line, and end of this new
	;; group
	(end-of-line)			; so search works if on folder line
	(if (not (re-search-backward "^--- \\([^ ]*\\)" nil t))
	    (error "Not in a folder?"))
	(set-marker bat-cur-folder-beg (point))
	(setq bat-cur-folder-name (buffer-match 1))
	(forward-line 1)
	(beginning-of-line)
	(cond ((not (re-search-forward bat-quit-pat nil t))
	       ;; no quit-line, no next group
	       (goto-char (point-max))
	       (set-marker bat-update-mark nil))
	      (t
	       (beginning-of-line)
	       (cond ((looking-at (concat bat-cuid-exact-pat "_"))
		      ;; found a quit-line
		      (set-marker bat-update-mark (point))
		      ;; resume looking for the next folder
		      (if (re-search-forward "^---" nil t)
			  (beginning-of-line)
			(goto-char (point-max))))
		     (t
		      ;; no quit-line, but a next folder
		      (set-marker bat-update-mark nil)))))
	;; now positioned after the end of the current folder
	(set-marker bat-orig-update-mark
		    (marker-position bat-update-mark)
		    (marker-buffer bat-update-mark))
	(backward-char)		; so subsequent inserts don't move end'
	(set-marker bat-cur-folder-end (point))))
  (marker-position bat-update-mark))

;;; by convention, setting the update-mark while sitting on folder header
;;; will unset it

(defun bat-set-orig-update-mark ()
  (bat-set-update-mark)
  (set-marker bat-orig-update-mark
	      (marker-position bat-update-mark)
	      (marker-buffer bat-update-mark)))

(defun bat-bump-update-mark ()
  "If the update mark is on the current line, it will be moved somewhere else."
  (save-excursion
    (beginning-of-line)
    (when (equal (marker-position bat-update-mark) (point))
      (forward-line -1)
      (bat-set-update-mark))))

(defun bat-update-update-mark ()
  "Updates the current update mark to the point position if it's before POS."
  (let ((um (bat-get-update-mark)))
      (if (or (null um) (> (point) um))
	  (bat-set-update-mark))))

(defun bat-set-update-mark ()
  "Sets the current update mark to point."
  (save-excursion
    (beginning-of-line)
    (bat-get-update-mark)	       ; this ensures that it''s valid
    ;; at this point, all cur-folder info should be up to date
    (if (marker-position bat-update-mark) ; was there one before?
	;; yes, get rid of it
	(save-excursion
	  (goto-marker bat-update-mark)
	  (forward-char bat-cuid-cols)
	  (ignoring-read-only
	    (delete-char 1)
	    (insert " "))))
   ;; put the new one in place
   (cond ((looking-at "^[0-9]")
	  (set-marker bat-update-mark (point))
	  (forward-char 4)
	  (ignoring-read-only
	    (delete-char 1)
	    (insert "_")))
	 (t
	  (set-marker bat-update-mark nil)))))

;;; hint says what direction to look in first
(defun bat-find-caption (cuid &optional hint)
  "Sets the point position to the beginning of the caption for CUID.
If the optional position HINT is supplied, it will look in that
direction first."
  (end-of-line)
  (setq match (concat "^" cuid "[^0-9]"))
  (if (if (and hint (> (point) hint))
	  (or (re-search-backward match nil t)
	      (re-search-forward match nil t))
	(or (re-search-forward match nil t)
	    (re-search-backward match nil t)))
      (progn (beginning-of-line) t)))


(defun bat-maybe-save-pos (pos beg end)
  "Returns the cuid at POS, but only if POS is between BEG and END.
Otherwise it returns nil."
  (when (and pos
	     (or (and (> pos end) (< pos beg))
		 (and (< pos end) (> pos beg))))
    (save-excursion
      (goto-char pos)
      (beginning-of-line)
      (and (looking-at bat-cuid-pat)
	   (string-to-int (buffer-match 0))))))

(defun bat-maybe-restore-pos (cuid pos)
  "If CUID is greater than 0, sets the current point position to that
caption and returns it.  Otherwise it returns POS."
  (if (> cuid 0)
      (save-excursion
	(if (bat-find-caption cuid pos) (point) pos))
    pos))

;;; ----------------------------------------------------------------

(defun bat-get-cuid ()
  "Returns the cuid of the caption at the current point position."
  (beginning-of-line)
  (when (looking-at bat-cuid-pat)
    (goto-char (match-end 0))
    (string-to-int (buffer-match 0))))

(defvar bat-local-strip t)

(defvar bat-last-cuid 0)

(defun bat-pick ()
  "If the current point position is on a different caption than the last
time this was called, asks robin to display the message associated with
that caption.  Otherise, if the currently displayed message can be scrolled,
does so, else does (bat-next-caption)."
  (interactive)
  (let ((cuid (bat-get-cuid)))
    (cond ((null cuid)
	   (bat-next-caption))
	  ((/= cuid bat-last-cuid)
	   (setq bat-local-strip 1)
	   (bat-command
	    "[\n"
	    "b" cuid ":" (- (window-width) 1) ":s\n"
	    "MUi" cuid "\n"
	    "C(bat-update-to-cuid " cuid ")\n"
	    "]\n")
	   ;; try and keep next line visible...
	   (or (eobp)
	       (save-excursion
		 (end-of-line)
		 (or (not (re-search-forward bat-cuid-pat nil t))
		     (pos-visible-in-window-p)))
	       (recenter 1)))
	  ((bat-scroll-body)		; returns t if at end
	   (bat-next-caption)))))

(defun bat-update-to-cuid (cuid)
  (temp-set-buffer bat-captions-buf
    (save-excursion
      (and (bat-find-caption cuid) (bat-update-update-mark)))))

(defun bat-current-cuid ()
  "Returns the cuid of the currently displayed message, using bat-pick to
try and display one if necessary."
  (if (= bat-last-cuid 0)
      (bat-pick))
  bat-last-cuid)

(defun bat-active-cuid ()
  "Returns the \"currently active message\", which is the result
of either (bat-current-cuid) or (bat-get-cuid), when the variable
bat-body-active is t or nil, respectively."
  (or (if bat-body-active
	  (bat-current-cuid)
	(bat-get-cuid))
      (error "No current message")))

;;; ----------------------------------------------------------------

(defvar bat-begin-body-mark (make-marker))

;;; callback from robin
(defun ~bat-body (file cuid)
  "Makes the contents of FILE associated with CUID the currently
displayed message, and updates the current update mark if necessary."
  (temp-set-or-pop-to-buffer bat-display-buf (not bat-suspended)
; (read-string (concat "1" (prin1-to-string (current-buffer))))
    (delete-current-buffer-contents)
    (insert-file-contents file)
; (read-string (concat "2" (prin1-to-string (current-buffer))))
   (delete-file file)
    (setq bat-last-cuid cuid)
    (bat-set-display (concat "Body of message " cuid))
    (refresh-modeline)
    (search-forward "\n\n" nil t)
; (read-string (concat "3" (prin1-to-string (current-buffer))))

    (while (and (not (eobp)) (eolp))
      (delete-char 1))

    (set-marker bat-begin-body-mark (point))
; (read-string (concat "4" (prin1-to-string (current-buffer))))

    (save-excursion (run-hooks 'bat-body-hook))
; (read-string (concat "5" (prin1-to-string (current-buffer))))
    (bat-fixup-display-buf)
; (read-string (concat "6" (prin1-to-string (current-buffer))))
    )
; (read-string (concat "7" (prin1-to-string (current-buffer))))

  (bat-set-default-folder nil))

;;; callback from robin
(defun ~bat-metamail (file cuid)
  "Runs metamail on the contents of FILE with CUID the currently
displayed message, and updates the current update mark if necessary."
  (temp-set-or-pop-to-buffer bat-metamail-buf (not bat-suspended)
     (delete-current-buffer-contents)
     (comint-exec bat-metamail-buf "metamail" "metamail" nil
		  (list "-m" "batmail" "-d" "-z" file))
     (setq bat-last-cuid cuid)
     (bat-set-display (concat "Body of message " cuid))
     (refresh-modeline))
  (bat-set-default-folder nil))
     

(defun bat-toggle-stripping ()
  "Re-displays the currently displayed message with header stripping
toggled."
  (interactive)
  (cond ((> bat-last-cuid 0)
	 (setq bat-local-strip (not bat-local-strip))
	 (bat-command
	  "b" (bat-current-cuid)
	  ":" (- (window-width) 1)
	  ":" (if bat-local-strip "s" "n") "\n"))
	(t
	 (message "No current message"))))

;;; ----------------------------------------------------------------

(defun bat-scroll-body ()
  "If the currently displayed message can be scrolled forward any farther,
does so and returns nil, otherwise returns t."
  (interactive)
  (temp-pop-to-buffer bat-display-buf
    (move-to-window-line -1)
    (if (looking-at "[ \t\n]*\\'") ; eobp, ignoring white space
	t
      (scroll-up nil)
      (bat-fixup-display-buf)
      nil)))

(defun bat-reverse-scroll-body ()
  "Scrolls the currently displayed message backwards."
  (interactive)
  (temp-pop-to-buffer bat-display-buf
    (if (not (pos-visible-in-window-p (point-min)))
	(scroll-down nil)
      (message "Beginning of message"))
    (bat-fixup-display-buf)))		; this only if at beginning

(defun bat-next-caption ()
  "Moves the point forward to the next displayable caption (skipping
killed and duplicate captions), and if there is one, does a (bat-pick)."
  (interactive)
  ;; Yuck!
  ;; It would be much cleaner with recursion, but what about those 500
  ;; sequential duplicate messages?
  (let ((done nil))
    (while (not done)
      (setq done
	    (or (/= (forward-line 1) 0)
		(not (re-search-forward bat-cuid-pat nil t))))
      (beginning-of-line)
      (cond (done
	     (message "End of captions"))
	    ((looking-at (concat bat-cuid-exact-pat "[ _][^ 2K]*[2K]"))
	     (bat-update-update-mark))
	    (t
	     (bat-pick)
	     (setq done t))))))

(defun bat-prev-caption ()
  "Moves the point backward to the previous displayable caption (skipping
killed and duplicate captions), and if there is one, does a (bat-pick)."
  (interactive)
  (beginning-of-line)
  (if (re-search-backward (concat bat-cuid-exact-pat "[ _][^ 2K]* ") nil t)
      (bat-pick)
      (message "Beginning of captions")))

(defun bat-rot13-body ()
  "Replaces the contents of the currently displayed message with its
rot-13'd equivalent."
  (interactive)
  (~bat-info "Rotting message...")
  (temp-pop-to-buffer bat-display-buf
    (goto-marker bat-begin-body-mark)
    (call-process-region (marker-position bat-begin-body-mark)
			 (point-max)
			 "tr"
			 t
			 t
			 nil
			 "n-za-mN-ZA-M" "a-zA-Z")
    (bat-fixup-display-buf)
    (~bat-info "Rotting message...done")))

(defun bat-metamail-body ()
  "Runs metamail on current message"
  (interactive)
  (let ((cuid (bat-active-cuid))
	(fname (bat-temp-file)))
    (bat-command
     "[\n"
     "b" cuid ":2000:m\n"
     "MUi" cuid "\n"
     "C(bat-update-to-cuid " cuid ")\n"
     "]\n")
    ;; try and keep next line visible...
    (or (eobp)
	(save-excursion
	  (end-of-line)
	  (or (not (re-search-forward bat-cuid-pat nil t))
	      (pos-visible-in-window-p)))
	(recenter 1))))

(defun bat-un-underline-body ()
  "Removes any overstruck underlines in the currently displayed message."
  (interactive)
  (~bat-info "Removing underlines...")
  (temp-pop-to-buffer bat-display-buf
    (ununderline-region (marker-position bat-begin-body-mark) (point-max))
    (bat-fixup-display-buf))
  (~bat-info "Removing underlines...done"))

(defun bat-digest-forward ()
  "Scrolls the currently displayed message so that the next digest entry
is aligned with the top of the screen.  The variable bat-digest-separator
is a regexp which controls what is considered to separate digest entries."
  (interactive)
  (temp-pop-to-buffer bat-display-buf
    (move-to-window-line 0)
    (cond ((re-search-forward bat-digest-separator nil t)
	   (re-search-forward "^[^ \t\n]" nil t)
	   (beginning-of-line)
	   (recenter 0))
	  (t
	   (message "End of digest")))
    (bat-fixup-display-buf)))

(defun bat-digest-backward ()
  "Scrolls the currently displayed message so that the previous digest entry
is aligned with the top of the screen.  The variable bat-digest-separator
is a regexp which controls what is considered to separate digest entries."
  (interactive)
  (temp-pop-to-buffer bat-display-buf
    (move-to-window-line 0)
    (cond ((or (not (re-search-backward bat-digest-separator nil t))
	       (not (re-search-backward bat-digest-separator nil t)))
	   (goto-char (point-min))
	   (sit-for 0)			; bug-de-bug bug bug
	   (message "Beginning of digest"))
	  (t
	   (forward-line 1)
	   (re-search-forward "^[^ \t\n]" nil t)
	   (beginning-of-line)
	   (recenter 0)))
    (bat-fixup-display-buf)))

(defun bat-beginning-of-body ()
  "Scrolls the currently displayed message so that its beginning is
displayed."
  (interactive)
  (temp-pop-to-buffer bat-display-buf
    (goto-char (point-min))
    (sit-for 0)				; work around emacs bug
    (bat-fixup-display-buf)))

(defun bat-end-of-body ()
  "Scrolls the currently displayed message so that its end is displayed."
  (interactive)
  (temp-pop-to-buffer bat-display-buf
    (goto-char (point-max))
    (sit-for 0)				; emacs bug
    (bat-fixup-display-buf)))

;;; ----------------------------------------------------------------

(defun bat-beginning-of-folder ()
  (interactive)
  (if (not (re-search-backward "^---" nil t))
      (goto-char (point-min))))

(defun bat-end-of-folder ()
  (interactive)
  (forward-line 1)
  (beginning-of-line)
  (if (not (re-search-forward "^---" nil t))
      (goto-char (point-max)))
  (beginning-of-line))

(defun bat-punt ()
  "Moves the current point position to the end of the current folder
and sets the current update mark there as well."
  (interactive)
  (bat-end-of-folder)
  (save-excursion
    (forward-line -1)
    (beginning-of-line)
    (if (looking-at "^$")
	(forward-line -1))		; low density
    (if (looking-at bat-cuid-pat)
	(bat-set-update-mark))))

(defun bat-set-quit-line ()
  "Sets the current update mark to the previous line."
  (interactive)
  (save-excursion
    (forward-line -1)
    (beginning-of-line)
    (bat-set-update-mark)))

;;; ----------------------------------------------------------------

(defun bat-delete (cuid)
  "Deletes the message associated with CUID."
  (interactive (list (bat-get-cuid)))
  (and cuid (bat-command "Mdn" cuid "\n")))

(defun bat-undelete (cuid)
  "Undeletes the message associated with CUID."
  (interactive (list (bat-get-cuid)))
  (and cuid (bat-command "MDn" cuid "\n")))

(defun bat-purge-deletions (&optional all)
  "Purges deleted messages from the current folder.  If optional prefix
argument ALL is non-nil, purges deleted messages from all folders."
  (interactive "P")
  (if all
      (bat-command "pa\n")
    (let ((purge-folder (bat-current-folder)))
      (if purge-folder
	  (bat-command "d" purge-folder "\n"
		       "pd\n" )
	(error "No current folder")))))


(defun bat-unread (cuid)
  "Marks the message associated with CUID as unread."
  (interactive (list (bat-get-cuid)))
  (and cuid (bat-command "Mun" cuid "\n")))

(defvar bat-folder-default "")

(defun bat-move (cuid dir &optional append)
  "Moves the message associated with CUID into FOLDER, appending (rather
than inserting by date) if the optional parameter APPEND is non-nil."
  (interactive
   (let ((pa current-prefix-arg)
	 (x (bat-active-cuid)))
     (list x
	   (get-tty-str (concat (if pa "Append-Delete" "Move")
				" message " x " to folder")
			bat-folder-default)
	   pa)))
  (if (/= cuid 0)
      (bat-command "d" dir "\n"
		   "c" (if append "a" "c") cuid "\n"))
  (setq bat-folder-default dir))

(defun bat-copy (cuid dir &optional append)
  "Copies the message associated with CUID into FOLDER, appending (rather
than inserting by date) if the optional parameter APPEND is non-nil."
  (interactive
   (let ((pa current-prefix-arg)
	 (x (bat-active-cuid)))
     (list x
	   (get-tty-str (concat (if pa "Append" "Copy")
				" message " x " to folder")
			bat-folder-default)
	   pa)))
  (if (/= cuid 0)
      (bat-command "d" dir "\n"
		   "c" (if append "A" "C") cuid "\n"))
  (setq bat-folder-default dir))

(defun bat-vote (cuid)
  "Asks robin to administer the vote for the message associated with CUID."
  (interactive (list (bat-active-cuid)))
  (bat-command "v" cuid "\n"))
  
(defun bat-print-body (cuid)
  "Prints the message associated with CUID."
  (interactive (list (bat-active-cuid)))
  (bat-command "P" cuid "\n"))

(defun bat-old-kill ()
  (interactive)
  (message "%s"
   (substitute-command-keys
    "This keybinding has been changed to \\[bat-kill]")))

(defun bat-kill (cuid folder &optional from permanent)
  "Takes the either the subject or from fields of the caption associated
with CUID, and adds a kill entry in FOLDER for it.  If FROM is nil
then the subject is used, otherwise the from field is.  If PERMANENT
is t, no timeout will be used."
  (interactive
   (list (bat-get-cuid) (bat-current-folder) current-prefix-arg))
  (and cuid folder
       (let ((fcuid (bat-first-cuid folder)))
	 (and fcuid
	      (bat-command
	       "d" folder "\n"
	       "k" (if from "f" "s") (if permanent "n" "t")
	       fcuid ":" cuid "\n")))))

(defun bat-unkill ()
  (interactive)
  (message "Not implemented yet"))

(defun bat-kill-regexp (regexp folder &optional from permanent)
  "Takes the either the subject or from fields of the caption associated
with CUID, and adds a kill entry in FOLDER for it.  If FROM is nil
then the subject is used, otherwise the from field is.  If PERMANENT
is t, no timeout will be used."
  (interactive
   (list (read-string "Kill regexp: ")
	 (bat-current-folder)
	 current-prefix-arg))
  (and regexp folder
       (let ((fcuid (bat-first-cuid folder)))
	 (and fcuid
	      (bat-command
	       "d" folder "\n"
	       "K" (if from "f" "s") (if permanent "n" "t")
	       fcuid ":/" regexp "\n")))))

(defun bat-append-to-file (cuid file)
  "Append the message associated with CUID to FILE."
  (interactive
   (let ((x (bat-active-cuid)))
     (list x
	   (if current-prefix-arg
	       (concat "|"
		       (read-string
			(concat "Pipe message " x " to command: ")))
	     (read-file-name (concat "Append message " x " to file ["
				     bat-default-append-file "] : ")
			     bat-default-append-directory
			     (concat bat-default-append-directory
				     bat-default-append-file))))))
  (if current-prefix-arg
      nil
    (setq bat-default-append-directory (file-name-directory file)) 
    (setq bat-default-append-file (file-name-nondirectory file)))
  (bat-command "a" cuid ":" file "\n"))

(defun bat-append-displayed-to-file (file)
  "Append the currently displayed message (IN THE DISPLAYED FORM!) to FILE."
  (interactive
   (list
    (read-file-name
     (concat "Append message " (bat-current-cuid) " to file ["
	     bat-default-append-file "] : ")
     bat-default-append-directory
     (concat bat-default-append-directory bat-default-append-file))))
  (save-excursion
    (set-buffer bat-display-buf)
    (append-to-file (point-min) (point-max) file)))

(defun bat-get-updated-captions (folder)
  "Update the captions for FOLDER since the last read date."
  (interactive (list (bat-current-folder)))
  (and folder (bat-get-captions folder "updates")))

(defun bat-read-subs-meth (def)
  "Prompts for and returns a subscription method, default DEF."
  (save-excursion
    (set-buffer bat-scratch-buf)
    (delete-current-buffer-contents)
    (insert "Subscription methods:\n"
	    "n\tNormal subscription\n"
	    "a\tAsk suscription (updates shown only upon demand)\n"
	    "s\tShow-all subscription (always see every message)\n"
	    "p\tPrint subscription (This doesn't work)\n")
    (get-prompted-key bat-scratch-buf
		      "Subscription method"
		      def
		      "---- BatSubscription methods"
		      nil)))

(defun bat-subscribe (folder method)
  "Subscribe to FOLDER using method METHOD (a character, one of [nasp])."
  (interactive
   (list (get-tty-str "Subscribe to folder" (bat-default-folder))
	 (bat-read-subs-meth ?n)))
  (bat-command
   "d" folder "\n"
   "S" (char-to-string method) "\n"))

(defun bat-unsubscribe (folder)
  "Unsubscribe to FOLDER."
  (interactive
   (list (get-tty-str "Unsubscribe to folder" (bat-current-folder))))
  (or (null folder)
      (equal folder "")
      (bat-command "d" folder "\nSu\n")))

(defun bat-list-folders (under imm-only)
  "Asks robin to send a list of folders that are descendants of the folder
UNDER and their subscription status.  If the second parameter, IMM-ONLY is
non-nil, only immediate children are included."
  (interactive
   (let* ((folder (get-tty-str "List subfolders of" (bat-current-folder)))
	  (wild (string-match "\\.*\\*$" folder)))
     (or (and wild (list (substring folder 0 wild) nil))
	 (list folder t))))
  (bat-command "d" under "\n" (if imm-only "l" "L") "\n"))

(defun bat-read-help-topic (def)
  "Prompts for and returns a help topic, default DEF."  
  (save-excursion
    (set-buffer bat-scratch-buf)
    (delete-current-buffer-contents)
    (insert "The following help topics are available:\n"
	    "o\tOverview of BatMail\n"
	    "c\tKeyboard commands\n"
	    "v\tUser-settable variables\n"
	    "h\tUser-defineable hooks\n"
	    "f\tFiles you can supply BatMail\n")
    (get-prompted-key bat-scratch-buf
		      "BatHelp on"
		      def
		      "---- BatHelp topics"
		      nil)))

(defun bat-help (topic)
  "Makes a help message for TOPIC (a character, one of [ocvhf]) the
currently displayed message."
  (interactive (list (bat-read-help-topic ?o)))
  (message "Getting help info...")
  (temp-pop-to-buffer bat-display-buf
    (delete-current-buffer-contents)
    (setq bat-last-cuid 0)
    (bat-set-display "Help")
    (cond ((= topic ?c)
	   (insert (buffer-bindings-string bat-captions-buf)))
	  (t
	   (let ((file
		  (concat bat-help-dir "/"
			  (cond ((= topic ?o) "overview.gnu")
			        ((= topic ?v) "variables.gnu")
				((= topic ?h) "hooks.gnu")
				((= topic ?f) "files.both")))))
	     (cond ((file-readable-p file)
		    (insert-file-contents file))
		   (t
		    (error "Can't read help file"))))))
    (bat-fixup-display-buf))
    (message "Getting help info...done"))

;;; ----------------------------------------------------------------

(defun ~bat-want-subscription (folder)
  "Displays message advertising a new folder, FOLDER, and sets the
subscription/read default to it."
  (~bat-info
   (concat
    "To subscribe to " folder
    (substitute-command-keys ", use the \\[bat-subscribe] command; to read use \\[bat-get-captions]")))
  (bat-set-default-folder folder))  

(defun ~bat-want-vote ()
  "Advertises a vote is availabe for this message."
  (~bat-info
   (substitute-command-keys
    "A vote is requested; use the \\[bat-vote] command to vote")))

;;; ----------------------------------------------------------------

(defvar bat-default-folder nil)

(defun bat-default-folder ()
  "Returns the \"default\" folder, which is normally the current folder,
but sometimes not."
  (or bat-default-folder (bat-current-folder) bat-default-folders))

(defun bat-set-default-folder (folder)
  "Sets the \"default\" folder to FOLDER.  Nil will revert back to
the current folder."
  (setq bat-default-folder folder))

;;; ----------------------------------------------------------------

(defun disabled-key ()
  "Displays an error message saying that this key is disabled."
  (interactive)
  (error "That key is disabled here"))

(defun bat-make-captions-keymap ()
  "Returns the default bat-captions-mode keymap."
  (let ((km (make-keymap)))


    ;; disable alphabetic keys
    (let ((i 32))
      (while (< i 128)
	(define-key km (char-to-string i) 'disabled-key)
	(setq i (1+ i))))

    (define-key km "u" (make-sparse-keymap))

    (define-key km " " 'bat-pick)
    (define-key km "n" 'bat-next-caption)
    (define-key km "p" 'bat-prev-caption)
    (define-key km "v" 'bat-scroll-body)
    (define-key km "b" 'bat-reverse-scroll-body)
;;; (define-key km "\C-H" 'bat-reverse-scroll-body)
    (define-key km "\C-?" 'bat-reverse-scroll-body)
    (define-key km "K" 'bat-kill)
    (define-key km "k" 'bat-old-kill)
    (define-key km "uK" 'bat-unkill)
    (define-key km "uk" 'bat-unkill)	; synonym for convenience
    (define-key km "\C-k" 'bat-kill-regexp)
    (define-key km "<" 'bat-beginning-of-body)
    (define-key km ">" 'bat-end-of-body)
    (define-key km "Q" 'bat-quit)
    (define-key km "q" 'bat-suspend)
;;; (define-key km "\C-C" 'bat-quit)
    (define-key km "\C-X\C-C" 'bat-quit)
;;; (define-key km "m" 'bat-send)	; for bags people
    (define-key km "s" 'bat-send)
    (define-key km "r" 'bat-reply)
    (define-key km "f" 'bat-forward)
    (define-key km "w" 'bat-wide-reply)
    (define-key km "W" 'bat-all-reply)
    (define-key km "F" 'bat-resend)
    (define-key km "\r" 'next-line-no-insert)
    (define-key km "\C-n" 'next-line-no-insert)
    (define-key km "j" 'next-line-no-insert)
    (define-key km "k" 'previous-line)
    (define-key km "'" 'previous-line)
    (define-key km "\C-F" 'bat-digest-forward)
    (define-key km "\C-B" 'bat-digest-backward)
    (define-key km "h" 'bat-help)
    (define-key km "?" 'bat-help)
    (define-key km "a" 'bat-append-to-file)
    (define-key km "A" 'bat-append-displayed-to-file)
    (define-key km "\C-W" 'bat-toggle-stripping)
    (define-key km "R" 'bat-get-captions)
    (define-key km "^" 'bat-set-quit-line)
    (define-key km "/" 'bat-punt)
    (define-key km "," 'bat-beginning-of-folder)
    (define-key km "." 'bat-end-of-folder)
    (define-key km "N" 'bat-get-updated-captions)
    (define-key km "S" 'bat-subscribe)
    (define-key km "U" 'bat-unsubscribe)
    (define-key km "uS" 'bat-unsubscribe)
    (define-key km "us" 'bat-unsubscribe) ; synonym for convenience
    (define-key km "L" 'bat-list-folders)
    (define-key km "d" 'bat-delete)
    (define-key km "ud" 'bat-undelete)
    (define-key km "c" 'bat-move)
    (define-key km "C" 'bat-copy)
    (define-key km "ur" 'bat-unread)
    (define-key km "u " 'bat-unread)
    (define-key km "x" 'bat-rot13-body)
    (define-key km "_" 'bat-un-underline-body)
    (define-key km "g" 'bat-gripe)
    (define-key km "P" 'bat-post)
    (define-key km "\ep" 'bat-purge-deletions)
    (define-key km "V" 'bat-vote)
    (define-key km "!" 'bat-metamail-body)
    km))

(defvar bat-captions-keymap (bat-make-captions-keymap))

(defvar bat-init-captions-hook nil
  "If non-nil, should be the name of a function to call after\
initializing the captions buffer.")

(defvar bat-new-mail-p nil "T if there is new mail.")
(defvar bat-new-mail-count 0 "Number of pieces of new mail.")
(defvar bat-new-mail-count-str "" "For displaying in modleine.")
(defvar bat-state nil "String describing current state of batmail.")

(defun ~bat-new-mail-count (count)
  "Set the number of pending new mail messages to COUNT."
  (when (and bat-announce-new-mail (/= count bat-new-mail-count))
    (setq bat-new-mail-count count)
    (setq bat-new-mail-p (> count 0))
    (when bat-new-mail-p
      (setq bat-new-mail-count-str (concat count))
      (~bat-urgent
       (concat "You have new mail ("
	       count " piece" (if (= count 1) ")" "s)"))))
    (refresh-modeline)))

(defun ~bat-begin-state (str)
  "Record STRING as being robin's current activity."
  (setq bat-state (concat str "..."))
  (refresh-modeline)
  (if bat-stingy-display (setq bat-state (substring bat-state 0 1)))
  (if (not (buffer-visible-p bat-captions-buf))
      (~bat-info bat-state)))

(defun ~bat-end-state ()
  "Indicate that robin's current activity has finished successfully."
  (setq bat-state (concat bat-state (if bat-stingy-display "D" "done")))
  (refresh-modeline)
  (if (not (buffer-visible-p bat-captions-buf))
      (~bat-info bat-state))
  (sit-for 0)
  (setq bat-state nil))

(defun ~bat-fail-state ()
  "Indicate that robin's current activity has failed."
  (setq bat-state (concat bat-state (if bat-stingy-display "F" "failed")))
  (refresh-modeline)
  (if (not (buffer-visible-p bat-captions-buf))
      (~bat-info bat-state))
  (sit-for 0)
  (setq bat-state nil))

(defun bat-create-captions-buf ()
  "Create the bat-captions buffer, and put it in to bat-captions-mode."
  (set-buffer (get-buffer-create bat-captions-buf))
  (erase-buffer)
  (set-mark 0)
  (set-buffer-modified-p nil)
  (bat-captions-mode))

(defvar bat-captions-help-mode-str "[? for help]")

(defun bat-captions-mode ()
  "Major mode for browsing AMS folder captions using BatMail.
The following keys have special meaning.  Most other Emacs commands
still work as they normally do.  To make one-handed browsing easier,
the return and j keys work like C-n (next-line) and the k key works
like C-p (previous-line).

--key--	    --description--
SPC	    Scroll forward in message or read next message
n	    Read next message
p	    Read previous message
RET	    Next line
j	    Next line
k	    Previous line

q	    Temporarily suspend batmail; use \\[batmail] to resume
Q	    Exit batmail

s	    Send mail
r	    Reply to currently displayed message
w	    Reply to readers of message
W	    Reply to readers and sender of message
P	    Send to the current folder (i.e., post)
f	    Forward body of a message
F	    Resend a message
g	    Send a bat-gripe

!	    Run MetaMail on message
a	    Append message to file
\\[universal-argument] a	    Pipe message to a command
v	    Scroll currently displayed message forward
b	    Scroll currently displayed message backward
C-f	    Move to next message in digest
C-b	    Move to previous message in digest
x	    Descramble (rot13) current message
_	    Remove overstruck underlines from current message
C-w	    Toggle stripping of headers in current body
\\[bat-print-body]	    Print the current message body
<	    Scroll currently displayed message to beginning
>	    Scroll currently displayed message to end
V	    Vote on a message (only for messages with votes in them)

d	    Delete a message
u d	    Undelete a message
\\[bat-purge-deletions]	     Purge deletions in folder
\\[universal-argument] \\[bat-purge-deletions]	     Purge deletions in all folders

u SPC	    Mark a message as unseen
c	    Move a message to another folder
\\[universal-argument] c	    Append and delete a message to another folder
C	    Copy a message to another folder
\\[universal-argument] C	    Append a message to another folder

K	    Kill a message subject
\\[universal-argument] K	    Kill a message author
u k	    Remove a kill spec
C-k	    Kill a regular expression

R	    Read messages from a folder
N	    Read new messages from current folder
S	    Subscribe to folder
U	    Un-subscribe to folder
L	    List all folders under this one

,	    Goto the start of the current folder
.	    Goto the end of the current folder
/	    Punt (goto end, mark all messages read) this folder
^	    Set quit-line (Put the last-read mark before this message)"
  (setq track-eol nil)
  (use-local-map bat-captions-keymap)
  (setq bat-state nil)
  (setq mode-line-format '("---- BatCaptions  "
			   (bat-new-mail-p
			    ("(Mail:" bat-new-mail-count-str ")  "))
			   (global-mode-string ("  " global-mode-string))
			   "  " bat-captions-help-mode-str "  (%p)"
			   (bat-state ("  " bat-state))))
  (setq truncate-lines t)
  (setq major-mode 'bat-captions-mode)
  (setq mode-name "BatMail-Captions")
  (setq buffer-read-only t)

  (save-excursion (run-hooks 'bat-captions-mode-hook))

  ;; These are here so they will get any keybindings from the hook
  (setq bat-captions-help-mode-str
	(substitute-command-keys "[\\[bat-help] for help]"))
  (setq bat-display-scroll-help-str
	(substitute-command-keys "[\\[bat-scroll-body] to scroll]")))

(defvar bat-display "")
(defvar bat-more nil)

(defun bat-set-display (str)
  "Sets the currently displayed message indicator to STRING."
  (setq bat-display str)
  (refresh-modeline))

(defun bat-fixup-display-buf ()
  "Fixes up the modeline for the bat-display buffer to more accurately
reflect the actual state of the buffer."
;  (read-string (prin1-to-string (current-buffer)))
  (when (buffer-visible-p)
;    (read-string "is-visible")
    (move-to-window-line -1)
    (let ((more (not (looking-at "[ \t\n]*\\'"))))
      (if (not (equal more bat-more))
	  (refresh-modeline))
      (setq bat-more more))
    (move-to-window-line nil)))

(defun bat-create-display-buf ()
  "Create the bat-display buffer, put it in bat-display-mode,
and put the initial display text into it."
  (set-buffer (get-buffer-create bat-metamail-buf))
  (comint-mode)
  (set-buffer (get-buffer-create bat-display-buf))
  (erase-buffer)
  (bat-display-mode)
  (let ((introf
	 (concat bat-etc-dir "/"
		 (if bat-stingy-display "intro.small" "intro")))
	(newsf (concat bat-etc-dir "/news")))
    (if (file-readable-p introf)
	(insert-file-contents introf))
    (replace "$bv$" bat-version)
    (goto-char (point-max))
    (if (file-readable-p newsf)
	(progn (newline) (insert-file-contents newsf)))))

(defvar bat-display-scroll-help-str "[v to scroll]")

(defun bat-display-mode ()
  "Major mode for batmail's main display buffer"
  (text-mode)
  (setq bat-more nil)
  (setq mode-line-format
	'("---- Bat" bat-display
	  (bat-more ("    " bat-display-scroll-help-str "  --More--(%p)"))))
  (setq bat-display "Mail")
  (setq major-mode 'bat-display-mode)
  (setq mode-name "BatMail-Display"))

;;; ----------------------------------------------------------------

(defun bat-old-quit ()
  (interactive)
  (error
   (substitute-command-keys
    "This keybinding has been changed to \\[bat-quit]")))

(defun bat-quit ()
  "Gracefully exit batmail."
  (interactive)
  (bat-save-last-update t)
  (bat-exit))

;;; ----------------------------------------------------------------

(defun bat-check-mail ()
  "Move any new mail into the appropiate folders."
  (interactive)
  (bat-command "na\n"))

(defvar bat-goto-new-message t)	; this is a stupid hack

(defun bat-get-captions (folders since)
  "Update captions for FOLDERS, which is a list of comma/whitespace
separated folder names, each optionally suffixed with '*' to indicate
the entire subtree under it, since DATE."
  (interactive
   (list (get-tty-str "Folders" (bat-default-folder))
	 (get-tty-str "Since" "updates")))
  (if (equal since "u") (setq since ""))
  (if (equal since "updates") (setq since ""))
  (if (equal since "c") (setq since "creation"))
  (bat-set-caption-width (window-width))
  (bat-save-last-update)
  (if (equal folders "*")
      (temp-set-buffer bat-captions-buf
	(bat-enter)
	(ignoring-read-only (erase-buffer))))
  (let ((pos 0))
    (while (setq pos (string-match "[^ ,\t]+" folders pos))
      (let* ((end (match-end 0))
	     (folder (substring folders pos end))
	     (len (- end pos)))
	(if (equal (substring folder (1- len)) "*")
	    (~bat-info
	     (concat "Checking the headlines"
		     (if (equal folder "*") "" (concat " on " folder))
		     "...")))
	(if (or (equal folder "*")
		(string-match bat-check-mail-regexp folder))
	    (bat-check-mail))
	(cond ((equal (substring folder (1- len)) "*")
	       (setq folder (substring folder 0 (setq len (1- len))))
	       ;; chop '.' separators
	       (while (and (> len 0)
			   (equal (substring folder (1- len)) "."))
		 (setq folder (substring folder 0 (setq len (1- len)))))
	       (bat-command
		"[\n"
		"d" folder "\n"
		(if bat-folders-only "F" "H") since "\n"
		"C(~bat-urgent \"Done checking "
		(if (> len 0) folder "folders") "\")\n"
		"]\n"))
	      (t
	       (bat-command "d" folder "\nh" since "\n")))
	(setq pos end)))))
		 

;;; ----------------------------------------------------------------

(defun bat-set-up-screen ()
  "Takes over the display, putting the various bat-buffers in their
correct locations and sizes."
  (when bat-reading
    (switch-to-buffer bat-display-buf)
    (delete-other-windows)
    (let ((h (window-height))
	  (w (window-width)))
      (if (>= bat-caption-percentage 0)
	  (split-window (selected-window)
			(/ (* h (- 100 bat-caption-percentage)) 100))
	(split-window (selected-window)
		      (/ (* w (+ 100 bat-caption-percentage)) 100)
		      t))
      (other-window 1)
      (switch-to-buffer bat-captions-buf)))
  (when bat-sending
    (bat-set-up-comp-screen)))

(defvar bat-active nil)
(defvar bat-reading nil)
(defvar bat-sending nil)
(defvar bat-suspended nil)

(defvar bat-saved-window-configuration nil)

(defun bat-enter ()
  "Makes sure everything is intialized for batmail to run."
  (bat-call-robin)
  (when (not bat-active)
    (get-buffer-create bat-scratch-buf)
    (bat-create-captions-buf)
    (bat-create-display-buf)
    (bat-create-composition-buf)
    (bat-command "g" bat-headers "\n")
    (setq bat-saved-window-configuration
	  (current-window-configuration))
    (setq bat-active t)
;   (bat-command "C(bat-check-new-user)\n")
    t))

	
(defun bat-exit-composition ()
  "For use after sending mail, either restores the main batmail display
or exits batmail entirely."
  (setq bat-sending nil)
  (cond (bat-suspended
	 nil)
	(bat-reading
	 (bat-set-up-screen)
	 t)
	(t
	 (bat-exit)
	 nil)))

(defun bat-suspend ()
  "Temporarily exits batmail, hiding all its windows, and restoring
the previous screen layout."
  (interactive)

  (setq bat-suspended t)

  (set-window-configuration bat-saved-window-configuration)
  
  (bat-save-last-update)
  (bat-command "U\n")

  (bury-buffer bat-display-buf)
  (bury-buffer bat-captions-buf)
  (bury-buffer bat-composition-buf)

  (message "%s" (substitute-command-keys "Type \\[batmail] to resume batmail")))

(defvar bat-started-from-shell nil)

(defun bat-auto-start ()
  "Starts up batmail using the values of the environment variables
\"BATFOLDERS\" and \"BATSINCE\" as the folder list and date."
  (setq bat-started-from-shell t)
  (batmail (getenv "BATFOLDERS") (getenv "BATSINCE")))

(defun batmail (&optional folders since)
  "Starts up batmail reading FOLDERS since DATE, prompting for either
 missing parameter."
  (interactive)

  (cond ((not bat-reading)
	 (save-window-excursion
	   (cond ((and bat-started-from-shell
		       (not bat-stingy-display)
		       bat-obnoxious-logo)
		  (switch-to-buffer "*bat-obnoxious-logo*")
		  (setq mode-line-format "In color!")
		  (setq truncate-lines t)
		  (erase-buffer)
		  (delete-other-windows)
		  (let ((obnxf (concat bat-etc-dir "/" "initscreen")))
		    (if (file-readable-p obnxf)
			(insert-file-contents obnxf)))))
	   (if (not folders)
	       (setq folders (get-tty-str "Read folders" bat-default-folders)))
	   (if (not since)
	       (setq since (get-tty-str "Since" "updates"))))
	 (if (get-buffer "*bat-obnoxious-logo*")
	     (kill-buffer "*bat-obnoxious-logo*"))
	 (setq bat-reading t)
	 (bat-enter)
	 (if bat-suspend-on-start
	     (bat-suspend)
	     (bat-set-up-screen))
	 (if (and folders (not (equal folders "")))
	     (bat-get-captions folders since)))
	(bat-suspended
	 ;; redo the window config if un-suspending
	 (if bat-suspended 
	     (setq bat-saved-window-configuration
		   (current-window-configuration)))
	 (setq bat-suspended nil)
	 (bat-set-up-screen))
	(t
	 (bat-set-up-screen))))

(defun bat-exit ()
  "Exits batmail."
  (if (not (bat-flush-robin t))
      (bat-cleanup t)
    (bat-command "CRemember, crime does not pay!\nC(bat-cleanup)\n")))

(defun bat-cleanup (&optional dead)
  "Does final bat-cleanup.  Only parameter is DEAD, non-nil if robin
is dead."
  (if (not bat-suspended)
      (progn (set-window-configuration bat-saved-window-configuration)
	     ; Work around apparent bug in set-window-configuration
	     (set-buffer (window-buffer (selected-window)))))
  (setq bat-reading nil)
  (setq bat-active nil)
  (setq bat-last-cuid 0)
  (kill-buffer bat-captions-buf)
  (kill-buffer bat-display-buf)
  (bury-buffer bat-composition-buf)
  (if dead
      (~bat-urgent "Robin is dead!"))
  (if (and bat-started-from-shell bat-exit-to-shell)
      (save-buffers-kill-emacs)))

;;; ----------------------------------------------------------------

(autoload 'ispell-word "ispell" "Check spelling of word at or before point." t)
(autoload 'ispell-buffer "ispell" "Check spelling of all words in buffer." t)

(defun bat-submit-2 () (interactive) (bat-submit))

(defun bat-make-composition-keymap ()
  "Returns the default bat-composition-mode keymap."
  (let ((km (make-sparse-keymap)))
    (define-key km "\C-C\C-C" 'bat-submit-2) ; gnu-style
    (define-key km "\C-X\C-S" 'bat-submit)
    (define-key km "\C-X\C-N" 'bat-submit-no-validate)
    (define-key km "\C-XS" 'bat-submit-no-ispell)
    (define-key km "\C-X\C-C" 'bat-abort-mail)
    (define-key km "\C-Xq" 'bat-suspend)
    (define-key km "\C-M" 'bat-mail-newline)
    (define-key km "\n" 'bat-mail-newline); for braindamaged telnet server
    (define-key km "\C-X\C-Y" 'bat-insert-body)
    (define-key km "\C-X\C-B" 'bat-toggle-blind)
    (define-key km "\C-X?" 'bat-mail-help)
    (define-key km "\er" 'bat-validate)
    (define-key km "\e$" 'ispell-word)
    km))

(defvar bat-composition-keymap (bat-make-composition-keymap))

(defun bat-create-composition-buf ()
  "Create the bat-composition buffer, and put it in to bat-composition-mode."
  (set-buffer (get-buffer-create bat-composition-buf))
  (bat-composition-mode))

(defvar bat-composition-help-mode-str
  "C-x C-s sends, C-x C-c aborts, C-x ? for help]")

;;(defvar bat-comp-old-auto-fill-op nil)
;;(defmacro set-auto-fill-function ()
;;
;;
;;  (if (>= 	(let*  (
;;			(ev  (emacs-version))
;;			(start (string-match   "[1-9][0-9]*" ev))  )
;;			(string-to-int (substring ev start)))
;; 		19)
;;	'(progn	(setq bat-comp-old-auto-fill-op 
;;			(symbol-function 'auto-fill-function))
;;		(fset 'auto-fill-function 'bat-comp-auto-fill) )
;;	'( progn 
;;		(setq bat-comp-old-auto-fill-op auto-fill-hook)
;;		(setq auto-fill-hook 'bat-comp-auto-fill) ) )
;;)
;;
;;(defun bat-comp-auto-fill ()
;;  (if (bat-in-headers-p)
;;      (setq fill-prefix "\t")
;;    (if (string= fill-prefix "\t")
;;	(setq fill-prefix nil)))
;;  (funcall bat-comp-old-auto-fill-op))

(defun bat-composition-mode ()
  "Major mode for composing (bat)mail.
Most normal GNU Emacs commands work here. (for example, C-f to go forward,
C-b to go backward, C-n for next line, C-p for previous line.)

--key--	    --description--
\\[bat-submit]	    Send the message
\\[bat-submit-no-validate]	    Send the message without validating addresses
\\[bat-submit-no-ispell]	    Send the message without checking spelling
	    (assuming bat-ispell-before-submit is non-nil)
\\[bat-abort-mail]	    Abort sending this message

\\[bat-suspend]	    Temporarily suspend batmail; use \\[batmail] to resume
\\[bat-insert-body]	    Insert the body of the last message read here
\\[bat-toggle-blind]	    Toggle whether a bcc is sent to you (The initial state is the
	    value of your bat-send-bcc variable)

\\[bat-validate]	    Validate the destination addresses, without sending
\\[ispell-word]	    Correct spelling of word at or before point

RET works intelligently in the headers area.
Use \\[open-line] in the headers area to add a new header."
  (text-mode)
  (auto-fill-mode 1)
  (setq mode-line-format
	'("---- BatComposition (" (bat-local-blind "Blind" "No blind")
	  ")    " bat-composition-help-mode-str))
  (setq major-mode 'bat-composition-mode)
  (setq mode-name "BatMail-Composition")
  (setq buffer-auto-save-file-name bat-composition-ckp-file)
  (use-local-map bat-composition-keymap)

;;  (make-variable-buffer-local 'bat-comp-old-auto-fill-op)
;;  (set-auto-fill-function)

  (save-excursion (run-hooks 'bat-composition-mode-hook))

  ;; This is here so it will get any keybindings from the hook
  (setq bat-composition-help-mode-str
	(substitute-command-keys
"[\\[bat-submit] sends, \\[bat-abort-mail] aborts, \\[bat-mail-help] for help]")))

;;; ----------------------------------------------------------------

(defvar bat-replying-to 0 "Cuid of mail we're replying to.")

(defun bat-send ()
  "Sets up things to send a new message."
  (interactive)
  (bat-get-template "m" nil nil))

(defun bat-reply ()
  "Sets up things to reply to the sender of the currently active message."
  (interactive)
  (bat-get-template "r" t t))

(defun bat-wide-reply ()
  "Sets up things to reply to the readers of the currently active message."
  (interactive)
  (bat-get-template "w" t t))

(defun bat-all-reply ()
  "Sets up things to reply to the sender and readers of the currently active
message."
  (interactive)
  (bat-get-template "a" t t))

(defun bat-forward ()
  "Sets up things to forward the currently active message."
  (interactive)
  (bat-get-template "f" nil t))

(defun bat-gripe ()
  "Sets up things to send a gripe about batmail to the maintainer."
  (interactive)
  (bat-get-template "g" nil nil))

(defun bat-post ()
  "Sets up things to send a new message to the current folder."
  (interactive)
  (bat-get-template "p" nil nil))

(defun bat-set-up-comp-screen ()
  (cond ((buffer-visible-p bat-composition-buf)
	 (pop-to-buffer bat-composition-buf))
	(t
	 (pop-to-buffer bat-display-buf)
	 (cond ((not (equal bat-replying-to 0))
		(split-window)
		(other-window 1)))
	 (switch-to-buffer bat-composition-buf))))

(defun bat-get-template (code reply needscuid)
  "Sets up things for an outgoing message.  CODE is a character in [mrwafgp],
indicating what type of message it is, REPLY is non-nil if this is a reply
to the currently displayed message, and NEEDSCUID is non-nil if the currently
active message is referenced."
  (bat-enter)
  (setq bat-sending t)
  (let ((cuid (if needscuid (bat-active-cuid) 0)))
    (setq bat-replying-to (if reply cuid 0))
    (bat-set-up-comp-screen)
    (if (or (not (buffer-modified-p)) (get-tty-bool "Erase old message" t))
	(progn
	  (erase-buffer)
	  (cond ((equal code "m")
		 (~bat-mail nil "To: \nCC: \nSubject: \n\n"))
;;;		((equal code "g")
;;;		 (~bat-mail nil
;;;			    (concat "To: " bat-gripe-address "\n"
;;;				    "CC: \n"
;;;				    "Subject: \n\n")))
		((equal code "p")
		 (~bat-mail nil
			    (concat "To: " (bat-current-folder) "\n"
				    "CC: \nSubject: \n\n")))
		(t
		 (bat-command "m" code cuid "\n")))))))

(defun bat-resend (cuid addr)
  "Resends CUID to ADDRESS."
  (interactive
   (let* ((cuid (bat-active-cuid))
	  (to (get-tty-str (concat "Resend message " cuid " to"))))
     (list cuid to)))
  (bat-command "z" cuid ":" addr "\n"))

;;; ----------------------------------------------------------------

; (defvar bat-end-of-headers-mark (make-marker))
(defvar bat-local-blind nil)
(defvar bat-outgoing-file nil)

(defun ~bat-mail (file &optional template)
  "Creates an outgoing message composition buffer.  The initial contents
are either FILE or STRING, if FILE is nil."
  (pop-to-buffer bat-composition-buf)

  (let ((init-contents (buffer-substring (point-min) (point-max)))
	to-addr)
    (erase-buffer)
    (cond (file
	   (insert-file-contents file))
	  (t
	   (if template (insert template))
	   (goto-char (point-min))
	   (setq file (bat-temp-file))))

    (setq bat-outgoing-file file)
    (setq bat-local-blind bat-send-bcc)

    (cond ((search-forward "\n\n" nil t)
	   (let* ((to-contents
		   (save-excursion
		     (let* ((to-start
			     (and (re-search-backward "^To:[ \t]*" nil t)
				  (match-end 0)))
			    (to-stop
			     (and (= (forward-line 1) 0)
				  (re-search-forward "^[A-Za-z-]*:[ \t]*\\|^$"
						     nil t)
				  (match-beginning 0))))
		       (and to-start to-stop
			    (buffer-substring to-start (1- to-stop))))))
		  (sig-file
		   (car (cdr (regexp-assoc to-contents bat-sig-alist)))))

	     (forward-line -1)
	     (save-excursion
	       (run-hooks 'bat-init-message-hook))

	     (if (file-readable-p "~/.headers")
		 (insert-file-contents "~/.headers"))
	     (and sig-file
		  (file-readable-p sig-file)
		  (save-excursion
		    (goto-char (point-max))
		    (insert-file-contents sig-file)))

	     ; (set-marker bat-end-of-headers-mark (point))
	     ))
	  (t
	   ; (set-marker bat-end-of-headers-mark (point-max))
	   ))

    (goto-char (point-min))
    (re-search-forward "^[^ ]*:[ \t]*$\\|\n\n" nil t)

    (if init-contents (insert init-contents))))

(defun bat-toggle-blind ()
  "Toggle whether or not blind carbon copies are made of outgoing
messages."
  (interactive)
  (setq bat-local-blind (not bat-local-blind))
  (refresh-modeline))

(defun bat-in-headers-p ()
; (< (point) (marker-position bat-end-of-headers-mark))
  (not (save-excursion (search-backward "\n\n" nil t))))

(defun bat-mail-newline ()
  "In the bat-composition buffer, move to either the next empty header,
the beginning of the message body, or insert a newline if already in
the message body."
  (interactive)
  (if (bat-in-headers-p)
      (re-search-forward "^[^ ]*:[ \t]*$\\|\n\n" nil t)
      (newline)))

(defun bat-mail-help ()
  "Displays a buffer with info on bat-composition keybindings."
  (interactive)
  (message "Getting help info...")
  (when (not (buffer-visible-p bat-display-buf))
    (split-window)
    (switch-to-buffer bat-display-buf)
    (other-window 1))
  (temp-pop-to-buffer bat-display-buf
    (erase-buffer)
    (insert (buffer-bindings-string bat-composition-buf))
    (setq bat-last-cuid 0)		; so not confused with any other cuid
    (bat-fixup-display-buf))		; so the more prompt is informative
  (message "Getting help info...done"))

(defun bat-insert-body ()
  "Inserts at the current point position a form of the currently displayed
message with each line prepended by \"> \", and the senders name at the
beginning."
  (interactive)
  (save-excursion
    (let ((from nil))
      (set-buffer bat-display-buf)
      (save-excursion
	(let ((beg (point-min)))
	  (goto-char (point-max))
	  (beginning-of-line)
	  (while (and (looking-at "^[> \t]*$") (= (forward-line -1) 0))
	    )
	  (if (not (looking-at "^[> \t]*$"))
	      (forward-line 1))
	  (copy-to-buffer bat-scratch-buf beg (1- (point)))))
      (set-buffer bat-scratch-buf)
      (goto-char (point-min))
      (if (re-search-forward "^From:\\|^$" nil t)
	  (cond ((looking-at "[ \t]*\\(.*\\)")
		 (setq from (buffer-match 1))
		 (if (re-search-forward "^$" nil t)
		     (forward-line 1)
		   (goto-char (point-max))))
		(t
		 (forward-line 1))))
      (while (and (looking-at "^[> \t]*$") (= (forward-line 1) 0))
	)
      (delete-region (point-min) (point))
      (goto-char (point-min))
      (and from (insert from " writes:\n"))
      (replace "^" "> " t)
      (goto-char (point-max))
      (newline)))
  (insert-buffer bat-scratch-buf))

;;; ----------------------------------------------------------------
;;; submitting mail for transmission

(defun bat-abort-mail ()
  "Exit bat-composition mode without sending."
  (interactive)
  (set-buffer bat-composition-buf)
  (if (file-exists-p bat-outgoing-file)
      (delete-file bat-outgoing-file))
  (set-buffer-modified-p nil)
  (bat-exit-composition))

;;; rewrite-headers, but don't send
(defun bat-validate ()
  "Validate and rewrite the headers in the bat-composition buffer."
  (interactive)
  (goto-char (point-min))
  (bat-command
   "[\n"
   "C(~bat-begin-state \"Validating addresses\")\n"
   "rTo:" (bat-header-contents "To" t) "\n"
   "rCC:" (bat-header-contents "CC" nil) "\n"
   "rBCC:" (bat-header-contents "BCC" nil) "\n"
   "C(~bat-end-state)\n"
   "|\n"		       ; end state even if errors
   "C(~bat-fail-state)\n"
   "]\n"))

;;; submits mail, but only if validation succeeds [but that doesn't work]
(defun bat-submit ()
  "Check spelling if bat-ispell-before-submit is non-nil, Validate,
and then submit the bat-composition buffer."
  (interactive)
  (goto-char (point-min))
  (and bat-ispell-before-submit (ispell-buffer))
  (bat-command
   "[\n"
   "[\n"		       ; two levels to handle end-state
   "C(~bat-begin-state \"Validating addresses\")\n"
   "rTo:" (bat-header-contents "To" t) "\n"
   "rCC:" (bat-header-contents "CC" nil) "\n"
   "rBCC:" (bat-header-contents "BCC" nil) "\n"
   "C(~bat-end-state)\n"
   "|\n"		       ; end state even if errors
   "C(~bat-fail-state)\n"
   "]\n"
   "C(bat-submit-no-validate)\n"
   "]\n"))

;;; submits mail, but only if validation succeeds [but that doesn't work]
(defun bat-submit-no-ispell ()
  "Validate, and then submit the bat-composition buffer."
  (interactive)
  (goto-char (point-min))
  (bat-command
   "[\n"
   "[\n"		       ; two levels to handle end-state
   "C(~bat-begin-state \"Validating addresses\")\n"
   "rTo:" (bat-header-contents "To" t) "\n"
   "rCC:" (bat-header-contents "CC" nil) "\n"
   "rBCC:" (bat-header-contents "BCC" nil) "\n"
   "C(~bat-end-state)\n"
   "|\n"		       ; end state even if errors
   "C(~bat-fail-state)\n"
   "]\n"
   "C(bat-submit-no-validate)\n"
   "]\n"))

(defun bat-submit-no-validate ()
  "Submit the bat-composition buffer without validating addresses."
  (interactive)
  (temp-set-buffer bat-composition-buf
    (save-excursion
      (run-hooks 'bat-submit-message-hook))
    (write-current-buffer bat-outgoing-file)
    (bat-command
     "[\n"
     "s" (if bat-local-blind "b" "n") bat-replying-to ":" bat-outgoing-file "\n"
     "C(bat-mail-sent)\n"
     "]\n")
    (setq bat-outgoing-file (bat-temp-file)))) ; get around message server bug

(defun bat-header-contents (name req)
  "Return the contents of the header NAME, signalling an error if REQUIRED is
non-nil, and the header is non-existant."
  (save-excursion
    (set-buffer bat-composition-buf)
    (goto-char (point-min))
    (cond ((re-search-forward (concat "^" name ":[ \t]*") nil t)
	   (cond ((looking-at "[ \t]*\n[^ \t]")
		  (cond (req
			 (error (concat "You must have a " name ": header")))
			(t
			 (beginning-of-line)
			 (let ((bol (point)))
			   (forward-line 1)
			   (delete-region bol (point)))
			 "")))
		 (t
		  (let ((beg (point)))
		    (cond ((re-search-forward "\n[^ \t]")
			   (backward-char 2))
			  (t
			   (goto-char (point-max))))

		    ;; use continuation lines to avoid swamping pty
		    (copy-to-buffer bat-scratch-buf beg (point))
		    (set-buffer bat-scratch-buf)
		    (goto-char (point-min))
		    (replace ",[ \t\n][ \t\n]*" ",\n " t)
		    (goto-char (point-min))
		    (replace "\n" "\\\n")
		    (buffer-substring (point-min) (point-max))))))
	  (t
	   (if req
	       (error (concat "You must have a " name ": header"))
	     "")))))

;;; callback from robin
(defun ~bat-rewrite-header (name contents)
  "Rewrite the contents of header NAME in the bat-composition buffer to
be CONTENTS."
  (temp-set-buffer bat-composition-buf
    (goto-char (point-min))
    (when (re-search-forward (concat "^" name ":[ \t]*"))
      (insert contents)
      (let ((beg (point)))
	(cond ((re-search-forward "^[^ \t\n]\\|^$")
	       (beginning-of-line)
	       (backward-char 1))
	      (t
	       (goto-char (point-max))))
	(delete-region beg (point))))))

;;; callback from robin
(defun bat-mail-sent ()
  "Cleanup after mail has been sent successfully."
  (temp-set-buffer bat-composition-buf
    (set-buffer-modified-p nil))
  (and buffer-auto-save-file-name
       (file-exists-p buffer-auto-save-file-name)
       (delete-file buffer-auto-save-file-name))
  (bat-exit-composition))

;;; ----------------------------------------------------------------
;;; * end of batmail.el *
