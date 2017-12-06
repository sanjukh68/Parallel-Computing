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

int SumArraySer(int *a, int n)
{
	int sum =0;
	#pragma omp parallel for schedule(runtime) reduction(+:sum)
    for (int i= 0; i<n;i++)
		sum += a[i];
	return sum;	
}

int SumArrayPar(int* arr, int n, int threads)
{
	if (n == 0)
		return 0;
	if (n==1)
		return arr[0];
	int x = 0;
	int y = 0;
	if (threads == 1)
		return SumArraySer(arr, n);
	else if (threads > 1)
	{
		#pragma omp parallel
		{
			#pragma omp single nowait
			{
				
				#pragma omp task shared(x)
				x += SumArrayPar(arr, n/2, threads/2);
				
				#pragma omp task shared(y)
				y += SumArrayPar(arr+(n/2), n-(n/2), threads - (threads/2));
			}
		}
		#pragma omp taskwait
		x += y;
	}

	return x;
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

	int * arr = new int[atoi(argv[1])];

	generateReduceData(arr, atoi(argv[1]));

	int sum = 0;

	if (scheduling == "static")
		omp_set_schedule(omp_sched_static, granularity);
	else if (scheduling == "dynamic")
		omp_set_schedule(omp_sched_dynamic, granularity);
	else if (scheduling == "guided")
		omp_set_schedule(omp_sched_guided, granularity);

	chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();

	omp_set_nested(1);
	sum = SumArrayPar(arr, n, nbthreads);
	chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
	chrono::duration<double> elapsed_seconds = end - start;
	cerr << elapsed_seconds.count() << endl;

	cout << sum;

	delete[] arr;

	return 0;
}
