// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#ifndef SRC_MASTER_MASTER_H_
#define SRC_MASTER_MASTER_H_

#include <mutex>

#include "src/message/message.pb.h"
#include "src/master/task_config.h"

namespace rpscc {

class Master {
 public:
  Master();

  // wait for servers & agents ready
  void WaitForClusterReady();

  bool DeliverConfig();

  void MainLoop();

 private:
  std::mutex config_mutex_;
  TaskConfig config_;
};

}  // namespace rpscc

#endif  // SRC_MASTER_MASTER_H_
