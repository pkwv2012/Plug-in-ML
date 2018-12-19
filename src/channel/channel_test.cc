// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#include "src/channel/fifo.h"
#include "src/channel/sharedmemory.h"

#include <iostream>

namespace rpscc {

int main() {
  pid_t pid;
  const int kflag = 0666;
  std::string filename1 = "/tmp/test_fifo1";
  std::string filename2 = "/tmp/test_fifo2";
  std::string ipc_name1 = "test1";
  std::string ipc_name2 = "test2";
  mkfifo(filename1.c_str(), kflag);
  mkfifo(filename2.c_str(), kflag);
  shm_unlink(ipc_name1.c_str());
  int fd = shm_open(ipc_name1.c_str(), O_RDWR | O_CREAT, FILE_MODE);
  ftruncate(fd, sizeof(struct shmstruct*));
  close(fd);
  fd = shm_open(ipc_name2.c_str(), O_RDWR | O_CREAT, FILE_MODE);
  ftruncate(fd, sizeof(struct shmstruct*));
  close(fd);
  if ((pid = fork()) == 0) {  // the child process
    Fifo fifo1, fifo2;
    fifo1.Initialize(filename1, true);
    fifo2.Initialize(filename2, false);
    SharedMemory read_mem, write_mem;
    read_mem.Initialize(ipc_name1.c_str());
    write_mem.Initialize(ipc_name2.c_str());
    shmstruct store_data;
    store_data.values.push_back(13.3);
    store_data.keys.push_back(1111L);
    fifo1.Wait();
    std::cout << "child read fifo." << std::endl;
    shmstruct* data = read_mem.Read();
    std::cout << data->keys.size();
    write_mem.Write(&store_data);
    fifo2.Signal();
    fifo1.Wait();
    std::cout << data->keys.size();
    exit(0);
  }
  Fifo fifo1, fifo2;
  fifo1.Initialize(filename1, false);
  fifo2.Initialize(filename2, true);
  SharedMemory read_mem, write_mem;
  read_mem.Initialize(ipc_name2.c_str());
  write_mem.Initialize(ipc_name1.c_str());
  shmstruct store_data;
  store_data.values.push_back(33.3);
  store_data.keys.push_back(3111L);
  write_mem.Write(&store_data);
  fifo1.Signal();
  fifo2.Wait();
  shmstruct* data = read_mem.Read();
  std::cout << "parent : " << data->keys.size() << std::endl;
  store_data.values.push_back(323123.3);
  store_data.keys.push_back(32312);
  write_mem.Write(&store_data);
  fifo1.Signal();
  exit(0);
}

}