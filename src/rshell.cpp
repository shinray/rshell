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
#include <stdlib.h>
#include <signal.h> // to catch signals
//#include <uinordered_map> //hash table for storing paused jobs
#include <limits.h> //needed for HOST_NAME_MAX

using namespace std;

int mainpid; // store the pid for the shell, if it's not this then we don't care
int pausedpid = -1;

void sighandler(int i) // FIXME: do something crazy
{
	int callerpid = getpid();
	if(i == SIGINT) // quit current job, not quit shell
	{
		if (callerpid != mainpid) // if child
		{
			if(kill(callerpid,SIGINT)==-1) // send interrupt (^C) to child
			{
				perror("kill(SIGINT)");
			}
		}
		// else //not child, do nothing
			// return;
	}
	if(i == SIGTSTP) // pause foreground process, return to shell
	{
		if (callerpid != mainpid) // if child
		{
			if (kill(callerpid,SIGTSTP) == -1) // pause child process
			{
				perror("kill(SIGTSTP)");
			}
			pausedpid = callerpid; //store pid
		}
		// else
			// return; // will also require fg and bg
	}
}

string mycwd() //like cwd but returns a string instead
{
	char buffer[BUFSIZ];
	string ret = getcwd(buffer, BUFSIZ);
	if (ret.empty())
	{
		perror("getcwd");
	}
	
	return ret;
}

void searchforpath(string cmdname, char **cmdWithArgs) // might as well make cmdname a string, since I'm converting it to a string here, then converting it BACK to a cstring. Do it all here
{
	if (cmdname.front() == '.') // currentdir
	{
		string newpath = mycwd();
		string updir = ".."; //updir
		if(cmdname.compare(0,updir.length(),updir) == 0)
		{
			while(newpath.back() != '/')
			{
				newpath.pop_back();
			}
			newpath += cmdname.substr(2);
			//cout << "newpath: " << newpath <<endl;
		}
		else
		{
			newpath += cmdname.substr(1);
		}
		if (execv(newpath.c_str(), cmdWithArgs) == -1)
		{
			perror("execv");
			return;
		}
	}
	else
	{
		vector<string> userpaths;
		string env = getenv("PATH"); // returns the user's PATH environment variable, separated by colons, terminated by '.'
		
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> colonsep(":"); // $PATH s are separated by colons
		tokenizer tokens(env, colonsep);
		for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
		{
			userpaths.push_back(*tok_iter);
		}
		
		//path is terminated in a '.' so I need to find and delete this thing
		
		if(userpaths.back().back() == '.')
		{
			userpaths.back().pop_back(); // requires c++11, will need to modify make
		}
		
		userpaths.push_back(mycwd()); //add current dir
		
		string currentPath;
		for (unsigned i = 0; i < userpaths.size(); ++i)
		{
			currentPath = userpaths.at(i) + '/' + cmdname;
			execv(currentPath.c_str(), cmdWithArgs); //most likely we're gonna get a bunch of fails so we only
		}
		perror("execv"); // perror if ALL fail
	}
}

