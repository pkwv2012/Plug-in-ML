// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#ifndef SRC_NODE_WORKER_H_
#define SRC_NODE_WORKER_H_

#include "src/util/common.h"

#include "src/channel/fifo.h"

namespace rpscc {

class Worker {
 public:
  Worker() {}

  ~Worker() {}

  bool Initialize(int32 worker_id);
  bool Start();
  void Terminate();

 private:
  int32 worker_id_;
  Fifo fifo_writer_;
  bool live_
  bool DoJobs();

};

}  // namespace rpscc

#endif  // SRC_NODE_WORKER_H_
