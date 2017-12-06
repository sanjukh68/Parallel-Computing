#include <iostream>
#include <ratio>
#include <chrono>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>


#define ARR_START_SEND 100 
#define ARR_START_RECV 100 
#define ARR_END_SEND 101
#define ARR_END_RECV 101 
#define RESULT_TAG 1002
#define CHUNK_SIZE 100

#define MASTER_MPI 0

#ifdef __cplusplus
extern "C" {
#endif

	void generateReduceData(int* arr, size_t n);

#ifdef __cplusplus
}
#endif

using namespace std;
int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr<<"usage: mpirun "<<argv[0]<<" <n>"<<std::endl;
        return -1;
    }

    int n = atoi(argv[1]);

    if(n==0)
    {
        std::cout<<0<<std::endl;
        std::cerr<<0<<std::endl;
        return 0;
    }

    double reduceResult = 0.0;
    MPI_Comm comm;
	int startEnd[2]={0,0};
    int * arr = new int [n];
    generateReduceData (arr, atoi(argv[1]));
	
    double globalResult = 0.0;
    int rank, size;
    int master = 0;
    int arrIndex = 0; 
    double receivedResult = 0.0;
    int startIndex, endIndex;
    MPI_Status status;
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	//#if 0
    if(rank == 0)
    {
		int j = 1;
        for(; j < size && startEnd[0] < n; ++j)
        {
			startEnd[1] += CHUNK_SIZE;
			if (startEnd[1] > n)
				startEnd[1] = n;
            MPI_Send(&startEnd,         // index being sent
                    2,                  // 1byte 
                    MPI_INT,            // data type of var being sent 
                    j,                  // Destination 
                    ARR_START_SEND,     // tag 
                    MPI_COMM_WORLD);    // MPI Communicator 

			startEnd[0] = startEnd[1];
        }
		
		for (int i = j; i < size;i++)
		{
			startEnd[0] = n;
			MPI_Send(&startEnd,                // index being sent
                        2,                  // 1byte 
                        MPI_INT,            // data type of var being sent 
                        i,  				// Destination 
                        ARR_START_SEND,     // tag
                        MPI_COMM_WORLD);    // MPI Communicator 
		}
        for(int i=1; i < j;)
        {
            MPI_Recv(&receivedResult,       // Value in whuch the message is received 
                    1,                      // length of data being received 
                    MPI_DOUBLE_PRECISION,   // data type of var 
                    MPI_ANY_SOURCE,         // Any source 
                    RESULT_TAG,             // tag - 1002
                    MPI_COMM_WORLD,         // MPI Communicator 
                    &status);               // stats of received message 

            // std::cout<<"Received result :"<<receivedResult<<" from node "<<status.MPI_SOURCE<<std::endl;
            reduceResult += receivedResult; 
            if(startEnd[0] >= n)
            {
                MPI_Send(&startEnd,                // index being sent
                        2,                  // 1byte 
                        MPI_INT,            // data type of var being sent 
                        status.MPI_SOURCE,  // Destination 
                        ARR_START_SEND,     // tag
                        MPI_COMM_WORLD);    // MPI Communicator 
                ++i;
            }
            else 
            {
				startEnd[1] += CHUNK_SIZE;
				if (startEnd[1] > n)
					startEnd[1] = n;
                MPI_Send(&startEnd,         // index being sent
                        2,                  // 1byte 
                        MPI_INT,            // data type of var being sent 
                        status.MPI_SOURCE,  // Destination
                        ARR_START_SEND,     // tag
                        MPI_COMM_WORLD);    // MPI Communicator 
						
				startEnd[0] = startEnd[1];
            }
        }
        std::cout<<reduceResult<<std::endl;
    }
    else
    {
        while(true)
        {
            // Receive array start index
            MPI_Recv(&startEnd,           // Value in whuch the message is received 
                    2,                      // length of data being received 
                    MPI_INT,                // data type of var 
                    MASTER_MPI,             // receive from Master (0) 
                    ARR_START_RECV,         // tag 
                    MPI_COMM_WORLD,         // MPI Communicator 
                    &status);               // stats of received message 
            if(startEnd[0] == n)
            {
                break;
            }

            double result = 0.0;
			#pragma omp parallel
			{
				#pragma omp for schedule(runtime) reduction(+:result)
				for (int x = startEnd[0]; x < startEnd[1]; ++x) 
                { 
                    result += arr[x];
                }
			}
			
            // Send result to master 
            MPI_Send(&result,               // computed result to send to master 
                    1,                      // length of data being sent 
                    MPI_DOUBLE_PRECISION,   // data type of var 
                    MASTER_MPI,             // Destination - Master (0) 
                    RESULT_TAG,             // tag - 1002
                    MPI_COMM_WORLD);        // MPI Communicator 
        }
    }
    
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now(); 
    std::chrono::duration<double> elapsed_seconds = end-start; 
    if(rank == 0)
        std::cerr<<elapsed_seconds.count()<<std::endl;

	MPI_Finalize();
	return 0;
}
