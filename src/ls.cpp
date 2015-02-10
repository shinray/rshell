#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <time.h>
#include <libgen.h>
#include <cstdio>
#include <unistd.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <iomanip>

using namespace std;

/*
 * This is a BARE BONES example of how to use opendir/readdir/closedir.
 * Notice that there is no error checking on these functions.
 * You MUST add error checking yourself.
 */

/* keeps track of arguments/flags
bool aflag = false, lflag = false, Rflag = false; */

/* searches argv for flags, returns a quick flagcount */
void checkargs(int argcc, char *argvv[], vector<char*> &vfiles, vector<char*> &vdirs, bool &aflag, bool &lflag, bool &Rflag)
{
	//int numflags = 0; // this actually should count the nonflags aka "files"
	for (int i = 1; i < argcc; i++)
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

/* void print(const char *dir, bool recursion, bool showhidden, bool first, stringstream &sp) // ls without -l
{
	DIR *dirp = opendir(dir); // opendir
	if (dirp == NULL)
	{
		perror("print: opendir"); // perror!
		exit(1);
	}
	dirent *direntp;
	while ((direntp = readdir(dirp)))
	{
		errno = 0; // ensure errors are a result of readdir
		string currfile(direntp->d_name);
		if (!showhidden) // -a flag
		{
			if (boost::starts_with(currfile, ".") || boost::starts_with(currfile, "..")) // ignore . and ..
			{
				continue; // skips this iteration
			}
		}
		bool isdir = (direntp->d_type & DT_DIR); // bitwise AND to check if it's a dir
		if (recursion)
		{
			if (first)
			{
				cout << dir << ":\n";
				cout << currfile;
				if (isdir)
				{
					cout << '/';
				}
				cout << "  ";
			}
			if (isdir)
			{
				// cout << dir << ":\n";
				string path = dir + '/' + currfile;
				print(path.c_str(), recursion, showhidden, false, sp);
				cout << path << ":\n";
				cout << sp.str();
				sp.clear();
				sp.str(string());
				cout << '\n' << endl;
			}
			else
			{
				sp << currfile;
				if (isdir)
				{
					sp << '/';
				}
				sp << "  ";
			}
		}
		else
		{
			cout << currfile;
			if (isdir)
			{
				cout << '/';
			}
			cout << "  ";
		}
	}
	if (errno != 0)
	{
		perror("error reading directory"); //perror for readdir
	}
	if (closedir(dirp) == -1) // closedir for every opendir
	{
		perror("closedir"); //always call perror!
		exit(1);
	}
	cout << endl;
} */

void print(const char *path, bool recursion, bool &showhidden) // ls without -l
{
    DIR *dirp = opendir(path); // opendir
    if(dirp == NULL) 
	{
        perror("print: opendir"); // perror!
		cerr << path << endl;
        exit(1);
    }
    struct dirent *direntp;
	string stringbuff;
	cout << "/" << path << ":\n";
    while ((direntp = readdir(dirp)))
	{
		errno = 0; // ensure errors are a result of readdir
		string currfile(direntp->d_name);
		if (showhidden || (!boost::starts_with(currfile, ".")))
		{
			cout << currfile << "  ";
		}
	}
	if (errno != 0)
	{
		perror("error reading directory"); //perror for readdir
	}
	cout << '\n' << '\n';
    if (closedir(dirp) == -1) // closedir for every opendir
	{
		perror("closedir"); //always call perror!
		exit(1);
	}
    dirp = opendir(path); // opendir
	if(dirp == NULL) 
	{
        perror("print: opendir"); // perror!
        exit(1);
    }
    while((direntp = readdir(dirp)))
	{
		errno = 0; // ensure errors are a result of readdir
		string currfile(direntp->d_name);
		if (strcmp(direntp->d_name, ".") == 0 || (strcmp(direntp->d_name, "..") == 0))
		{
			continue;
		}
		if (showhidden || (!boost::starts_with(currfile, ".")))
		{
			
			if(recursion && direntp->d_type == 4) // easier than S_ISDIR tbh
			{
				stringbuff = path + '/' + string(direntp->d_name); // can't go deeper than 1 dir???
				print(stringbuff.c_str(), true, showhidden);
			}
		}
	}
	if (errno != 0)
	{
		perror("error reading directory"); //perror for readdir
	}
    if (closedir(dirp) == -1) // closedir for every opendir
	{
		perror("closedir"); //always call perror!
		exit(1);
	}
}


void printlong(char *dir, bool recursion, bool showhidden, bool first) // ls with -l
{
	DIR *dirp = opendir(dir);
	if (dirp == NULL)
	{
		perror("printlong: opendir");
		exit(1);
	}
	dirent *direntp;
	struct stat statbuf;

	int blocktotal = 0;
	stringstream ss;
	while((direntp = readdir(dirp)))
	{
		errno = 0;
		if (stat(direntp->d_name, &statbuf) == -1) //syscall
		{
			perror("printlong: stat"); //perror
			exit(1);
		}
		blocktotal += statbuf.st_blocks / 2;
		if (!showhidden && (boost::starts_with(direntp->d_name, ".")))
		{
			continue;
		}
		// this is the permissions field
		if (S_ISLNK(statbuf.st_mode))
		{
			ss << 'l'; // link
		}
		else if (S_ISDIR(statbuf.st_mode))
		{
			ss << 'd'; // directory
		}
		else//else if (statbuf.st_mode & S_ISREG)
		{
			ss << '-'; // regular file
		}
		//user permissions rwx
		ss << ((statbuf.st_mode & S_IRUSR) ? 'r' : '-');
		ss << ((statbuf.st_mode & S_IWUSR) ? 'w' : '-');
		ss << ((statbuf.st_mode & S_IXUSR) ? 'x' : '-');
		//group permissions rwx
		ss << ((statbuf.st_mode & S_IRGRP) ? 'r' : '-');
		ss << ((statbuf.st_mode & S_IWGRP) ? 'w' : '-');
		ss << ((statbuf.st_mode & S_IXGRP) ? 'x' : '-');
		//other permissions rwx
		ss << ((statbuf.st_mode & S_IROTH) ? 'r' : '-');
		ss << ((statbuf.st_mode & S_IWOTH) ? 'w' : '-');
		ss << ((statbuf.st_mode & S_IXOTH) ? 'x' : '-');
		
		ss << ' ';
		// number of links
		ss << statbuf.st_nlink;
		
		ss << ' ';
		// name of file owner
		struct passwd *pw = getpwuid(statbuf.st_uid); //syscall
		if (pw == NULL)
		{
			perror("getpwuid"); // perror
			exit(1);
		}
		else
		{
			ss << pw->pw_name;
		}
		
		ss << ' ';
		// group of file (anyone who is in this group can do this)
		struct group *gp = getgrgid(statbuf.st_gid); // syscall
		if (gp == NULL)
		{
			perror("getgrgid"); // perror
			exit(1);
		}
		else
		{
			ss << gp->gr_name;
		}
		
		ss << setw(6) << right;
		// file size in bytes
		ss << statbuf.st_size;
		
		// date modified st_mtime
		string timey = ctime(&statbuf.st_mtime);
		timey.erase(timey.begin()+24);
		timey.erase(0,3);
		ss << ' ' << timey;
		
		ss << ' ';
		// name of the file
		ss << direntp->d_name;
		ss << endl;
	}
	cout << "total: " << blocktotal << '\n';
	cout << ss.str();
	if (errno != 0)
	{
		perror("error reading directory");
	}
	if (closedir(dirp) == -1)
	{
		perror("closedir");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	vector<char*> vfiles, vdirs;
	bool is_hidden = false, is_long = false, is_recursive = false;
	
	checkargs(argc, argv, vfiles, vdirs, is_hidden, is_long, is_recursive);

	char dirName[] = ".";
	//(is_long) ? printlong(dirName, is_recursive, is_hidden, true) : print(dirName, is_recursive, is_hidden, true);
	(is_long) ? printlong(dirName, is_recursive, is_hidden, true) : print(dirName, is_recursive, is_hidden);

	return 0;
}
