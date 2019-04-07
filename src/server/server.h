// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)
#ifndef SRC_SERVER_SERVER_H_
#define SRC_SERVER_SERVER_H_

#include <deque>
#include <map>
#include <queue>
#include <hash_set>
#include <string>
#include <vector>

#include "src/communication/zmq_communicator.h"
#include "src/message/message.pb.h"
#include "src/server/key_value_list.h"
#include "src/server/pull_info.h"
#include "src/util/common.h"

namespace rpscc {

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
  uint32 local_index_;
  uint32 local_id_;
  uint32 start_key_;
  uint32 parameter_length_;
  uint32 consistency_bound_;
  uint32 bottom_version_;
  uint32 agent_num_;
  uint32 server_num_;
  uint32 backup_size_;

  std::vector<uint32> master_ids_;
  std::vector<uint32> server_ids_;
  std::unordered_map<uint32, uint32> servers_;
  __gnu_cxx::hash_set<uint32> agent_ids_;
  std::vector<float> parameters_;
  std::vector<std::vector<float>> backup_parameters_;
  std::vector<std::queue<KeyValueList> > version_buffer_;
  std::deque<uint32> finish_count_;
  std::queue<PullInfo> pull_request_;
  std::map<uint32, uint32> id_to_index_;

  // Thread for heartbeat
  pthread_t heartbeat_;

  bool RespondToAll();
  void UpdateParameter();
  void ServePull(uint32 sender_id, const Message_RequestMessage &request);
  void ServePush(uint32 sender_id, const Message_RequestMessage &request);
  static void* HeartBeat(void* arg);
  void Reconfigurate(const Message_ConfigMessage &config);
  // UNKNOWN: Use a new thread or not?
  void RequestBackup();
  void RespondBackup(uint32 server_id);
};

}  // namespace rpscc

#endif  // SRC_SERVER_SERVER_H_
