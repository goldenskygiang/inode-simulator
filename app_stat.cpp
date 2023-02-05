#include <time.h>

#include "api_file.h"

string get_formatted_time(timeval t)
{
    char buf[35], milisec[9];
    auto tm = localtime(&t.tv_sec);

    strftime(buf, 26, "%Y-%m-%d %H:%M:%S.", tm);
    sprintf(milisec, "%d", (int)t.tv_usec);

    strcat(buf, milisec);
    return string(buf);
}

void statp(vector<string> args)
{
    if (args.size() < 2)
    {
        cerr << "stat: too few parameters (must have filename)" << endl;
        return;
    }
    else if (args.size() > 2)
    {
        cerr << "stat: too many parameters" << endl;
        return;
    }

    string filename = args[1];
    if (filename == "--help")
    {
        cout << "stat [filename]: show filestat." << endl;
        return;
    }

    int fd = open_file(filename);
    if (fd == ERR_FILE_NOT_FOUND)
    {
        cerr << "stat: file not found." << endl;
        return;
    }
    else if (fd == ERR_UNKNOWN)
    {
        cerr << "stat: unknown error occured." << endl;
        return;
    }

    cout << "File: " << filename << "\n";
    cout << "Size: " << get_filesize(fd) << "\t Blocks: " << get_num_blocks(fd) << "\t regular file\n";
    cout << "Inode: " << get_inode(fd) << "\n";

    cout << "Access: " << get_formatted_time(get_access_time(fd)) << "\n";
    cout << "Modify: " << get_formatted_time(get_modify_time(fd)) << "\n";
    cout << "Birth: " << get_formatted_time(get_birth_time(fd));

    cout << endl;

    close_file(fd);
}