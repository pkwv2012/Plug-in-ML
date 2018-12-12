// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)
#ifndef SRC_SERVER_SERVER_H_
#define SRC_SERVER_SERVER_H_

#include <deque>
#include <queue>
#include <vector>

#include "/src/server/key_value_list.h"
#include "/src/server/pull_info.h"
#include "/src/util/common.h"

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
  uint32 server_id;
  uint32 start_key;
  uint32 parameter_length;
  uint32 consistency_bound;
  uint32 bottom_version;
  uint32 agent_num;

  std:vecotr<float> parameters;
  std:vector<std:queue<std:unique_ptr<KeyValueList> > > version_buffer
  std:deque<uint32> finish_count;
  std:queue<PullInfo> pull_request;

  void ResponseAll();
  void UpdateParameter();
};

}  // namespace rpscc

#endif  // SRC_SERVER_SERVER_H_
