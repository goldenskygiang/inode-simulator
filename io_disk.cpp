#include "inode_sim.h"

int read_block(int id, void* buf)
{
    if (id < 0 || id >= NUM_OF_BLOCKS)
    {
        cerr << "read_block: index out of bounds" << endl;
        return -1;
    }

    if (fseek(diskptr, id * BLOCK_SIZE_IN_BYTES, SEEK_SET) != 0)
    {
        cerr << "read_block: fseek failed" << endl;
        return -1;
    }

    int br = fread(buf, sizeof(char), BLOCK_SIZE_IN_BYTES, diskptr);
    if (br < 0)
    {
        cerr << "read_block: fread failed" << endl;
        return -1;
    }

    return br;
}

int write_block(int id, void* buf, int sz)
{
    if (id < 0 || id >= NUM_OF_BLOCKS)
    {
        cerr << "write_block: index out of bounds" << endl;
        return -1;
    }

    if (fseek(diskptr, id * BLOCK_SIZE_IN_BYTES, SEEK_SET) != 0)
    {
        cerr << "write_block: fseek failed" << endl;
        return -1;
    }

    int bw = fwrite(buf, sizeof(char), sz, diskptr);
    if (bw < 0)
    {
        cerr << "write_block: fwrite failed" << endl;
        return -1;
    }

    return bw;
}

void save_disk_meta()
{
    fseek(diskptr, 0, SEEK_SET);
    int len = sizeof(struct diskman);
    char buff[len];
    memset(buff, 0, len);
    memcpy(buff, &diskinfo, len);
    fwrite(buff, sizeof(char), len, diskptr);

    fseek(diskptr, diskinfo.diskman_blocks * BLOCK_SIZE_IN_BYTES, SEEK_SET);
    len = sizeof(file_inode_mapping_arr);
    char map_buff[len];
    memset(map_buff, 0, len);
    memcpy(map_buff, file_inode_mapping_arr, len);
    fwrite(map_buff, sizeof(char), len, diskptr);

    fseek(diskptr, diskinfo.inode_index_start * BLOCK_SIZE_IN_BYTES, SEEK_SET);
    len = sizeof(inode_arr);
    char inode_buff[len];
    memset(inode_buff, 0, len);
    memcpy(inode_buff, inode_arr, len);
    fwrite(inode_buff, sizeof(char), len, diskptr);
}

void load_disk_meta()
{
    fseek(diskptr, 0, SEEK_SET);
    int len = sizeof(struct diskman);
    char buff[len];
    memset(buff, 0, len);
    fread(buff, sizeof(char), len, diskptr);
    memcpy(&diskinfo, buff, len);

    fseek(diskptr, diskinfo.diskman_blocks * BLOCK_SIZE_IN_BYTES, SEEK_SET);
    len = sizeof(file_inode_mapping_arr);
    char map_buff[len];
    memset(map_buff, 0, len);
    fread(map_buff, sizeof(char), len, diskptr);
    memcpy(file_inode_mapping_arr, map_buff, len);

    fseek(diskptr, diskinfo.inode_index_start * BLOCK_SIZE_IN_BYTES, SEEK_SET);
    len = sizeof(inode_arr);
    char inode_buff[len];
    memset(inode_buff, 0, len);
    fread(inode_buff, sizeof(char), len, diskptr);
    memcpy(inode_arr, inode_buff, len);
}

int reserve_data_block()
{
    if (buff_free_datablocks.empty())
    {
        return NULL_BLOCK;
    }

    int id = buff_free_datablocks.front();
    buff_free_datablocks.pop_front();
    diskinfo.datablock_free[id] = false;

    char empty_buf[BLOCK_SIZE_IN_BYTES];
    memset(empty_buf, 0, BLOCK_SIZE_IN_BYTES);
    write_block(id, empty_buf);

    save_disk_meta();

    return id;
}

void release_data_block(int id)
{
    if (diskinfo.datablock_free[id]) return;

    diskinfo.datablock_free[id] = true;
    buff_free_datablocks.push_back(id);

    save_disk_meta();
}