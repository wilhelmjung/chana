#include <cinttypes>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

using namespace std;

extern int test_disk_file();
extern void dump_hex(uint8_t *bs, long len);

const int ORDER = 1024;

struct data_file
{
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
struct node *root_node;

// create empty data file.
int create_data_file(const char *file_path)
{
    fstream fs;
    fs.open(file_path, ios::out | ios::binary);
	if (!fs)
    {
		cout << "Failed to create file: " << file_path << endl;
        return -1;
	}
    // write file header
    struct data_file head =
    {
        .head = ('d'<<24)+('a'<<16)+('t'<<8)+'a',
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
    char *addr = (char *) mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct data_file *df = (struct data_file *) addr;
    if (df->type == 0)
        df->type = 1;
    df->size += 16;

    dump_hex((uint8_t *)addr, len);

    close(fd);
    return 0;
}

int main(int c, char **v)
{
    (void)test_disk_file();
    return 0;
}

