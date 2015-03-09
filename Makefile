FLAGS = -W -Wall -ansi -pedantic -std=c++11 #-Werror

all: rshell ls | bin
#all: rshell ls cp | bin
rshell: | bin
	g++ $(FLAGS) src/rshell.cpp -o bin/rshell
ls: | bin
	g++ $(FLAGS) -o bin/ls src/ls.cpp
cp: | bin
	g++ $(FLAGS) -o bin/cp src/cp.cpp
bin:
	mkdir bin
clean:
	rm -rf bin
