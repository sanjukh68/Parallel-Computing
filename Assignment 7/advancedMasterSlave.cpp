#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>

#define MASTER_NODE 0
#define MASTER_SEND 100 
#define CHUNK_SIZE 100 
#define MASTER_RECV 101

#ifdef __cplusplus
extern "C" {
#endif
	float f1(float x, int intensity);
	float f2(float x, int intensity);
	float f3(float x, int intensity);
	float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif

// Function pointer to define the function to be called from the library
float (*funcPtr) (float,int);

int main (int argc, char* argv[]) 
{

	if (argc < 6) {
		std::cerr<<"usage: mpirun "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
		return -1;
	}

	int a = atoi(argv[2]);
	int n = atoi(argv[4]);
	int intensity = atoi(argv[5]);
	int chunkSize = atoi(argv[8]);
	float multiplier = (atoi(argv[3]) - a) / (float)n;

	double localResult = 0.0;
	double globalResult = 0.0;
	int rank, size, arr[2]={0,0};
	MPI_Status status;
	MPI_Request request;

	switch(atoi(argv[1]))
	{
		case 1:
			funcPtr = &f1;
			break;
		case 2: 
			funcPtr = &f2;
			break;
		case 3: 
			funcPtr = &f3;
			break;
		case 4: 
			funcPtr = &f4;
			break;
		default:
			std::cerr<<"Invalid function number provided\n";
			exit(-1);
	}	

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();


	if(rank == 0) 
	{
		if (size == 1)
		{
			for (int i=0;i<=n;i++)
				globalResult +=  (double)funcPtr(a + (i + 0.5) * multiplier, intensity) * multiplier;
		}
		else
		{
			int j = 1;
			int numChunks = 0;
			for ( ; j < size && arr[0] < n; ++j)
			{
				int k = 0;
				while (k < 3 && arr[0] < n)
				{
					arr[1] += CHUNK_SIZE;
					if (arr[1] > n)
						arr[1] = n;
					MPI_Isend(&arr, 2, MPI_INT, j, MASTER_SEND, MPI_COMM_WORLD, &request);
					arr[0] = arr[1];
					k++;
					++numChunks;
				}
			}

			for (int i = j; i < size; ++i)
			{
				arr[0] = n;
				MPI_Isend(&arr, 2, MPI_INT, i, MASTER_SEND, MPI_COMM_WORLD, &request); 
			}

			for(int k=0; numChunks > 0;)
			{
				--numChunks;
				MPI_Recv(&localResult, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 102, MPI_COMM_WORLD, &status);
				globalResult += localResult;
				if (arr[0] >= n)
				{
					MPI_Send(&arr, 2, MPI_INT, status.MPI_SOURCE, MASTER_SEND, MPI_COMM_WORLD);
					k++;
					continue;
				}
				else
				{
					++numChunks;
					arr[1] += CHUNK_SIZE;
					if (arr[1] > n)
						arr[1] = n;
					MPI_Isend(&arr, 2, MPI_INT, status.MPI_SOURCE, MASTER_SEND, MPI_COMM_WORLD, &request);
					arr[0] = arr[1];
				}

			}
		}


		std::cout<<globalResult<<std::endl;
		std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;  
		std::cerr<<elapsed_seconds.count()<<std::endl;
	}
	else
	{
		while(1)
		{
			MPI_Irecv(&arr, 2, MPI_INT, MASTER_NODE, MASTER_SEND, MPI_COMM_WORLD, &request);
			MPI_Wait (&request, &status);

			if (arr[0] == n)
				break;

			localResult = 0.0f;
			for (int i = arr[0]; i < arr[1]; i++)
			{
				localResult +=  (double)funcPtr(a + (i + 0.5) * multiplier, intensity) * multiplier;
			}
			MPI_Isend(&localResult, 1, MPI_DOUBLE, 0, 102, MPI_COMM_WORLD, &request);
		}
	}	
	MPI_Finalize();

	return 0;
}