int output_redir(vector<vector<string> > &v, int index)
{
	int ret = 1;
	unsigned tempindex = index;
	if (tempindex >= v.size()-1)
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
			searchforpath(v[index][0], argv.data());
			// if (execvp(v[index][0].c_str(), argv.data()) == -1)
			// {
				// perror("execvp");
				// exit(1);
			// }
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

int output_append(vector<vector<string> > &v, int index)
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
			searchforpath(v[index][0],argv.data());
			// if (execvp(v[index][0].c_str(),argv.data()) == -1)
			// {
				// perror("Execvp");
				// exit(1);
			// }
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

int input_redir(vector<vector<string> > &v, int index)
{
	int ret = 1;
	unsigned tempindex = index;
	if (tempindex >= v.size()-1)
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
			searchforpath(v[index][0], argv.data());
			// if (execvp(v[index][0].c_str(),argv.data()) == -1)
			// {
				// perror("Execvp");
				// exit(1);
			// }
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
				searchforpath(v[index][0],argv.data());
				// if (execvp(v[i][0].c_str(),argv.data()) == -1) //transform here into command
					// {
						// perror("Execvp");
						// exit(1);
					// }	
					// cerr << "this did not just happen" << endl;
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
		if (v[i][0] == "exit")
		{
			exit(0);
		}
		if (v[i][0] == "cd")
		{
			if (v[i].size() < 2) // no directory specified
			{
				if (chdir(getenv("HOME")) == -1) //setdir to home
				{
					perror("chdir(HOME)");
					return;
				}
				// cerr << "Error: no target directory specified." << endl;
			}
			else
			{
				if (v[i][1].front() == '/') //full path
				{
					if(chdir(v[i][1].c_str())== -1)
					{
						perror("chdir");
						return;
					}
				}
				else if (v[i][1].front() == '~') // homedir
				{
					string newpath = getenv("HOME") + v[i][1].substr(1);
					if (chdir(newpath.c_str()) == -1) //setdir to home
					{
						perror("chdir(HOME)");
						return;
					}
				}
				else if (v[i][1].front() == '.') // currentdir
				{
					string newpath = mycwd();
					string updir = "..";
					if(v[i][1].compare(0,updir.length(),updir) == 0)
					{
						while(newpath.back() != '/')
						{
							newpath.pop_back();
						}
						newpath += v[i][1].substr(2);
						//cout << "newpath: " << newpath <<endl;
					}
					else
					{
						newpath += v[i][1].substr(1);
					}
					
					if (chdir(newpath.c_str())== -1)
					{
						perror("chdir");
						return;
					}
				}
				else //relative path
				{
					string newpath = mycwd() + '/' + v[i][1];
					if(chdir(newpath.c_str())== -1)
					{
						perror("chdir");
						return;
					}
				}
				
			}
			continue;
		}
		if (v[i][0] == "fg")
		{
			if (pausedpid != -1)
			{
				if(kill(pausedpid,SIGCONT) == -1)//continue
				{
					perror("kill(SIGCONT)");
				}
			}
			if(waitpid(pausedpid,&childstatus,WNOHANG)==-1)
			{
				perror("waitpid");
			}
			continue;
		}
		if (v[i][0] == "bg")
		{
			if (pausedpid != -1)
			{
				if(kill(pausedpid,SIGCONT) == -1)//continue
				{
					perror("kill(SIGCONT)");
				}
			}
			continue;
		}
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
			int ioffset = input_redir(v,i);
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
			int ioffset = output_redir(v,i);
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
			int ioffset = output_append(v,i);
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
			// if (execvp(v[i][0].c_str(), argv.data()) == -1) // syscall execvp
			// {
				// perror("execvp fail"); // perror execvp
				// exit(1);
			// }
			searchforpath(v[i][0], argv.data());
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
	*/
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
	if(SIG_ERR == signal(SIGINT, sighandler)) //catch ^C
	{
		perror("signal(SIGINT)");
		exit(1);
	}
	if(signal(SIGTSTP, sighandler) == SIG_ERR) //catch ^Z
	{
		perror("signal(SIGTSTP)");
		exit(1);
	}
	
	char *username = getlogin();
	if (username == NULL)
	{
		perror("getlogin()");
		exit(1);
	}
	char machinename[HOST_NAME_MAX]; //internet said that unix hostnames are limited to 255
	if (gethostname(machinename, sizeof machinename) == -1)
	{
		perror("gethostname");
		exit(1);
	}
	string myhostname = machinename;
	if (myhostname.find('.') != string::npos)
	{
		myhostname.resize(myhostname.find('.'));
	}
	
	while (1)
	{
		mainpid = getpid(); //identify this process as our main, so we can't interrupt or pause it
		string input;
		cout << username << '@' << myhostname << ":"<< mycwd() << " $ ";
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
		}
	}
	return 0;
}
