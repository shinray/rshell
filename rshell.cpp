#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <boost/tokenizer.hpp>

using namespace std;

void dostuff(char **argv)
{
	char *cmd =  argv[1];
	int pid = fork();
	if (pid == -1)
	{
		perror("fork failed");
		exit(1);
	}
	else if (pid == 0) // child
	{
		if (execvp(cmd,argv) != 0)
			perror("error in execvp");
	}
	else // parent
	{
		if (wait(0) != 0)
		{
			perror("waiting for child process");
			exit(1);
		}
	}
}

int main(int argc, char **argv)
{
	while (1)
	{
		string input;
		cout << "$ ";
		getline(cin,input);
		char *cstylestring = new char[input.length()+1];
		strcpy(cstylestring,input.c_str());
		unsigned int andcount = 0;
		unsigned int orcount = 0;
		for(unsigned int i = 0; i < strlen(cstylestring); ++i)
		{
			if (cstylestring[i] == '#')
			{
				cstylestring[i] == '\0';
			}
			if (cstylestring[i] == '&')
			{
				if (cstylestring[i+1] == '&')
				{
					++andcount;
				}
			}
			if (cstylestring[i] == '|')
			{
				if (cstylestring[i+1] == '|')
				{
					++orcount;
				}
			}
		}
		char deliminators[] = ";&&||";
		char* tokens = strtok_r(cstylestring, deliminators,);
	}
	return 0;
}
