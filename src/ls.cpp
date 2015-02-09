#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <iostream>

using namespace std;

/*
 * This is a BARE BONES example of how to use opendir/readdir/closedir.
 * Notice that there is no error checking on these functions.
 * You MUST add error checking yourself.
 */

/* keeps track of arguments/flags */
bool aflag = false, lflag = false, Rflag = false;

/* searches argv for flags, returns a quick flagcount */
int flagcounter(int argcc, char **argvv)
{
	int numflags = 0; // this actually should count the nonflags aka "files"
	for (unsigned i = 1; i < argcc; i++)
	{
		if (argvv[i][0] == '-')
		{
			if (strchr(argv[i],'a') != NULL)
			{
				aflag = true;
				//numflags++;
			}	
			if (strchr(argv[i],'l') != NULL)
			{
				lflag = true;
				//numflags++;
			}	
			if (strchr(argv[i],'R') != NULL)
			{
				Rflag = true;
				//numflags++;
			}
			//break; //done searching?
		}
		else // it doesn't start with a '-' therefore not a flag therefore a file!
		{
			numflags++;
		}
	}

	return numflags;
}

int main(int argc, char **argv)
{
	int numflags = flagcounter(argc, argv);
	char *dirName = ".";
	if (numflags < argc - 1)
	{
		
	}
	DIR *dirp = opendir(dirName);
	dirent *direntp;
	while ((direntp == readdir(dirp)))
		cout << direntp->d_name << endl;	// use stat here to find attributes of file
	closedir(dirp);
}
