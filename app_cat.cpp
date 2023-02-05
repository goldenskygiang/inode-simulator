#include "api_file.h"

void cat(vector<string> args)
{
    if (args.size() < 2)
    {
        cerr << "cat: too few parameters (must have filename)" << endl;
        return;
    }
    else if (args.size() > 2)
    {
        cerr << "cat: too many parameters" << endl;
        return;
    }

    string filename = args[1];

    if (filename == "--help")
    {
        cout << "cat [filename]: to read filename's content." << endl;
        return;
    }

    int fd = open_file(filename);
    if (fd < 0)
    {
        cerr << "cat: file not found" << endl;
        return;
    }

    string content = read_file(fd);
    cout << content << endl;
    close_file(fd);
}