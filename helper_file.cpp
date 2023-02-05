#include "inode_sim.h"

bool is_valid_filename(string filename)
{
    if (filename.empty()) return false;
    if (filename.length() > MAX_FILENAME_LENGTH) return false;
    if (filename == ".") return false;

    for (const char &c : RESTRICTED_CHARS_FILENAME)
    {
        if (filename.find(c) != filename.npos) return false;
    }

    if (filename.back() == '.') return false;

    return true;
}

bool file_exists(string filename)
{
    return buff_file_to_inode_map.find(filename) != buff_file_to_inode_map.end();
}