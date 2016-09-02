\begindata{text,269219396}
\textdsversion{12}
\template{default}
\define{italic
menu:[Font~1,Italic~11]
attr:[FontFace Italic Int Set]}
\define{bold
menu:[Font~1,Bold~10]
attr:[FontFace Bold Int Set]}
\define{bigger
menu:[Font~1,Bigger~20]
attr:[FontSize PreviousFontSize Point 2]}
\define{indent
menu:[Region~4,Indent~20]
attr:[LeftMargin LeftMargin Inch 32768]
attr:[RightMargin RightMargin Inch 32768]}
\define{typewriter
menu:[Font~1,Typewriter~40]
attr:[FontFace FixedFace Int Set]
attr:[FontFamily Monastery Int 0]}
\define{display
menu:[Region~4,Display~14]
attr:[LeftMargin LeftMargin Inch 32768]
attr:[RightMargin RightMargin Inch 32768]}
\define{example
menu:[Region~4,Example~12]
attr:[LeftMargin LeftMargin Inch 32768]
attr:[FontFace FixedFace Int Set]
attr:[FontFamily AndyType Int 0]}
\define{description
menu:[Region~4,Description~11]
attr:[LeftMargin LeftMargin Inch 32768]
attr:[Indent LeftMargin Inch -32768]}
\define{quotation
menu:[Region~4,Quotation~10]
attr:[LeftMargin LeftMargin Inch 32768]
attr:[RightMargin RightMargin Inch 32768]
attr:[FontFace Italic Int Set]}
\define{subscript
menu:[Font~1,Subscript~30]
attr:[Script PreviousScriptMovement Point 2]
attr:[FontSize PreviousFontSize Point -2]}
\define{superscript
menu:[Font~1,Superscript~31]
attr:[Script PreviousScriptMovement Point -6]
attr:[FontSize PreviousFontSize Point -2]}
\define{smaller
menu:[Font~1,Smaller~21]
attr:[FontSize PreviousFontSize Point -2]}
\define{heading
menu:[Title~3,Heading~11]
attr:[LeftMargin ConstantMargin Inch -13107]
attr:[FontFace Bold Int Set]}
\define{majorheading
menu:[Title~3,MajorHeading~10]
attr:[Justification Centered Point 0]
attr:[FontSize PreviousFontSize Point 4]}
\define{formatnote
menu:[Region~4,FormatNote~60]
attr:[Flags PassThru Int Set]}
\define{subheading
menu:[Title~3,Subheading~12]
attr:[Justification LeftJustified Point 0]
attr:[FontFace Bold Int Set]}
\define{center
menu:[Justify~2,Center~10]
attr:[Justification Centered Point 0]}
\define{flushleft
menu:[Justify~2,FlushLeft~20]
attr:[Justification LeftJustified Point 0]}
\define{flushright
menu:[Justify~2,FlushRight~21]
attr:[Justification RightJustified Point 0]}
\define{leftindent
menu:[Region~4,LeftIndent~21]
attr:[LeftMargin LeftMargin Inch 32768]}
\define{programexample
menu:[Region~4,ProgramExample~13]
attr:[LeftMargin LeftMargin Inch 32768]
attr:[FontFace FixedFace Int Set]
attr:[FontFamily AndyType Int 0]
attr:[FontSize ConstantFontSize Point 10]}
\define{initial
menu:[Region~4,Initial~23]
attr:[LeftMargin LeftMargin Inch 26214]
attr:[RightMargin RightMargin Inch 16384]
attr:[FontFamily AndySans Int 0]
attr:[FontSize ConstantFontSize Point 10]}
\define{sans
menu:[Font~1,Sans]
attr:[FontFamily AndySans Int 0]}
Empower this Ness and click on <DATE> below to get the proper date.

\begindata{ness,269563520}
\origin{01\\4 Jan 1990 at 13:46:04 EST\\wjh:  Fred Hansen\\00}
\template{default}
extend "date" 

on mouse "any"

	if mouseaction = mouseleftup then

		-- remove date inset and this Ness

		-- store proper date 

		firstobject(base(currentselection(defaulttext)))

		marker nessmarker := WhereItWas()

		firstobject(next(nessmarker))

		firstobject(next(WhereItWas()))

		marker m := WhereItWas()

		replace(m, date_text(date_canonical(system("date"))))

		replace(extent(base(nessmarker), nessmarker), "")

	end if

