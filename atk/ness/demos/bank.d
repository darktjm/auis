\begindata{text,538506344}
\textdsversion{12}
\template{default}
\define{plus
}
\plus{


\begindata{lset,539340360}
\V 1
\begindata{lset,539340824}
\V 1
\begindata{lset,539341032}
\V 1
\begindata{cel,539017080}
\V 2
\begindata{value,538916504}
>180
\enddata{value,538916504}
10 538916504 1 0 0 0 
>OBJ< value
>VIEW< sliderV
>REF< Amount
\begindata{text,539341240}
\textdsversion{12}
[string] <style> ()

[string] <immediate-update> ()

[string] <read-only> ()

[string] <color> ()

[string] <shade-color> ()

[long] <min_value> (20)

[long] <max_value> (300)

[long] <bodyfont-size> (14)

[string] <bodyfont> (andysans)

[string] <label> (Amount)

\enddata{text,539341240}
\enddata{cel,539017080}
0 0 0 539017080 0 0 0
>OBJ< cel
>VIEW< celview
>REF< 
\enddata{lset,539341032}
\begindata{lset,539341592}
\V 1
\begindata{lset,539341800}
\V 1
\begindata{cel,539342008}
\V 2
\begindata{value,539015928}
>48
\enddata{value,539015928}
10 539015928 1 0 0 0 
>OBJ< value
>VIEW< thumbstrV
>REF< Months
>LINK< t
\begindata{text,539342152}
\textdsversion{12}
[string] <immediate-update> ()

[string] <background-color> ()

[string] <foreground-color> ()

[long] <increment> (6)

[long] <min_value> (6)

[long] <max_value> (300)

[long] <bodyfont-size> (14)

[string] <bodyfont> (andysans)

[string] <label> (Months)

\enddata{text,539342152}
\enddata{cel,539342008}
0 0 0 539342008 0 0 0
>OBJ< cel
>VIEW< celview
>REF< 
\enddata{lset,539341800}
\begindata{lset,539342504}
\V 1
\begindata{cel,539342712}
\V 2
\begindata{value,539018088}
>10
\enddata{value,539018088}
10 539018088 1 0 0 0 
>OBJ< value
>VIEW< thumbstrV
>REF< Interest
>LINK< INK< t
\begindata{text,539342856}
\textdsversion{12}
[string] <immediate-update> ()

[string] <background-color> ()

[string] <foreground-color> ()

[long] <increment> (1)

[long] <min_value> (5)

[long] <max_value> (20)

[long] <bodyfont-size> (14)

[string] <bodyfont> (andysans)

[string] <label> (Interest)

\enddata{text,539342856}
\enddata{cel,539342712}
0 0 0 539342712 0 0 0
>OBJ< cel
>VIEW< celview
>REF< 
\enddata{lset,539342504}
2 53 0 0 539341800 539342504 0
>OBJ< 
>VIEW< 
>REF< 
\enddata{lset,539341592}
2 50 0 0 539341032 539341592 0
>OBJ< 
>VIEW< 
>REF< 
\enddata{lset,539340824}
\begindata{lset,539343208}
\V 1
\begindata{cel,539343416}
\V 2
\begindata{text,539343688}
\textdsversion{12}
\template{default}
\define{upbase

attr:[Script PreviousScriptMovement Point -15]}
\leftindent{Dear Valued Customer,

Your application for a loan of\upbase{ 
\begindata{cel,539390680}
\V 2
\begindata{value,539390824}
>0
\enddata{value,539390824}
10 539390824 1 0 0 0 
>OBJ< value
>VIEW< stringV
>REF< ShowPrincipal
>LINK< 
\begindata{text,539391000}
\textdsversion{12}
[string] <background-color> ()

[string] <foreground-color> ()

[long] <bodyfont-size> (14)

[string] <bodyfont> (andysans)

[string] <label> (Principal)

\enddata{text,539391000}
\enddata{cel,539390680}
\view{celview,539390680,9,94,23} }dollars has been approved at a rate 
of\upbase{ 
\begindata{cel,539395544}
\V 2
\begindata{value,539395688}
>0
\enddata{value,539395688}
10 539395688 1 0 0 0 
>OBJ< value
>VIEW< stringV
>REF< ShowInterest
>LINK< 
\begindata{text,539395864}
\textdsversion{12}
[string] <background-color> ()

[string] <foreground-color> ()

[long] <bodyfont-size> (14)

[string] <bodyfont> (andysans)

[string] <label> (Interest)

\enddata{text,539395864}
\enddata{cel,539395544}
\view{celview,539395544,10,72,25} }with repayment over a period of\upbase{ 
\begindata{cel,539397144}
\V 2
\begindata{value,539396632}
>0
\enddata{value,539396632}
10 539396632 1 0 0 0 
>OBJ< value
>VIEW< stringV
>REF< ShowMonths
>LINK< 
\begindata{text,539458680}
\textdsversion{12}
[string] <background-color> ()

[string] <foreground-color> ()

[long] <bodyfont-size> (14)

[string] <bodyfont> (andysans)

[string] <label> (Months)

\enddata{text,539458680}
\enddata{cel,539397144}
\view{celview,539397144,11,78,24} }months.   Each month you will make a 
payment of 
\upbase{ \
\begindata{cel,539459464}
\V 2
\begindata{value,539459608}
>0
\enddata{value,539459608}
10 539459608 1 0 0 0 
>OBJ< value
>VIEW< stringV
>REF< Payment
>LINK< 
\begindata{text,539459784}
\textdsversion{12}
[string] <background-color> ()

[string] <foreground-color> ()

[long] <bodyfont-size> (14)

[string] <bodyfont> (andysans)

[string] <label> (Payment)

\enddata{text,539459784}
\enddata{cel,539459464}
\view{celview,539459464,12,100,27} }and the total repaid to the bank over 
the lifetime of the loan will be\upbase{ 
\begindata{cel,539460968}
\V 2
\begindata{value,539397032}
>0
\enddata{value,539397032}
10 539397032 1 0 0 0 
>OBJ< value
>VIEW< stringV
>REF< Total
>LINK< t
\begindata{text,539461176}
\textdsversion{12}
[string] <background-color> ()

[string] <foreground-color> ()

[long] <bodyfont-size> (14)

[string] <bodyfont> (andysans)

[string] <label> (Total Paid)

\enddata{text,539461176}
\enddata{cel,539460968}
\view{celview,539460968,13,132,26}}.



With best wishes,


Friendly National Bunk\
}\enddata{text,539343688}
0 539343688 0 0 0 0 
>OBJ< text
>VIEW< textview
>REF< text
\enddata{cel,539343416}
0 0 0 539343416 0 0 0
>OBJ< cel
>VIEW< celview
>REF< 
\enddata{lset,539343208}
1 85 0 0 539340824 539343208 0
>OBJ< 
>VIEW< 
>REF< 
\enddata{lset,539340360}
\view{lsetview,539340360,14,479,370}
\leftindent{
}
}
\begindata{ness,539467688}
\origin{00\\8 Dec 1995 at 10:08:31 EST\\wjh:  Fred Hansen\\00}
\template{default}
\define{sans
menu:[Font,Sans]
attr:[FontFamily AndySans Int 0]}
--edit_dollars(v) convert to a string with two digits after decimal place

