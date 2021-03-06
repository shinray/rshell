#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
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

/*int maxstrlength(vector<string> &v)
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
}*/

int output_redir(vector<vector<string> > &v, vector<int> &q, int index)
{
	int ret = 1;
	unsigned tempindex = index;
	if (tempindex >= v.size())
	{
		cerr << "syntax error: no target file" << endl;
		return -1;
	}
	else
	{
		string destfile = v[index+1][0];
		//ret++;

		// converts vector<string> to char**
		vector<char *> argv(v[index].size() + 1);
		for(unsigned j = 0; j < v[index].size(); ++j)
		{
			argv[j] = &v[index][j][0];
		}

		int backup_stdout = dup(1); // backup stdout
		if (backup_stdout == -1)
		{
			perror("dup");
			exit(1);
		}
		int fddestflags = (O_CREAT | O_WRONLY | O_TRUNC);
		int modeflags = (S_IRWXU | S_IRWXG | S_IRWXO);
		int fddest = open(destfile.c_str(), fddestflags, modeflags);
		if (fddest == -1)
		{
			perror("open");
			return ret; // do not increment ret
		}
		if (dup2(fddest, 1) == -1)
		{
			perror("dup2");
			exit(1);
		}
		// do normal child fork stuff
		int childstatus;
		int pid = fork();
		
		if (pid == -1)
		{
			perror("fork");
			exit(1);
		}
		else if (pid == 0) // child
		{
			if (execvp(v[index][0].c_str(), argv.data()) == -1)
			{
				perror("execvp");
				exit(1);
			}
		}
		else //parent
		{
			if (wait(&childstatus) == -1)
			{
				perror("wait");
				exit(1);
			}
		}
		// restore stdout
		if (dup2(backup_stdout, 1)== -1)
		{
			perror("dup2_restore");
			exit(1);
		}
		if(close(fddest)==-1) //always close your filedescriptors once you're done!! fixme:cp.cpp
		{
			perror("close");
			exit(1);
		}
	}

	return ret+1;
}

int output_append(vector<vector<string> > &v, vector<int> &q, int index)
{
	int ret = 1;
	unsigned tempindex = index;
	if (tempindex >= v.size())
	{
		cerr << "syntax error: no target file" << endl;
		return -1;
	}
	else
	{
		string destfile = v[index+1][0];

		vector<char *> argv(v[index].size()+1);
		for (unsigned j = 0; j < v[index].size(); ++j)
		{
			argv[j] = &v[index][j][0];
		}
		
		int backup_stdout = dup(1);
		if (backup_stdout == -1)
		{
			perror("dup");
			exit(1);
		}
		
		int fddestflags = (O_CREAT | O_WRONLY | O_APPEND);
		int modeflags = (S_IRWXU | S_IRWXG | S_IRWXO);
		int fddest = open(destfile.c_str(), fddestflags, modeflags);
		if (fddest == -1)
		{
			perror("open");
			return ret;
		}

		if (dup2(fddest, 1) == -1)
		{
			perror("dup2");
			exit(1);
		}

		// normal fork stuff ( i really should have made this a func)
		int childstatus;
		int pid = fork();
		if (pid == -1)
		{
			perror("fork");
			exit(1);
		}
		else if (pid == 0)
		{
			if (execvp(v[index][0].c_str(),argv.data()) == -1)
			{
				perror("Execvp");
				exit(1);
			}
		}
		else
		{
			if (wait(&childstatus) == -1)
			{
				perror("wait");
				exit(1);
			}
		}

		if (dup2(backup_stdout, 1)==-1)
		{
			perror("dup2_restore");
			exit(1);
		}
		if (close(fddest) == -1)
		{
			perror("close");
			exit(1);
		}
	}

	return ret+1;
}

