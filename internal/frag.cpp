#include "common.h"

using namespace std;

// size of fragmented file.
const uint64_t FRAG_SIZE = 64*1024*1024;

const uint64_t MAGIC_WORD = 0xDEADFACE;

const int MAX_PREFIX_LEN = 64;

// header of fragment.
class file_fragment
{
public:
    uint64_t magic_word;    // end of frag
    uint64_t offset;        // offset in mm file
    uint64_t size;          // size of frag
    uint64_t allocated;     // alloc bytes in frag
    char     prefix[MAX_PREFIX_LEN];
    char     data[0];       // data in fragment

    file_fragment(string name, uint64_t offset)
        : magic_word(MAGIC_WORD),
        : offset(offset),
        : size(0),
        : allocated(0)
    {
        int n = MAX_PREFIX_LEN <= name.length() ? MAX_PREFIX_LEN - 1 : name.length(); 
        strncpy(prefix, name.c_str(), n);
    }
};

// make it real.
static file_fragment *file_fragment::make(string name, uint64_t offset, uint64_t frag_size)
{
    char *buffer = new char[sizeof(file_fragment) + FRAG_SIZE];
    file_fragment *ff = new(buffer)file_fragment(name, offset);
    int ret = write_file(name, buffer, frag_size);
    if (ret != 0)
    {
        cout << "Failed to make file frament." << endl;
        return NULL;
    }
    return ff;
}

int write_file(string name, char *buffer, size_t size)
{
    fstream fs;
    fs.open(name, ios::out | ios::binary);
	if (!fs)
    {
		cout << "Failed to open file: " << name << endl;
        return -1;
    }
    fs.write(buffer, size);
    cout << size << " Bytes written for file: " << name << endl;
    return 0
}

// header of meta fragment.
class meta_fragment
{
public:
    uint64_t magic_word;    // end of frag
    uint64_t last_fragment; // number of last fragment
    char     prefix[64];    // prefix of file name    
};

int test_frag()
{
    file_fragment *ff = file_fragment::make("index.0", FRAG_SIZE);
    if (ff != NULL)
        delete ff;
    return 0;
}