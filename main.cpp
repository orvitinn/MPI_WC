#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <algorithm>

#include "mpi.h"

using std::cout;
using std::endl;
using std::time;
using std::vector;
using std::string;

void mapper();
void partitioner();
void reducer();


int main(int argc, char* argv[]) {
    MPI_Status status;
    int rank, size, color, errs=0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    srand((unsigned)time(NULL) + rank*100);
    
    START = MPI_Wtime();
    
    // búum til com fyrir hópana
    MPI_Group mappers, partitioners, reducers, everyone;
    MPI_Comm lyftucomm, haedacomm;

    vector<int> partarar = {0, 1};
    vector<int> reddarar = {2, 3, 4};
    
    MPI_Comm_group(MPI_COMM_WORLD, &everyone);

    // ákveðum
    
    MPI_Group_incl(allir, 2, &lyftur[0], &lyftuhopur);
    MPI_Comm_create(MPI_COMM_WORLD, lyftuhopur, &lyftucomm);
    
    MPI_Group_incl(allir, 3, &haedir[0], &haedahopur);
    MPI_Comm_create(MPI_COMM_WORLD, haedahopur, &haedacomm);
    
    if (rank < 2)
    {
        simulateElevator(rank, lyftucomm);
    }
    else
    {
        simulateFloor(rank, haedacomm);
    }
    
    cout << "Node " << rank << " stopping." << endl;
    MPI_Barrier(MPI_COMM_WORLD);
    cout << "Node " << rank << " stopped." << endl;
    
    MPI_Finalize();
}
