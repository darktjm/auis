;;; BatMail.ml -- mlisp interface to the Andrew Message System.
;;;	       designed to work the robin program

;;; ***************************************************************** 
;;;	Copyright 1986-1990 by Miles Bader
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

;;; Version 2.8
;;; 27 feb 1986 - Miles Bader
;;; Copyright 1986,1987,1988,1989,1990 by Miles Bader
;;; Copyright 1994 Carnegie Mellon University
;;; Last edit by wjh, 16 Nov 1994

;;; ----------------------------------------------------------------
;;; variables that the user can set to effect BatMail behavior

;;; if non-zero, will put subscribed folders in the caption area,
;;; even if there are no messages
(if (! (is-bound bat-show-unchanged))
    (setq-default bat-show-unchanged 0))

;;; if non-zero, will put an extra blank line between folders
(if (! (is-bound bat-low-density))
    (setq-default bat-low-density 0))

;;; list of headers to show, seperated by ':' OR a '!' followed
;;; by a list of headers NOT to show
(if (! (is-bound bat-headers))
    (setq-default bat-headers "from:resent-from:resent-to:date:subject:to:cc:newsgroups"))

;;; If zero, some commands use the current message under the
;;; cursor instead of the message whos body is displayed
(if (! (is-bound bat-body-active))
    (setq-default bat-body-active 1))

;;; compatibility
(if (is-bound bat-reply-to-body)
    (setq bat-body-active bat-reply-to-body))

;;; If non-zero, don't look for captions, only folders when doing a large scan
(if (! (is-bound bat-folders-only))
    (setq-default bat-folders-only 0))

;;; If non-zero, send a blind carbon copy of outgoing mail to yourself
(if (! (is-bound bat-send-bcc))
    (setq-default bat-send-bcc 0))

;;; if non-zero, purge deleted message without asking when quitting
;(if (! (is-bound bat-purge-on-quitting))
;    (setq-default bat-purge-on-quitting 0))

;;; Percentage of screen height used for the captions
(if (! (is-bound bat-caption-percentage))
    (setq-default bat-caption-percentage 30))

;;; Default set folders to prompt with when starting batmail
(if (! (is-bound bat-default-folders))
    (setq-default bat-default-folders "*"))

(if (! (is-bound bat-stingy-display))
    (setq-default bat-stingy-display 0))

;;; RE for separators in digests
(if (! (is-bound bat-digest-separator))
    (setq-default bat-digest-separator "^-----------------------------"))

;;; if non-zero, will record interactions with the robin subprocess in the 
;;; buffer bat-log
(if (! (is-bound bat-trace))
    (setq-default bat-trace 0))

(if (! (is-bound bat-cave))
    (setq-default bat-cave
		  (if (error-occurred (getenv "BATCAVE")) 
		      "/usr/contributed/lib/batcave"
		      (getenv "BATCAVE"))))

(if (! (is-bound robin))
    (setq-default robin
		  (if (error-occurred (getenv "ROBIN")) 
		      "robin"
		      (getenv "ROBIN"))))

(if (! (is-bound bat-bin-dir))
    (setq-default bat-bin-dir (concat bat-cave "/bin/")))

;;; where to look for the BatMail help file
(if (! (is-bound bat-help-dir))
    (setq-default bat-help-dir (concat bat-cave "/help")))

(if (! (is-bound bat-etc-dir))
    (setq-default bat-etc-dir (concat bat-cave "/etc")))

(setq-default ~bat-old-emacs (= (emacs-version) "Emacs #265 of Oct 30/83"))

;;; ----------------------------------------------------------------

(autoload "split-window-nicely" "window")
(autoload "delete-window-nicely" "window")
(autoload "display-string" "prompt.ml")
(autoload "get-tty-char" "prompt.ml")
(autoload "get-tty-str" "prompt.ml")
(autoload "get-tty-bool" "prompt.ml")
(autoload "pop-up-buffer" "prompt.ml")
(autoload "get-prompted-key" "prompt.ml")
(autoload "correct-spelling-of-word" "ispell.ml")

;;; ----------------------------------------------------------------
;;; old emacs emulations
(if ~bat-old-emacs
    (progn
      (defun (line-to-bottom-of-window (novalue)))
      (defun (split-window-nicely (split-current-window)))
      (defun (delete-window-nicely (delete-window)))
      (defun (buffer-is-visible
	(save-excursion (temp-use-buffer (arg 1)) (dot-is-visible))))
      (defun (top-window (move-dot-to-x-y 1 1)))
      ))

;;; ----------------------------------------------------------------

(setq-default ~bat-temp-file-counter 0)

(defun (~bat-temp-file
  (setq ~bat-temp-file-counter (+ ~bat-temp-file-counter 1))
  (concat "/tmp/BatMail.temp." (process-id " robin") "." ~bat-temp-file-counter)))

;;; ----------------------------------------------------------------

(setq-default ~bat-goto-new-message 1)

(defun (~bat-prompt-get-captions mpgh-def
  (setq mpgh-def (arg 1 "Default dir: "))
  (~bat-get-captions (get-tty-str "Folders" mpgh-def)
		     (get-tty-str "Since" "updates"))
  (sit-for 0)))

;;; Takes a list of directories (or "" for your subscription) and
;;; a date (or "" for updates) and retrieves the headers for them.
(defun (~bat-get-captions mgh-dirs mgh-since mgh-dir
  (setq mgh-dirs (arg 1))
  (setq mgh-since (arg 2))
  (if (= mgh-since "updates")
      (setq mgh-since ""))
  (setq ~bat-goto-new-message 1)
  (sit-for 0)			       ; so window-width is accurate
  (~bat-set-caption-width (window-width))
  (message "Checking the headlines"
	   (if (!= mgh-dirs "*") (concat " on " mgh-dirs) "")
	   "...")
  (~bat-save-last-update)
  (~bat-command "na\n")
  (temp-use-buffer " bat-scratch")
  (erase-buffer)
  (insert-string mgh-dirs)
  (beginning-of-file)
  (while (! (error-occurred
	     (re-search-forward "\\([^ ,\t]*\\)[ ,\t]*")))
    (region-around-match 1)
    (setq mgh-dir (region-to-string))
    (if (= (substr mgh-dir -1 1) "*")
	(progn
	  (setq mgh-dir (substr mgh-dir 1 -1))
	  (while (= (substr mgh-dir -1 1) ".")
	    (setq mgh-dir (substr mgh-dir 1 -1)))
	  (~bat-command (concat "d" mgh-dir "\n"
				 (if bat-folders-only "F" "H") mgh-since "\n")))
	(~bat-command (concat "d" mgh-dir "\n"
			       "h" mgh-since "\n")))
    (region-around-match 0))
  (novalue)))

;;; ----------------------------------------------------------------
;;; getting captions for a folder

(setq-default ~bat-cuid-cols 4)
(setq-default ~bat-last-caption-mark 0)
(setq-default ~bat-folder-name 0)
(setq-default ~bat-folder-mark 0)
(setq-default ~bat-folder-did-exist 0)
(setq-default ~bat-saved-new-msgs 0)
(setq-default ~bat-saved-dot 0)
(setq-default ~bat-saved-mark 0)
(setq-default ~bat-first-new-msg 0)
(setq-default ~bat-old-captions-width 0)

