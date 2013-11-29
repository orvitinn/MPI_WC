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

const int MAX_MAPP_ID = 6;
int overlap = 20;
int nodechucksize=50000;
int collector_rank;

vector<int> mapparar = {0, 1, 2, 3, 4, 5};
vector<int> reddarar = {6, 7, 8};
vector<unsigned char> reddarar_range_start = {'a','e', 'm', 255};

int main(int argc, char* argv[]) {
    int rank, size;
    string readfilename = "/home/maa33/code/mpi_wc/pg100.txt";
    // string readfilename = "/home/maa33/code/mpi_wc/smaller.txt";
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    collector_rank = size-1; // sá síðasti er kollektorinn
    
    /*
    for (int i=0; i<collector_rank; i++)
    {
        if (rank < MAX_MAPP_ID)
        {
            mapparar.push_back(i++);
        }
        else
        {
            reddarar.push_back(i++);
        }
    }*/
    
    // búum til com fyrir hópana
    MPI_Group mappers, reducers, everyone;
    MPI_Comm mapparacom, reddararcomm;

    
    MPI_Comm_group(MPI_COMM_WORLD, &everyone);

    // ákveðum
    
    MPI_Group_incl(everyone, mapparar.size(), &mapparar[0], &mappers);
    MPI_Comm_create(MPI_COMM_WORLD, mappers, &mapparacom);
    
    MPI_Group_incl(everyone, reddarar.size(), &reddarar[0], &reducers);
    MPI_Comm_create(MPI_COMM_WORLD, reducers, &reddararcomm);
    
    cout << "DONE creating groups and comms" << endl;
    
    int i = 0;
    if (rank < MAX_MAPP_ID)
    {
        mapper(mapparacom, rank, readfilename);
        mapparar.push_back(i++);
    }
    else if (rank < size-1)
    {
        reducer(reddararcomm, rank);
        reddarar.push_back(i++);
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
