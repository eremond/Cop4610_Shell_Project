# proj_1

COP 4610 Project 1 : Making a shell

The members of this project are:
1) Emmanuel Gonzalez
2) John Sanchez
3) Redden Money

project1_gonzalez_sanchez_money.tar contents:
  README
  shell.c // main implementation
  makefile


Completed using: linprog

To Build:

$> make

To Clean:
$> make clean

Known Bugs & Unfinished Portions:
 • Piping works, but only one can be used at a time.
   -> Results from lack of implementation for when an additional "pipe-in" target is supplied.
 • Background processing is somewhat unfinished. (Zombie processes might occur)
 • "io" command is unfinished.
    ->Attempted to complete the built-in using flags and opening the proc/pid/io file.
    ->Attempt resulted in the fact that variables are not universal between processes, as well as possible path resolution issues.
 • Path resolution is simple.
    ->Ex: "cd ~" works, "cd ~/DIRNAME" gives an error.
    ->Resulting from the lack of splitting the special characters and environment variables from the rest of the line.
 • There needs to be whitespace between the redirection/pipe and the command.
 • "echo" built-in does not signal an error when there is a non-existent environment variable.
 • "$SHELL" environment variable p

Special Considerations:
 • In the "exit" built-in, clock_gettime() was used instead of gettimeofday(), which is not always available to certain (notably older)
  Linux versions, along with it being absent on Mac...
 • execvp()
==============================
==============================
Report
==============================
==============================



==============================
Problem Statement
==============================
In this project we are implementing a bash shell using c.  We are essentially
trying to get the output of this program to be as similar as possible to bash
with all the neat features like piping and the required built-ins.

==============================
Steps Taken To Solve Problem
==============================
1) Experimented with bash on the linprog server.
2) Took notes on output for required built ins, piping , I/O, etc.
3) Implemented User Input / Parsing to get commands/arguments
4) Implemented Built-Ins
5) Implemented Input Redirection
6) Implemented Environment Variables
7) Implemented piping
8) Implemented Background Processing
9) Ran tests to make sure all requirements were working and satisfied
10) Completed


==============================
Division of Labor
==============================

Gonzalez:
• Parsing
• Environment Variables
• Prompt
• Path Resolution
• Built In: io
• Built In: pwd

Sanchez:
• Execution
• Piping
• Built-In: cd
• Built-In: exit
• Error Handling

Money:
• Background Processing
• I/O Redirection
