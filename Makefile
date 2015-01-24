FLAGS = -W -Wall -Werror -ansi -pedantic

all: rshell

rshell: rshell.o
	mkdir bin || [-d bin
	g++ $(FLAGS) rshell.cpp -o bin/rshell

clean:
	rm -rf bin