end mouse

end extend\
\enddata{ness,269563520}
\view{nessview,269563520,3,0,72}


\indent{\smaller{

\flushright{\formatnote{\\&}\
\begindata{raster,269318144}
2 0 65536 65536 0 0 118 122
bits 269318144 118 122
u |
u |
u |
l07e0f8l |
l1fe1fcl |
k1e1fe3fe08k |
k7e3fe3fe1fk |
j01fe3fe3fe3f	c0j |
j03fe3fe3fe3f	e0j |
j03fe3fe3fe3f	e0j |
i01e3fe3fe3fe	3fe180i |
i03e3fe3fe3fe	3fe3c0i |
i0fe3fe3fe3fe	3fe3f0i |
i1fe3fe3fe3fe	3fe3f8i |
i1fe3fe3fe3fe	3fe3fci |
i3fe3fe3fe3fe	3fe3fei |
i3fe3fe3fe3fe	3fe3fei |
i3fe3fe3fe3fe	3fe3fei |
h063fe3fe3fe3	fe3fe3fei |
h0e3fe3fe3fe3	fe3fe3fe38h |
h1e3fe3fe3fe3	fe3fe3fe3ch |
h3e3fe3fe3fe3	fe3fe3fe3eh |
h7e3fe3fe3fe3	fe3fe3fe3fh |
hfe3fe3fe3fe3	fe3fe3fe3fh |
g01fe3fe3fe3f	e3fe3fe3fe3f80	g |
g01fe3fe3fe3f	e3fe3fe3fe3fc0	g |
g01fe3fe3fe3f	e3fe3fe3fe3fc0	g |
g03fe3fe3fe3f	e3fe3fe3fe3fc0	g |
g03fe3fe3fe3f	e3fe3fe3fe3fc0	g |
g03fe3fe3fe3f	e3fe3fe3fe3fc0	g |
g03fe3fe3fe3f	e3fe3fe3fe3fc0	g |
g03fe3fe3fe3f	e3fe3fe3fe3fc0	g |
g03fe3fe3fe3f	e3fe3fe3fe3fc6	g |
g63fe3fe3fe3f	e3fe3fe3fe3fc7	g |
g63fe3fe3fe3f	03fe3fe3fe3fc7	g |
ge3fe3fe3fc3e	03fe3fe3fe3fc7	80 |
ge3fe3fe3f03e	03fe3fe3fe3fc7	80 |
01e3fe3fe3f03e	03fe3fe3fe3fc7	c0 |
01e3fe3fc3f03e	03fe3fe3fe3fc7	c0 |
03e3fe3f83f03e	03fe3fe3fe3fc7	c0 |
03e3fe3f03f03e	03fe3fe3fe3fc7	e0 |
03e3fe3f03f03e	03fe3fe3fe3fc7	e0 |
07e3fe3e03f03e	03fe3fe3fe3fc7	e0 |
07e3fe3e03f03e	03fe3fe3fe3fc7	f0 |
0fe3fe3e03f03e	03fe3fe3fe3fc7	f0 |
0fe3fc3e03f03e	03fe3fe3fe3fc7	f0 |
0fe3f83e03f03e	03fe3fe3fe3fc7	f0 |
1fe3f03e03f03e	03fe3fe3fe3fc7	f8 |
1fe3f03e03f03e	03fe3fe3fe3fc7	f8 |
1fe3e03e03f03e	03fe3fe3fe3fc7	f8 |
1fe3e03e03f03e	03fe3fe3fe3fc7	f8 |
3fe3e03e03f03e	03fe3fe3fe3fc7	f8 |
3fe3e03e03f03e	03fe3fe3fe3fc7	f8 |
3fe3e03e03f03e	03fe3fe3fe3fc7	f8 |
3fe3e03e03f03e	03fe3fe3fe3fc7	f8 |
3fe3e03e03f03e	03fe3fe3fe3fc7	f8 |
3fe3e03e03f03e	03fe3fe3fe3fc7	f8 |
3fc3e03e03f03e	03fe3fe3fe3fc7	f8 |
3f83e03e03f03e	03fe3fe3fe3fc7	f8 |
3f83e03e03f03e	03fe3fe3fe3fc7	f8 |
3f03e03e03f03e	03fe3fe3fe3fc7	f0 |
3f03e03e03f03e	03fe3fe3fe3fc7	f0 |
3f03e03e03f03e	03fe3fe3fe3fc7	f0 |
3f03e03e03f03e	03fe3fe3fe3fc7	e0 |
3f03e03e03f03e	03fe3fe3fe3fc7	e0 |
3f03e03e03f03e	03fe3fe3fe3fc7	e0 |
3f03e03e03f03e	03fe3fe3fe3fc7	e0 |
3f03e03e03f03e	03fe3fe3fe3fc7	c0 |
3f03e03e03f03e	03fe3fe3fe3fc7	c0 |
3f03e03e03f03e	03fe3fe3fe3fc7	c0 |
1f03e03e03f03e	03fe3fe3fe3fc7	c0 |
1f03e03e03f03e	03fe3fe3fe3fc7	c0 |
1f03e03e03f03e	03fe3fe3fe3fc7	c0 |
1f03e03e03f03e	03fe3fe3fe3fc7	c0 |
1f03e03e03f03e	03fe3fe3fe3fc7	c0 |
0f03e03e03f03e	03fe3fe3fe3f87	c0 |
0f03e03e03f03e	03fe3fe3fe3f87	c0 |
0f03e03e03f03e	03fe3fe3fe3f07	c0 |
0f03e03e03f03e	03fe3fe3fe3e07	c0 |
0f03e03e03f03e	03fe3fe3fe3e07	c0 |
0703e03e03f03e	03fe3fe3fe3e07	c0 |
0703e03e03f03e	03fe3fe3fe3e07	c0 |
0703e03e03f03e	03fe3fe3fc3e07	c0 |
!03e03e03f03e	03fe3fe3fc3e07	c0 |
0203e03e03f03e	03fe3fe3f83e07	c0 |
g03e03e03f03e	03fe3fe3e03e07	c0 |
g03e03e03f03e	03fe3fe3e03e07	c0 |
g03e03e03f03e	03fe3fc3e03e07	80 |
g03e03e03f03e	03fe3fc3e03e07	80 |
g03e03e03f03e	03fe3f03e03e07	g |
g03e03e03f03e	03fc3e03e03e06	g |
g03e03e03f03e	03f03e03e03eh |
g03e03e03f03e	03e03e03e03eh |
g03e03e03f03e	03e03e03e03eh |
g03e03e03f03e	03e03e03e03eh |
g03e03e03f03e	03e03e03e03eh |
g03e03e03f03e	03e03e03e03eh |
g03e03e03f03e	03e03e03e03eh |
g01e03e03f03e	03e03e03e03eh |
g01e03e03f03e	03e03e03e03eh |
he03e03f03e03	e03e03e03eh |
h603e03f03e03	e03e03e03eh |
i3e03f03e03e0	3e03e03ch |
i3e03f03e03e0	3e03e03ch |
i3e03f03e03e0	3e03e038h |
i3e03f03e03e0	3e03e020h |
i3e03f03e03e0	3e03e0i |
i3e03f03e03e0	3e03e0i |
i3e03f03e03e0	3e03e0i |
i3e03f03e03e0	3e03e0i |
i1e03f03e03e0	3e03e0i |
i0e03f03e03e0	3e03e0i |
j03f03e03e03e	03c0i |
j03f03e03e03e	0380i |
j03f03e03e03e	k |
j03f03e03e03e	k |
j03f03e03e03e	k |
kc03e03e03ek |
l3e03e03ck |
l3e03e0l |
l0c03c0l |
u |
\enddata{raster, 269318144}
\view{rasterview,269318144,4,0,0}\formatnote{\\&}


Information Technology Center

Carnegie Mellon University

Pittsburgh, PA 15213-3890}

\begindata{cel,268686524}
\V 2
\begindata{raster,269583100}
2 0 68266 68266 0 0 52 20
bits 269583100 52 20
m |
m |
m |
m |
m |
g3e061fefc0g |
g3306030ch |
g318f030ch |
03318903!0cg |
0e3199830f87g |
38319f830c01c0 |
0e31b0c30c07g |
033330c3!0cg |
g3e30c30fc0g |
m |
m |
m |
m |
m |
m |
\enddata{raster, 269583100}
0 269583100 0 0 0 0 
>OBJ< raster
>VIEW< rasterview
>REF< date
\enddata{cel,268686524}
\view{celview,268686524,5,0,0}


Inside Address


Dear X:


Letter body.


Sincerely,




Wilfred J. Hansen}}




\enddata{text,269219396}
