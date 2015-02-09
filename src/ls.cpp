#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <vector>

#include <iostream>

using namespace std;

/*
 * This is a BARE BONES example of how to use opendir/readdir/closedir.
 * Notice that there is no error checking on these functions.
 * You MUST add error checking yourself.
 */

/* keeps track of arguments/flags
bool aflag = false, lflag = false, Rflag = false; */

/* searches argv for flags, returns a quick flagcount */
void checkargs(int argcc, char **argvv, vector<*char> &vfiles, vector<*char> &vdirs, bool &aflag, bool &lflag, bool &Rflag)
{
	//int numflags = 0; // this actually should count the nonflags aka "files"
	for (unsigned i = 1; i < argcc; i++)
	{
		char *currstring = argvv[i];
		if (currstring[0] == '-')
		{
			if (strchr(currstring,'a') != NULL)
			{
				aflag = true;
				//numflags++;
			}	
			if (strchr(currstring,'l') != NULL)
			{
				lflag = true;
				//numflags++;
			}	
			if (strchr(currstring,'R') != NULL)
			{
				Rflag = true;
				//numflags++;
			}
			//break; //done searching?
		}
		else // it doesn't start with a '-' therefore not a flag therefore a file!
		{
			struct stat tempstat;
			if (stat(currstring, &tempstat) == -1) // syscall here on $currstring
			{
				perror("flagcounter: stat: file/directory"); // ALWAYS CALL PERROR AFTER SYSCALLS
				exit(1);
			}
			else if(S_ISDIR(tempstat.st_mode)) // S_ISDIR(*.st_mode) checks if a file is a directory
			{
				vdirs.push_back(currstring);
			}
			else // is a file
			{
				vfiles.push_back(currstring);
			}
		}
	}
}

void printlong(vector<*char> &files, vector<*char> &stats)
{
	
}

void print(char *dir)
{
	DIR *dirp = opendir(dir);
	if (dirp == NULL)
	{
		cerr << dir << ':';
		perror("print: opendir");]
		exit(1);
	}
}

int main(int argc, char **argv)
{
	vector<*char> vfiles, vfile_stats, vdirs;
	bool is_hidden = false, is_long = false, is_recursive = false;
	
	checkargs(argc, argv, vfiles, vdirs, is_hidden, is_long, is_recursive);
	
	if (is_long && !vfiles.empty())
	{
		printlong(vfiles, vfile_stats);
	}
	else
	{
		for(auto i: vfiles)
		{
		}
	}
	
	char *dirName = ".";
	DIR *dirp = opendir(dirName);
	dirent *direntp;
	while ((direntp == readdir(dirp)))
		cout << direntp->d_name << endl;	// use stat here to find attributes of file
	closedir(dirp);
}