int input_redir(vector<vector<string> > &v, vector<int> &q, int index)
{
	int ret = 1;
	unsigned tempindex = index;
	if (tempindex >= v.size())
	{
		cerr << "syntax error: no target file" << endl;
		return -1;
	}
	else
	{
		string srcfile = v[index+1][0];

		vector<char *> argv(v[index].size()+1);
		for (unsigned j = 0; j < v[index].size(); ++j)
		{
			argv[j] = &v[index][j][0];
		}
		
		int backup_stdin = dup(0);//save stdin
		if (backup_stdin == -1)
		{
			perror("dup");
			exit(1);
		}
		
		int fdsrcflags = (O_RDONLY);
		//int modeflags = (S_IRWXU | S_IRWXG | S_IRWXO);
		int fdsrc = open(srcfile.c_str(), fdsrcflags);
		if (fdsrc == -1)
		{
			perror("open");
			return ret;
		}

		if (dup2(fdsrc, 0) == -1)//replace stdin with file input
		{
			perror("dup2");
			exit(1);
		}

		// normal fork stuff ( i really should have made this a func)
		int childstatus;
		int pid = fork();
		if (pid == -1)
		{
			perror("fork");
			exit(1);
		}
		else if (pid == 0)
		{
			if (execvp(v[index][0].c_str(),argv.data()) == -1)
			{
				perror("Execvp");
				exit(1);
			}
		}
		else
		{
			if (wait(&childstatus) == -1)
			{
				perror("wait");
				exit(1);
			}
		}

		if (dup2(backup_stdin, 0)==-1) // restore stdin
		{
			perror("dup2_restore");
			exit(1);
		}
		if (close(fdsrc) == -1)
		{
			perror("close");
			exit(1);
		}
	}

	return ret+1;
}

