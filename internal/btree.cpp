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

//0. index node, store leaf node and internal node for btree.
class node
{
public:
    uint32_t head;              // "node"
    uint32_t type;              // 2:root node; 1:internal node; 0:leaf node;
    uint64_t keys[ORDER - 1];   // for now only long int is supported.
    uint64_t ptrs[ORDER];       // pointer to child node.
    uint64_t vals[0];           // value array, optional for leaf node.
    //TODO: variable length key or value is to be supported.
    //currently, key and value are 64-bit integers.
};
node *root_node;

//1. node manager;
class node_manager
{
public:
    uint32_t head;              // "inod" for index node; "dnod" for data node;
    uint32_t type;              // 1:index;2:data;
    uint32_t node_size;         // node size;
    uint32_t capacity;          // max node number;
    uint32_t allocated;         // allocated nodes;
    uint32_t data[0];           // nodes, append only.

    node_manager(){}
    node_manager(string name, int type, int sz, int cap);
};

node_manager::node_manager(string name, int type, int node_size, int capacity)
{
    this->head = (name[0]<<24)+(name[1]<<16)+(name[2]<<8)+name[3];
    this->type = type;
    this->node_size = node_size;
    this->capacity  = capacity;
    // serialize
    fstream fs;
    fs.open(name.c_str(), ios::binary | ios::out);
    if (!fs)
    {
        cout << "file: " << name << ", open failed!" << endl;
        exit(-1);
    }
    fs.write((char *)this, sizeof(*this) + capacity);
    fs.close();
}

//2. index node manager.
const string index_node_file = "inode.bin";
node_manager *inode_manager = NULL;

//3. data  node manager.
const string data_node_file  = "dnode.bin";
node_manager *dnode_manager = NULL;

//TODO 4. page manager.
const string page_file = "page.bin";

// init both inode/dnode managers/files.
int init_node_managers()
{
    node_manager *iman, *dman;
    iman = new node_manager(index_node_file, 1, sizeof(node), 0);
    dman = new node_manager(data_node_file,  2, sizeof(node), 8*1024);
    delete iman;
    delete dman;
    return 0;
}

int main(int c, char **v)
{
    (void)test_disk_file();
    init_node_managers();
    return 0;
}

