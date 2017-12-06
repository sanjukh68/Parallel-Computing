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

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

	void generateReduceData(int* arr, size_t n);

#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[]) {

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <n>" << std::endl;
		return -1;
	}

	int nbthreads, n, granularity;
	string scheduling;
	
	sscanf(argv[1], "%d", &n);
	
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

	int * arr = new int[atoi(argv[1])];

	generateReduceData(arr, atoi(argv[1]));
	chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();

	int sum = 0;

	for (int i=0; i<n;i++)
	{
		sum += arr[i];
	}
	
	chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
	chrono::duration<double> elapsed_seconds = end - start;
	cerr << elapsed_seconds.count() << endl;

	cout << sum;

	delete[] arr;

	return 0;
}
