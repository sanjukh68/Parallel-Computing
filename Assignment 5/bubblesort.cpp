#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

	void generateMergeSortData(int* arr, size_t n);
	void checkMergeSortResult(int* arr, size_t n);


#ifdef __cplusplus
}
#endif

using namespace std;

void swap (int* a, int* b)
{
	if (*a==*b)
		return;
	int temp = *a;
	*a = *b;
	*b = temp;
}

void bubbleSortParallel(int* a, int n, int nbthreads) {
   int length, i, j, size_low, size_up;

   omp_lock_t lock[nbthreads];
   int chunk_size = n/nbthreads;

   for(i=0;i<nbthreads;i++) 
	   omp_init_lock(&lock[i]);

   omp_set_num_threads(nbthreads);
   
   #pragma omp parallel private(length, i, j, size_low, size_up)
   {
       for(length=chunk_size;length>=1;length--)
	   {
           for(j=0;j<nbthreads;j++)
		   {
               size_low= j*chunk_size;
			   
               if(j==nbthreads-1) 
				   size_up = n;
               else 
				   size_up = (j+1)*chunk_size;
			   
               if(j==0) 
				   omp_set_lock(&lock[0]);

               for(i=size_low;i<size_up-1;i++)
                   if(a[i]>a[i+1])
                       swap(a[i], a[i+1]);
			   
               if(j==nbthreads-1)
			   {
                   omp_unset_lock(&lock[j]);
                   break;
               }

               omp_set_lock(&lock[j+1]);
               
			   if(a[i]>a[i+1])
                   swap(a[i],a[i+1]);
               
			   omp_unset_lock(&lock[j]);
           }
        }
    }
}

int main(int argc, char* argv[]) {

	if (argc < 5) {
		std::cerr << "Usage: " << argv[0] << " <n> <nbthreads> <schedule> <granularity>" << std::endl;
		return -1;
	}

	int n, nbthreads, granularity;
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

	if (scheduling == "static")
		omp_set_schedule(omp_sched_static, granularity);
	else if (scheduling == "dynamic")
		omp_set_schedule(omp_sched_dynamic, granularity);
	else if (scheduling == "guided")
		omp_set_schedule(omp_sched_guided, granularity);
	
	int * arr = new int[atoi(argv[1])];
	
	//using alreaddy existing library function for bubble sort
	generateMergeSortData(arr, atoi(argv[1]));

	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

	bubbleSortParallel(arr, n, nbthreads);

	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;

	std::cerr << elapsed_seconds.count() << std::endl;

	//using alreaddy existing library function for bubble sort
	checkMergeSortResult(arr, atoi(argv[1]));

	delete[] arr;

	return 0;
}
