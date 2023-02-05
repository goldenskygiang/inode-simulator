# Inode simulator

An Inode simulator written in C++. Feel like a real terminal yet?

## Build

Install the C++ build tools. In Ubuntu, you can

`sudo apt install build-essential make`

Run the Makefile in this repository, then

`./inode_sim`

Create a new 128MB virtual disk or mount an existing one to get started.

## Supported commands
`ls` - list all files

`cat [filename]` - read file text content

`nano [filename]` - create new file or append text to existing file

`stat [filename]` - show filestat

`rm [filename]` - remove (unlink) a file from the virtual disk

`exit` or `umount` - unmount the current virtual disk and exit the simulator

## Limitations

- ~Theoretically it can work properly up to approx. 40MB of file size~. Text content limit up to 4096 bytes due to `getline()`. Haven't tried larger file size yet.
- No directory. Just files right at the root directory.
- You can only add only one line of text content to the file. I don't intend to make a fully-featured text editor.
