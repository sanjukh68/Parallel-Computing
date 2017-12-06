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
	int rank, size, tag=100;
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
	//cout<<rank<<endl;
	int s = (floor(n/size) * rank);
	int e = (floor(n/size) * (rank + 1));

	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

	for (int i = s; i < e; i++)
	{
		localSum += calculateValue(functionId, a, b, n, intensity, i);
	}
	
	if(rank == 0) 
	{
		localresult = localSum;
		for (int i = 1 ; i < size ; i++) 
		{
		  source = i;
		  MPI_Recv(&localSum, 1, MPI_REAL, source, tag, MPI_COMM_WORLD, &status);
		  localresult += localSum;
		}
		cout<<localresult<<endl;
		std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;  
		std::cerr<<elapsed_seconds.count()<<std::endl;
	}
	else
		MPI_Send(&localSum, 1, MPI_REAL, 0, tag, MPI_COMM_WORLD);
	
	MPI_Finalize();
	
	return 0;
}
