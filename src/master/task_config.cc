// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/12.
//

#include "gflags/gflags.h"
#include "task_config.h"

DEFINE_int32(worker_num, 0, "The number of worker.");
DEFINE_int32(server_num, 0, "The number of server.");
DEFINE_int32(key_range, 0, "The total number of features.");
DEFINE_int32(bound, 0, "The definition of consistency.");

namespace rpscc {

void rpscc::TaskConfig::Initialize(const std::string &config_file) {
  worker_num_ = FLAGS_worker_num;
  server_num_ = FLAGS_server_num;
  key_range_ = FLAGS_key_range;
  bound_ = FLAGS_bound;

}

TaskConfig::TaskConfig() {

}

void TaskConfig::Initialize(const std::string &config_file) {

}

Message_ConfigMessage *TaskConfig::ToMessage() {
  return nullptr;
}

void TaskConfig::AppendNode(const std::string& ip, const int32_t& port) {
}

void TaskConfig::AppendServer(const std::string &ip, const int32_t &port) {
  server_ip_.push_back(ip);
  server_port_.push_back(port);
}

void TaskConfig::AppendAgent(const std::string &ip, const int32_t &port) {
  agent_ip_.push_back(ip);
  agent_port_.push_back(port);
}
}  // namespace rpscc

