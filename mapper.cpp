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
#include <cctype>
#include <typeinfo>
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
extern vector<int> mapparar;
extern vector<int> reddarar;
extern vector<unsigned char> reddarar_range_start;

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

void send_buffer_to_reducers(const WordList& data, int destination);
void process_buffer(vector<unsigned char>& buffer, int rank);

void mapper(MPI_Comm communicator, int rank, const string& filename)
{
    int new_rank, new_size;
    MPI_Comm_rank(communicator,&new_rank);
    cout << "mapper " << new_rank << endl;
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
    vector<unsigned char> text_buffer(chunksize+overlap);


    MPI_Offset mypart = filesize/new_size;
    for (int i=0; i <= ceil(mypart/chunksize); i++){
        cout << "rank : " << new_rank << " Trying to read from " << new_rank*sizeof(char)*chunksize+(loopoffset*i) << " to "  << new_rank*sizeof(char)*chunksize+(loopoffset*i)+chunksize+readoverlap << endl;
        if(((new_rank*sizeof(char)*chunksize+(loopoffset*i)+chunksize+readoverlap) > filesize)){
            chunksize = filesize - ((new_rank*sizeof(char)*chunksize+(loopoffset*i))-readoverlap);
            cout << "rank : " << new_rank << " Trying to read from " << new_rank*sizeof(char)*chunksize+(loopoffset*i) << " to "  << new_rank*sizeof(char)*chunksize+(loopoffset*i)+chunksize+readoverlap << " file is only " << filesize << " will only read " << chunksize << endl;
            if (chunksize < 0){
                cout << "Gone to far in file exiting" << endl;
                break;
            }
        }
        
        MPI_File_set_view(fh,(new_rank*chunksize+(loopoffset*i)),MPI_UNSIGNED_CHAR,MPI_UNSIGNED_CHAR,"native",MPI_INFO_NULL);
        MPI_File_read(fh, &text_buffer[0], chunksize+readoverlap, MPI_UNSIGNED_CHAR, &status);
    
        int read_bytes;
        MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &read_bytes);
        text_buffer.resize(read_bytes);
        process_buffer(text_buffer, new_rank);
        // text_buffer.clear();
    }
    MPI_File_close(&fh);
    
    // send empty list for close
    WordList output;
    
    for (int d: reddarar)
        send_buffer_to_reducers(output, d);
}

void process_buffer(vector<unsigned char>& text_buffer, int rank)
{
    cout << "process_buffer" << endl;
    vector<std::string> lines;
    std::vector<std::string> words;
    std::map<string, int> teljari;
    
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
            {
                // ss << tolower(stafur);
                ss << stafur;
            }
            
            if(!isalpha(stafur) && ss.str().length() > 0)
            {
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
                ss.str("");
                ss.clear();
            }
        }
    }

    
    // nú inniheldur teljari öll orð og tíðni þeirra...
    // nú förum við í gegnum listann sem er í stafrófsröð og sendum
    // bita á sérhvern reducers. Allt sem er minna en reddarar_range_start[i+1]
    // fer á destination reddarar[i]
    WordList output;
    int i=0;
    for (auto it: teljari)
    {
        // cout << "[" << it.first << "] : " << it.second << endl;
        Word* new_word = output.add_words();
        new_word->set_word(it.first);
        new_word->set_count(it.second);
        if (it.first[0] > reddarar_range_start[i+1])
        {
            send_buffer_to_reducers(output, reddarar[i]);
            output.Clear();
            i++;
        }
    }
    if (output.words_size() > 0)
    {
        send_buffer_to_reducers(output, reddarar[i]);
    }
}

void send_buffer_to_reducers(const WordList& data, int destination)
{
    int size = data.ByteSize();
    vector<unsigned char> buffer(size);
    data.SerializeToArray(&buffer[0], size);
    buffer.resize(size);
    MPI_Send(&buffer[0], size, MPI_UNSIGNED_CHAR, destination, 0, MPI_COMM_WORLD);
    
#if 0
    // prófa...
    WordList test;
    if (test.ParseFromArray(&buffer[0], size) == true)
    {
        cout << "Mapper parsing ok." << endl;
    }
    else
    {
        cout << "Mapper parsing failed." << endl;
    }
#endif
}

