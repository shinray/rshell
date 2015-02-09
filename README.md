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
* 'exit' to quit
* anything after '#' should be considered a comment
and ignored
* multiple commands can be input at once using:
	1. ls ; echo (will run ls followed by echo)
	2. ls && echo (will attempt to run ls; echo will run iff ls is successful)
	3. ls || echo (will attempt to run ls; echo will run iff ls fails)

--------------------
CURRENT BUGS:
--------------------
*currently does not accept multiple commands or any kind of connectors (!!)
*can handle spaces but not tabs
*i am an incompetent moron
*not finished yet
*SEND HELP
*NEW: added own ls function
*ls function currently does not compile
*ls function does not yet have support for -R
