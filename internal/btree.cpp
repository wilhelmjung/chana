#include <cinttypes>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "node.cpp"
#include "frag.cpp"

using namespace std;

typedef uint64_t KeyType;
typedef uint64_t ValType;

class btree
{
public:
    node *root;
    node_manager *idx_man;
    node_manager *dat_man;
    btree()
    {
        //
    }
    // set/insert will overwritten old value.
    int set(KeyType key, ValType  val);
    int get(KeyType key, ValType& val);
    // delete is not supported.
    int del(KeyType key) { return -1; }
};

int btree::set(KeyType key, ValType  val)
{
    return -1;
}

int btree::get(KeyType key, ValType& val)
{
    return -1;
}
