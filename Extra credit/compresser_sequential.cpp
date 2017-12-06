#include<iostream>
#include<sstream>
#include <stdlib.h>
#include <omp.h>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

void generateRandomData(char *str, int n)
{
    static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"; 
     srand (time(NULL));
    //"0123456789" "!@#$%^&*"
    int stringLength = sizeof(alphanum) - 1;
    int j= 0, count = 0;
    char randChar;
    for (int i=0;i<n;)
    {
        count = (rand() % 9)+ 1;
        randChar = alphanum[rand() % stringLength];
        while(count--)
        {
            str[i++] = randChar;
        }
    }
}

int main(int argc, char* argv[])
{
    if(argc<2)
    {
        std::cout<<"Usage : "<<argv[0]<<" <n>"<<std::endl;
        exit(-1);
    }
    int n = atoi(argv[1]);
    char* str = (char*)malloc(n* sizeof(char)); 
    generateRandomData(str, n);
    std::vector<std::string> zippedStrings;
    std::stringstream localResult;
    std::chrono::time_point<std::chrono::system_clock> tstart = std::chrono::system_clock::now();
    for(int x = 0; x< n;++x)
    {
        int count = 1;
        while(str[x]==str[x+1]) 
            ++count,++x;
        localResult<<str[x];
        localResult<<count;
    }
    std::chrono::time_point<std::chrono::system_clock> tend = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = tend-tstart;
    std::cerr<<elapsed_seconds.count()<<std::endl;    
    //std::cout<<localResult.str()<<std::endl;
    free(str);
    return 0;
}
