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

void readFile(MPI_Comm communicator, char filename[]){
    int overlap = 10;
    int nodechucksize=40;
    int loopoffset;
    int new_rank;
    int new_size;
    MPI_File fh;
    MPI_Status status;
    MPI_Comm_rank(communicator,&new_rank);
    MPI_Comm_size(communicator,&new_size);
    int rc = MPI_File_open( communicator, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_Offset  filesize;
    MPI_File_get_size(fh, &filesize);
    int chunksize = 40 * sizeof(char);
    vector<char> text_buffer(chunksize);
    loopoffset = new_size*nodechucksize;
    for (int i=0; i < 3; i++){
        overlap = (i > 0) ? 10: 0;
        MPI_File_set_view(fh,(new_rank*sizeof(char)*nodechucksize+(loopoffset*i))-overlap,MPI_CHAR,MPI_CHAR,"native",MPI_INFO_NULL);
        MPI_File_read(fh, &text_buffer[0], chunksize, MPI_CHAR, &status);
        for (int i=0; i<text_buffer.size(); i++)
            cout << text_buffer.at(i);
        cout << endl;
    }
    MPI_File_close(&fh);
    
}


int main(int argc, char* argv[]) {
    MPI_Status status;
    int rank, size, color, errs=0;
    double START;
    char readfilename[] = "pg100.txt";
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    srand((unsigned)time(NULL) + rank*100);
    
    START = MPI_Wtime();
    
    // búum til com fyrir hópana
    MPI_Group mappers, partitioners, reducers, everyone;
    MPI_Comm partararcomm, reddararcomm;

    vector<int> partarar = {0, 1};
    vector<int> reddarar = {2, 3, 4};
    
    MPI_Comm_group(MPI_COMM_WORLD, &everyone);

    // ákveðum
    
    MPI_Group_incl(everyone, 2, &partarar[0], &partitioners);
    MPI_Comm_create(MPI_COMM_WORLD, partitioners, &partararcomm);
    
    MPI_Group_incl(everyone, 3, &reddarar[0], &mappers);
    MPI_Comm_create(MPI_COMM_WORLD, mappers, &reddararcomm);
    
    if (rank > 1)
        readFile(reddararcomm,readfilename);
    
    cout << "DONE" << endl;
    
    if (rank < 2)
    {
        //simulateElevator(rank, partararcomm);
    }
    else
    {
        //simulateFloor(rank, reddararcomm);
    }
    
    cout << "Node " << rank << " stopping." << endl;
    MPI_Barrier(MPI_COMM_WORLD);
    cout << "Node " << rank << " stopped." << endl;
    
    MPI_Finalize();
}
