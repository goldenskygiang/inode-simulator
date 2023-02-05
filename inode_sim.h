#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/time.h>

using namespace std;

// default disk size 128MB = 2^7 * 2^10 * 2^10 MB
#define DISK_SIZE_IN_BYTES (1 << 27)
// default block size 4096 bytes
#define BLOCK_SIZE_IN_BYTES (1 << 12)
// number of disk blocks
#define NUM_OF_BLOCKS (DISK_SIZE_IN_BYTES / BLOCK_SIZE_IN_BYTES)
// number of inodes (50% number of disk blocks)
#define NUM_OF_INODES (NUM_OF_BLOCKS >> 1)
// number of file descriptors
#define NUM_OF_FD 32

#define MAX_FILENAME_LENGTH 30

#define NULL_BLOCK -1

const string RESTRICTED_CHARS_FILENAME = "\"*/:<>?\\|+,;=[]";

struct inode
{
    int filesize;
    int pointer[12]; /* 10 direct pointers, 1 single indirect, 1 double indirect pointer */
    timeval access, modify, birth;
};

struct file_to_inode_mapping // total size = 36Byte
{
    char file_name[30]; // filename
    int inode_num;      // inode number
};

struct diskman
{
    int diskman_blocks = ceil(((float)sizeof(diskman) / BLOCK_SIZE_IN_BYTES));
    int file_inode_map_blocks = ceil(((float)sizeof(file_to_inode_mapping) / BLOCK_SIZE_IN_BYTES));

    int inode_index_start = diskman_blocks + file_inode_map_blocks;
    int inode_blocks = ceil(((float)NUM_OF_INODES * sizeof(inode) / BLOCK_SIZE_IN_BYTES));

    int data_index_start = inode_index_start + inode_blocks;

    bool inode_free[NUM_OF_INODES];
    bool datablock_free[NUM_OF_BLOCKS];
};

extern FILE *diskptr;

extern struct diskman diskinfo;
extern struct file_to_inode_mapping file_inode_mapping_arr[NUM_OF_INODES];
extern struct inode inode_arr[NUM_OF_INODES];

extern string buff_inode_to_file_map[NUM_OF_INODES];
extern map<string, int> buff_file_to_inode_map;
extern deque<int> buff_free_inodes, buff_free_datablocks;
extern deque<int> buff_free_fd;
extern int buff_fd_to_inode_map[NUM_OF_FD];

void ls(vector<string> args);
void nanop(vector<string> args);
void cat(vector<string> args);
void rm(vector<string> args);
void statp(vector<string> args);

int read_block(int id, void* buf);
int write_block(int id, void* buf, int sz = BLOCK_SIZE_IN_BYTES);
int erase_block();

int reserve_data_block();
void release_data_block(int id);

void save_disk_meta();
void load_disk_meta();