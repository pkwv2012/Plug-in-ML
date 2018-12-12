// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#ifndef SRC_MASTER_TASK_CONFIG_H_
#define SRC_MASTER_TASK_CONFIG_H_

#include <vector>

#include "src/message/message.pb.h"
#include "src/util/common.h"

namespace rpscc {

class TaskConfig {
 public:
  TaskConfig();

  // convert to ConfigMessage
  Message_ConfigMessage* ToMessage();

 private:
  int32 worker_num_;
  int32 server_num_;
  int32 key_range_;
  std::vector <int32> server_ip;
  std::vector <int32> partition_;
  std::vector <int32> server_id;
  int32 bound_;
};

}  // namespace rpscc

#endif  // SRC_MASTER_TASK_CONFIG_H_
