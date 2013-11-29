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
#include <ctype.h>
#include <typeinfo>
#include <sstream>

#include "mpi.h"

using std::cout;
using std::endl;
using std::time;
using std::vector;
using std::string;

extern void mapper(MPI_Comm communicator, int rank, const string& filename);
extern void reducer(MPI_Comm communicator, int rank);
extern void collector(int rank);

const int MAX_MAPP_ID = 2;
const int MAX_PART_ID = 4;
int overlap = 20;
int nodechucksize=50000;
int collector_rank;

vector<int> mapparar = {0, 1};
vector<int> reddarar = {2, 3};
vector<unsigned char> reddarar_range_start = {'a','i', 255};

int main(int argc, char* argv[]) {
    int rank, size;
    string readfilename = "/home/maa33/code/mpi_wc/pg100.txt";
    // string readfilename = "/home/maa33/code/mpi_wc/smaller.txt";
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    collector_rank = size-1; // sá síðasti er kollektorinn
    
    // búum til com fyrir hópana
    MPI_Group mappers, reducers, everyone;
    MPI_Comm mapparacom, reddararcomm;

    
    MPI_Comm_group(MPI_COMM_WORLD, &everyone);

    // ákveðum
    
    MPI_Group_incl(everyone, 2, &mapparar[0], &mappers);
    MPI_Comm_create(MPI_COMM_WORLD, mappers, &mapparacom);
    
    MPI_Group_incl(everyone, 2, &reddarar[0], &reducers);
    MPI_Comm_create(MPI_COMM_WORLD, reducers, &reddararcomm);
    
    cout << "DONE creating groups and comms" << endl;
    
    if (rank < MAX_MAPP_ID)
    {
        mapper(mapparacom, rank, readfilename);
    }
    else if (rank < size-1)
    {
        reducer(reddararcomm, rank);
    }
    else
    {
        collector(rank);
    }
    
    // einn process er í gather lógík, fær niðurstöður frá öllum reducers
    cout << "Node " << rank << " stopping." << endl;
    MPI_Barrier(MPI_COMM_WORLD);
    cout << "Node " << rank << " stopped." << endl;
    
    MPI_Finalize();
}
