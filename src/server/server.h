// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)
#ifndef SRC_SERVER_SERVER_H_
#define SRC_SERVER_SERVER_H_

#include <deque>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "src/communication/zmq_communicator.h"
#include "src/message/message.pb.h"
#include "src/server/key_value_list.h"
#include "src/server/pull_info.h"
#include "src/util/common.h"

namespace rpscc {

DECLARE_string(master_ip_port);
DECLARE_int32(server_port);
DECLARE_string(net_interface);
// The Server class manages a segment of parameters.
// A Server in rpscc receives pull and push requests from the agents.
// The server then updates the parameters it is in charge of, or return the
// parameters requested by the agent (if it's not blocked for consistency).
// The consistency of parameter versions is handled by servers. For every
// agent, it signals all servers after finishing its work. A server maintains
// finish_count of every parameter version. When a count equals to the number
// of agents, the iteration of the current parameter version is considered
// to be finished. A server can use these counts to meet the demands of BSP
// or SSP consistency model.
class Server {
 public:
  Server() { }

  bool Initialize();
  void Start();

 protected:
  std::unique_ptr<Communicator> sender_;
  std::unique_ptr<Communicator> receiver_;
  std::unique_ptr<Communicator> receiver_heatbeat_;

  std::string local_address_;
  int32 local_index_;
  int32 local_id_;
  int32 start_key_;
  int32 parameter_length_;
  int32 consistency_bound_;
  int32 bottom_version_;
  int32 agent_num_;
  int32 server_num_;
  int32 key_range_;
  int32 backup_size_;

  std::vector<int32> master_ids_;
  std::vector<int32> server_ids_;
  std::unordered_map<int32, int32> servers_;
  std::unordered_set<int32> agent_ids_;
  std::vector<float> parameters_;
  std::vector<std::vector<float>> backup_parameters_;
  std::vector<std::queue<KeyValueList>> version_buffer_;
  std::deque<int32> finish_count_;
  std::queue<PullInfo> pull_request_;
  std::map<int32, int32> id_to_index_;

  // Thread for heartbeat
  pthread_t heartbeat_;

  bool RespondToAll();
  void UpdateParameter();
  void ServePull(int32 sender_id, const Message_RequestMessage &request);
  void ServePush(int32 sender_id, const Message_RequestMessage &request);
  static void* HeartBeat(void* arg);
  void Reconfigure(const Message_ConfigMessage &config);
  // UNKNOWN: Use a new thread or not?
  void RequestBackup();
  void Backup(const Message& msg);
  void RespondBackup(int32 server_id);
  void ExtendParameter();
};

}  // namespace rpscc

#endif  // SRC_SERVER_SERVER_H_
