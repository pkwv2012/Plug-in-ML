// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#ifndef SRC_CHANNEL_FIFO_H_
#define SRC_CHANNEL_FIFO_H_

#include <string>

#include "src/util/comon.h"

namespace rpscc {

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
