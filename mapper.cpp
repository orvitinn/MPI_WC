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
#include <map>

#include "mpi.h"
#include "words.pb.h"

using std::cout;
using std::endl;
using std::time;
using std::vector;
using std::string;

extern int overlap;
extern int nodechucksize;
extern vector<int> mapparar;


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

void send_buffer_to_partitioner(int rank, char* buffer, int size);


void mapper(MPI_Comm communicator, int rank, const string& filename){
    int new_rank, new_size;
    MPI_Comm_rank(communicator,&new_rank);
    cout << "reddari " << new_rank << endl;
    int loopoffset, readoverlap;

    MPI_File fh;
    MPI_Comm_rank(communicator,&new_rank);
    MPI_Comm_size(communicator,&new_size);
    MPI_File_open( communicator, const_cast<char*>(filename.c_str()), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_Offset  filesize;
    MPI_Status status;
    MPI_File_get_size(fh, &filesize);
    
    int chunksize = nodechucksize * sizeof(char);
    loopoffset = new_size*nodechucksize;
    readoverlap = overlap * sizeof(char);
    vector<char> text_buffer(chunksize+overlap);
    vector<std::string> lines;
    std::vector<std::string> words;
    std::map<string, int> teljari;
    for (int i=0; i < 3; i++){
        MPI_File_set_view(fh,(new_rank*sizeof(char)*nodechucksize+(loopoffset*i)),MPI_CHAR,MPI_CHAR,"native",MPI_INFO_NULL);
        MPI_File_read(fh, &text_buffer[0], chunksize+readoverlap, MPI_CHAR, &status);
        std::string text_string_buffer(text_buffer.begin(), text_buffer.end());
        split(text_string_buffer, '\n', lines);
        for(string &line: lines){
            std::stringstream ss;
            int first=0;
            for (char &stafur: line){
                if(isalpha(stafur) && first == 0)
                    continue;
                
                if(!isalpha(stafur) && first == 0){
                    first = 1;
                    continue;
                }
                
                if(isalpha(stafur))
                    ss << stafur;
                
                if(!isalpha(stafur) && ss.str().length() > 0){
                    string ord = ss.str();
                    auto it = teljari.find(ord);
                    if (it != teljari.end())
                    {
                        it->second++;
                    }
                    else
                    {
                        teljari[ord] = 1;
                    }
                    ss.str(std::string());
                }
            }
        }
        // nú inniheldur teljari öll orð og tíðni þeirra...
        WordList output;
        for (auto it: teljari)
        {
            cout << it.first << " : " << it.second << endl;
            Word* new_word = output.add_words();
            new_word->word = it.first;
            new_word->count = it.second;
        }
        
        int size = ret.ByteSize();
        char* buffer = new char[size];
        ouput.SerializeToArray(buffer, size);
        send_buffer_to_partitioner(new_rank, buffer, size);
    }
    MPI_File_close(&fh);
}

void send_buffer_to_partitioner(int rank, char* buffer, int size);
{
    // select the corresponding partitioner (one for each mapper
    int destination_rank = rank + mapparar.size();
    MPI_Send(buffer, size, MPI_CHAR, destination_rank, 0, MPI_COMM_WORLD);
    delete[] buffer;
}
