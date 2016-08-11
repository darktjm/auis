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

;;; Nice prompting type stuff
;;; 1986 - Miles Bader
;;; adapted from prompt.ml Dec, 1988

(defvar prompt-requests-in-progress 0)
(defvar prompt-saved-str "")

(defun display-string (str)
  (if (> prompt-requests-in-progress 0)
      (setq prompt-saved-str str)
    (progn
      (message "%s" str)
      (setq prompt-saved-str ""))))

(defun prompt-in ()
  (setq prompt-requests-in-progress (+ prompt-requests-in-progress 1)))

(defun prompt-out ()
  (setq prompt-requests-in-progress (- prompt-requests-in-progress 1))
  (if (and (< prompt-requests-in-progress 1) (not (equal prompt-saved-str "")))
      (display-string prompt-saved-str)))

(require 'backquote)

(put 'in-echo-area 'lisp-indent-hook 0)

(defmacro in-echo-area (&rest body)
  (` (let ((ociea cursor-in-echo-area))
       (setq cursor-in-echo-area t)
       (unwind-protect (progn (,@ body))
	 (setq cursor-in-echo-area ociea)))))

(defun get-tty-char (prompt)
  (prompt-in)
  (message "%s" prompt)
  (save-window-excursion
    (unwind-protect (in-echo-area (read-char))
      (prompt-out))))

(defun get-tty-bool (prompt &optional def)
  (setq prompt (concat prompt "? "))
  (prompt-in)
  (unwind-protect (in-echo-area (y-or-n-p prompt))
    (prompt-out)))

(defun get-tty-str (msg &optional def)
  (setq msg (concat msg ": "))
  (if (and def (not (equal def "")))
      (setq msg (concat msg "[" def "] ")))
  (prompt-in)
  (let ((str (unwind-protect (read-string msg) (prompt-out))))
    (if (and def (equal str ""))
	(setq str def))
    str))

(defun top-window ()
  (let ((win (selected-window)))
    (while (let ((edges (window-edges win)))
	     (or (/= (car edges) 0) (/= (car (cdr edges)) 0)))
      (setq win (next-window win)))
    win))

(defun pop-up (sz modeln)
  (let ((buf (current-buffer))
	(window-min-height 0))
    (setq sz (1+ sz))
    (select-window (top-window))
    (let ((window-min-height sz))
      (if (> sz (- (window-height) 2))
	  (enlarge-window (- sz (window-height)))
	(split-window (selected-window) sz))
      (switch-to-buffer buf)
      (goto-char (point-min))
      (setq window-min-height sz)
      (setq mode-line-format modeln))))

(defun get-prompted-key (buf msg def mdln popup)
  (let ((len 0) (key 0))
    (set-buffer buf)
    (goto-char (point-min))
    (setq msg (concat msg "? ("))
    (while (re-search-forward "^.\t" nil t)
      (setq msg
	    (concat msg
		    (buffer-substring (car (match-data))
				      (- (car (cdr (match-data))) 1))
		    ",")))
    (goto-char (point-min))
    (while (= (forward-line 1) 0)
      (setq len (1+ len)))
    (setq msg
	  (if popup		       ; only show ? if not already there
	      (concat (substring msg 0 (- (length msg) 1)) ") ")
	    (concat msg "?) ")))
    (if (and def (> def 0))
	(setq msg (concat msg "[" (char-to-string def) "] ")))
    (prompt-in)
    (unwind-protect
	(save-window-excursion
	  (if popup
	      (pop-up len mdln))
	  (while (equal key 0)
	    (setq key (get-tty-char msg))
	    (cond ((equal key ? )
		   (if popup
		       (scroll-up nil)
		     (setq popup t)))
		  ((equal key ?\C-?)
		   (if popup
		       (scroll-down nil)
		     (setq popup t)))
		  ((and (equal key ?\r)
			(and def (not (equal def 0))))
		   (setq key def))
		  ((progn
		     (set-buffer buf)
		     (goto-char (point-min))
		     (not
		      (re-search-forward (concat "^" (char-to-string key) "\t")
					 nil
					 t)))
		   (setq key 0)
		   (if (not popup)
		       (pop-up len mdln))
		   (setq popup t))))
	  key)
      (prompt-out))))
