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

void merge(std::string* result,const std::string str)
{
    int resultLen = (*result).size();
    if(resultLen > 0)
    {
        // 49 - ascii value of 1
        // 57 - ascii value of 9
        int temp = resultLen-1;
        while((*result)[temp-1] > 49 && (*result)[temp] < 58)
            --temp;
        if((*result)[temp] == str[0])
        {
            std::stringstream localString;
            // Eg: result : x4y3 str: y2t45
            // result should be x4y5t45
            int a,b;
            a = (resultLen-temp>1)?atoi(((*result).substr(temp+1)).c_str()):(atoi(((*result).substr(temp)).c_str()));
            int temp1 = 1;
            while(str[temp1] > 49 && str[temp1] < 58)
                ++temp1;
            b = (temp1 > 0) ? atoi(str.substr(1, temp1).c_str()):atoi(str.substr(1,2).c_str());
            localString<<(*result).substr(0,temp+1)<<(a+b)<<str.substr(temp1+1, str.size());
            *result = localString.str();
        }
        else
            *result += str;
    }
    else
        *result = str;
}

int main(int argc, char* argv[])
{
    if(argc<3)
    {
        std::cout<<"Usage : "<<argv[0]<<" <n> <numThreads>"<<std::endl;
        exit(-1);
    }
    int n = atoi(argv[1]);
    int numThreads = atoi(argv[2]);
    omp_set_num_threads(numThreads);
    char* str = (char*)malloc(n* sizeof(char)); 
    generateRandomData(str, n);
    std::vector<std::string> zippedStrings;
    std::string result ="";
    std::chrono::time_point<std::chrono::system_clock> tstart = std::chrono::system_clock::now();
#pragma omp parallel
    {
#pragma omp single
        {
            numThreads = omp_get_num_threads();
            zippedStrings.resize(numThreads);
        }
        std::stringstream localResult;
        int j = 0;
        int k =0 ;
        int start = (n/numThreads) * omp_get_thread_num();
        int end = (n/numThreads) * (omp_get_thread_num() + 1);
        if( start > n )
            start = n;
        if( end > n)
            end = n;
        for(int x = start; x< end;++x)
        {
            int count = 1;
            while(str[x]==str[x+1]) //&& i < n/numThreads)
                ++count,++x;
            localResult<<str[x];
            localResult<<count;
        }
        zippedStrings[omp_get_thread_num()] = localResult.str();
#pragma omp barrier
#pragma omp single 
        {
            //Use merge function to merge the generated zipped strings 
            for (int x = 0; x < numThreads; ++x)
            {
                merge(&result, zippedStrings[x]);
            }
            //std::cout<<result<<std::endl;
        }
    }
    std::chrono::time_point<std::chrono::system_clock> tend = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = tend-tstart;
    std::cerr<<elapsed_seconds.count()<<std::endl;    
    free(str);
    return 0;
}
