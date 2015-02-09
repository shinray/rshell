#include <stdio.h>
#include <istream>
#include <ostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include "Timer.h"

#define BUFSIZE 512

void cp(char *input, char *output)
{
    std::vector<char> vect;

std::ifstream is(input);   //open file
    char c;
    while(is.get(c)){
    vect.push_back(c);
    }
    vect.push_back('\0');//ending char

    is.close();
 //now the copy
  char test[1024];
    strcpy(test,input);

    std::ofstream outfile(output);
    char ch;
    int i=0;
    do{
        ch = vect[i];
        if(ch!='\0')outfile.put(ch);
        i++;
    }while(ch!='\0');
}

void cpy1(char* src, char* destination) //FIXME src is const
{
    char buf[1];
    int n, fdsrc, fddest, outputflag;
    outputflag = (O_RDWR | O_CREAT); // new file is r/w, will be created if does not exist
    if ((fdsrc = open(src,O_RDONLY)) == -1) // opens file read-only
    {
        perror("open");
    }
    if ((fddest = open(destination, outputflag, S_IRWXU)) == -1) //user has RWX perms for new file
    {
        perror("open");
    }
    while ((n = read(fdsrc, buf, 1)) > 0)
    {
        if (n == -1)
        {
            perror("read");
        }
        if (write(fddest,buf,n) == -1)
        {
            perror("write");
        }
    }
}

void cpy2(char* src, char* destination) //FIXME src is const
{
    char buf[BUFSIZE];
    int n, fdsrc, fddest, outputflag;
    outputflag = (O_RDWR | O_CREAT); // new file is r/w, will be created if does not exist
    if ((fdsrc = open(src,O_RDONLY)) == -1) // opens file read-only
    {
        perror("open");
    }
    if ((fddest = open(destination, outputflag, S_IRWXU)) == -1) //user has RWX perms for new file
    {
        perror("open");
    }
    while ((n = read(fdsrc, buf, BUFSIZE)) > 0)
    {
        if (n == -1)
        {
            perror("read");
        }
        if (write(fddest,buf,n) == -1)
        {
            perror("write");
        }
    }
}
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Not enough arguments!\n";
        std::cerr << "Usage: cp [file1] [file2] [optional arg]" << std::endl;
        exit(1);
    }
    char* inputdir = argv[1];
    char* outputdir = argv[2];
    struct stat s;
    if ((stat(argv[1],&s)!=0) || S_ISDIR(s.st_mode))
    {
        std::cerr << "invalid arg1";
        perror("stat:arg1");
        exit(1);
    }
    if ((stat(argv[2],&s)!=0) || S_ISDIR(s.st_mode))
    {
        std::cerr << "invalid arg2";
        perror("stat:arg2");
        exit(1);
    }
	if (argc == 4)
    {
        Timer t1;
        double eTime1;
        double eWalltime1;
        double eSystime1;
        t1.start();
        cp(inputdir, outputdir);
        t1.elapsedUserTime(eTime1);
        t1.elapsedWallclockTime(eWalltime1);
        t1.elapsedSystemTime(eSystime1);
        std::cout << "copy 1\n" << "usertime " << eTime1 << " walltime " << eWalltime1 << " systemtime " << eSystime1 << std::endl;
       
        Timer t2;
        double eTime2;
        double eWalltime2;
        double eSystime2;
        t2.start();
        cpy1(inputdir, outputdir);
        t2.elapsedUserTime(eTime2);
        t2.elapsedWallclockTime(eWalltime2);
        t2.elapsedSystemTime(eSystime2);
        std::cout << "copy 2\n" << "usertime " << eTime2 << " walltime " << eWalltime2 << " systemtime " << eSystime2 << std::endl;
       
        Timer t3;
        double eTime3;
        double eWalltime3;
        double eSystime3;
        t3.start();
cpy2(inputdir, outputdir);
        t3.elapsedUserTime(eTime3);
        t3.elapsedWallclockTime(eWalltime3);
        t3.elapsedSystemTime(eSystime3);
        std::cout << "copy 3\n" << "usertime " << eTime3 << " walltime " << eWalltime3 << " systemtime " << eSystime3 << std::endl;
    }
    else
    {
        cpy2(inputdir,outputdir);
    }
    return 0;
}

