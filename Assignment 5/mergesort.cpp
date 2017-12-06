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
void merge(int* arr, int size, int* temp);
void mergesortSerial(int* arr, int size, int* tempArr);
void mergesortParallel(int* arr, int size, int* tempArr, int threads);
int NextLargestBinSearch(int key, int* data, int len);

bool flag = true;
	
int NextLargestBinSearch(int key, int* data, int len)
{
	int low = 0;
	int high = len - 1;

	while (low <= high)
	{
		int mid = low + ((high - low) / 2);

		if (data[mid] < key) low = mid + 1;
		else if (data[mid] > key) high = mid - 1;
		else return mid + 1;
	}

	if (high < 0)
		return 0;
	else
	{
		if (low > (len - 1))
			return len;
		else
			return (low < high) ? low + 1 : high + 1;
	}
}

void merge(int * arr, int * tempArr, int n) {

	if (flag)
	{
		int i = 0;
		int j = n / 2;
		int k = 0;

		while (i<n / 2 && j<n) 
		{
			if (arr[i] < arr[j]) 
			{
				tempArr[k] = arr[i];
				k++;
				i++;
			}
			else 
			{
				tempArr[k] = arr[j];
				k++;
				j++;
			}
		}
		while (i<n / 2) 
		{
			tempArr[k] = arr[i];
			k++; i++;
		}
		while (j<n) 
		{
			tempArr[k] = arr[j];
			k++; j++;
		}
		memcpy(arr, tempArr, n * sizeof(int));
	}
	else
	{

		int * left = arr;
		int * right = arr + n/2;
		int size1 = n/2;
		int size2 = n - (n/2);
		int mid1 = size1 / 2;
		int midElement = arr[mid1];
		int mid2 = NextLargestBinSearch(midElement, arr+n/2, size2);
		cout << "cane here 2" << endl;
		cout << "size1=" << size1 << "size2=" << size2 << "mid1=" << mid1 << "mid2=" << mid2 << "midele=" << midElement << endl;
#pragma omp parallel 
		{
#pragma omp single nowait
			{
#pragma omp task
				merge(arr, tempArr, mid1);

#pragma omp task firstprivate(midElement)
				{
					merge(arr + mid1, tempArr + mid1 + mid2 + 1, size1 - mid1);
					tempArr[mid1 + mid2] = midElement;
				}
			}
		}
#pragma omp taskwait
		//mergeParallel(left, right, n/2, n/2);
	}

}

void
mergesortParallel(int* arr, int size, int* tempArr, int threads)
{
	//threads will be assigned tasks 1 by 1. if thread is only 1 then mergesortSerial() will be called 
	if (threads == 1)
		mergesortSerial(arr, size, tempArr);
	else if (threads > 1)
	{
		#pragma omp parallel
		{
			#pragma omp single nowait
			{
				#pragma omp task
				mergesortParallel(arr, size / 2, tempArr, threads / 2);
				
				#pragma omp task
				mergesortParallel(arr + size / 2, size - size / 2, tempArr + size / 2, threads - threads / 2);
			}
		#pragma omp taskwait
		}

		merge(arr, tempArr, size);
	}
	else
	{
		return;
	}
}

//if thread is 1, this function will be called
void
mergesortSerial(int* arr, int size, int* tempArr)
{
	if (size < 2)
	{
		return;
	}
	mergesortSerial(arr, size / 2, tempArr);
	mergesortSerial(arr + size / 2, size - size / 2, tempArr);

	merge(arr, tempArr, size);
}

int main(int argc, char* argv[]) {

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <n> <nbthreads> " << std::endl;
		return -1;
	}

	int n, nbthreads;

	sscanf(argv[1], "%d", &n);
	sscanf(argv[2], "%d", &nbthreads);

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
	int * tempArr = new int[atoi(argv[1])];
	generateMergeSortData(arr, atoi(argv[1]));

	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

	omp_set_nested(1);
	mergesortParallel(arr, n, tempArr, nbthreads);
	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;

	std::cerr << elapsed_seconds.count() << std::endl;

	checkMergeSortResult(arr, atoi(argv[1]));

	delete[] arr;

	return 0;
}
