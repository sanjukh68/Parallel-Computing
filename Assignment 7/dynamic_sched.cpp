#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>

using namespace std;

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

float calculateValue(int functionId, int a, int b, int n, int intensity, int i)
{
	float val = 0.0f;
	float compute = a + ((i + 0.5)*(b - a) / n);
	if (functionId == 1)
		val = f1(compute, intensity);
	else if (functionId == 2)
		val = f2(compute, intensity);
	else if (functionId == 3)
		val = f3(compute, intensity);
	else if (functionId == 4)
		val = f4(compute, intensity);
	return val*(b - a) / n;
}
  
int main (int argc, char* argv[]) {

  if (argc < 6) {
    std::cerr<<"usage: mpirun "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    return -1;
  }

	int functionId, a, b, n, intensity;
	float sum = 0.0f;
	float localSum = 0.0f;
	float localresult = 0.0f;
	int rank, size, tag=100, i=0, arr[2]={0,0};
	int source = 0;
	MPI_Status status;
	
	sscanf(argv[1], "%d", &functionId);
	sscanf(argv[2], "%d", &a);
	sscanf(argv[3], "%d", &b);
	sscanf(argv[4], "%d", &n);
	sscanf(argv[5], "%d", &intensity);

	if (functionId > 4 || functionId < 1)
		return -1;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int s = (floor(n/size) * rank);
	int e = (floor(n/size) * (rank + 1));

	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();


	if(rank == 0) 
	{
		if (size == 1)
		{
			for (int i=0;i<=n;i++)
				localresult += calculateValue(functionId, a, b, n, intensity, i);
		}
		else
		{
			int j = 1;
			for ( ; j < size && arr[0] < n ; j++)
			{
				arr[1] += 100;
				if (arr[1] > n)
					arr[1] = n;
				MPI_Send(&arr, 2, MPI_INT, j, 100, MPI_COMM_WORLD);
				arr[0] = arr[1];
			}
			
			for (int i = j; i < size;i++)
			{
				arr[0] = n;
				MPI_Send(&arr, 2, MPI_INT, i, 100, MPI_COMM_WORLD); 
			}
			for(int k=1; k<j;)
			{
				MPI_Recv(&localSum, 1, MPI_FLOAT, MPI_ANY_SOURCE, 102, MPI_COMM_WORLD, &status);
				localresult += localSum;
				if (arr[0] >= n)
				{
					MPI_Send(&arr, 2, MPI_INT, status.MPI_SOURCE, 100, MPI_COMM_WORLD);
					k++;
					continue;
				}
				else
				{
					arr[1] += 100;
					if (arr[1] > n)
						arr[1] = n;
					MPI_Send(&arr, 2, MPI_INT, status.MPI_SOURCE, 100, MPI_COMM_WORLD);
					arr[0] = arr[1];
				}

			}
		}
		
	
		cout<<localresult<<endl;
		std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;  
		std::cerr<<elapsed_seconds.count()<<std::endl;
	}
	else
	{
		while(1)
		{
			MPI_Recv(&arr, 2, MPI_INT, 0, 100, MPI_COMM_WORLD, &status);
			if (arr[0] == n)
				break;

			localSum = 0.0f;
			for (int i = arr[0]; i < arr[1]; i++)
			{
				localSum += calculateValue(functionId, a, b, n, intensity, i);
			}
			MPI_Send(&localSum, 1, MPI_FLOAT, 0, 102, MPI_COMM_WORLD);
		}
	}	
	
	MPI_Finalize();
	
	return 0;
}
