#include "api_file.h"

void rm(vector<string> args)
{
    if (args.size() < 2)
    {
        cerr << "rm: too few parameters (must have filename)" << endl;
        return;
    }
    else if (args.size() > 2)
    {
        cerr << "rm: too many parameters" << endl;
        return;
    }

    string filename = args[1];
    if (filename == "--help")
    {
        cout << "rm [filename]: remove (unlink) filename from the filesystem." << endl;
        return;
    }

    int fd = open_file(filename);

    if (fd == ERR_FILE_NOT_FOUND)
    {
        cerr << "rm: file \"" << filename << "\" not found." << endl;
        return;
    }

    // removing a file is to simply unlink its inode with its datablocks
    // then remove the inode itself
    unlink_file(fd);
}