# raiz_CGT

This was written on a linux system, but in theory should work on any system with a console and a Gnu or Clang compiler (I guess - I haven't tried it).  
to build:
`gcc transactions.c -o cgt_calc` or `clang transactions.c -o cgt_calc`
For Windows you might need `mingw` or similar (I haven't done anything on windows for a *long* time).

Once it's built run it:
`cgt_calc <filename>` where `filename` is the CSV file downloaded from RAIZ (go to `My Settings`->`Statements` and click `Send CSV of All Trades` (RHS).  This will email you a CSV file you can feed in.

**NOTE:**  I haven't thoroughly debugged.  There is some variation that IMO is bigger than just rounding errors (although some of it is just differences in what the statement rounds compared to raiz).  The onus is on the user to double check any calculations and decide whether they're accurate.

**NOTE2:**  I can (over time) put some binaries on for specific OS's (if I can get my hands on the OS in question).

IT is what it is - it comes with no guarantees - if the tax man comes looking for you, it is not my responsibility, because I never said this perfectly calculates CGT liability.

Feedback is more than welcome (however I have many hobbies and with something like this I'll get to it when I get to it).  You are more than welcome to copy, refactor, improve, or rewrite any part of this (in hindsight I think I could have made a more user-friendly thing in another language - but I like C and I did it for me).