(defun (~bat-set-caption-width w
  (setq w (arg 1 "Caption width: "))
  (if (!= w ~bat-old-captions-width)
      (progn
	(~bat-command (concat "w" (- w (+ ~bat-cuid-cols 1) 2) "\n"))
	(setq ~bat-old-captions-width w)))))

;;; callback from robin
;;; Rewrites an existing caption
(defun (~bat-alter-caption match
  (temp-use-buffer "bat-captions")
  (save-excursion
   (~bat-find-caption (arg 1 "Cuid: ") (dot))
   (provide-prefix-argument (+ ~bat-cuid-cols 1) (forward-character))
   (set-mark)
   (end-of-line)
   (erase-region)
   (insert-string (arg 2 "Caption: ")))
  (sit-for 0)))

(defun (~bat-region-around-folder
  (end-of-file)
  (if (! (error-occurred
	  (search-reverse (concat "--- " (arg 1 "Folder: ") " "))))
      (progn
	(beginning-of-line)
	(set-mark)		       ; at beginning of msg group
	(if (error-occurred (search-forward "\n---"))
	    (end-of-file)
	    (beginning-of-line)))
      (set-mark))))

;;; callback from robin
;;; First of the three part protocol from the robin process; 
;;; If the give directory exists, it is erased and its new message count
;;; is subtracted from the total.  It then records the directory name
;;; and the location of the directory (or the end of the buffer if it
;;; didn't exist)
(defun (~bat-start-folder-update
  (setq ~bat-folder-name (arg 1 "Folder: "))
  (~bat-begin-state (concat "Checking " ~bat-folder-name))
  (temp-use-buffer "bat-captions")
  (save-excursion old-dot old-mark
   (setq old-dot (dot))
   (if (error-occurred (setq old-mark (mark)))
       (setq old-mark 0))
   (~bat-region-around-folder ~bat-folder-name)
   (setq ~bat-folder-did-exist (!= (dot) (mark)))
   (if ~bat-folder-did-exist
       (progn
	 (setq ~bat-saved-dot (~bat-maybe-save-pos old-dot))
	 (setq ~bat-saved-mark (~bat-maybe-save-pos old-mark)))
       (progn
	 (setq ~bat-saved-dot 0)
	 (setq ~bat-saved-mark 0)))
   (erase-region)		       ; erase whole thing (restored later)
   (setq ~bat-folder-mark (dot))
   (setq ~bat-last-caption-mark (dot))
   (setq ~bat-first-new-msg 0))
  (novalue)))

;;; callback from robin
(defun (~bat-add-caption
  (temp-use-buffer "bat-captions")
  (save-excursion
   (goto-character ~bat-last-caption-mark)
   (if (& (= ~bat-first-new-msg 0) (arg 3 "newp? "))
       (setq ~bat-first-new-msg (dot)))
   (insert-string (arg 1 "Cuid: "))
   (to-col (+ ~bat-cuid-cols 2))       ; +1 for update-mark column
   (insert-string (arg 2 "Caption: ") "\n")
   (setq ~bat-last-caption-mark (dot)))))

;;; callback from robin
(defun (~bat-end-folder-update since num new
  (setq since (arg 1 "Since: "))
  (setq num (arg 2 "Number of messages: "))
  (setq new (arg 3 "Number new: "))
  (temp-use-buffer "bat-captions")
  (if (| num bat-show-unchanged ~bat-folder-did-exist)
      (progn new-message-mark
	(save-excursion
	 (goto-character ~bat-folder-mark)
	 (if (& bat-low-density (eobp) (! (bobp)))
	     (newline))
	 (insert-string "--- " ~bat-folder-name " - ")
	 (if (= since "")
	     (insert-string "Updates")
	     (insert-string "Since " since))
	 (insert-string
	  " ("
	  (if num num "No")
	  (if (& (= new num) (| (> new 0) (= since ""))) " new" "")
	  " message" (if (= num 1) "" "s")
	  (if (& (> new 0) (> num new)) (concat ", " new " new") "")
	  ")\n")
	 (if bat-low-density
	     (progn
	       (goto-character ~bat-last-caption-mark)
	       (newline)))
	 ;; maybe put dot on a new message
	 (if ~bat-goto-new-message
	     (progn
	       (if (> new ~bat-saved-new-msgs)
		   (provide-prefix-argument
		    (- num (- new ~bat-saved-new-msgs))
		    (next-line)))
	       (setq new-message-mark (dot))
	       (setq ~bat-goto-new-message 0))
	     (setq new-message-mark 0))
	 )
	(~bat-invalidate-update-mark-for ~bat-folder-name)
	(if (!= new num)	       ; need a quit line
	    (save-excursion
	     (if (= ~bat-first-new-msg 0); all old, quit line at end
		 (goto-character ~bat-last-caption-mark)
		 ;; else, folder line before 1st new message
		 (goto-character ~bat-first-new-msg))
	     (previous-line)
	     (~bat-set-orig-update-mark)))
	;; special case for browsing through bboards
	(if (& (>= (dot) ~bat-folder-mark)
	       (<= (dot) ~bat-last-caption-mark))
	    (next-line))
	(if ~bat-saved-mark
	    (save-excursion
	     (goto-character (~bat-maybe-restore-pos ~bat-saved-mark (mark)))
	     (set-mark)))
	(if (!= new-message-mark 0)
	    (goto-character new-message-mark)
	    (if ~bat-saved-dot
		(goto-character
		 (~bat-maybe-restore-pos ~bat-saved-dot (dot)))))
	))
  (~bat-end-state)		       ; does a (sit-for 0), we don\'t have to
  (novalue)))

(defun (~bat-fail-folder-update
  (temp-use-buffer "bat-captions")
  (save-excursion
   (goto-character ~bat-folder-mark)
   (if (& bat-low-density (eobp) (! (bobp)))
       (newline))
   (insert-string "--- " ~bat-folder-name " - Error getting captions\n")
   (if bat-low-density
       (progn
	 (goto-character ~bat-last-caption-mark)
	 (newline))))
  (~bat-fail-state)))

(defun (~bat-folders-only-folder-update fname since
  (setq fname (arg 1 "Folder: "))
  (setq since (arg 2 "Since: "))
  (temp-use-buffer "bat-captions")
  (save-excursion
   (~bat-region-around-folder fname)
   (erase-region)
   (if (& bat-low-density (eobp) (! (bobp))) (newline))
   (insert-string "--- " fname " - "
		  (if (= since "") "Updates" (concat "Since " since))
		  " (New messages)\n")
   (if bat-low-density (newline)))
  (sit-for 0)))


(defun (~bat-current-folder
  (temp-use-buffer "bat-captions")
  (save-excursion
   (beginning-of-line)
   (next-line)
   (if (error-occurred (re-search-reverse "^--- \\([^ ]*\\)"))
       (progn
	 (message "No current folder.")
	 "")
       (progn
	 (region-around-match 1)
	 (region-to-string))))))

;;; ----------------------------------------------------------------
;;; Callback routines for the robin process

(defun (~bat-error
  (~bat-trace-always (concat "Error: <<<" (arg 1) ">>>\n"))
  (display-string (arg 1))
  (novalue)))

(defun (~bat-info
  (display-string (arg 1))
  (novalue)))

(defun (~bat-get-boolean-from-user prompt def ans
  (if (error-occurred
       (~bat-command
	(concat "i" (get-tty-bool (arg 1 "Prompt: ") (arg 2 "Default: ")) "\n")))
      (~bat-command (concat "i" def "\n")))))

(defun (~bat-get-string-from-user
  (if (error-occurred
       (~bat-command (concat "i" (get-tty-str (arg 1 "Prompt: ") "") "\n")))
      (~bat-command "i\n"))))

(defun (~bat-choose-from-list count def choice kdef
  (setq def (arg 1))
  (setq count 1)
  (temp-use-buffer " bat-scratch")
  (erase-buffer)
  (insert-string (arg 2) "\n")
  (while (<= (+ count 2) (nargs))
    (insert-string (if (< count 10) count (char-to-string (+ (- count 10) 'a')))
		   "\t" (arg (+ count 2)) "\n")
    (setq count (+ count 1)))
  (setq kdef (if (< def 10) (string-to-char def) (+ (- count 10) 'a')))
  (if (error-occurred
       (setq choice
	     (get-prompted-key " bat-scratch"
			       "BatChoice"
			       kdef
			       "---- BatChoices"
			       1)))
      (setq choice kdef))
  (sit-for 0)			       ; flush output?
  (~bat-command (concat "i"
			 (if (& (>= choice '0') (<= choice '9'))
			     (char-to-string choice)
			     (+ (- choice 'a') 10))
			 "\n"))))

;;; ----------------------------------------------------------------
;;; 

(setq-default ~bat-command 0)
(setq-default ~bat-output "")
(setq-default ~bat-prev-exit-hook "")

(defun (~bat-exit-hook
; the variable doesn't seem to get bound, so use a hardwired values
; (| (execute-mlisp-line (concat "(" ~bat-prev-exit-hook ")"))
  (| (execute-mlisp-line "(#edit-exit-hook)")
     (progn
       (eot-process " robin")	       ; get around a pty bug on vaxen
       0))))

(defun (~bat-call-robin
  (if (= (process-status " robin") -1)
      (progn
	(~bat-info "Atomic batteries to power...")
	(start-filtered-process (concat bat-bin-dir "/" robin)
				" robin"
				"~bat-filter")
	(if (= ~bat-prev-exit-hook "")
	    (setq ~bat-prev-exit-hook (set-exit-emacs-hook "~bat-exit-hook")))
	(~bat-info "Turbines to speed..."))
      (~bat-info "Robin's already here..."))
  (novalue)))

;;; clean up after ourselves...
(defun (~bat-flush-robin
  (~bat-command
   (concat "[\n"
	   "C(~bat-begin-state \"Updating state\")\n"
	   "pa\nU\n"
	   "C(~bat-end-state)\n"
	   "|\n"
	   "C(~bat-fail-state)\n"
	   "]\n"))))

(defun (~bat-command com
  (setq com (arg 1 "Command: "))
  (~bat-trace com)
  (string-to-process " robin" com)))

(defun (~bat-filter output
  (setq output (process-output))
  (if (= ~bat-output "")
      (progn
	(setq ~bat-command (= (substr output 1 1) "("))
	(setq ~bat-output output))
      (setq ~bat-output
	    (concat ~bat-output (process-output))))
  (if (= (substr ~bat-output -1 1) "\n")
      (progn
	(~bat-trace ~bat-output)
	(if ~bat-command
	    (if (error-occurred
		 (execute-mlisp-line ~bat-output))
		(~bat-error
		 (concat "Callback error: "
			 (substr ~bat-output 1 -1))))
	    (if (> (length ~bat-output) 1)
		(progn
		  (~bat-info
		   (concat "robin: " (substr ~bat-output 1 -1)))
		  (~bat-trace-always
		   (concat "robin: " (substr ~bat-output 1 -1) "\n")))))
	(setq ~bat-output "")))))

(defun (~bat-exec-buffer file
  (temp-use-buffer (arg 1 "Buffer: "))
  (end-of-file)
  (setq file (~bat-temp-file))
  (insert-string "D" file "\n")	       ; delete file after its done
  (write-named-file file)
  (~bat-command (concat "I" (current-file-name) "\n"))))

;;; ----------------------------------------------------------------

(defun (~bat-trace
  (if bat-trace
      (save-excursion
       (temp-use-buffer "bat-log")
       (end-of-file)
       (insert-string (arg 1 "String: "))))))

(defun (~bat-trace-always
  (save-excursion
   (temp-use-buffer "bat-log")
   (end-of-file)
   (insert-string (arg 1 "String: ")))))

;;; ----------------------------------------------------------------

(setq-default ~bat-update-mark 0)
(setq-default ~bat-orig-update-mark 0)
(setq-default ~bat-cur-folder-name "")
(setq-default ~bat-cur-folder-beg -1)
(setq-default ~bat-cur-folder-end -1)

(defun (~bat-invalidate-update-mark-for
  (if (= (arg 1 "Folder: ") ~bat-cur-folder-name)
      (progn
	(setq ~bat-cur-folder-name "")
	(setq ~bat-cur-folder-beg -1)
	(setq ~bat-cur-folder-end -1)
	(setq ~bat-update-mark 0)
	(setq ~bat-orig-update-mark 0)))))

;;; update robins idea of the current groups last-update
;;; If there is no current group, update-mark should be 0
(defun (~bat-save-last-update
  (if (!= ~bat-update-mark ~bat-orig-update-mark)
      (~bat-command
       (concat "d" ~bat-cur-folder-name "\n"
	       "u"
	       (save-excursion
		(goto-character ~bat-update-mark)
		(if (looking-at "[0-9]*\\w")
		    (progn
		      (region-around-match 0)
		      (region-to-string))
		    0))
	       "\n")))))

(defun (~bat-get-update-mark
  (if (| (< (dot) ~bat-cur-folder-beg)
	 (> (dot) ~bat-cur-folder-end))
      ;; reading a new group, set up state info for it
      (save-excursion
       (message "Slow...")
       ;; first, try and save last-update info for prev group we read...
       (~bat-save-last-update)
       ;; now look for the beginning, name, update-line, and end of this new
       ;; group
       (if (error-occurred (re-search-reverse "^--- \\([^ ]*\\)"))
	   (error-message "Not in a folder?"))
       (setq ~bat-cur-folder-beg (dot))
       (region-around-match 1)
       (setq ~bat-cur-folder-name (region-to-string))
       (next-line)
       (beginning-of-line)
       (if (error-occurred
	    (re-search-forward "^---\\|^...._")
	    (beginning-of-line))
	   ;; no quit-line, no next group
	   (progn (end-of-file) (setq ~bat-update-mark 0))
	   (looking-at "^...._")
	   ;; found a quit-line
	   (progn
	     (setq ~bat-update-mark (dot))
	     ;; resume looking for the next folder
	     (if (error-occurred
		  (re-search-forward "^---")
		  (beginning-of-line))
		 (end-of-file)))
	   ;; no quit-line, but a next folder
	   (setq ~bat-update-mark 0))
       ;; now positioned after the end of the current folder
       (setq ~bat-orig-update-mark ~bat-update-mark)
       (backward-character)	       ; so subsequent inserts don't move end'
       (setq ~bat-cur-folder-end (dot))))
  ~bat-update-mark))

;;; by convention, setting the update-mark while sitting on folder header
;;; will unset it

(defun (~bat-set-orig-update-mark
  (~bat-set-update-mark)
  (setq ~bat-orig-update-mark ~bat-update-mark)))

(defun (~bat-set-update-mark
  (save-excursion
   (beginning-of-line)
   (~bat-get-update-mark)	       ; this ensures that it''s valid
   ;; at this point, all cur-folder info should be up to date
   (if (> ~bat-update-mark 0)	       ; was there one before?
       ;; yes, get rid of it
       (save-excursion
	(goto-character ~bat-update-mark)
	(provide-prefix-argument 4 (forward-character))
	(delete-next-character)
	(insert-string " ")))
   ;; put the new one in place
   (if (looking-at "^[0-9]")
       (progn
	 (setq ~bat-update-mark (dot))
	 (provide-prefix-argument 4 (forward-character))
	 (delete-next-character)
	 (insert-string "_"))
       (setq ~bat-update-mark 0)))))

(defun (~bat-find-caption mfc-cuid mfc-hint match
  (setq mfc-cuid (arg 1 "Cuid: "))
  (setq mfc-hint (arg 1 "Hint: "))
  (end-of-line)
  (setq match (concat "^" mfc-cuid "[^0-9]"))
  (if (if (> (dot) mfc-hint)
	  (& (error-occurred (re-search-reverse match))
	     (error-occurred (re-search-forward match)))
	  (& (error-occurred (re-search-forward match))
	     (error-occurred (re-search-reverse match))))
      (error-message "Can't find caption " mfc-cuid "!"))
  (beginning-of-line)))

(setq-default ~bat-last-cuid 0)

(defun (~bat-current-cuid
  (if (= ~bat-last-cuid 0)
      (~bat-pick))
  ~bat-last-cuid))

(defun (~bat-maybe-save-pos bmsp-mark
  (setq bmsp-mark (arg 1))
  (save-excursion
   (if (| (& (> bmsp-mark (mark)) (< bmsp-mark (dot)))
	  (& (< bmsp-mark (mark)) (> bmsp-mark (dot))))
       (progn
	 (goto-character bmsp-mark)
	 (beginning-of-line)
	 (if (looking-at "[0-9]*\\w")
	     (progn
	       (region-around-match 0)
	       (region-to-string))
	     0))
       0))))

(defun (~bat-maybe-restore-pos bmrp-cuid bmrp-place
  (setq bmrp-cuid (arg 1))
  (setq bmrp-place (arg 2))
  (if (> bmrp-cuid 0)
      (save-excursion
       (if (! (error-occurred
	       (~bat-find-caption bmrp-cuid bmrp-place)))
	   (dot)
	   bmrp-place))
      bmrp-place)))

;;; ----------------------------------------------------------------

(defun (~bat-get-cuid
  (beginning-of-line)
  (if (error-occurred (re-search-forward "^[0-9][0-9]*"))
      (progn
	(message "End of captions.")
	0)
      (progn newdot cuid
	(save-excursion
	 (region-around-match 0)
	 (setq newdot (dot))
	 (setq cuid (region-to-string)))
	(goto-character newdot)
	cuid))))

(setq-default ~bat-local-strip 1)
(setq-default ~bat-maybe-update-mark 0)

(defun (~bat-pick num
  (setq num (~bat-get-cuid))
  (if (= num 0)
      (novalue)
      (!= num ~bat-last-cuid)
      (progn
	(setq ~bat-local-strip 1)
	(setq ~bat-maybe-update-mark (dot))
	(~bat-command
	 (concat "b" num ":" (- (window-width) 1) ":s\n"
		 "MUi" num "\n"))
        ;; try and keep next line visible...
	(if (! (| (eobp)
		  (save-excursion
		   (end-of-line)
		   (| (error-occurred
		       (re-search-forward "^[0-9]")); skip bb headers
		      (dot-is-visible)))))
	    (progn
	      (line-to-top-of-window)
	      (scroll-one-line-down))))
      (~bat-scroll-body)
      (~bat-next-caption))))

(setq-default ~bat-begin-body-mark 0)

(defun (~bat-body file
  (save-excursion
   (setq file (arg 1 "Body file: "))
   (pop-to-buffer "bat-display")
   (read-file file)
   (unlink-file file)
   (setq ~bat-last-cuid (arg 2 "Cuid: "))
   (~bat-build-display-modeline (concat "Body of message " ~bat-last-cuid))
   (error-occurred (search-forward "\n\n"))
   (while (& (! (eobp)) (eolp))
     (delete-next-character))
   (setq ~bat-begin-body-mark (dot))
   (save-excursion (error-occurred (bat-body-hook)))
   (end-of-window)
   (temp-use-buffer "bat-captions")
   (save-excursion
    (goto-character ~bat-maybe-update-mark)
    (if (> (dot) (~bat-get-update-mark)) (~bat-set-update-mark)))
   (~bat-set-default-folder ""))
  (sit-for 0)))

(defun (~bat-toggle-stripping
  (if (> ~bat-last-cuid 0)
      (progn
	(setq ~bat-local-strip (! ~bat-local-strip))
	(~bat-command
	 (concat "b" (~bat-current-cuid)
		 ":" (- (window-width) 1)
		 ":" (if ~bat-local-strip "s" "n") "\n")))
      (message "No current message."))))

(defun (~bat-want-vote
  (~bat-info "A vote is requested.  Use the 'V' command to vote.")))

(defun (~bat-want-subscription folder
  (setq folder (arg 1 "Folder: "))
  (~bat-info
   (concat
    "To subscribe to " folder ", use the 'S' command; to read use 'R'."))
  (~bat-set-default-folder folder)))

;;; ----------------------------------------------------------------

(defun (~bat-active-cuid cuid
  (setq cuid
        (if bat-body-active
	    (~bat-current-cuid)
	    (~bat-get-cuid)))
  (if (= cuid 0) 
      (error-message "No current message!"))
  cuid))

(defun (~bat-punt
  (next-line)
  (beginning-of-line)
  (if (error-occurred (re-search-forward "^---"))
      (end-of-file))
  (save-excursion
   (previous-line)
   (beginning-of-line)
   (if (looking-at "^$") (previous-line)); low density
   (if (looking-at "^[0-9]") (~bat-set-update-mark)))))

(defun (~bat-set-quit-line
  (save-excursion
   (previous-line)
   (beginning-of-line)
   (~bat-set-update-mark))))

(defun (~bat-delete cuid
  (setq cuid (~bat-get-cuid))
  (if (!= cuid 0)
      (~bat-command (concat "Mdn" cuid "\n")))))

(defun (~bat-undelete cuid
  (setq cuid (~bat-get-cuid))
  (if (!= cuid 0)
      (~bat-command (concat "MDn" cuid "\n")))))

(defun (~bat-unread cuid
  (setq cuid (~bat-get-cuid))
  (if (!= cuid 0)
      (~bat-command (concat "Mun" cuid "\n")))))

(setq-default ~bat-folder-default "")

(defun (~bat-copy-delete cuid dir
  (setq cuid (~bat-active-cuid))
  (setq dir (arg 1 (concat "Move message " cuid " to directory: [" ~bat-folder-default "] ")))
  (if (= dir "")
      (setq dir ~bat-folder-default))
  (if (!= cuid 0)
      (~bat-command (concat "d" dir "\n"
			    "cc" cuid "\n")))
  (setq ~bat-folder-default dir)))

(defun (~bat-copy cuid dir
  (setq cuid (~bat-active-cuid))
  (setq dir (arg 1 (concat "Copy message " cuid " to directory: [" ~bat-folder-default "] ")))
  (if (= dir "")
      (setq dir ~bat-folder-default))
  (if (!= cuid 0)
      (~bat-command (concat "d" dir "\n"
			    "cC" cuid "\n")))
  (setq ~bat-folder-default dir)))

;;; ----------------------------------------------------------------

(defun (~bat-rot13-body
  (temp-use-buffer "bat-display")
  (save-excursion
   (goto-character ~bat-begin-body-mark)
   (set-mark)
   (end-of-file)
   (~bat-info "Rotting message...")
   (fast-filter-region "tr n-za-mN-ZA-M a-zA-Z")
   (~bat-info "Rotting message...Done"))))

(defun (~bat-scroll-body
  (save-excursion
   (pop-to-buffer "bat-display")
   (end-of-window)
   (if (looking-at "[ \t\n]*\\'")      ; eobp, ignoring white space
       1
       (progn
	 (previous-line)
	 (line-to-top-of-window)
	 (end-of-window)
	 0)))))

(defun (~bat-reverse-scroll-body
  (save-excursion
   (pop-to-buffer "bat-display")
   (beginning-of-window)
   (next-line)
   (end-of-line)
   (line-to-bottom-of-window)
   (end-of-window))))		       ; this only if at beginning

(defun (~bat-digest-forward
  (save-excursion
   (pop-to-buffer "bat-display")
   (beginning-of-window)
   (if (error-occurred (re-search-forward bat-digest-separator))
       (message "End of digest.")
       (progn
	 (error-occurred (re-search-forward "^[^ \t\n]"))
	 (beginning-of-line)
	 (line-to-top-of-window)
	 (end-of-window))))))

(defun (~bat-digest-backward
  (save-excursion
   (pop-to-buffer "bat-display")
   (beginning-of-window)
   (if (| (error-occurred (re-search-reverse bat-digest-separator))
	  (error-occurred (re-search-reverse bat-digest-separator)))
       (message "Beginning of digest")
       (progn
	 (next-line)
	 (error-occurred (re-search-forward "^[^ \t\n]"))
	 (beginning-of-line)
	 (line-to-top-of-window)
	 (end-of-window))))))

(defun (~bat-beginning-of-body
  (save-excursion
   (pop-to-buffer "bat-display")
   (beginning-of-file)
   (sit-for 0)			       ; work around emacs bug
   (end-of-window))))		       ; make percent displayed more accurate

(defun (~bat-end-of-body
  (save-excursion
   (pop-to-buffer "bat-display")
   (end-of-file))))

(defun (~bat-next-caption
  (next-line)
  (~bat-pick)))

(defun (~bat-vote
  (~bat-command (concat "v" (~bat-active-cuid) "\n"))))
  
(defun (~bat-print-body
  (~bat-command (concat "P" (~bat-active-cuid) "\n"))
  (novalue)))

(defun (~bat-previous-caption
  (beginning-of-line)
  (if (error-occurred (re-search-reverse "^[0-9]*\\w"))
      (message "Beginning of captions.")
      (~bat-pick))))

(defun (~bat-quit
  (~bat-save-last-update)
  (~bat-exit)))

(defun (~bat-append-to-file cuid
  (setq cuid (~bat-active-cuid))
  (~bat-command
   (concat "a" cuid ":" (arg 1 (concat "Append message " cuid " to file: ")) "\n"))))

(defun (~bat-append-displayed-to-file
  (temp-use-buffer "bat-display")
  (append-to-file (arg 1 (concat "Append message " (~bat-current-cuid) " to file: ")))))

(defun (~bat-read dir
  (~bat-prompt-get-captions (~bat-default-folder))))

;;; this is intended for people stepping through ask-subscribed bboards
(defun (~bat-read-updates-here dir
  (setq dir (~bat-current-folder))
  (if (= dir "")
      (message "No current folder!?")
      (~bat-get-captions dir ""))))

(setq-default ~bat-default-folder "")

(defun (~bat-default-folder
  (if (= ~bat-default-folder "")
      (~bat-current-folder)
      ~bat-default-folder)))

(defun (~bat-set-default-folder
  (setq ~bat-default-folder (arg 1))))

(defun (~bat-prompt-subscribe dir
  (setq dir (get-tty-str "Subscribe to folder" (~bat-default-folder)))
  (if (!= dir "")
      (progn
	(temp-use-buffer " bat-scratch")
	(erase-buffer)
	(insert-string "Subscription methods:\n"
		       "n\tNormal subscription\n"
		       "a\tAsk suscription (updates shown only upon demand)\n"
		       "s\tShow-all subscription (always see every message)\n"
		       "p\tPrint subscription (This doesn't work)\n")
	(~bat-command
	 (concat "d" dir "\n"
		 "S" (char-to-string
		      (get-prompted-key " bat-scratch"
					"Subscription method"
					'n'
					"---- BatSubscription methods"
					0))
		 "\n"))))))

(defun (~bat-prompt-unsubscribe dir
  (setq dir (get-tty-str "Unsubscribe to folder" (~bat-current-folder)))
  (if (!= dir "") (~bat-command (concat "d" dir "\nSu\n")))))

(defun (~bat-help hf key
  (save-excursion
   (setq hf "")
   (temp-use-buffer " bat-scratch")
   (erase-buffer)
   (insert-string "The following help topics are available:\n"
		  "c\tKeyboard commands\n"
		  "v\tUser-settable variables\n"
		  "h\tUser-defineable hooks\n"
		  "f\tFiles you can supply BatMail\n")
   (setq key
	 (get-prompted-key " bat-scratch"
			   "BatHelp on"
			   'c'
			   "---- BatHelp topics"
			   0))
   (if (= key 'c') (setq hf "commands.gos")
       (= key 'v') (setq hf "variables.gos")
       (= key 'h') (setq hf "hooks.gos")
       (= key 'f') (setq hf "files.both"))
   (pop-to-buffer "bat-display")
   (read-file (concat bat-help-dir "/" hf))
   (setq ~bat-last-cuid -1)	       ; so not confused with any other cuid
   (~bat-build-display-modeline "Help")
   (end-of-window)		       ; so the more prompt is informative
   )
  ))

;;; ----------------------------------------------------------------

(defun (~bat-set-up-screen siz
  (switch-to-buffer "bat-captions")
  (delete-other-windows)
  (setq siz (window-height))
  (split-window-nicely)
  (next-window)
  (switch-to-buffer "bat-display")
  (setq siz (/ (* siz (- 100 bat-caption-percentage)) 100))
  (while (< (window-height) siz)
    (enlarge-window))
  (while (> (window-height) siz)
    (shrink-window))
  (next-window)
  (novalue)))

(defun (disabled-key
  (error-message "That key is disabled here.")))

(defun (~bat-unbind-keys key
  (setq key ' ')
  (while (< key 128)
    (local-bind-to-key "disabled-key" key)
    (setq key (+ key 1)))
  (local-bind-to-key "disabled-key" "\t")
  (local-bind-to-key "disabled-key" "\n")
  (local-bind-to-key "disabled-key" "\r")
  (local-bind-to-key "disabled-key" "\^W")
  (local-bind-to-key "disabled-key" "\^K")
  (local-bind-to-key "disabled-key" "\^D")
  (local-bind-to-key "disabled-key" "\^H")
  (local-bind-to-key "disabled-key" "\^T")
  (local-bind-to-key "disabled-key" "\^O")
  (local-bind-to-key "disabled-key" "\^Y")
  (local-bind-to-key "disabled-key" "\^Q")
  (local-bind-to-key "disabled-key" "\^X\^F")
  (local-bind-to-key "disabled-key" "\^X\^I")
  (local-bind-to-key "disabled-key" "\^X\^L")
  (local-bind-to-key "disabled-key" "\^X\^R")
  (local-bind-to-key "disabled-key" "\^X\^?")
  (local-bind-to-key "disabled-key" "\ed")
  (local-bind-to-key "disabled-key" "\eh")
  (local-bind-to-key "disabled-key" "\ek")
  ))

(setq-default ~bat-state "")
(setq-default ~bat-new-mail-count 0)

(defun (~bat-new-mail-count ncnt
  (setq ncnt (arg 1 "Num messages: "))
  (if (!= ncnt ~bat-new-mail-count)
      (progn
        (setq ~bat-new-mail-count ncnt)
	(~bat-build-caption-modeline "")
	(if (> ncnt 0)
	    (~bat-info
	     (concat "You have new mail ("
		     ncnt " piece" (if (= ncnt 1) ")" "s)"))))))))

(defun (~bat-begin-state
  (setq ~bat-state (concat (arg 1 "State: ") "..."))
  (if bat-stingy-display
      (setq ~bat-state (substr ~bat-state 1 1)))
  (save-excursion
   (~bat-build-caption-modeline ~bat-state )
   (if (! (buffer-is-visible "bat-captions"))
       (message ~bat-state))
   (sit-for 0))))

(defun (~bat-end-state
  (save-excursion
   (~bat-build-caption-modeline
    (concat ~bat-state (if bat-stingy-display "D" "Done")))
   (if (! (buffer-is-visible "bat-captions"))
       (message (concat ~bat-state (if bat-stingy-display "D" "Done"))))
   (sit-for 0)
   (~bat-build-caption-modeline ""))))

(defun (~bat-fail-state
  (save-excursion
   (~bat-build-caption-modeline
    (concat ~bat-state (if bat-stingy-display "F" "Failed")))
   (sit-for 0)
   (~bat-build-caption-modeline ""))))

(defun (~bat-build-caption-modeline
  (temp-use-buffer "bat-captions")
  (setq mode-line-format
	(concat "---- BatCaptions  "
		(if (> ~bat-new-mail-count 0)
		    (concat "(Mail:" ~bat-new-mail-count ")  ")
		    "")
		(if (| bat-stingy-display ~bat-old-emacs) "%M" "%t%M")
		"  [? for help]  (%p)  "
		(arg 1 "State: ")))))

(defun (~bat-init-caption-buf
  (temp-use-buffer "bat-captions")
  (if (! ~bat-old-emacs) (setq show-time-in-mode-line 1))
  (~bat-build-caption-modeline "")
  (setq needs-checkpointing 0)
  (erase-buffer)
  (setq buffer-is-modified 0)
  (~bat-unbind-keys)
  (local-bind-to-key "~bat-pick" " ")
  (local-bind-to-key "~bat-next-caption" "n")
  (local-bind-to-key "~bat-previous-caption" "p")
  (local-bind-to-key "~bat-scroll-body" "v")
  (local-bind-to-key "~bat-reverse-scroll-body" "b")
  (local-bind-to-key "~bat-reverse-scroll-body" "\^H")
  (local-bind-to-key "~bat-reverse-scroll-body" "\^?")
  (local-bind-to-key "~bat-beginning-of-body" "<")
  (local-bind-to-key "~bat-end-of-body" ">")
  (local-bind-to-key "~bat-quit" "q")
; (local-bind-to-key "~bat-quit" "\^C")
  (local-bind-to-key "~bat-quit" "\^X\^C")
  (local-bind-to-key "bat-send" "s")
  (local-bind-to-key "bat-send" "m")  ; for bags people
  (local-bind-to-key "~bat-reply" "r")
  (local-bind-to-key "~bat-forward" "f")
  (local-bind-to-key "~bat-wide-reply" "w")
  (local-bind-to-key "~bat-all-reply" "W")
  (local-bind-to-key "~bat-resend" "F")
  (local-bind-to-key "next-line" "\r")
  (local-bind-to-key "~bat-digest-forward" "\^F")
  (local-bind-to-key "~bat-digest-backward" "\^B")
  (local-bind-to-key "~bat-help" "h")
  (local-bind-to-key "~bat-help" "?")
  (local-bind-to-key "~bat-append-to-file" "a")
  (local-bind-to-key "~bat-append-displayed-to-file" "A")
  (local-bind-to-key "~bat-toggle-stripping" "\^W")
  (local-bind-to-key "~bat-read" "R")
  (local-bind-to-key "~bat-set-quit-line" "^")
  (local-bind-to-key "~bat-punt" "/")
  (local-bind-to-key "~bat-read-updates-here" "N")
  (local-bind-to-key "~bat-prompt-subscribe" "S")
  (local-bind-to-key "~bat-prompt-unsubscribe" "U")
  (local-bind-to-key "~bat-delete" "d")
  (local-bind-to-key "~bat-undelete" "u")
  (local-bind-to-key "~bat-copy-delete" "c")
  (local-bind-to-key "~bat-copy" "C")
  (local-bind-to-key "~bat-unread" ".")
  (local-bind-to-key "~bat-rot13-body" "x")
  (local-bind-to-key "~bat-gripe" "g")
  (local-bind-to-key "~bat-post" "P")
  (local-bind-to-key "~bat-print-body" "\ep")
  (local-bind-to-key "~bat-vote" "V")
  (error-occurred (bat-captions-mode-hook))
  ))

(defun (~bat-build-display-modeline
  (temp-use-buffer "bat-display")
  (setq mode-line-format
	(concat "---- Bat" (arg 1 "Displayed: ")
		"%<L    [v to scroll]  --More--(%p)%>"))))

(defun (~bat-init-body-buf
  (temp-use-buffer "bat-display")
  (setq needs-checkpointing 0)
  (setq wrap-long-lines 0)
  (setq track-eol-on-^N-^P 0)
  (erase-buffer)
  (error-occurred
   (read-file (concat bat-etc-dir "/" (if bat-stingy-display "intro.small" "intro"))))
  (if (file-exists (concat bat-etc-dir "/news"))
      (save-excursion
       (end-of-file)
       (setq read-only 0)	       ; stupid emacs
       (newline)
       (insert-file (concat bat-etc-dir "/news"))
       (setq buffer-is-modified 0)))   ; stupid emacs
  (~bat-build-display-modeline "Mail")
  (setq buffer-is-modified 0)
  ))

(setq-default ~bat-initialized 0)
(setq-default ~bat-in-progress 0)

(defun (~bat-enter
  (~bat-call-robin)
  (if (! ~bat-initialized)
      (progn
	(~bat-init-caption-buf)
	(~bat-init-body-buf)
	(~bat-init-mail-buf)
	(~bat-command (concat "g" bat-headers "\n"))
	(setq ~bat-initialized 1)
;	(~bat-command "C(~bat-check-new-user)\n")
	1)
      0)))

(defun (~bat-restore
  (if ~bat-in-progress
      1
      (progn (~bat-exit) 0))))

(defun (~bat-auto-start
  (setq use-memory-file 0)	       ; defeat emacs "bug(?)"
  (if (error-occurred
       (batmail (getenv "BATFOLDERS") (getenv "BATSINCE")))
      (batmail))))

(defun (batmail mdirs msince
  (if (! ~bat-in-progress)
      (progn
	(if (! bat-stingy-display)
	    (progn
	      (switch-to-buffer " batmail")
	      (setq mode-line-format "In color!")
	      (setq wrap-long-lines 0)
	      (delete-other-windows)
	      (error-occurred
	       (read-file (concat bat-etc-dir "/" "initscreen")))))
	(if (error-occurred
	     (if (| (= (nargs) 0)
		    (error-occurred
		     (setq mdirs (arg 1))
		     (setq msince (arg 2))))
		 (progn
		   (setq mdirs
			 (get-tty-str "Read folders" bat-default-folders))
		   (setq msince (get-tty-str "Since" "updates")))))
	    (if (! bat-stingy-display) (delete-buffer " batmail"))
	    (progn
	      (setq ~bat-in-progress 1)
	      (~bat-enter)
	      (~bat-set-up-screen)
	      (if (!= mdirs "")
		  (~bat-get-captions mdirs msince)))))
      (~bat-set-up-screen))
  (novalue)))

(defun (~bat-exit
  (if (error-occurred (~bat-flush-robin))
      (~bat-clean-up 0)
      (~bat-command "C(~bat-clean-up 1)\n"))))

(defun (~bat-clean-up
  (setq ~bat-in-progress 0)
  (setq ~bat-initialized 0)
  (setq ~bat-last-cuid 0)
  (error-occurred (delete-buffer "bat-captions"))
  (error-occurred (delete-buffer "bat-display"))
  (error-occurred (delete-buffer "bat-composition"))
  (error-occurred (delete-buffer " bat-scratch"))
  (error-occurred (delete-buffer " batmail"))
  (if (! (arg 1)) (message "Robin is dead!"))
  (sit-for 0)))

;;; ----------------------------------------------------------------

(defun (~bat-check-new-user
  (if (! (file-exists "~/.bat-user"))
      (~bat-new-user))
  (novalue)))

(defun (~bat-new-user file response
  (temp-use-buffer " bat-scratch")
  (erase-buffer)
  (insert-string "You aren't on record as having used BatMail before; since\n"
		 "I would like to keep track of who is using BatMail, would\n"
		 "you like to:\n"
		 "1	Be recorded as a user of BatMail.\n"
		 "2	Be recorded as a user of BatMail, and subscribe to the\n"
		 "	BatMail bboard, which will contain information about\n"
		 "	future releases and enhancements.\n"
		 "3	Stay incognito.  (no one will ever know...)\n"
		 "\n"
		 "The BatMail bboard is called andrew.ms.batmail\n")
  (setq response
	(get-prompted-key " bat-scratch" "BatChoice" '1' "---- BatUser choices" 1))
  (sit-for 0)
  (if (!= response '3')
      (progn
	(erase-buffer)
	(insert-string "To: bader+mail.batmail.newuser\n"
		       "Subject: new BatMail user\n"
		       "\n"
		       "New BatMail User: "
		       (users-full-name) " (" (users-login-name) ")"
		       " on " (getenv "HOST") "\n")
	(if (= response '2')
	    (~bat-command "dandrew.ms.batmail\nSn\n")
	    )
	(setq file (~bat-temp-file))
	(write-named-file file)
	(~bat-command (concat "sn0:" file "\n"))))
  (erase-buffer)
  (write-named-file "~/.bat-user")))

;;; ----------------------------------------------------------------

(defun (~bat-build-mail-modeline
  (temp-use-buffer "bat-composition")
  (setq mode-line-format 
	(concat "---- BatComposition ("
		(if (arg 1 "Blind? ") "Blind" "No blind")
		")    [^X^S sends, ^X^C aborts, ^X? for more help]"))))

(defun (~bat-init-mail-buf
  (temp-use-buffer "bat-composition")
  (~bat-build-mail-modeline bat-send-bcc)
  (text-mode)
  (erase-buffer)
  (setq buffer-is-modified 0)
  (local-bind-to-key "~bat-submit" "\^X\^S")
  (local-bind-to-key "~bat-submit-no-validate" "\^X\^N")
  (local-bind-to-key "~bat-abort-mail" "\^X\^C")
; (local-bind-to-key "~bat-abort-mail" "\^C")
  (local-bind-to-key "~bat-mail-newline" "\^M")
  (local-bind-to-key "~bat-mail-newline" "\n"); for braindamaged telnet server
  (local-bind-to-key "~bat-insert-body" "\^X\^Y")
  (local-bind-to-key "~bat-insert-body" "\^Xi"); for bags people
  (local-bind-to-key "~bat-toggle-blind" "\^X\^B")
  (local-bind-to-key "~bat-mail-help" "\^X?")
  (local-bind-to-key "~bat-scroll-body" "\^Xv")
  (local-bind-to-key "~bat-reverse-scroll-body" "\^Xb")
  (local-bind-to-key "~bat-validate" "\er")

  (local-bind-to-key "correct-spelling-of-word" "\e$")

  (save-excursion (error-occurred (bat-composition-mode-hook)))

  (temp-use-buffer " bat-scratch")
  (setq needs-checkpointing 0)

  (temp-use-buffer " robin")
  (setq needs-checkpointing 0)
  (novalue)))

;;; ----------------------------------------------------------------

(setq-default ~bat-replying-to 0)

(defun (bat-send
  (~bat-get-template "m" 0 0)
  (novalue)))

(defun (~bat-reply
  (~bat-get-template "r" 1 1)
  (novalue)))

(defun (~bat-wide-reply
  (~bat-get-template "w" 1 1)
  (novalue)))

(defun (~bat-all-reply
  (~bat-get-template "a" 1 1)
  (novalue)))

(defun (~bat-forward
  (~bat-get-template "f" 0 1)
  (novalue)))

(defun (~bat-gripe
  (~bat-get-template "g" 0 0)
  (novalue)))

(defun (~bat-post dir
  (setq dir (~bat-current-folder))
  (if (!= dir "")
      (progn
	(~bat-command (concat "d" dir "\n")); dir to post to
	(~bat-get-template "p" 0 0)))
  (novalue)))

(defun (~bat-get-template code reply needscuid cuid
  (~bat-enter)
  (setq code (arg 1 "Code: "))
  (setq reply (arg 2 "Reply? "))
  (setq needscuid (arg 3 "Needs CUID? "))
  (if (& needscuid (= (setq cuid (~bat-active-cuid)) 0))
      (error-message "No current message!?"))
  (setq ~bat-replying-to (if needscuid cuid 0))
  (if (! (buffer-is-visible "bat-composition"))
      (progn
	(pop-to-buffer "bat-display")
	(if reply
	    (split-window-nicely))
	(switch-to-buffer "bat-composition"))
      (pop-to-buffer "bat-composition"))
  (if (| (! buffer-is-modified) (get-tty-bool "Erase old message" 1))
      (progn
	(erase-buffer)
	(~bat-command (concat "m" code cuid "\n"))))))

(defun (~bat-restore-normal-buffers
  (if (! (buffer-is-visible "bat-display"))
      (switch-to-buffer "bat-display")
      (delete-window-nicely))
  (pop-to-buffer "bat-captions")))

(defun (~bat-resend to cuid
  (setq cuid (~bat-active-cuid))
  (setq to (arg 1 (concat "Resend message " cuid " to: ")))
  (~bat-command
   (concat "z" cuid ":" to "\n"))
   (novalue)))

;;; ----------------------------------------------------------------

(setq-default ~bat-end-of-headers-mark 0)
(setq-default ~bat-local-blind 0)

(defun (~bat-mail file
  (setq file (arg 1 "Mail template file: "))
  (pop-to-buffer "bat-composition")
  (read-file file)
  (error-occurred
   (search-forward "\n\n")
   (previous-line)
   (save-excursion (error-occurred (bat-init-message-hook)))
   (error-occurred (insert-file "~/.headers"))
   (save-excursion
    (error-occurred (end-of-file) (insert-file "~/.signature")))
   (setq ~bat-end-of-headers-mark (dot)))
  (beginning-of-file)
  (error-occurred
   (re-search-forward "^[^ ]*:[ \t]*$\\|\n\n"))
  (~bat-build-mail-modeline (setq ~bat-local-blind bat-send-bcc))
  (sit-for 0)
  (novalue)))

(defun (~bat-toggle-blind
  (~bat-build-mail-modeline (setq ~bat-local-blind (! ~bat-local-blind)))))

(defun (~bat-mail-newline
  (if (< (dot) ~bat-end-of-headers-mark)
      (error-occurred (re-search-forward "^[^ ]*:[ \t]*$\\|\n\n"))
      (newline))))

(defun (~bat-mail-help
  (save-excursion
   (if (! (buffer-is-visible "bat-display"))
       (progn
	 (split-window-nicely)
	 (previous-window)
	 (switch-to-buffer "bat-display"))
       (pop-to-buffer "bat-display"))
   (read-file (concat bat-help-dir "/mcommands.gos"))
   (setq ~bat-last-cuid -1)	       ; so not confused with any other cuid
   (~bat-build-display-modeline "Help")
   (end-of-window)		       ; so the more prompt is informative
   )))

(defun (~bat-insert-body buf
  (save-excursion from
   (temp-use-buffer "bat-display")
   (save-excursion
    (beginning-of-file)
    (set-mark)
    (end-of-file)
    (while (& (! (bobp)) (bolp))
      (backward-character))
    (if (! (bobp))
	(forward-character))
    (copy-region-to-buffer " bat-scratch"))
   (temp-use-buffer " bat-scratch")
   (beginning-of-file)
   (if (! (error-occurred (re-search-forward "^From:\\|^$")))
       (if (looking-at " \\(.*\\)")
	   (progn
	     (region-around-match 1)
	     (setq from (region-to-string))
	     (if (! (error-occurred (re-search-forward "^$")))
		 (next-line)
		 (end-of-file)))
	   (next-line)))
   (while (& (! (eobp)) (eolp))
     (next-line))
   (set-mark)
   (beginning-of-file)
   (erase-region)
   (if (!= from "")
       (insert-string from " writes:\n"))
   (error-occurred (re-replace-string "^" "> ")))
  (set-mark)
  (yank-buffer " bat-scratch")
  (exchange-dot-and-mark)))

;;; ----------------------------------------------------------------
;;; submitting mail for transmission

(defun (~bat-abort-mail
  (temp-use-buffer "bat-composition")
  (setq buffer-is-modified 0)
  (if (~bat-restore)
      (~bat-restore-normal-buffers))))

;;; rewrite-headers, but don't send
(defun (~bat-validate
  (~bat-command
   (concat "[\n"
	   "C(~bat-begin-state \"Validating addresses\")\n"
	   "rTo:" (~bat-header-contents "To" 1) "\n"
	   "rCC:" (~bat-header-contents "CC" 0) "\n"
	   "rBCC:" (~bat-header-contents "BCC" 0) "\n"
	   "C(~bat-end-state)\n"
	   "|\n"		       ; end state even if errors
	   "C(~bat-fail-state)\n"
	   "]\n"))))

;;; submits mail, but only if validation succeeds
(defun (~bat-submit
  (~bat-command
   (concat "[\n"
	   "[\n"		       ; two levels to handle end-state
	   "C(~bat-begin-state \"Validating addresses\")\n"
	   "rTo:" (~bat-header-contents "To" 1) "\n"
	   "rCC:" (~bat-header-contents "CC" 0) "\n"
	   "rBCC:" (~bat-header-contents "BCC" 0) "\n"
	   "C(~bat-end-state)\n"
	   "|\n"		       ; end state even if errors
	   "C(~bat-fail-state)\n"
	   "]\n"
	   "C(~bat-submit-no-validate)\n"
	   "]\n"))))

(defun (~bat-submit-no-validate
  (temp-use-buffer "bat-composition")
  (save-excursion (error-occurred (bat-submit-message-hook)))
  (write-current-file)
  (~bat-command
   (concat "[\n"
	   "s" (if ~bat-local-blind "b" "n") ~bat-replying-to ":" (current-file-name) "\n"
	   "C(~bat-mail-sent)\n"
	   "]\n"))))

(defun (~bat-header-contents name req
  (setq name (arg 1 "Header name: "))
  (setq req (arg 2 "Required? "))
  (temp-use-buffer "bat-composition")
  (beginning-of-file)
  (if (! (error-occurred
	  (re-search-forward (concat "^" name ":[ \t]*"))))
      (progn
	(if (looking-at "[ \t]*\n[^ \t]")
	    (if req
		(error-message "You must have a " name ": header!")
		(progn
		  (beginning-of-line)
		  (set-mark)
		  (next-line)
		  (erase-region)
		  ""))
	    (progn
	      (set-mark)
	      (if (error-occurred (re-search-forward "\n[^ \t]"))
		  (end-of-file)
		  (progn (backward-character) (backward-character)))
	      ;; eliminate newlines which screw up commands to robin
	      (copy-region-to-buffer " bat-scratch")
	      (temp-use-buffer " bat-scratch")
	      (beginning-of-file)
	      (error-occurred (re-replace-string "[ \t\n][ \t\n]*" " "))
	      (set-mark)
	      (end-of-file)
	      (region-to-string))))
      (if req
	  (error-message "You must have a " name ": header!")
	  ""))))

;;; callback from robin
(defun (~bat-rewrite-header
  (temp-use-buffer "bat-composition")
  (beginning-of-file)
  (if (! (error-occurred
	  (re-search-forward (concat "^" (arg 1 "Header name: ") ":[ \t]*"))))
       (progn
	(insert-string (arg 2 "Header contents: "))
	(set-mark)
	(if (error-occurred (re-search-forward "^[^ \t\n]\\|^$"))
	    (end-of-file)
	    (progn (beginning-of-line) (backward-character)))
	(erase-region)))))

;;; callback from robin
(defun (~bat-mail-sent
  (setq buffer-is-modified 0)
  (if (~bat-restore)
      (~bat-restore-normal-buffers))
  (sit-for 0)))
  
;;; ----------------------------------------------------------------
;;; * end of batmail.ml *
