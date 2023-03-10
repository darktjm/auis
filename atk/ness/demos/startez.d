\begindata{text,538478848}
\textdsversion{12}
\template{default}
\define{global
}


This file shows how to start an editor (or other program) from Ness.


\begindata{cel,538590472}
\V 2
\begindata{value,539104264}
>1
\enddata{value,539104264}
10 539104264 1 0 0 0 
>OBJ< value
>VIEW< buttonV
>REF< Button
\begindata{text,538607708}
\textdsversion{12}
[long] <bodyfont-size> ()

[string] <bodyfont> ()

[string] <label> (Start ez)

\enddata{text,538607708}
\enddata{cel,538590472}
\view{celview,538590472,576,73,24}

\begindata{ness,538752008}
\origin{00\1 Jun 1989 at 14:28:28 EDT\wjh:  Fred Hansen\}
\template{default}
\define{fullwidth
menu:[Justify,Full Width]
attr:[LeftMargin LeftMargin Cm -25431]
attr:[RightMargin RightMargin Cm -27743]}
\define{sans
menu:[Font,Sans]
attr:[FontFamily AndySans Int 0]}


extend "Button" on mouse "any"

	marker andy;

	if mouseaction = mouseleftup then

		andy := system("andrewdirs andrewdir");

		if first(andy) /= "/" then 

			andy := system("echo $$ANDREWDIR");

		end if;

		if first(andy) /= "/" then 

			andy := "/usr/andrew";

		else -- strip \\n from system() result

			andy := extent(andy, start(last(andy)));

		end if;

		TellUser("Wait while I start an editor process");

		im_ForceUpdate();		-- make message appear 

		system(" ez " ~ andy 

			~ "/doc/AtkTour/Birth "

			~ " </dev/null  >/dev/null 2>&1 &");

		TellUser("Editor will appear shortly");

	end if;

end mouse end extend

\enddata{ness,538752008}
\view{nessview,538752008,580,0,0}
\begindata{bp,537558784}
\enddata{bp,537558784}
\view{bpv,537558784,581,0,0}
Copyright 1992 Carnegie Mellon University and IBM.  All rights reserved.
\enddata{text,538478848}
