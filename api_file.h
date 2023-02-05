#include <bits/stdc++.h>
#include <sys/time.h>
#include <time.h>

using namespace std;

#define ERR_FILE_NOT_FOUND -1
#define ERR_FILE_EXISTS -2
#define ERR_NO_FD -3
#define ERR_UNKNOWN -4

int create_file(string filename);
int open_file(string filename);

string read_file(int fd);
int write_file(int fd, string content);
int close_file(int fd);
int unlink_file(int fd);

int get_filesize(int fd);
int get_num_blocks(int fd);
int get_inode(int fd);

timeval get_access_time(int fd);
timeval get_modify_time(int fd);
timeval get_birth_time(int fd);

bool is_valid_filename(string filename);
bool file_exists(string filename);