FLAGS = -g -W -Wall -Werror -ansi -pedantic

all: rshell ls | bin

rshell: | bin
	g++ $(FLAGS) src/rshell.cpp -o bin/rshell

ls:	| bin
	g++ $(FLAGS) src/ls.cpp -o bin/ls
	
bin:
	mkdir bin

clean:
	rm -rf bin
