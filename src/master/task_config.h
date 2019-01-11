// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#ifndef SRC_MASTER_TASK_CONFIG_H_
#define SRC_MASTER_TASK_CONFIG_H_

#include <random>
#include <vector>

#include "src/message/message.pb.h"
#include "src/util/common.h"

namespace rpscc {

class TaskConfig {
 public:
  TaskConfig();

  void Initialize(const std::string& config_file);

  // convert to ConfigMessage
  Message_ConfigMessage* ToMessage();

  // Get ip from id.
  std::string GetIp(const int32_t& id) { return server_ip_[id]; }

  // Append a new node.
  void AppendNode(const std::string& ip, const int32_t& port);

  // Append a new server.
  void AppendServer(const std::string& ip, const int32_t& port);

  // Append a new agent.
  void AppendAgent(const std::string& ip, const int32_t& port);

  // Is cluster ready?
  bool Ready() {
    return server_ip_.size() == server_num_ && agent_ip_.size() == worker_num_;
  }

  // Generate partition
  void GeneratePartition();

 private:
  int32 worker_num_;
  int32 server_num_;
  int32 key_range_;
  std::vector <std::string> server_ip_;
  std::vector <int32_t> server_port_;
  std::vector <std::string> agent_ip_;
  std::vector <int32_t> agent_port_;
  std::vector <int32> partition_;
  std::vector <int32> server_id_;
  int32 bound_;

  static std::default_random_engine generator;
  static std::unique_ptr<std::uniform_int_distribution<int>> distribution;
};

std::default_random_engine generator;
std::unique_ptr<std::uniform_int_distribution<int>> distribution;

}  // namespace rpscc

#endif  // SRC_MASTER_TASK_CONFIG_H_