--

function edit_dollars(real v)

	marker a, c

	a := textimage(round(v * 100.0))

	c := extent(previous(last(a)), last(a))

	return extent(a, start(c)) ~ "." ~ c

end function


--ipow(i, n)

--	compute i\superscript{n}

--

real function ipow(real i, integer n)

	integer j

	real v

	j := 0

	v := 1.0

	while j < n do

		j := j + 1

		v := v * i

	end while

	return v

end function


--UpdateResults()

--	Store new value in results field

--

	void

function UpdateResults()

	integer principal, months

	real monthlyInterest, payment, totalpaid, z


	if mouseaction /= mouseleftup then \


		exit function

	end if


-- The periodic payment for

--	q periods per year

--	n years

--	i annual interest

--	P principal

-- is

--	payment = iPz/(z-1),     where  z = (1 + i/q)\superscript{nq}


	principal := value_GetValue(inset("Amount"))

	months := value_GetValue(inset("Months"))

	monthlyInterest := float(value_GetValue(inset("Interest"))) / 100.0 / 12.0

		-- divide by 100 to convert from a percentage,

		-- and then divide by 12 to get monthly interest.


	z := ipow(1.0 + monthlyInterest, months)      -- Compute  z  \


	payment := float(principal * 1000) * monthlyInterest * z / (z - 1.0) \



	totalpaid := payment * float(months)

	value_SetString(inset("Payment"), edit_dollars(payment))

	value_SetString(inset("Total"), edit_dollars(totalpaid))


	value_SetString(inset("ShowPrincipal"), textimage(principal * 1000))

	value_SetString(inset("ShowInterest"),

		textimage(value_GetValue(inset("Interest"))) ~ " %")

	value_SetString(inset("ShowMonths"), textimage(months))


end function



extend "Amount"

	on mouse "any"

		dohit (currentinset, mouseaction, mousex, mousey)

		UpdateResults()

	end mouse

end extend


extend "Interest"

	on mouse "any"

		dohit (currentinset, mouseaction, mousex, mousey)

		UpdateResults()

	end mouse

end extend


extend "Months"

	on mouse "any"

		dohit (currentinset, mouseaction, mousex, mousey)

		UpdateResults()

	end mouse

end extend\
\enddata{ness,539467688}
\view{nessview,539467688,15,0,486}


-- 
\begindata{bp,539491528}
Version 2
n 0
\enddata{bp,539491528}
\view{bpv,539491528,16,0,0}

-- Copyright 1992 Carnegie Mellon University and IBM.  All rights reserved.
\enddata{text,538506344}
