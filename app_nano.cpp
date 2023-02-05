#include "api_file.h"

string user_input()
{
    string input;
    getline(cin, input);
    return input;
}

void nanop(vector<string> args)
{
    if (args.size() < 2)
    {
        cerr << "nano: too few parameters (must have filename)" << endl;
        return;
    }
    else if (args.size() > 2)
    {
        cerr << "nano: too many parameters" << endl;
        return;
    }

    string filename = args[1];
    if (filename == "--help")
    {
        cout << "nano [filename]: to create or append filename's content." << endl;
        return;
    }

    if (!is_valid_filename(filename))
    {
        cerr << "nano: invalid filename" << endl;
        return;
    }

    int fd = open_file(filename);
    string content = "";

    if (fd == ERR_FILE_NOT_FOUND)
    {
        cout << "Enter content to NEW file \"" << filename << "\" in 1 line below (Enter to save):" << endl;
        content = user_input();

        cout << "Creating new file " << filename << "..." << endl;
        if (create_file(filename) < 0)
        {
            cerr << "nano: create file failed" << endl;
            return;
        }

        fd = open_file(filename);
    }
    else
    {
        content = read_file(fd);
        cout << "Append content to file \"" << filename << "\" in 1 line below (Enter to save):" << endl;
        cout << content;

        string append = user_input();
        content += append;
    }

    int bytes_written = write_file(fd, content);

    if (bytes_written < 0)
    {
        cerr << "nano: write file failed." << endl;
        return;
    }

    cout << "nano: " << bytes_written << " bytes written." << endl;
    close_file(fd);
}