# raiz_CGT

This was written on a linux system, but in theory should work on any system with a console and a Gnu or Clang compiler (I guess - I haven't tried it).  
to build:
`gcc transactions.c -o cgt_calc` or `clang transactions.c -o cgt_calc`
For Windows you might need `mingw` or similar (I haven't done anything on windows for a *long* time).

Once it's built run it:
`cgt_calc <filename>` where `filename` is the CSV file downloaded from RAIZ (go to `My Settings`->`Statements` and click `Send CSV of All Trades` (RHS).  This will email you a CSV file you can feed in.

`temp.csv` is included as a valid downloaded file that can be used as an example.

**NOTE:**  I haven't thoroughly debugged.  There is some variation that IMO is bigger than just rounding errors (although some of it is just differences in what the statement rounds compared to raiz).  The onus is on the user to double check any calculations and decide whether they're accurate.

**NOTE2:**  I can (over time) put some binaries on for specific OS's (if I can get my hands on the OS in question).

**NOTE3:** This calculates FIFO (First in first out).  It is not the only way to calculate CGT, but there's an implicit presumption that in the past you have used FIFO too (these results are even less valid if you haven't).  There's also some complexity around how gains and losses are accounted for before the CGT discount is calculated which might affect your personal situation if you have other CGT events outside the RAIZ sphere

IT is what it is - it comes with no guarantees - if the tax man comes looking for you, it is not my responsibility, because I never said this perfectly calculates CGT liability.

Feedback is more than welcome (however I have many hobbies and with something like this I'll get to it when I get to it).  You are more than welcome to copy, refactor, improve, or rewrite any part of this (in hindsight I think I could have made a more user-friendly thing in another language - but I like C and I did it for me).

Incidentally, I have considered porting this to a slightly easier to use language/format.  Feel free to let me know if that would be useful (might not have time to do it in time for this tax cycle).  Alternatively, feel free to taake some ofd the ideas an port it ot another language yourself (to me it feels like something that should be reasonably easy in Python).

Easy Reader Version:  This attempts to calculate CGT liability by assesiing your transactions FIFO.  It is probably a useful styarting point for tax preparation, but often your situation might be more complicated than this tool can account for.


