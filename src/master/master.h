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
#include "src/util/logging.h"

namespace rpscc {

DECLARE_int32(listen_port);

class Master {
 public:
  Master();

  static Master* Get() {
    static Master master;
    return &master;
  }

  void Initialize(const int16& listen_port=12018);
  // Return: the number of valid master.
  int32_t Initialize(const std::string& master_ip_port);

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
  void ProcessHeartbeatMsg(const Message& msg);

  std::mutex config_mutex_;
  TaskConfig config_;
  std::unique_ptr<Communicator> sender_;
  std::unique_ptr<Communicator> receiver_;
  std::unordered_map<int, time_t> alive_node_;
  std::unordered_set<int32_t> terminated_node_;
};

}  // namespace rpscc

#endif  // SRC_MASTER_MASTER_H_
