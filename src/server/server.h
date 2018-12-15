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

 private:
  std::unique_ptr<Communicator> sender_;
  std::unique_ptr<Communicator> receiver_;

  std::string local_ip_;
  uint32 local_id_;
  uint32 start_key_;
  uint32 parameter_length_;
  uint32 consistency_bound_;
  uint32 bottom_version_;
  uint32 agent_num_;
  uint32 server_num_;

  std::vector<std::string> server_ips_;
  std::vector<uint32> server_ids_;
  std::vector<float> parameters_;
  std::vector<std::queue<KeyValueList> > version_buffer_
  std::deque<uint32> finish_count_;
  std::queue<PullInfo> pull_request_;
  std::map<uint32, uint32> id_to_index_;


  bool RespondToAll();
  void UpdateParameter();
  void ServePull(uint32 sender_id, Message_RequestMessage &request);
  void ServePush(uint32 sender_id, Message_RequestMessage &request);
};

}  // namespace rpscc

#endif  // SRC_SERVER_SERVER_H_
