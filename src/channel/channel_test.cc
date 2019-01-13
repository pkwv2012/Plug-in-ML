// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#include "src/channel/fifo.h"
#include "src/channel/shared_memory.h"

#include <iostream>

using namespace rpscc;

int main() {
  const int kvmax = 4096;
  pid_t pid;
  const int kflag = 0666;
  std::string filename1 = "/tmp/test_fifo1";
  std::string filename2 = "/tmp/test_fifo2";
  std::string ipc_name1 = "/test_sharedMemory1";
  std::string ipc_name2 = "/test_sharedMemory2";
  mkfifo(filename1.c_str(), kflag);
  mkfifo(filename2.c_str(), kflag);
  if ((pid = fork()) == 0) {  // the child process
    std::cout << "child" << std::endl;
    Fifo fifo1, fifo2;
    fifo1.Initialize(filename1, true);
    fifo2.Initialize(filename2, false);
    fifo1.Open();
    fifo2.Open();
    SharedMemory read_mem, write_mem;
    read_mem.Initialize(ipc_name1.c_str());
    write_mem.Initialize(ipc_name2.c_str());
    std::cout << "child" << std::endl;
    shmstruct store_data;
    store_data.values[0] = 13.3;
    store_data.keys[0] = 1111L;
    fifo1.Wait();
    shmstruct* data = read_mem.Read();
    std::cout << "child first read " <<  data->keys[0] << std::endl;
    write_mem.Write(&store_data);
    fifo2.Signal();
    fifo1.Wait();
    data = read_mem.Read();
    std::cout << "child second read " << data->values[1] << std::endl;
    exit(0);
  }
  std::cout << "parent" << std::endl;
  Fifo fifo1, fifo2;
  fifo1.Initialize(filename1, false);
  fifo2.Initialize(filename2, true);
  fifo1.Open();
  fifo2.Open();
  SharedMemory read_mem, write_mem;
  read_mem.Initialize(ipc_name2.c_str());
  write_mem.Initialize(ipc_name1.c_str());
  shmstruct store_data;
  store_data.values[0] = 33.3;
  store_data.keys[0] = 3111L;
  store_data.size = 1;
  write_mem.Write(&store_data);
  std::cout << "parent write finish" << std::endl;
  fifo1.Signal();
  fifo2.Wait();
  shmstruct* data = read_mem.Read();
  std::cout << "parent : " << data->keys[0] << std::endl;
  store_data.values[1] = 323123.3;
  store_data.keys[1] = 32312;
  store_data.size = 2;
  write_mem.Write(&store_data);
  fifo1.Signal();
  exit(0);
}
