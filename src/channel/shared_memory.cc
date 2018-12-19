// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#include <iostream>

#include "src/channel/shared_memory.h"

namespace rpscc {

char* px_ipc_name(const char* name);

void SharedMemory::Initialize(const char *ipc_name) {
//  initialize the shared memory
  int fd = shm_open(px_ipc_name(ipc_name),
      O_RDWR | O_CREAT, FILE_MODE);
  ftruncate(fd, sizeof(struct shmstruct));
  shared_data_ = (struct shmstruct*)mmap(NULL, sizeof(struct shmstruct),
      PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
}

shmstruct SharedMemory::Read() {
  // read
//  memcpy(data, shared_data_, sizeof(*shared_data_));
  shmstruct data;
  data.keys = shared_data_->keys;
  data.values = shared_data_->values;
  return data;
}

void SharedMemory::Write(shmstruct* data) {
//  memcpy(shared_data_, data, sizeof(data));
//  *shared_data_ = data;
//  memcpy(shared_data_, data, sizeof(*data));
  shared_data_->keys = data->keys;
  shared_data_->values = data->values;
}

// this function target to get a px_ipc_name more capable
char* px_ipc_name(const char* name) {
  char *dir, *dst, *slash;
  if ((dst = reinterpret_cast<char*>(malloc(PATH_MAX))) == NULL)
    return (NULL);

  if ((dir=getenv("PX_IPC_NAME")) == NULL) {
#ifdef POSIX_IPC_PREFIX
    dir = POSIX_IPC_PREFIX
#else
    dir = "/tmp/";
#endif
  }
  if (dir[strlen(dir)-1] == '/') {
    slash = "";
  } else {
    slash = "/";
  }
  snprintf(dst, PATH_MAX, "%s%s%s", dir, slash, name);
  return (dst);
}

}  // namespace rpscc
