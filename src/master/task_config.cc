// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/12.
//

#include <algorithm>
#include <sstream>

#include "gflags/gflags.h"
#include "src/master/task_config.h"
#include "src/util/logging.h"
#include "task_config.h"


namespace rpscc {

DEFINE_int32(worker_num, 1, "The number of worker.");
DEFINE_int32(server_num, 1, "The number of server.");
DEFINE_int32(key_range, 0, "The total number of features.");
DEFINE_int32(bound, 0, "The definition of consistency.");

std::default_random_engine TaskConfig::generator_;
std::unique_ptr<std::uniform_int_distribution<int>> TaskConfig::distribution_;

void rpscc::TaskConfig::Initialize(const std::string &config_file) {
  LOG(INFO) << FLAGS_server_num << std::endl;
  worker_num_ = FLAGS_worker_num;
  server_num_ = FLAGS_server_num;
  key_range_ = FLAGS_key_range;
  bound_ = FLAGS_bound;
  // [1, key_range - 2], because we should not generate index 0 and index key_range - 2
  distribution_.reset(new std::uniform_int_distribution<int>(1, key_range_ - 2));
}

TaskConfig::TaskConfig() {

}

Message_ConfigMessage *TaskConfig::ToMessage() {
  Message_ConfigMessage* config_msg = new Message_ConfigMessage();
  config_msg->set_worker_num(worker_num_);
  config_msg->set_server_num(server_num_);
  config_msg->set_bound(bound_);
  config_msg->set_key_range(key_range_);
  //assert(server_ip_.size() == server_port_.size());
  std::vector<std::pair<int32_t, std::string>> temp(id_to_addr_.begin(),
    id_to_addr_.end());
  std::sort(temp.begin(), temp.end());
  for (auto pr: temp) {
    config_msg->add_node_ip_port(pr.second);
  }
  // using set, instead of add
  for (auto pr: id_to_addr_) {
    config_msg->set_node_ip_port(pr.first, pr.second);
  }
  for (auto id : server_id_) {
    config_msg->add_server_id(id);
  }
  for (auto id : agent_id_) {
    config_msg->add_worker_id(id);
  }
  for (auto id : master_id_) {
    config_msg->add_master_id(id);
  }

//  for (size_t i = 0; i < server_ip_.size(); ++ i) {
//    config_msg->add_server_ip(
//      std::to_string(server_ip_[i]) + ":"
//      + std::to_string(server_port_[i]));
//    config_msg->add_server_id(id ++);
//  }
  for (auto p : partition_) {
    config_msg->add_partition(p);
  }
  //assert(agent_ip_.size() == agent_port_.size());
//  for (size_t i = 0; i < agent_ip_.size(); ++ i) {
//    config_msg->add_server_ip(
//      std::to_string(agent_ip_[i]) + ":"
//      + std::to_string(agent_port_[i])
//    );
//    config_msg->add_worker_id(id ++);
//  }
  return config_msg;
}

void TaskConfig::AppendNode(const std::string& ip, const int32_t& port) {
}

void TaskConfig::AppendServer(const std::string &ip, const int32_t &port) {
  //server_ip_.push_back(ip);
  //server_port_.push_back(port);
  //node_ip_.push_back(ip);
  //node_port_.push_back(port);
  server_id_.push_back(node_id_);
  id_to_addr_[node_id_] = ip + ":" + std::to_string(port);
  node_id_ ++;
}

void TaskConfig::AppendAgent(const std::string &ip, const int32_t &port) {
  //agent_ip_.push_back(ip);
  //agent_port_.push_back(port);
  //node_ip_.push_back(ip);
  //node_port_.push_back(port);
  agent_id_.push_back(node_id_);
  id_to_addr_[node_id_] = ip + ":" + std::to_string(port);
  ++ node_id_;
}

void TaskConfig::GeneratePartition() {
  // partition_.push_back(0);
  for (int i = 0; i < server_num_; ++ i) {
    partition_.push_back(distribution_->operator()(generator_));
  }
  // partition_.push_back(key_range_);
  std::sort(partition_.begin(), partition_.end());
}

void TaskConfig::AppendMaster(const std::string &ip, const int32_t &port) {
  //node_ip_.push_back(ip);
  //node_port_.push_back(port);
  master_id_.push_back(node_id_);
  id_to_addr_[node_id_] = ip + ":" + std::to_string(port);
  ++ node_id_;
}

void TaskConfig::AppendMaster(const std::string &ip_port) {
  std::stringstream ss(ip_port);
  if (ss.good()) {
    std::string ip;
    std::getline(ss, ip, ':');
    //node_ip_.push_back(ip);
    std::string port;
    std::getline(ss, port, ':');
    //node_port_.push_back(std::stoi(port));
  }
  master_id_.push_back(node_id_);
  id_to_addr_[node_id_] = ip_port;
  ++ node_id_;
}

void TaskConfig::FixConfig(const std::vector<int> &dead_node) {
  if (dead_node.size() == 0) return;
  std::unique_lock<std::mutex> ul(mu_);
  config_changed_ = true;
  for (auto node_id : dead_node) {
    if (IsAgentId(node_id)) {
      auto iter = std::find(agent_id_.begin(), agent_id_.end(), node_id);
      agent_id_.erase(iter);
    } else if (IsServerId((node_id))) {
      auto iter = std::find(server_id_.begin(), server_id_.end(), node_id);
      int32_t index = iter - server_id_.begin();
      server_id_.erase(iter);
      partition_.erase(partition_.begin() + index);
    }
  }
}

}  // namespace rpscc

