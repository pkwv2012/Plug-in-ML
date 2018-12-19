// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#ifndef RPSCC_SHARED_MEMORY_H
#define RPSCC_SHARED_MEMORY_H

#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

#include <vector>

#include "src/util/common.h"
#include "src/channel/fifo.h"

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

namespace rpscc {

// the struct to store data;
struct shmstruct {
  std::vector<int64> keys;
  std::vector<float32> values;
};

// a shared memory with fifo for write and read control.
class SharedMemory {
 public:
  SharedMemory() {}
  ~SharedMemory() {}

  void Initialize(const char* ipc_name_read);

  void Write(shmstruct* data);

  shmstruct* Read();
 private:
  struct shmstruct* shared_data_;
};

}

#endif //RPSCC_SHARED_MEMORY_H
