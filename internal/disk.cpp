#include <cinttypes>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

using namespace std;

const string disk_data_file_path = "data.bin";
const string disk_index_file_path = "index.bin";
const int max_file_size = 1024 * 1024 * 1024; // 1GB
const int DATA_FILE = 0;
const int INDEX_FILE = 1;

struct disk_file
{
    uint32_t head; // "data"
    uint32_t type; // 0:data; 1:index.
    uint32_t root; // root node offset, subject to change.
    uint32_t size; // size of data file.
};
struct disk_file *my_disk_file;

// for data.bin and index.bin files.
struct disk_file *disk_file_headers[2] = {NULL, NULL};

// utils:
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

// create empty data file.
int create_disk_file(const char *file_path, int file_type)
{
    fstream fs;
    fs.open(file_path, ios::out | ios::binary);
	if (!fs)
    {
		cout << "Failed to open file: " << disk_data_file_path << endl;
        return -1;
	}
    // write file header
    struct disk_file head =
    {
        .head = ('d'<<24)+('a'<<16)+('t'<<8)+'a',
        .type = file_type,
        .root = -1, // no root node has been created yet.
        .size = 0
    };
    char *bs = (char *)&head;
    long sz = sizeof(head);
    fs.write(bs, sz);
    cout << sz << " bytes written." << endl;
    return 0;
}

int load_disk_file(const char *file_path)
{
    int fd = open(file_path, O_RDWR);
    if (fd < 0)
    {
        cout << "No data file found, please create one." << endl;
        return -1;
    }

    // map the whole data file.
    //int len = lseek(fd, 0, SEEK_END);
 
    // map only header of data file;
    int len = sizeof(disk_file);
    char *addr = (char *) mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct disk_file *df = (struct disk_file *) addr;
    switch (df->type) {
    case 0:
        disk_file_headers[0] = df;
    break;
    case 1:
        disk_file_headers[1] = df;
    break;
    default:
        cout << "unknow file type: " << df->type << ", exit!" << endl;
        return -2;
    break;
    }
    df->size += 16;

    dump_hex((uint8_t *)addr, len);

    close(fd);
    return 0;
}

int test_disk_file()
{
    std::cout << "hello,world!" << std::endl;
    (void)create_disk_file(disk_data_file_path.c_str(), DATA_FILE);
    (void)load_disk_file(disk_data_file_path.c_str());
    return 0;
}
