CC = g++
CFLAGS = -Wall -std=c++14 -g
DEPS = inode_sim.h
OBJ = io_disk.o io_file.o helper_file.o app_cat.o app_ls.o app_nano.o app_rm.o app_stat.o inode_sim.o

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

inode_sim: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -rf *.o inode_sim

cleano:
	rm -rf *.o