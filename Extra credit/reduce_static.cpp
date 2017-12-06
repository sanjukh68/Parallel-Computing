#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <ratio>
#include <ctime>
#include <omp.h>
#include <mpi.h>

#define CHUNK 100

#ifdef __cplusplus
extern "C" {
#endif

    void generateReduceData (int* arr, size_t n);

#ifdef __cplusplus
}
#endif

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr<<"usage: "<<argv[0]<<" <n> "<<std::endl;
        return -1;
    }

    int n = atoi(argv[1]);
    MPI_Comm comm;

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::syst                                                                                                                                  em_clock::now();
    double globalResult = 0.0;
    int *arr = new int [n];
    generateReduceData (arr, atoi(argv[1]));

    int rank, size;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int chunkSize = n / size;
    int tag = MPI_ANY_TAG;

    int arrStart = rank * chunkSize;
    int arrEnd = (rank + 1) * chunkSize;
    if (rank == size - 1)
    {
        arrEnd = n;
    }

    double result = 0.0;
#pragma omp parallel reduction(+:result)
    {
#pragma omp parallel for schedule(static, CHUNK)
        for (int x = arrStart; x < arrEnd; ++x)
        {
            result += arr[x];
        }
    }
    if (rank != 0)
    {
        MPI_Send(&result, 1, MPI_DOUBLE_PRECISION, 0, 100+rank, MPI_COMM_WORLD);
    }
    else
    {
        globalResult = result;
        for (int x = 1; x < size; ++x)
        {
            MPI_Status status;
            MPI_Recv(&result, 1, MPI_DOUBLE_PRECISION, x, 100+x, MPI_COMM_WORLD,                                                                                                                                   &status);
            globalResult += result;
        }
    }

    MPI_Finalize();
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system                                                                                                                                  _clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;

    if(rank == 0)
    {
        std::cout<<globalResult<<std::endl;
        std::cerr<<elapsed_seconds.count()<<std::endl;
    }

    return 0;
}
