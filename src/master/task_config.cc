// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/12.
//

#include <algorithm>

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
  // [1, key_range - 2], because we should not generate index 0 and index key_range - 2
  distribution_.reset(new std::uniform_int_distribution<int>(1, key_range_ - 2));
}

TaskConfig::TaskConfig() {

}

void TaskConfig::Initialize(const std::string &config_file) {

}

Message_ConfigMessage *TaskConfig::ToMessage() {
  Message_ConfigMessage* config_msg = new Message_ConfigMessage();
  config_msg->set_worker_num(worker_num_);
  config_msg->set_server_num(server_num_);
  config_msg->set_bound(bound_);
  config_msg->set_key_range(key_range_);
  assert(server_ip_.size() == server_port_.size());
  int id = 0;
  for (size_t i = 0; i < server_ip_.size(); ++ i) {
    config_msg->add_server_ip(
      std::to_string(server_ip_[i]) + ":"
      + std::to_string(server_port_[i]));
    config_msg->add_server_id(id ++);
  }
  for (auto p : partition_) {
    config_msg->add_partition(p);
  }
  assert(agent_ip_.size() == agent_port_.size());
  for (size_t i = 0; i < agent_ip_.size(); ++ i) {
    config_msg->add_server_ip(
      std::to_string(agent_ip_[i]) + ":"
      + std::to_string(agent_port_[i])
    );
    config_msg->add_worker_id(id ++);
  }
  return config_msg;
}

void TaskConfig::AppendNode(const std::string& ip, const int32_t& port) {
}

void TaskConfig::AppendServer(const std::string &ip, const int32_t &port) {
  server_ip_.push_back(ip);
  server_port_.push_back(port);
  node_ip_.push_back(ip);
  node_port_.push_back(port);
  server_id_.push_back(node_ip_.size() - 1);
  id_to_addr_[node_ip_.size() - 1] = ip + ":" + std::to_string(port);
}

void TaskConfig::AppendAgent(const std::string &ip, const int32_t &port) {
  agent_ip_.push_back(ip);
  agent_port_.push_back(port);
  node_ip_.push_back(ip);
  node_port_.push_back(port);
  agent_id_.push_back(node_ip_.size() - 1);
  id_to_addr_[node_ip_.size() - 1] = ip + ":" + std::to_string(port);
}

void TaskConfig::GeneratePartition() {
  partition_.push_back(0);
  for (int i = 1; i < server_num_; ++ i) {
    partition_.push_back(distribution_(generator_));
  }
  partition_.push_back(key_range_);
  std::sort(partition_.begin(), partition_.end());
}

}  // namespace rpscc

