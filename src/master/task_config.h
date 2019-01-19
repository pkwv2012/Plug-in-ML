// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#ifndef SRC_MASTER_TASK_CONFIG_H_
#define SRC_MASTER_TASK_CONFIG_H_

#include <algorithm>
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
  std::string GetIp(const int32_t& id) { return id_to_addr_[id]; }

  // Append a new node.
  void AppendNode(const std::string& ip, const int32_t& port);

  // Append a new server.
  void AppendServer(const std::string& ip, const int32_t& port);

  // Append a new agent.
  void AppendAgent(const std::string& ip, const int32_t& port);

  // Append a new master.
  void AppendMaster(const std::string& ip, const int32_t& port);
  void AppendMaster(const std::string& ip_port);

  // Is cluster ready?
  bool Ready() {
    return server_id_.size() == server_num_ && agent_id_.size() == worker_num_;
  }

  std::vector<std::string> get_node_ip() {
    std::vector<std::string> ip;
    for (auto id_ip : id_to_addr_) ip.push_back(id_ip.second);
    return ip;
  }

  bool IsMasterId(const int32_t& id) const {
    return std::find(master_id_.begin(), master_id_.end(), id) != master_id_.end();
  }

  // Generate partition
  void GeneratePartition();

 private:
  int32 worker_num_;
  int32 server_num_;
  int32 key_range_;
  //std::vector <std::string> server_ip_;
  //std::vector <int32_t> server_port_;
  //std::vector <std::string> agent_ip_;
  //std::vector <int32_t> agent_port_;
  std::vector <int32_t> partition_;
  std::vector <int32_t> server_id_;
  std::vector <int32_t> agent_id_;
  //std::vector <std::string> node_ip_;
  //std::vector <int32_t> node_port_;
  std::vector <int32_t> master_id_;
  // id_to_addr is the most important hash table,
  // it will be prefer used.
  std::unordered_map<int32_t, std::string> id_to_addr_;
  std::unordered_map<std::string, int32_t> addr_to_id_;
  int32 bound_;
  int32_t node_id_ = 0;

  static std::default_random_engine generator_;
  static std::unique_ptr<std::uniform_int_distribution<int>> distribution_;
};

}  // namespace rpscc

#endif  // SRC_MASTER_TASK_CONFIG_H_
