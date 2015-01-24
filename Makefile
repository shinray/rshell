FLAGS = -W -Wall -Werror -ansi -pedantic

all: rshell

rshell: rshell.o
	g++ $(FLAGS) src/rshell.cpp -o bin/rshell

clean:
	rm -rf bin
