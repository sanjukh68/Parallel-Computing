#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <chrono>
#include<math.h>
#include<algorithm>
#include <cstdlib>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

	void generateReduceData(int* arr, size_t n);

#ifdef __cplusplus
}
#endif

static const char alphanum[] = 
"0123456789"
"!@#$%^&*"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

int stringLength = sizeof(alphanum) - 1;

char genRandom()
{
    return alphanum[rand() % stringLength];
}

int LCS_length(string str1, string str2, int **mat, int nbthreads)
{
    int i, j, k;
    int len1 = str1.size();
    int len2 = str2.size();

	omp_set_num_threads(nbthreads);
    /* Zero out first column & row */
    for( i = 0; i <= len1; i++ )
        mat[i][0] = 0;
    for( j = 1; j <= len2; j++ )
        mat[0][j] = 0;

#pragma omp parallel default(none) shared(str1, str2, mat, len1, len2) private(i,j,k)

    for(k = 2; k <= len1+len2; k++){
		#pragma omp for schedule(runtime)
        for(i =  min(len1, k-1); i >= max(1, k-len2); i--)
        {
            j = k - i;

            if(str1[i-1] == str2[j-1]) 
			{
                mat[i][j] = mat[i-1][j-1] + 1;
            }
            else 
			{
                mat[i][j] = max( mat[i-1][j], mat[i][j-1] );
            }
        }
    }
    return mat[len1][len2];
}

int main(int argc, char* argv[]) {

	if (argc < 5) {
		std::cerr << "Usage: " << argv[0] << " <n> <nbthreads> <scheduling> <granularity>" << std::endl;
		return -1;
	}

	int nbthreads, n, granularity;
	string scheduling;
	
	sscanf(argv[1], "%d", &n);
	sscanf(argv[2], "%d", &nbthreads);
	scheduling = argv[3];
	sscanf(argv[4], "%d", &granularity);
	//making m as n/2, just for simplicity in sending the arguments
	int m = n/2;
	omp_set_num_threads(nbthreads);
	//forces openmp to create the threads beforehand
#pragma omp parallel
	{
		int fd = open(argv[0], O_RDONLY);
		if (fd != -1) {
			close(fd);
		}
		else {
			std::cerr << "something is amiss" << std::endl;
		}
	}
	
	string str1, str2;
	for(unsigned int i = 0; i < m; ++i)
    {
		str1 += genRandom();
    }
	for(unsigned int i = 0; i < n; ++i)
    {
		str2 += genRandom();
    }
	
	int **mat;
    int i;
	int len1 = str1.size();
    int len2 = str2.size();
	
   mat = new int *[len1+1];
    for( i = 0; i < len1+1; i++)
        mat[i] = new int[len2+1];

	int length = 0;
	if (scheduling == "static")
		omp_set_schedule(omp_sched_static, granularity);
	else if (scheduling == "dynamic")
		omp_set_schedule(omp_sched_dynamic, granularity);
	else if (scheduling == "guided")
		omp_set_schedule(omp_sched_guided, granularity);

	chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
	
	int length1 = LCS_length(str1,str2, mat, nbthreads);

	chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
	chrono::duration<double> elapsed_seconds = end - start;
	cerr << elapsed_seconds.count() << endl;

	cout <<length1<<endl;

	for (int i = 0; i < len1+1; i++) {
		delete [] mat[i];
	}
	delete [] mat;
	
	return 0;
}
