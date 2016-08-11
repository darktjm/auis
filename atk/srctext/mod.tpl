\begindata{text,538982696}
\textdsversion{12}
\define{comment

attr:[FontFace Italic Int Set]}
\define{keyword

attr:[FontFace Bold Int Set]}
\define{predefined

attr:[FontFace Bold Int Set]
attr:[FontFace Italic Int Set]}
\define{function

attr:[FontFace Bold Int Set]
attr:[FontSize PreviousFontSize Point 2]}
\define{userdef

attr:[FontFace Bold Int Set]}
\define{string
}
\define{global

attr:[LeftMargin LeftEdge Int 16]
attr:[Indent LeftMargin Int -16]
attr:[Justification LeftJustified Point 0]
attr:[Flags ContinueIndent Int Set]
attr:[Flags TabsCharacters Int Set]
attr:[FontFace FixedFace Int Set]
attr:[FontFamily AndyType Int 0]}
\define{preproc

attr:[FontFace Italic Int Set]}
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