int pipe_s(vector<vector<string> > &v, vector<int> &q, int index)
{
	int ret = 0;
	int tempindex = index;
	int pipecount = 0; //counts consecutive pipes (how deep does the rabbit hole go?)
	if (tempindex >= (int) v.size())
	{
		cerr << "syntax error: no target program" << endl; // FIXME: do something cooler than just exiting
		//return -1;
	}
	else
	{
		//string srcfile = v[index+1][0];
		while (q[tempindex] == 3)
		{
			pipecount++;
			++tempindex;
		}
		// if (pipecount == -)
		// {
			// tempindex++;
		// }

		// int backup_stdin = dup(0);//save stdin
		// if (backup_stdin == -1)
		// {
			// perror("dup");
			// exit(1);
		// }
		int backup_stdout = dup(1); //save stdout
		if (backup_stdout == -1)
		{
			perror("dup");
			exit(1);
		}

		int oldpipe[2];
		int newpipe[2];
		for (int i = index; i <= tempindex; ++i)
		{
			//cout << "test" << endl;
			if (i != tempindex) // if there is a next cmd
			{
				if(pipe(newpipe) == -1) // create new pipe
				{
					perror("pipe");
					exit(1);
				}
			}
			vector<char *> argv(v[i].size()+1); // this converts arguments into char**
			for (unsigned j = 0; j < v[i].size(); ++j)
			{
				argv[j] = &v[i][j][0];
			}
	
			//normal fork stuff ( i really should have made this a func)
			//int childstatus;
			int pid = fork();
			if (pid == -1)
			{
				perror("fork");
				exit(1);
			}
			else if (pid == 0) // child
			{
				if (i != index) // if not first command
				{
					if (dup2(oldpipe[0],0)==-1)
					{
						perror("dup2_input");
						exit(1);
					}
					if (close(oldpipe[0])==-1)
					{
						perror("closeold0test");
						exit(1);
					}
					if (close(oldpipe[1])==-1)
					{
						perror("closeold1");
						exit(1);
					}
				}
				if (i != tempindex) // if there is a next
				{
					if (dup2(newpipe[1],1)==-1)
					{
						perror("dup2_input");
						exit(1);
					}
					if (close(newpipe[0])==-1)
					{
						perror("closenew0");
						exit(1);
					}
					if (close(newpipe[1])==-1)
					{
						perror("closenew1");
						exit(1);
					}
				}
				if (i == tempindex)
				{
					if(dup2(backup_stdout,1)==-1)//restore stdout
					{
						perror("dup2_restore1");
						exit(1);
					}
				}
				if (execvp(v[i][0].c_str(),argv.data()) == -1) //transform here into command
					{
						perror("Execvp");
						exit(1);
					}	
					cerr << "this did not just happen" << endl;
			}
			else //parent
			{
				if (i != index) // if there is previous
				{
					if((close(oldpipe[0]))==-1)
					{
						perror("closeold0");
						exit(1);
					}
					if((close(oldpipe[1]))==-1)
					{
						perror("closeold1");
						exit(1);
					}
				}
				if( i < tempindex)
				{
					oldpipe[0] = newpipe[0];
					oldpipe[1] = newpipe[1];
				}
			}
			
		}
		for (int j = index; j <= tempindex; ++j)
		{
			int childstatus;
			if (wait(&childstatus) == -1)
			{
				perror("wait");
				exit(1);
			}
		}
		// if (pipecount>1)
		// {
			// if((close(oldpipe[0]))==-1)
			// {
				// perror("closeold0");
				// exit(1);
			// }
			// if((close(oldpipe[1]))==-1)
			// {
				// perror("closeold1");
				// exit(1);
			// }
		// }
		// if(dup2(backup_stdout,1)==-1)//restore stdout
		// {
			// perror("dup2_restore1");
			// exit(1);
		// }
		// if(dup2(backup_stdin,0)==-1) // restore stdin
		// {
			// perror("dup2_restore0");
			// exit(1);
		// }

		// vector<char *> argv2(v[tempindex].size()+1);
		// for (unsigned k = 0; k < v[tempindex].size(); ++k)
		// {
			// argv2[k] = &v[tempindex][k][0];
		// }

		// if (execvp(v[tempindex][0].c_str(), argv2.data()) == -1)
		// {
			// perror("execvp");
			// exit(1);
		// }
	}

	return ret+pipecount;
}
void execute(vector<vector<string> > &v, vector<int> &q)
{
	int currconnector, childstatus, pid;

	for(unsigned i = 0; i < v.size(); ++i)
	{	
		/*int stdinpipe[2];
		int stdoutpipe[2];
		int stderrpipe[2];
		if (pipe(stdinpipe) == -1)
		{
			perror("pipe0 fail");
			exit(1);
		}
		if (pipe(stdoutpipe) == -1)
		{
			perror("pipe1 fail");
			exit(1);
		}
		if (pipe(stderrpipe) == -1)
		{
			perror("pipe2 fail");
			exit(1);
		}*/

		if (i < q.size()) // if there are no more connectors
		{
			currconnector = q.at(i);
		}
		else
		{
			currconnector = 0;
		}

		if (currconnector == 3) // piping (may be more than 1!)
		{
			int ioffset = pipe_s(v,q,i);
			if (ioffset == -1)
			{
				return; // error
			}
			else
			{
				i+= ioffset;
				continue; // done with this loop
			}
		}
		if (currconnector == 4) // inputredir
		{
			int ioffset = input_redir(v,q,i);
			if (ioffset == -1)
			{
				return; //error
			}
			else
			{
				i+= ioffset;
				continue; // this iteration is complete
			}
		}
		if (currconnector == 5) // outputredir
		{
			int ioffset = output_redir(v,q,i);
			if (ioffset == -1)
			{
				return; // file does not exist
			}
			else
			{
				i += ioffset;
				continue; // this iteration is done
			}
		}
		if (currconnector == 6) // output append
		{
			int ioffset = output_append(v,q,i);
			if (ioffset == -1)
			{
				return; //error
			}
			else
			{
				i += ioffset;
				continue; // this iteration is done
			}
		}
		// converts vector<string> to char**
		vector<char *> argv(v[i].size() + 1);
		for(unsigned j = 0; j < v[i].size(); ++j)
		{
			argv[j] = &v[i][j][0];
		}

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
			// if child failed and connector is &&		// if child passed and connector is ||
			if ((childstatus != 0 && currconnector == 1) || (childstatus == 0 && currconnector == 2))
			{
				return; // we are done, no further commands necessary
			}
		}
		//q.pop();
	}
}

vector<vector<string> > parse(string &cmdstr, vector<int> &cmdq)
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
			cmdq.push_back(0);
		}
		else if (cmdstr[i] == '|')
		{
			if (cmdstr[i+1] == '|') //or
			{
				i++;
				cmdq.push_back(2);
			}
			else //pipe
			{
				cmdq.push_back(3);
			}
		}
		else if ((cmdstr[i] == '&') && (cmdstr[i+1] == '&'))
		{
			i++;
			cmdq.push_back(1);
		}
		else if (cmdstr[i] == '<')
		{
			cmdq.push_back(4); // in
		}
		else if (cmdstr[i] == '>')
		{
			if (cmdstr[i+1] == '>')
			{
				i++;
				cmdq.push_back(6); //append
			}
			else
			{
				cmdq.push_back(5); //out
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
			vector<int> connectorq;
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
