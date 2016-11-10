\begindata{text,538982696}
\textdsversion{12}
\template{modtext}
\define{predefined
attr:[FontFace Bold Int Set]
attr:[FontFace Italic Int Set]}
\define{preproc
attr:[FontFace Bold Int Set]}
(* File <@filename@> created by <@programmer@> on <@date@>.

*)


IMPLEMENTATION MODULE <@name@>;



TYPE

    Handle = ;


PROCEDURE Nil(): Handle;


(* Always returns an empty Handle. *)


BEGIN RETURN NIL END Nil;


PROCEDURE Equal(h1, h2: Handle): BOOLEAN;


(* Returns TRUE if h1 and h2 are the same Handle and FALSE otherwise. *)


BEGIN RETURN h1 = h2 END Equal;


BEGIN


END <@name@>.


(* CHANGE LOG

<@log@>

*)

\enddata{text,538982696}
