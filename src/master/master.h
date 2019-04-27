// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#ifndef SRC_MASTER_MASTER_H_
#define SRC_MASTER_MASTER_H_

#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "src/communication/communicator.h"
#include "src/message/message.pb.h"
#include "src/master/task_config.h"
#include "src/util/logging.h"

#ifdef USE_ZOOKEEPER
#include "zookeeper/zookeeper.h"
#endif

namespace rpscc {

DECLARE_int32(listen_port);

class Master {
 public:
  Master();

  static Master* Get() {
    static Master master;
    return &master;
  }

  ~Master() {
#ifdef USE_ZOOKEEPER
    zookeeper_close(zh_);
#endif
  }

  void Initialize(const int16& listen_port = 12018);
  // Return: the number of valid master.
  int32_t Initialize(const std::string& master_ip_port);

  // wait for servers & agents ready
  void WaitForClusterReady();

  bool DeliverConfig();

  // Main loop of master.
  // Deal with all type of message.
  void MainLoop();


  // Detecting dead node.
  void DetectDeadNode();

  // Get the dead node
  std::vector<int> GetDeadNode();

  // Deliver heartbeat loop.
  void DeliverHeartbeatLoop();

  // Deliver heartbeat message to all node.
  void DeliverHeartbeat();

#ifdef USE_ZOOKEEPER
  // The callback function used when notification received
  // from zookeeper.
  // void ZkCallback(zhandle_t* zh, int type, int state,
  //                 const char* path, void* watchCtx);

  // init zookeeper node, and listen to the notification from zookeeper.
  void init_zookeeper();

  // set the is_lead_
  void set_is_lead(const bool& is_lead) {
    is_lead_ = is_lead;
  }
#endif  // USE_ZOOKEEPER

 private:
  // Deal with register message
  void ProcessRegisterMsg(Message* msg);

  // Deal with heartbeat message
  void ProcessHeartbeatMsg(const Message& msg);

  std::mutex config_mutex_;
  TaskConfig config_;
  std::unique_ptr<Communicator> sender_;
  std::unique_ptr<Communicator> receiver_;
  std::unordered_map<int,
                     decltype(std::chrono::system_clock::now())> alive_node_;
  std::unordered_set<int32_t> terminated_node_;
  bool is_lead_;
#ifdef USE_ZOOKEEPER
  zhandle_t* zh_;
#endif  // USE_ZOOKEEPER
};

}  // namespace rpscc

#endif  // SRC_MASTER_MASTER_H_
