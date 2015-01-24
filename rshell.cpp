#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <boost/tokenizer.hpp>
#include <vector>

using namespace std;

vector<int> connectorlist;
int argCount = 0;

void dostuff(char **todolist, int sz)
{
	unsigned count = 0;
	char **cmd;
	cmd = new char *[argcount+1];
	
	while (count != sz)
	{
		unsigned it = 0;
		cmd[iterator] = strtok(todolist[count], " ");
		while (cmd[it] != NULL)
		{
			it++;
			cmd[it] = strtok(NULL, " ");
		}
		int pid = fork();
		if (pid == -1)
		{
			perror("fork failed");
			exit(1);
		}
		else if (pid == 0) // child
		{
			if (execvp(currcmd[0],currcmd) != 0)
			{
				perror("error in execvp");
			}
		else // parent
		{
			if (wait(0) != 0)
			{
				perror("waiting for child process");
				exit(1);
			}
			count++;
		}
	}
}

void parse(char* cmdstr)
{
	bool alreadyWord = false;
	for (unsigned i = 0; i < strlen(cmdstr); ++i)
	{
		if (cmdstr[i] == ';')
		{
			connectorlist.push_back(3);
		}
		else if (cmdstr[i] == '|' && cmdstr[i+1] == '|')
		{
			i++;
			connectorlist.push_back(6);
		}
		else if (cmdstr[i] == '&' && cmdstr[i+1] == '&')
		{
			i++;
			connectorlist.push_back(9);
		}
		else if (cmdstr[i] == ' ')
		{
			alreadyWord = false;
		}
		else if (cmdstr[i] != ' ' && !alreadyWord)
		{
			alreadyWord = true;
			argCount++;
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
		if (input == "exit")
		{
			exit(0);
		}
		if (!input.empty())
		{
			char *cstylestring = new char[input.length()+1];
			strcpy(cstylestring,input.c_str());
			cstylestring = strtok(cstylestring, "#");
			parse(cstylestring);
			// I need some kind of thingy here, like an array of arrays.
			char** commandQueue;
			commandQueue = new char* [input.length()+1];
			int size = 0;
			commandQueue[size] = strtok(cstylestring, ";|&");
			while(commandQueue[size] != NULL)
			{
				++size;
				commandQueue[size] = strtok(NULL, ";|&");
			}
			dostuff(commandQueue, size);
		}
		connectorlist.clear();
		argCount = 0;
	}
	return 0;
}
