#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <map>

#include "mpi.h"
#include "words.pb.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;


extern int overlap;
extern int nodechucksize;
extern int collector_rank;
extern vector<int> mapparar;

const int BUFFER_SIZE = 1024 * 1024;

void send_buffer_to_collector(vector<unsigned char>& buffer);

void reducer(MPI_Comm communicator, int rank)
{
    cout << "reducer " << rank << " starting." << endl;
    int new_rank;
    MPI_Comm_rank(communicator,&new_rank);
    MPI_Status status;

    vector<unsigned char> buffer(BUFFER_SIZE);
    std::map<string, int> teljari;
    unsigned int quit_count=0;
    
    while (true) {
        buffer.reserve(BUFFER_SIZE);
        MPI_Recv(&buffer[0], BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        int read_bytes;
        MPI_Get_count(&status, MPI_CHAR, &read_bytes);
        buffer.resize(read_bytes);
        WordList input;
        input.ParseFromArray(&buffer[0], read_bytes);
        
        // ef við fengum tóman lista er þessi nóða hætt.
        if (input.words_size() == 0)
        {
            cout << "Reducer " << rank << " got an empty list, quit_count = " << quit_count << endl;
            quit_count++;
            if (quit_count == mapparar.size())
            {
                break;
            }
            else
            {
                continue;
            }
        }
        
        cout << "Reducer " << rank << "read [ " << read_bytes << "," << buffer.size() << "] bytes and got " << input.words_size() << " words from a mapper. Processing." << endl;
        
        for (int i=0; i<input.words_size(); i++)
        {
            const Word& word = input.words(i);
            auto it = teljari.find(word.word());
            if (it != teljari.end())
            {
                it->second++;
            }
            else
            {
                teljari[word.word()] = 1;
            }
        }
    }
                                   
    // done... send everything to the collector
    cout << "reducer " << rank << "done receiving data, sending to the collector. Have " << teljari.size() << " words. " << endl;

    WordList output;
    for (auto it: teljari)
    {
        cout << "[[" << it.first << "]] : " << it.second << endl;
        Word* new_word = output.add_words();
        new_word->set_word(it.first);
        new_word->set_count(it.second);
    }
    
    int size = output.ByteSize();
    buffer.reserve(size);
    output.SerializeToArray(&buffer[0], size);
    buffer.resize(size);
    cout << "Recuder " << rank << " sending " << buffer.size() << " bytes to the collector." << endl;
    send_buffer_to_collector(buffer);
}


void send_buffer_to_collector(vector<unsigned char>& buffer)
{
    MPI_Send(&buffer[0], buffer.size(), MPI_CHAR, collector_rank, 0, MPI_COMM_WORLD);
}

