FLAGS = -W -Wall -Werror -ansi -pedantic

all: rshell

rshell: rshell.cpp
	([! -d bin ] && mkdir bin) || [-d bin]
	g++ $(FLAGS) rshell.cpp -o bin/rshell

clean:
	rm bin/*.o bin/rshell.out
