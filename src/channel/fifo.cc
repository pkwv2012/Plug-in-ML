// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#include "src/channel/fifo.h"
#include <fcntl.h>


namespace rpscc {

void Fifo::Initialize(const std::string& filename, bool is_reader) {
  //CHECK_NE(filename.empty(), true);
  filename_ = filename;
  is_reader_ = is_reader;
}

void Fifo::Open() {
  int flag = is_reader_ ? O_RDONLY : O_CREAT | O_WRONLY;
  fd_ = open(filename_.c_str(), flag, S_IREAD | S_IWRITE);
  if (fd_ == -1) {
    //LOG(FATAL) << "Cannot open file: " << filename_;
  }
}

void Fifo::Wait() {
  //CHECK_EQ(is_reader_, true);
  int sig = 0;
  ssize_t size = read(fd_, reinterpret_cast<char*>(&sig), sizeof(int));
  //CHECK_NE(size, -1);
}

void Fifo::Signal() {
  //CHECK_EQ(is_reader_, false);
  int sig = 0;
  ssize_t size = write(fd_, reinterpret_cast<char*>(&sig), sizeof(int));
  //CHECK_NE(size, -1);
}

void Fifo::CloseFifo() {
  close(fd_);
}

}  // namespace rpscc