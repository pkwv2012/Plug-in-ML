// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#include "src/node/worker.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

namespace rpscc {

bool Worker::Initialize(int32 worker_id) {
  worker_id_ = worker_id;
  std::string filename = "/tmp/fifo.agentworker";
  fifo_writer_.Initialize(filename, false);
  fifo_writer_.Open();
  live_ = true;
}

bool Worker::Start() {
  //LOG(INFO) << "Worker " << worker_id_ << " start.";
  if (!DoJobs()) {
    //LOG(ERROR) << "Worker work failed.";
    return false;
  }
  return true;
}

void Worker::Terminate() {
  sleep(1);  // wait a second for the worker to deal with the current works.
  live_ = false;
}

bool Worker::DoJobs() {
  while (true) {
//  there need to do some work.
//  do_some_thing() function.
    fifo_writer_.Signal();
    if (!live_) return true;
  }
  return false;
}

}  // namespace rpscc