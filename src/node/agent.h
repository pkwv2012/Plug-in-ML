// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#ifndef SRC_NODE_AGENT_H_
#define SRC_NODE_AGENT_H_

#include "src/channel/fifo.h"

namespace rpscc {

class Agent {
 public:
  Agent() {}
  ~Agent() {}

  bool Initialize(int32 agent_id);
  bool Start();
  void Terminate();

 private:
  int32 agent_id_;
//sender
//reciver
  Fifo fifo_reader_;
  bool AgentWork();
};

}  // namespace rpscc

#endif  // SRC_NODE_AGENT_H_
