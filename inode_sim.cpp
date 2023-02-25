#include "inode_sim.h"
#include "color.h"
#include <unistd.h>

FILE *diskptr;

struct diskman diskinfo;
struct file_to_inode_mapping file_inode_mapping_arr[NUM_OF_INODES];
struct inode inode_arr[NUM_OF_INODES];

string buff_inode_to_file_map[NUM_OF_INODES];
map<string, int> buff_file_to_inode_map;
deque<int> buff_free_inodes, buff_free_datablocks;

deque<int> buff_free_fd;
int buff_fd_to_inode_map[NUM_OF_FD];

void create_disk()
{
    cout << "Disk name without space: ";
    cout.flush();

    string name;
    cin >> name;

    if (access(name.c_str(), F_OK) != -1)
    {
        cout << "Error: Disk named \"" << name << "\" already exists!";
        exit(-1);
    }

    cout << "Creating a 128MB \"" << name << "\" virtual disk..." << endl;

    diskptr = fopen(name.c_str(), "wb");

    char buffer[BLOCK_SIZE_IN_BYTES];
    memset(buffer, 0, BLOCK_SIZE_IN_BYTES);
    for (int i = 0; i < NUM_OF_BLOCKS; i++)
    {
        fwrite(buffer, 1, BLOCK_SIZE_IN_BYTES, diskptr);
    }

    diskinfo = diskman();

    // reserved for diskinfo
    for (int i = 0; i < diskinfo.data_index_start; i++) diskinfo.datablock_free[i] = false;
    for (int i = diskinfo.data_index_start; i < NUM_OF_BLOCKS; i++) diskinfo.datablock_free[i] = true;
    for (int i = 1; i < NUM_OF_INODES; i++) diskinfo.inode_free[i] = true;

    for (int i = 0; i < NUM_OF_INODES; i++)
    {
        for (int j = 0; j < 12; j++) inode_arr[i].pointer[j] = NULL_BLOCK;
    }

    save_disk_meta();

    cout << "Virtual disk \"" << name << "\" created!" << endl;
    fclose(diskptr);

    return;
}

int mount_disk()
{
    cout << "Disk name without space: ";
    cout.flush();

    string name;
    cin >> name;

    diskptr = fopen(name.c_str(), "rb+");

    if (diskptr == NULL)
    {
        cerr << "Error: Disk named \"" << name << "\" not found!";
        return -1;
    }

    load_disk_meta();

    // load metadata into buffer objects
    for (int i = 1; i < NUM_OF_INODES; i++)
    {
        if (!diskinfo.inode_free[i])
        {
            string filename = string(file_inode_mapping_arr[i].file_name);
            buff_file_to_inode_map[filename] = i;
            buff_inode_to_file_map[i] = filename;
        }
        else buff_free_inodes.push_back(i);
    }

    for (int i = 0; i < NUM_OF_BLOCKS; i++)
    {
        if (diskinfo.datablock_free[i])
        {
            buff_free_datablocks.push_back(i);
        }
    }

    for (int i = 0; i < NUM_OF_FD; i++)
    {
        buff_free_fd.push_back(i);
        buff_fd_to_inode_map[i] = 0;
    }

    cout << "Virtual disk \"" << name << "\" mounted!" << endl;
    return 2;
}

void unmount_disk()
{
    save_disk_meta();
    fclose(diskptr);

    // free all buffer
    buff_file_to_inode_map.clear();
    buff_free_datablocks.clear();
    buff_free_fd.clear();
    buff_free_inodes.clear();

    cout << "Disk unmounted successfully. Program exit." << endl;
    
    exit(0);
}

void handle_user_commands()
{
    cin.ignore();
    
    while (!cin.eof())
    {
        cout << GREEN  << "you@localhost ~> " << COLOR_OFF;

        string cmd;
        getline(cin, cmd);

        istringstream iss(cmd);
        vector<string> args;
        while (iss >> cmd) args.push_back(cmd);

        if (args.size() == 0) continue;

        if (args[0] == "ls") ls(args);
        else if (args[0] == "cat") cat(args);
        else if (args[0] == "nano") nanop(args);
        else if (args[0] == "rm") rm(args);
        else if (args[0] == "stat") statp(args);
        else if (args[0] == "umount" || args[0] == "exit") break;
        else
        {
            cout << "Error: command \"" << args[0] << "\" not found!" << endl;
        }
    }

    unmount_disk();
}

int main()
{
    int choice = 0;
    while (choice < 1 || choice > 3)
    {
        cout << "(1) Create disk" << endl;
        cout << "(2) Mount disk" << endl;
        cout << "(3) Exit" << endl;

        cin >> choice;
        switch (choice)
        {
            case 1:
                create_disk();
                choice = 0;
                break;
            case 2:
                choice = mount_disk();
                break;
            case 3:
                return 0;
                break;
            default:
                cout << "Invalid choice, please type again: ";
                cout.flush();
                break;
        }
    }

    handle_user_commands();
    return 0;
}
