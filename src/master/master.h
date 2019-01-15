// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#ifndef SRC_MASTER_MASTER_H_
#define SRC_MASTER_MASTER_H_

#include <mutex>

#include "src/communication/communicator.h"
#include "src/message/message.pb.h"
#include "src/master/task_config.h"

namespace rpscc {

class Master {
 public:
  Master();

  void Initialize(const int16& listen_port=12018);

  // wait for servers & agents ready
  void WaitForClusterReady();

  bool DeliverConfig();

  // Main loop of master.
  // Deal with all type of message.
  void MainLoop();

  // Get the dead node
  std::vector<int> GetDeadNode();

 private:

  // Deal with register message
  void ProcessRegisterMsg(Message* msg);

  // Deal with heartbeat message
  void ProcessHeartbeatMsg(Message* msg);

  std::mutex config_mutex_;
  TaskConfig config_;
  std::unique_ptr<Communicator> sender_;
  std::unique_ptr<Communicator> receiver_;
  std::unordered_map<int, time_t> alive_node_;
};

}  // namespace rpscc

#endif  // SRC_MASTER_MASTER_H_
