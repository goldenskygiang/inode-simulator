#include "inode_sim.h"

void ls(vector<string> args)
{
    for (auto &p : file_inode_mapping_arr)
    {
        if (p.inode_num > 0) cout << p.file_name << "\t";
    }

    cout << endl;
}