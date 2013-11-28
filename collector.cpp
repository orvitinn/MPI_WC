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
#include <fstream>
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
extern vector<int> reddarar;

const int BUFFER_SIZE = 1024 * 1024;

void collector(int rank)
{
    cout << "Collector running, rank: " << rank << endl;
    MPI_Status status;

    vector<char> buffer(BUFFER_SIZE);
    std::map<string, int> teljari;
    int count=0;
    
    while (count < reddarar.size()) {
        MPI_Recv(&buffer[0], BUFFER_SIZE, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        int read_bytes;
        MPI_Get_count(&status, MPI_CHAR, &read_bytes);
        buffer.resize(read_bytes);
        WordList input;
        input.ParseFromArray(&buffer[0], read_bytes);
        count++;
        cout << "Collector read [" << read_bytes << "] bytes, [" << input.words_size() << "], " << data << " << count << "] << endl;
        
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
                teljari[it->first] = 1;
            }
        }
    }
                                   
    WordList output;
    for (auto it: teljari)
    {
        Word* new_word = output.add_words();
        new_word->set_word(it.first);
        new_word->set_count(it.second);
    }
    
    int size = output.ByteSize();
    buffer.reserve(size);
    output.SerializeToArray(&buffer[0], size);
    buffer.resize(size);
                                   
    // now just write the buffer to a single file.
    cout << "Writing file of size " << size << endl;
    std::ofstream outfile ("/home/maa33/code/mpi_wc/wc.data", std::ofstream::binary);
    outfile.write(&buffer[0], size);
    outfile.close();
}
