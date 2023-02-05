#include "inode_sim.h"
#include "api_file.h"

int create_file(string filename)
{
    if (file_exists(filename))
    {
        cerr << "create_file: file already exists" << endl;
        return ERR_FILE_EXISTS;
    }

    if (buff_free_inodes.empty())
    {
        cerr << "create_file: no free inodes available" << endl;
        return ERR_UNKNOWN;
    }

    int inode_id = buff_free_inodes.front();
    buff_free_inodes.pop_front();

    int datablock_id = reserve_data_block();
    if (datablock_id < 0)
    {
        cerr << "create_file: no free data block available." << endl;
        return ERR_UNKNOWN;
    }

    diskinfo.inode_free[inode_id] = false;
    inode_arr[inode_id].pointer[0] = datablock_id;
    inode_arr[inode_id].filesize = 0;
    gettimeofday(&inode_arr[inode_id].birth, NULL);

    file_inode_mapping_arr[inode_id].inode_num = inode_id;
    strcpy(file_inode_mapping_arr[inode_id].file_name, filename.c_str());

    diskinfo.datablock_free[datablock_id] = false;

    buff_inode_to_file_map[inode_id] = filename;
    buff_file_to_inode_map[filename] = inode_id;

    save_disk_meta();

    return inode_id;
}

int open_file(string filename)
{
    if (!file_exists(filename))
    {
        cerr << "open_file: file not found" << endl;
        return ERR_FILE_NOT_FOUND;
    }

    if (buff_free_fd.empty())
    {
        cerr << "open_file: no file descriptor available" << endl;
        return ERR_NO_FD;
    }

    int fd = buff_free_fd.front();
    buff_free_fd.pop_front();
    buff_fd_to_inode_map[fd] = buff_file_to_inode_map[filename];

    return fd;
}

int close_file(int fd)
{
    for (int free_fd : buff_free_fd)
    {
        if (free_fd == fd)
        {
            cerr << "close_file: file descriptor not available" << endl;
            return ERR_NO_FD;
        }
    }

    buff_free_fd.push_back(fd);
    buff_fd_to_inode_map[fd] = 0;
    return 0;
}

int write_file(int fd, string content)
{
    if (buff_fd_to_inode_map[fd] == 0)
    {
        cerr << "write_file: file descriptor not available" << endl;
        return ERR_NO_FD;
    }

    const char* buf_str = content.c_str();
    int buf_offset = 0;

    int blocks = content.size() / BLOCK_SIZE_IN_BYTES + (content.size() % BLOCK_SIZE_IN_BYTES != 0);

    int inode_id = buff_fd_to_inode_map[fd];

    inode_arr[inode_id].filesize = content.size();

    // write to the first <= 10 direct blocks
    for (int i = 0; i < min(10, blocks); i++)
    {
        int block_id = inode_arr[inode_id].pointer[i];
        if (block_id == NULL_BLOCK)
        {
            // assign new block
            block_id = reserve_data_block();
            if (block_id < 0)
            {
                cerr << "write_file: reserve data block failed" << endl;
                return ERR_UNKNOWN;
            }

            inode_arr[inode_id].pointer[i] = block_id;
        }

        int cpy_sz = min((unsigned long) BLOCK_SIZE_IN_BYTES, content.size() - buf_offset);

        char buf_block[BLOCK_SIZE_IN_BYTES];
        memset(buf_block, 0, BLOCK_SIZE_IN_BYTES);
        memcpy(buf_block, buf_str + buf_offset, cpy_sz);

        int bw = write_block(block_id, buf_block, cpy_sz);

        buf_offset += bw;
    }

    blocks -= min(blocks, 10);

    int si_block_num = BLOCK_SIZE_IN_BYTES / sizeof(int);
    int block_ptr[si_block_num];

    if (blocks > 0)
    {
        // single indirect pointer
        for (int &id : block_ptr) id = NULL_BLOCK;

        int si_block_id = inode_arr[inode_id].pointer[10];
        if (si_block_id == NULL_BLOCK)
        {
            si_block_id = reserve_data_block();
            if (si_block_id < 0)
            {
                cerr << "write_file: reserve data block failed" << endl;
                return ERR_UNKNOWN;
            }

            inode_arr[inode_id].pointer[10] = si_block_id;
            write_block(si_block_id, block_ptr);
        }
        else read_block(si_block_id, block_ptr);

        for (int i = 0; i < min(blocks, si_block_num); i++)
        {
            int block_id = block_ptr[i];

            if (block_id == NULL_BLOCK)
            {
                block_id = reserve_data_block();
                if (block_id < 0)
                {
                    cerr << "write_file: reserve block failed" << endl;
                    return ERR_UNKNOWN;
                }

                block_ptr[i] = block_id;
                write_block(si_block_id, block_ptr);
            }

            int cpy_sz = min((unsigned long) BLOCK_SIZE_IN_BYTES, content.size() - buf_offset);

            char buf_block[BLOCK_SIZE_IN_BYTES];
            memset(buf_block, 0, BLOCK_SIZE_IN_BYTES);
            memcpy(buf_block, buf_str + buf_offset, cpy_sz);

            int bw = write_block(block_id, buf_block, cpy_sz);

            buf_offset += bw;
        }

        blocks -= min(blocks, si_block_num);
    }

    if (blocks > 0)
    {
        // double indirect pointer
        for (int &id : block_ptr) id = NULL_BLOCK;

        int di_block_id = inode_arr[inode_id].pointer[11];
        if (di_block_id == NULL_BLOCK)
        {
            di_block_id = reserve_data_block();
            if (di_block_id < 0)
            {
                cerr << "write_file: reserve data block failed" << endl;
                return ERR_UNKNOWN;
            }

            inode_arr[inode_id].pointer[11] = di_block_id;
            write_block(di_block_id, block_ptr);
        }
        else read_block(di_block_id, block_ptr);

        for (int j = 0; j < si_block_num && blocks > 0; j++)
        {
            int lv1_id = block_ptr[j];
            int lv2_block_ptr[si_block_num];
            for (int &id : lv2_block_ptr) id = NULL_BLOCK;

            if (lv1_id == NULL_BLOCK)
            {
                lv1_id = reserve_data_block();
                if (lv1_id < 0)
                {
                    cerr << "write_file: reserve block failed" << endl;
                    return ERR_UNKNOWN;
                }

                block_ptr[j] = lv1_id;
                write_block(di_block_id, block_ptr);
                write_block(lv1_id, lv2_block_ptr);
            }
            else read_block(lv1_id, lv2_block_ptr);

            for (int i = 0; i < si_block_num && blocks > 0; i++, blocks--)
            {
                int lv2_id = lv2_block_ptr[i];
                if (lv2_id == NULL_BLOCK)
                {
                    lv2_id = reserve_data_block();
                    if (lv2_id < 0)
                    {
                        cerr << "write_file: reserve block failed" << endl;
                        return ERR_UNKNOWN;
                    }

                    lv2_block_ptr[i] = lv2_id;
                    write_block(lv1_id, lv2_block_ptr);
                }

                int cpy_sz = min((unsigned long) BLOCK_SIZE_IN_BYTES, content.size() - buf_offset);

                char buf_block[BLOCK_SIZE_IN_BYTES];
                memset(buf_block, 0, BLOCK_SIZE_IN_BYTES);
                memcpy(buf_block, buf_str + buf_offset, cpy_sz);

                int bw = write_block(lv2_id, buf_block, cpy_sz);

                buf_offset += bw;
            }
        }
    }

    gettimeofday(&inode_arr[inode_id].modify, NULL);
    save_disk_meta();

    if (blocks > 0)
    {
        cerr << "write_file: max file size exceeded" << endl;
        return ERR_UNKNOWN;
    }
    else if (buf_offset != content.size())
    {
        cerr << "write_file: unknown error occured. written " << buf_offset << "/" << content.size() << "bytes." << endl;
    }

    return buf_offset;
}

