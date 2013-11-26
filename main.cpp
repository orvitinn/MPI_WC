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
#include <boost/algorithm/string.hpp>
#include <ctype.h>
#include <typeinfo>

#include "mpi.h"

using std::cout;
using std::endl;
using std::time;
using std::vector;
using std::string;

void mapper();
void partitioner();
void reducer();

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

inline std::string trim(const std::string &s)
{
    auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
    auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
    return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}


void readFile(MPI_Comm communicator, char filename[]){
    int overlap = 20;
    int nodechucksize=200;
    int loopoffset, readoverlap;
    int new_rank, new_size;
    vector<std::string> lines;
    std::vector<std::string> words;
    MPI_File fh;
    MPI_Status status;
    
    MPI_Comm_rank(communicator,&new_rank);
    MPI_Comm_size(communicator,&new_size);
    int rc = MPI_File_open( communicator, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_Offset  filesize;
    MPI_File_get_size(fh, &filesize);
    
    int chunksize = nodechucksize * sizeof(char);
    loopoffset = new_size*nodechucksize;
    readoverlap = overlap * sizeof(char);
    vector<char> text_buffer(chunksize+overlap);
    for (int i=0; i < 3; i++){
        MPI_File_set_view(fh,(new_rank*sizeof(char)*nodechucksize+(loopoffset*i)),MPI_CHAR,MPI_CHAR,"native",MPI_INFO_NULL);
        MPI_File_read(fh, &text_buffer[0], chunksize+readoverlap, MPI_CHAR, &status);
        printf("read processor rank %i read : ",new_rank);
        std::string text_string_buffer(text_buffer.begin(), text_buffer.end());
        split(text_string_buffer, '\n', lines);
        # pragma omp for
        for(string &line: lines){
            for (char &stafur: line){
                while (isalpha(stafur))
                    continue
                
                cout << stafur << endl;
            }
        }
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
    vector<int> reddarar = {2, 3};
    
    MPI_Comm_group(MPI_COMM_WORLD, &everyone);

    // ákveðum
    
    MPI_Group_incl(everyone, 2, &partarar[0], &partitioners);
    MPI_Comm_create(MPI_COMM_WORLD, partitioners, &partararcomm);
    
    MPI_Group_incl(everyone, 2, &reddarar[0], &mappers);
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
