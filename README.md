# rshell v0.2
oh god how did this get here i am not good with computer
=================================
rshell Overview
=================================
This is a simple command shell written in c++.
So far it takes in user input and attempts to 
parse it as the appropriate system call. It
should be able to handle comments, 'AND' and
'OR' connectors, and multiple simultaneous 
commands ('#', '&&', '||', and ';').

HEY WHATS UP BOYS GONNA ADD ME SOME REDIRECTION (SOON<sup>tm</sup>)
* added signal catching
* switched from execvp to execv (searches using user's PATH ENV)
* implemented cd

--------------------
How to install
--------------------
1. git clone http://github.com/shinray/rshell.git
2. cd rshell
3. git checkout hw0
4. make
5. bin/rshell

--------------------
Prerequisites
--------------------
none yet I think

--------------------
How to use
--------------------
* `exit` to quit
* anything after `#` should be considered a comment
and ignored
* multiple commands can be input at once using:
	1. ls ; echo (will run ls followed by echo)
	2. ls && echo (will attempt to run ls; echo will run iff ls is successful)
	3. ls || echo (will attempt to run ls; echo will run iff ls fails)

--------------------
CURRENT BUGS:
--------------------
* i am an incompetent moron
* not finished yet
* SEND HELP
* NEW: added own ls function
* ls function does not yet have support for -R
* ls function currently only works with current directory (aka bin/ls)
* `&` is considered a connector and is regex'd out.
* piping '|' works, but certain combinations of piping and redirection fail to create new files when necessary
* redirecting < and > and >> to an empty file "" causes segfault
* ^Z is buggy, empty terminal
