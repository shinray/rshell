#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
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

void print(char *dir, bool recursion, bool showhidden, bool first) // ls without -l
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
		/* if (recursion)
		{
			cout << '.';
			if (!first)
			{
				cout << dir;
			}
			cout << ":\n";
			if (isdir)
			{
				string path = dir + "/" + currfile;
				print(path.c_str(), recursion, showhidden, false);
				cout << '\n' << endl;
			}
		} */
		cout << currfile;
		if (isdir)
		{
			cout << '/';
		}
		cout << '\t';
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
	while((direntp = readdir(dirp)))
	{
		errno = 0;
		if (stat(direntp->d_name, &statbuf) == -1) //syscall
		{
			perror("printlong: stat"); //perror
			exit(1);
		}
		// this is the permissions field
		if (S_ISLNK(statbuf.st_mode))
		{
			cout << 'l'; // link
		}
		else if (S_ISDIR(statbuf.st_mode))
		{
			cout << 'd'; // directory
		}
		else//else if (statbuf.st_mode & S_ISREG)
		{
			cout << '-'; // regular file
		}
		//user permissions rwx
		cout << ((statbuf.st_mode & S_IRUSR) ? 'r' : '-');
		cout << ((statbuf.st_mode & S_IWUSR) ? 'w' : '-');
		cout << ((statbuf.st_mode & S_IXUSR) ? 'x' : '-');
		//group permissions rwx
		cout << ((statbuf.st_mode & S_IRGRP) ? 'r' : '-');
		cout << ((statbuf.st_mode & S_IWGRP) ? 'w' : '-');
		cout << ((statbuf.st_mode & S_IXGRP) ? 'x' : '-');
		//other permissions rwx
		cout << ((statbuf.st_mode & S_IROTH) ? 'r' : '-');
		cout << ((statbuf.st_mode & S_IWOTH) ? 'w' : '-');
		cout << ((statbuf.st_mode & S_IXOTH) ? 'x' : '-');
		
		cout << ' ';
		// number of links
		cout << statbuf.st_nlink;
		
		cout << ' ';
		// name of file owner
		struct passwd *pw = getpwuid(statbuf.st_uid); //syscall
		if (pw == NULL)
		{
			perror("getpwuid"); // perror
			exit(1);
		}
		else
		{
			cout << pw->pw_name;
		}
		
		cout << ' ';
		// group of file (anyone who is in this group can do this)
		struct group *gp = getgrgid(statbuf.st_gid); // syscall
		if (gp == NULL)
		{
			perror("getgrgid"); // perror
			exit(1);
		}
		else
		{
			cout << gp->gr_name;
		}
		
		cout << setw(7) << right;
		// file size in bytes
		cout << statbuf.st_size;
		
		//cout << ' ';
		// date modified st_mtime
		cout << ' ' << ctime(&statbuf.st_mtime);
		
		cout << ' ';
		// name of the file
		cout << direntp->d_name;
		cout << endl;
	}
	if (errno != 0)
	{
		perror("error reading directory");
	}
	if (closedir(dirp) == -1)
	{
		perror("closedir");
		exit(1);
	}
	
	/* int count = scandir();
	if (count == -1)
	{
		perror("printlong: scandir");
	}
	else
	{
		if (count > 0)
		{
			cout << "total: " << count << '\n';
			for (unsigned i = 0; i < count; i++)
			{
				if (stat(files[i]->d_name, &statbuf) == 0)
			}
		}
	} */
}

int main(int argc, char *argv[])
{
	vector<char*> vfiles, vfile_stats, vdirs;
	bool is_hidden = false, is_long = false, is_recursive = false;
	
	checkargs(argc, argv, vfiles, vdirs, is_hidden, is_long, is_recursive);
	
	char dirName[] = ".";
	(is_long) ? printlong(dirName, is_recursive, is_hidden, true) : print(dirName, is_recursive, is_hidden, true);
	
	/* char *dirName = ".";
	DIR *dirp = opendir(dirName);
	dirent *direntp;
	while ((direntp == readdir(dirp)))
		cout << direntp->d_name << endl;	// use stat here to find attributes of file
	closedir(dirp); */
	return 0;
}
