#include <cinttypes>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

using namespace std;

extern int test_disk_file();

const int ORDER = 1024;

struct node
{
    uint32_t head;              // "node"
    uint32_t type;              // 0:root; 1:stem(internal node); 2:leaf
    uint64_t keys[ORDER - 1];   // TODO: for now only long int is supported.
    uint64_t ptrs[ORDER];       // ptr to child node.
    uint64_t vals[0];           // TODO: value array or ptr to real value string, optional for leaf node.
};
struct node *root_node;

int main(int c, char **v)
{
    (void)test_disk_file();
    return 0;
}