string read_file(int fd)
{
    if (buff_fd_to_inode_map[fd] == 0)
    {
        cerr << "read_file: file descriptor not available" << endl;
        return "\0";
    }

    int inode_id = buff_fd_to_inode_map[fd];
    
    int filesz = inode_arr[inode_id].filesize;
    string content = "";

    int eof = false;

    // direct pointers
    for (int i = 0; i < 10 && !eof; i++)
    {
        int block_id = inode_arr[inode_id].pointer[i];
        if (block_id == NULL_BLOCK)
        {
            eof = true;
            break;
        }

        char buffer[BLOCK_SIZE_IN_BYTES];
        memset(buffer, 0, BLOCK_SIZE_IN_BYTES);
        if (read_block(block_id, buffer) < 0)
        {
            cerr << "read_file: read block " << block_id << " failed." << endl;
            return "\0";
        }

        content += string(buffer);
    }

    int si_block_id = inode_arr[inode_id].pointer[10];
    eof = eof || (si_block_id == NULL_BLOCK);

    int si_block_num = BLOCK_SIZE_IN_BYTES / sizeof(int);

    if (!eof)
    {
        // single indirect pointer
        int block_ptr[si_block_num];
        for (int &id : block_ptr) id = NULL_BLOCK;

        read_block(si_block_id, block_ptr);

        for (int block_id : block_ptr)
        {
            if (block_id == NULL_BLOCK)
            {
                eof = true;
                break;
            }

            char buffer[BLOCK_SIZE_IN_BYTES];
            read_block(block_id, buffer);
            content += string(buffer);
        }
    }

    eof = eof || (inode_arr[inode_id].pointer[11] == NULL_BLOCK);
    if (!eof)
    {
        // double indirect pointer
        int lv1_block[si_block_id];
        for (int &id : lv1_block) id = NULL_BLOCK;

        read_block(inode_arr[inode_id].pointer[11], lv1_block);

        for (int lv1_block_id : lv1_block)
        {
            if (lv1_block_id == NULL_BLOCK)
            {
                eof = true;
                break;
            }

            int lv2_block[si_block_id];
            for (int &id : lv2_block) id = NULL_BLOCK;

            read_block(lv1_block_id, lv2_block);

            for (int lv2_block_id : lv2_block)
            {
                if (lv2_block_id == NULL_BLOCK)
                {
                    eof = true;
                    break;
                }

                char buffer[BLOCK_SIZE_IN_BYTES];
                memset(buffer, 0, BLOCK_SIZE_IN_BYTES);
                read_block(lv2_block_id, buffer);

                content += string(buffer);
            }
        }
    }

    if (content.size() != filesz)
    {
        cerr << "read_file: (warning) filesize mismatch. read " << content.size() << " bytes while inode data says " << filesz << "." << endl;
    }

    gettimeofday(&inode_arr[inode_id].access, NULL);
    save_disk_meta();

    return content;
}

