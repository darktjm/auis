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

;;; Nice prompting type stuff
;;; 1986 - Miles Bader

(setq-default prompt-requests-in-progress 0)
(setq-default prompt-saved-str "")

(defun (display-string sdbs-msg
  (if (> prompt-requests-in-progress 0)
      (setq prompt-saved-str (arg 1 "Save str: "))
      (progn
	(message (arg 1 "Str: "))
	(sit-for 0)
	(setq prompt-saved-str "")))
  (novalue)))

(defun (prompt-in
  (setq prompt-requests-in-progress (+ prompt-requests-in-progress 1))))

(defun (prompt-out
  (setq prompt-requests-in-progress (- prompt-requests-in-progress 1))
  (if (& (< prompt-requests-in-progress 1) (!= prompt-saved-str ""))
      (display-string prompt-saved-str))))

(defun (get-tty-char gtc-char gtc-prompt
  (setq gtc-prompt (arg 1 "Message: "))
  (prompt-in)
  (message gtc-prompt)
  (save-window-excursion
   (pop-to-buffer "  Minibuf")
   (sit-for 0)
   (setq gtc-char (get-tty-character)))
  (message gtc-prompt (char-to-string gtc-char))
  (prompt-out)
  gtc-char))

(defun (get-tty-bool gtb-key gtb-def gtb-said gtb-ans gtb-prompt
  (setq gtb-def (arg 2 "Default: "))
  (setq gtb-prompt (concat (arg 1 "Prompt: ") "? (y,n) [" (if gtb-def "y" "n") "] "))
  (setq gtb-said 0)
  (prompt-in)
  (while (progn
	   (setq gtb-key (get-tty-char gtb-prompt))
	   (if (| (= gtb-key 'y') (= gtb-key 'Y'))
	       (progn (setq gtb-ans 1) 0)
	       (| (= gtb-key 'n') (= gtb-key 'N'))
	       (progn (setq gtb-ans 0) 0)
	       (| (= gtb-key ' ') (= gtb-key '\r'))
	       (progn (setq gtb-ans gtb-def) 0)
	       gtb-said
	       1
	       (progn
		 (setq gtb-prompt (concat "Please use y or n.  " gtb-prompt))
		 (setq gtb-said 1)
		 1))))
  (message gtb-prompt (if gtb-ans "Yes" "No"))
  (prompt-out)
  gtb-ans))
	       

(defun (get-tty-str gts-msg gts-str gts-def
  (setq gts-msg (concat (arg 1 "Message: ") ": "))
  (setq gts-def (arg 2 "Default: "))
  (if (!= gts-def "")
      (setq gts-msg (concat gts-msg "[" gts-def "] ")))
  (prompt-in)
  (message gts-msg)
  (save-window-excursion
   (pop-to-buffer "  Minibuf")
   (sit-for 0)
   (if (error-occurred (setq gts-str (get-tty-string gts-msg)))
       (progn
	 (prompt-out)
	 (error-message "Aborted."))))
  (if (= gts-str "")
      (setq gts-str gts-def))
  (message gts-msg gts-str)
  (prompt-out)
  gts-str))

(defun (pop-up-buffer pubuf size pucount pumodeline tmark
  (setq pubuf (arg 1 "Buffer: "))
  (setq pucount (arg 2 "Size: "))
  (setq pumodeline (arg 3 "Modeline: "))
  (top-window)
  (setq size (window-height))
  (while (& (< size pucount)
	    (progn
	      (delete-window)
	      (!= (window-height) size)))
    (setq size (window-height)))
  (save-excursion
   (beginning-of-window)
   ;; This assumes that screen and logical lines are the same (sigh...)
   (provide-prefix-argument (+ pucount 1) (next-line))
   (setq tmark (dot)))
  (if (> size (+ pucount 1))
      (progn
	(split-window-nicely)
	(previous-window)))
  (switch-to-buffer pubuf)
  (setq mode-line-format pumodeline)
  (while (< pucount (window-height))
    (shrink-window))
  (while (> pucount (window-height))
    (enlarge-window))
  (beginning-of-file)
  (next-window)
  (save-excursion
   (goto-character tmark)
   (line-to-top-of-window))))

(defun (get-prompted-key gpk-msg gpk-mdln count gpk-buf len key gpk-def gpk-popup
  (setq gpk-buf (arg 1 "Buffer: "))
  (setq gpk-msg (arg 2 "Message: "))
  (setq gpk-def (arg 3 "Default: "))
  (setq gpk-mdln (arg 4 "Modeline: "))
  (setq gpk-popup (arg 5 "Popup? "))
  (setq count 4)
  (temp-use-buffer gpk-buf)
  (beginning-of-file)
  (setq gpk-msg (concat gpk-msg "? ("))
  (while (! (error-occurred (re-search-forward "^\\(.\\)\t")))
    (region-around-match 1)
    (setq gpk-msg (concat gpk-msg (region-to-string) ",")))
  (beginning-of-file)
  (setq len 0)
  (while (! (eobp))
    (next-line)
    (setq len (+ len 1)))
  (setq gpk-msg
	(if gpk-popup		       ; only show ? if not already there
	    (concat (substr gpk-msg 1 -1) ") ")
	    (concat gpk-msg "?) ")))
  (if (> gpk-def 0)
      (setq gpk-msg (concat gpk-msg "[" (char-to-string gpk-def) "] ")))
  (prompt-in)
  (save-window-excursion
   (if gpk-popup
       (pop-up-buffer gpk-buf len gpk-mdln))
   (setq key 0)
   (while (= key 0)
     (setq key (get-tty-char gpk-msg))
     (if (= key '\^G')
	 (progn
	   (prompt-out)
	   (error-message "Aborted."))
	 (& (| (= key ' ') (= key '\r')) (!= gpk-def 0))
	 (setq key gpk-def)
	 (error-occurred
	  (temp-use-buffer gpk-buf)
	  (beginning-of-file)
	  (re-search-forward (concat "^" (char-to-string key) "\t")))
	 (progn
	   (setq key 0)
	   (if (! gpk-popup)
	       (pop-up-buffer gpk-buf len gpk-mdln))
	   (setq gpk-popup 1)))))
  (message gpk-msg (char-to-string key))
  (prompt-out)
  key))
