#include <cinttypes>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

using namespace std;

extern int test_disk_file();
extern void dump_hex(uint8_t *, long);

const int ORDER = 1024;
const size_t PAGE_SIZE = 4 * 1024; // 4KB
const size_t MAX_FILE_SIZE = 1*1024*1024*1024; // 4GB

//0. index node, store leaf node and internal node for btree.
class node
{
public:
    uint32_t head;              // "node"
    uint32_t type;              // 1:internal node; 0:leaf node;
    uint64_t keys[ORDER - 1];   // for now only long int is supported.
    uint64_t ptrs[ORDER];       // pointer to child node.
    uint64_t vals[0];           // value array, optional for leaf node.
    //TODO: variable length key or value is to be supported.
    //currently, key and value are 64-bit integers.
    node()
    {
        string h = "node";
        head = (h[0]<<24)+(h[1]<<16)+(h[2]<<8)+h[3];
    }
    static node* make_node(bool root, bool leaf);
    bool is_root_node();
    bool is_leaf_node();
    bool is_internal_node();
};
node *root_node;

node *node::make_node(bool root, bool leaf)
{
    size_t node_size = sizeof(node);
    // leaf node, add extra value array.
    if (leaf) { node_size += sizeof(uint64_t) * ORDER; }
    char *buffer = new char[node_size];
    node *n = new(buffer)node;
    if (root) { n->type |= 1<<31; }
    // internal node
    if (!leaf) { n->type |= 1; }
    return n;
}

//1. node manager;
class node_manager
{
public:
    uint32_t head;              // "inod" for index node; "dnod" for data node;
    uint32_t type;              // 1:index; 2:data;
    uint32_t node_size;         // node size;
    uint32_t capacity;          // max size;
    uint32_t allocated;         // allocated size;
    uint32_t node_end;          // end of node;
    uint32_t data[0];           // nodes, append only.

    static node_manager *load_file(string name);
    int unload_file();
    node_manager() {}
    node_manager(string name, bool for_index, int node_size, int capacity);
    // align with page size.
    size_t get_node_start() { return PAGE_SIZE; }
    int add_node(char *node, size_t size);
};

node_manager::node_manager(string name, bool for_index, int node_size, int capacity)
{
    this->head = (name[0]<<24)+(name[1]<<16)+(name[2]<<8)+name[3];
    this->type = for_index ? 1 : 2;
    this->node_size = node_size;
    this->capacity  = capacity;
    this->node_end  = get_node_start();
    this->allocated = get_node_start();
    // serialize
    fstream fs;
    fs.open(name.c_str(), ios::binary | ios::out);
    if (!fs)
    {
        cout << "file: " << name << ", open failed!" << endl;
        exit(-1);
    }
    fs.write((char *)this, this->allocated);
    fs.close();
}

// grow file
void *grow_file(int fd, char *buffer, size_t old_size, size_t add_size)
{
    size_t new_size = old_size + add_size;
    if (ftruncate(fd, new_size) == -1)
    {
        perror("Error in ftruncate");
        return NULL;
    }

    void *temp = mremap(buffer, old_size, new_size, MREMAP_MAYMOVE);
    if (temp == (void *)-1)
    {
        perror("Error on mremap()");
        return NULL;
    }
    return temp;
}

//int
//node_manager::append_page()
//{
//}

int
node_manager::add_node(char *node, size_t size)
{
    //if (this->allocated)
    return 0;
}

// read-only
char *map_read(string file_name, int& file_len)
{
    int fd = open(file_name.c_str(), O_RDONLY);
    int len = lseek(fd, 0, SEEK_END);
    char *buffer = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    //  do something
    char *res = (char *)calloc(1, len);
    memcpy(res, buffer, len);
    munmap(buffer, len);
    file_len = len;
    return res;
}

// read-write
char *map_file_begin(string file_name, size_t& len)
{
    int fd = open(file_name.c_str(), O_RDWR);
    len = lseek(fd, 0, SEEK_END);
    char *buffer = (char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, 0);
    close(fd);
    return buffer;
}

int map_file_end(char *buffer, size_t len)
{
    return munmap(buffer, len);
}

int map_file_sync(char *buffer, size_t len)
{
    return msync(buffer, len, MS_SYNC);
}

node_manager *
node_manager::load_file(string name)
{
    size_t len = 0;
    char *buf = map_file_begin(name, len);
    if (buf == NULL || len == 0)
    {
        cout << "mmap file is emtpy." << endl;
        exit(-2);
    }
    node_manager *man = (node_manager *)buf;
    if (man->allocated != len)
    {
        cout << "allocated: " << man->allocated << ", len: " << len << endl;
    }
    man->allocated = len;
    return man;
}

int
node_manager::unload_file()
{
    char *buffer = (char *)this;
    size_t len = this->allocated;
    return map_file_end(buffer, len);
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
int 
init_node_managers()
{
    node_manager *iman, *dman;
    iman = new node_manager(index_node_file, 1, sizeof(node), 0);
    dman = new node_manager(data_node_file,  2, sizeof(node), 0);
    delete iman;
    delete dman;
    return 0;
}

int 
load_node_managers()
{
    node_manager *iman, *dman;
    iman = node_manager::load_file(index_node_file);
    dman = node_manager::load_file(data_node_file);

    cout << "idx type: "  << iman->type << endl;
    cout << "dat type: "  << dman->type << endl;
    cout << "idx alloc: " << iman->allocated << endl;
    cout << "dat alloc: " << dman->allocated << endl;

    iman->unload_file();
    dman->unload_file();
    return 0;
}

int
main(int c, char **v)
{
    (void)test_disk_file();
    init_node_managers();
    load_node_managers();
    return 0;
}
