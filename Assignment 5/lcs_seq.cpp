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

int LCS_length(string x, string y, int **c)
{
    int rowIt, colIt, digIt;
    int len_X = x.size();
    int len_Y = y.size();

    /* Zero out first column & row */
    for( rowIt = 0; rowIt <= len_X; rowIt++ )
        c[rowIt][0] = 0;
    for( colIt = 1; colIt <= len_Y; colIt++ )
        c[0][colIt] = 0;

    for(digIt = 2; digIt <= len_X+len_Y; digIt++){
        for(rowIt =  min(len_X, digIt-1); rowIt >= max(1, digIt-len_Y); rowIt--)
        {
            colIt = digIt - rowIt;

            if(x[rowIt-1] == y[colIt-1]) {
                c[rowIt][colIt] = c[rowIt-1][colIt-1] + 1;
            }
            else {
                c[rowIt][colIt] = max( c[rowIt-1][colIt  ], c[rowIt  ][colIt-1] );
            }
        }
    }
    return c[len_X][len_Y];
}

int main(int argc, char* argv[]) {

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <n>" << std::endl;
		return -1;
	}

	int m, n;
	sscanf(argv[1], "%d", &n);
	//making m as n/2, just for simplicity in sending the arguments
	m = n/2;
	
	//omp_set_num_threads(nbthreads);
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

	string X, Y;
	for(unsigned int i = 0; i < m; ++i)
    {
		X += genRandom();
    }
	for(unsigned int i = 0; i < n; ++i)
    {
		Y += genRandom();
    }
	
	int **c;
    int rowIt;
	int len_X = X.size();
    int len_Y = Y.size();
	
   c = new int *[len_X+1];
    for( rowIt = 0; rowIt < len_X+1; rowIt++)
        c[rowIt] = new int[len_Y+1];

	chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();

	int length1 = LCS_length(X,Y, c);

	chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
	chrono::duration<double> elapsed_seconds = end - start;
	cerr << elapsed_seconds.count() << endl;

	cout <<length1<<endl;
	
	for (int i = 0; i < len_X+1; i++) {
		delete [] c[i];
	}
	delete [] c;

	return 0;
}
