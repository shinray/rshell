#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <vector>
#include <string>
#include <queue>

using namespace std;

//vector<int> connectorlist;
//int argCount = 0;

/*void dostuff(char **todolist, int sz)
{
	int count = 0; // I need to stop randomly using unsigned it's mroe trouble than its worth
	char **cmd;
	cmd = new char *[argCount+1];
	
	while (count != sz)
	{
		unsigned it = 0;
		cmd[it] = strtok(todolist[count], " ");
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
			if (execvp(cmd[0],cmd) != 0)
			{
				perror("error in execvp");
			}
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
}*/

int maxstrlength(vector<string> &v)
{
	int ret = 0;
	for (unsigned i = 0; i < v.size(); ++i)
	{
		int temp = v[i].size();
		if (temp > ret)
		{
			ret = temp;
		}
	}

	return ret;
}

void execute(vector<vector<string> > &v, queue<int> &q)
{
	int currconnector, childstatus, pid;
	for(unsigned i = 0; i < v.size(); ++i)
	{
		vector<char *> argv(v[i].size() + 1);
		for(unsigned j = 0; j < v[i].size(); ++j)
		{
			argv[j] = &v[i][j][0];
		}
		
		currconnector = q.front();
		pid = fork(); // syscall fork
		if (pid == -1)
		{
			perror("fork fail"); // perror fork
			exit(1);
		}
		else if (pid == 0) //child
		{
			if (execvp(v[i][0].c_str(), argv.data()) == -1) // syscall execvp
			{
				perror("execvp fail"); // perror execvp
				exit(1);
			}
		}
		else //parent
		{
			if (wait(&childstatus) == -1) // wait syscall
			{
				perror("wait for child fail"); // wait perror
				exit(1);
			}
			//cout << "childstatus: " << childstatus << '\n';
			if ((childstatus != 0 && currconnector == 1) || (childstatus == 0 && currconnector == 2))
			{
				return;
			}
		}
		q.pop();
	}
}

vector<vector<string> > parse(string &cmdstr, queue<int> &cmdq)
{
	/*const string endconnector = ";"; // 0
	const string andconnector = "&&"; // 1
	const string orconnector = "||"; // 2
	const string pipeconnector = "|"; // 3
	const string inconnector = "<"; // 4
	const string outconnector = ">"; // 5
	const string appconnector = ">>"; // 6

	if (cmdstr.find(endconnector)!= string::npos)
	{
		cmdq.push(0);
	}
	*/
	//bool alreadyWord = false;
	vector<string> vtoken;
	for (unsigned i = 0; i < cmdstr.size(); ++i)
	{
		if (cmdstr[i] == ';')
		{
			cmdq.push(0);
		}
		else if (cmdstr[i] == '|')
		{
			if (cmdstr[i+1] == '|') //or
			{
				i++;
				cmdq.push(2);
			}
			else //pipe
			{
				cmdq.push(3);
			}
		}
		else if ((cmdstr[i] == '&') && (cmdstr[i+1] == '&'))
		{
			i++;
			cmdq.push(1);
		}
		else if (cmdstr[i] == '<')
		{
			cmdq.push(4); // in
		}
		else if (cmdstr[i] == '>')
		{
			if (cmdstr[i+1] == '>')
			{
				i++;
				cmdq.push(6); //append
			}
			else
			{
				cmdq.push(5); //out
			}
		}
		/*else if (cmdstr[i] == ' ')
		{
			alreadyWord = false;
		}
		else if (cmdstr[i] != ' ' && !alreadyWord)
		{
			alreadyWord = true;
			argCount++;
		}*/
	}
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(";&|<>"); // connectors
	tokenizer tokens(cmdstr, sep);
	for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
	{
		vtoken.push_back(*tok_iter);
	}
	
	//boost::algorithm::split_regex(vtoken, str, regex("")); // must find a way to elegantly do this

	vector<vector<string> > vtoken2;
	vector<string> temp;
	boost::char_separator<char> wsep("\t\r\n\a "); // all manner of whitespace
	for (unsigned j = 0; j < vtoken.size(); ++j)
	{
		temp.clear();
		tokenizer wspace(vtoken[j], wsep);
		for (tokenizer::iterator w_iter = wspace.begin(); w_iter != wspace.end(); ++w_iter)
		{
			temp.push_back(*w_iter);
		}
		vtoken2.push_back(temp);
		//cout << j << ": " << vtoken[j] << '\n';
	}

	return vtoken2;
}

int main()
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
			input.erase(find(input.begin(), input.end(), '#'), input.end()); // strips comments
			queue<int> connectorq;
			vector<vector<string> > cmdv = parse(input, connectorq);
			execute(cmdv, connectorq);
			/*for (unsigned i = 0; i < cmdv.size(); ++i)
			{
				for(unsigned j = 0; j < cmdv[i].size(); ++j)
				{
					cout << i << j << ' ' << cmdv[i][j] << '\n';
				}
			}*/
			
			//char *cstylestring = new char[input.length()+1];
			//strcpy(cstylestring,input.c_str());
			//cstylestring = strtok(cstylestring, "#");
			//parse(cstylestring);
			// I need some kind of thingy here, like an array of arrays.
			/*char** commandQueue;
			commandQueue = new char* [input.length()+1];
			int size = 0;
			//commandQueue[size] = strtok(cstylestring, ";|&");
			while(commandQueue[size] != NULL)
			{
				++size;
				commandQueue[size] = strtok(NULL, ";|&");
			}
			dostuff(commandQueue, size);
			*/
		}
		//connectorlist.clear();
		//argCount = 0;
	}
	return 0;
}
