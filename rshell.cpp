#include <iostream>
#include <unistd.h>
#include <stdio.h>

using namespace std;

void parse()
{
	
}

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
		for(unsigned int i = 0; i < strlen(cstylestring); ++i)
		{
			if (cstylestring[i] == '#')
			{
				cstylestring[i] == '\0';
			}
			if (cstylestring[i] == '&')
			{
				
			}
		}
	}
	return 0;
}