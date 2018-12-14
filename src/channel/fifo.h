// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

// fifo is just write sig 1 and read

#ifndef SRC_CHANNEL_FIFO_H_
#define SRC_CHANNEL_FIFO_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <string>

#include "src/util/common.h"

namespace rpscc {

// FIFO class for simple notifiy the agent that current work is done and to
// pull the new parameters from worker by worker and notify the worker
// there is updated parameters to recive which are already placed in shared memory

class Fifo {
 public:
  Fifo() {}
  ~Fifo() { CloseFifo(); }

  void Initialize(const std::string& filename, bool is_reader);

  void Open();

  void Signal();

  void Wait();

 private:
  std::string filename_;
  bool is_reader_;
  int fd_;

  void CloseFifo();

  DISALLOW_COPY_AND_ASSIGN(Fifo);
};

}  // namespace rpscc

#endif  // SRC_CHANNEL_FIFO_H_
