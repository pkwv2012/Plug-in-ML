// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#include "src/channel/fifo.h"
#include "src/channel/shared_memory.h"

#include <iostream>
#include <cstring>
using namespace rpscc;

int main() {
  system("python LR_sample.py &");
  const int kvmax = 4096;
  pid_t pid;
  const int kflag = 0777;
  std::string filename1 = "/tmp/test_fifo_sample";
  std::string filename2 = "/tmp/test_fifo_sample";
  std::string ipc_name1 = "/test_sharedMemory_sample1";
  std::string ipc_name2 = "/test_sharedMemory_sample2";
  Fifo fifo_read, fifo_write;
  mkfifo(filename1.c_str(), kflag);
  mkfifo(filename2.c_str(), kflag);
  fifo_read.Initialize(filename2.c_str(), true);
  fifo_write.Initialize(filename1.c_str(), false);
  SharedMemory read_mem, write_mem;
  read_mem.Initialize(ipc_name2.c_str());
  write_mem.Initialize(ipc_name1.c_str());
  fifo_read.Open(); fifo_write.Open();
  int t = 5;
  while(t--) {
    fifo_read.Wait();
    shmstruct* data_read = read_mem.Read();
    std::cout << data_read->size << std::endl;
    for(int i=0; i<data_read->size; i++) {
      std::cout << data_read->keys[i] << " " << data_read->values[i] << std::endl;
    }
    shmstruct store_data;
    store_data.size = data_read->size;
    // should be the values pulled from server to write to
    // write_mem other than data_read
    memset(store_data.values, 0, sizeof store_data.values);
    memset(store_data.keys, 0, sizeof store_data.keys);
    for(int i=0; i<data_read->size; i++) {
      store_data.values[i] = data_read->values[i];
      store_data.keys[i] = data_read->keys[i];
    }
    write_mem.Write(&store_data);
    fifo_write.Signal();
  }
  exit(0);
}