int unlink_file(int fd)
{
    if (buff_fd_to_inode_map[fd] == 0)
    {
        cerr << "unlink_file: file descriptor not available" << endl;
        return -1;
    }

    int inode_id = buff_fd_to_inode_map[fd];
    string filename = file_inode_mapping_arr[inode_id].file_name;

    // dereference all data blocks pointers
    bool eof = false;
    for (int i = 0; i < 10 && !eof; i++)
    {
        int block_id = inode_arr[inode_id].pointer[i];
        if (block_id != NULL_BLOCK)
        {
            release_data_block(block_id);
        }
        else eof = true;
    }

    int si_block_id = inode_arr[inode_id].pointer[10];
    int si_block_num = BLOCK_SIZE_IN_BYTES / sizeof(int);

    eof = eof || (si_block_id != NULL_BLOCK);

    if (!eof)
    {
        // single indirect pointer
        int block_ptr[si_block_num];
        for (int &id : block_ptr) id = NULL_BLOCK;

        read_block(si_block_id, block_ptr);

        for (int &id : block_ptr)
        {
            if (id == NULL_BLOCK)
            {
                eof = true;
                break;
            }

            release_data_block(id);
        }

        release_data_block(si_block_id);
    }

    int lv1_block_id = inode_arr[inode_id].pointer[11];
    eof = eof || (lv1_block_id == NULL_BLOCK);

    if (!eof)
    {
        // double indirect pointer
        int lv1_block[si_block_num];
        for (int &id : lv1_block) id = NULL_BLOCK;

        read_block(lv1_block_id, lv1_block);

        for (int &lv1_id : lv1_block)
        {
            if (lv1_id == NULL_BLOCK)
            {
                eof = true;
                break;
            }

            int lv2_block[si_block_num];
            for (int &id : lv2_block) id = NULL_BLOCK;

            read_block(lv1_id, lv2_block);

            for (int &lv2_id: lv2_block)
            {
                if (lv2_id == NULL_BLOCK)
                {
                    eof = true;
                    break;
                }

                release_data_block(lv2_id);
            }

            if (eof) break;
        }
    }

    // delete from buffer
    buff_file_to_inode_map.erase(filename);
    buff_inode_to_file_map[inode_id] = "";

    // then delete the inode itself
    diskinfo.inode_free[inode_id] = true;
    file_inode_mapping_arr[inode_id].inode_num = 0;
    inode_arr[inode_id].filesize = 0;
    inode_arr[inode_id].pointer[0] = NULL_BLOCK;

    buff_free_inodes.push_back(inode_id);
    
    save_disk_meta();

    close_file(fd);
    return 0;
}

int get_filesize(int fd)
{
    int inode_id = get_inode(fd);
    if (inode_id <= 0)
    {
        cerr << "get_filesize: file descriptor not available" << endl;
        return -1;
    }
    
    return inode_arr[inode_id].filesize;
}

int get_num_blocks(int fd)
{
    if (buff_fd_to_inode_map[fd] == 0)
    {
        cerr << "get_num_block: file descriptor not available" << endl;
        return -1;
    }
    
    int filesize = get_filesize(fd);
    
    int blocks = filesize / BLOCK_SIZE_IN_BYTES + (filesize % BLOCK_SIZE_IN_BYTES != 0);
    return blocks;
}

int get_inode(int fd)
{
    if (buff_fd_to_inode_map[fd] == 0)
    {
        cerr << "get_inode: file descriptor not available" << endl;
        return -1;
    }

    return buff_fd_to_inode_map[fd];
}

timeval get_access_time(int fd)
{
    if (buff_fd_to_inode_map[fd] == 0)
    {
        cerr << "get_access_time: file descriptor not available" << endl;
        return timeval();
    }

    int inode_id = buff_fd_to_inode_map[fd];

    return inode_arr[inode_id].access;
}

timeval get_birth_time(int fd)
{
    if (buff_fd_to_inode_map[fd] == 0)
    {
        cerr << "get_birth_time: file descriptor not available" << endl;
        return timeval();
    }

    int inode_id = buff_fd_to_inode_map[fd];

    return inode_arr[inode_id].birth;
}

timeval get_modify_time(int fd)
{
    if (buff_fd_to_inode_map[fd] == 0)
    {
        cerr << "get_modify_time: file descriptor not available" << endl;
        return timeval();
    }

    int inode_id = buff_fd_to_inode_map[fd];

    return inode_arr[inode_id].modify;
}
