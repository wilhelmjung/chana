#include <cinttypes>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

using namespace std;

const int ORDER = 1024;
const int DATA_FILE_SIZE = 1024 * 1024 * 1024;

const string DATA_FILE_PATH = "data.bin";

struct data_file {
    // head
    uint32_t head; // "data", "idx_"
    uint32_t type; // 0:data; 1:index.
    uint32_t root; // root node offset, subject to change.
    uint32_t size; // size of data file.
};

struct node
{
    uint32_t head;              // "node"
    uint32_t type;              // 0:root; 1:stem(internal node); 2:leaf
    uint64_t keys[ORDER - 1];   // TODO: for now only long int is supported.
    uint64_t ptrs[ORDER];       // ptr to child node.
    uint64_t vals[0];           // TODO: value array or ptr to real value string, optional for leaf node.
};

struct data_file *my_data_file;
struct node *root_node;

// create empty data file.
int create_data_file(const char *file_path)
{
    fstream fs;
    fs.open(file_path, ios::out | ios::binary);
	if (!fs)
    {
		cout << "Failed to create file: " << DATA_FILE_PATH << endl;
        return -1;
	}
    // write file header
    struct data_file head = {
        .head = 0xDEAD,
        .type = 0,
        .root = 0,
        .size = 0
    };
    char *bs = (char *)&head;
    long sz = sizeof(head);
    fs.write(bs, sz);
    cout << sz << " bytes written." << endl;
    return 0;
}

void dump_hex(uint8_t *str, long len)
{
    cout << "dump string: ";
    for (int i = 0; i < len; i++)
    {
        char ss[4] = {0};
        sprintf(ss, " %02X", str[i]);
        cout << ss;
    }
    cout << endl;
}

int load_data_file(const char *file_path)
{
    int fd = open(file_path, O_RDWR);
    if (fd < 0)
    {
        cout << "No data file found, create one." << endl;
        return create_data_file(file_path);
    }

    // map the whole data file.
    //int len = lseek(fd, 0, SEEK_END);
    // map only header of data file;
    int len = sizeof(data_file);
    char *addr = (char *) mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    char *data = new char[128];
    memcpy(data, addr, len);
    munmap(addr, len);

    dump_hex((uint8_t *)data, len);

    return 0;
}

int main(int c, char **v)
{
    std::cout << "hello,world!" << std::endl;
    // load file
    load_data_file(DATA_FILE_PATH.c_str());
    return 0;
}

