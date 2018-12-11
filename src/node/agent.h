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

  bool Initialize();
  bool Start();
  bool Terminate();

 private:
  int32 agent_id_;
//sender
//reciver
  Fifo fifo_reade_;
};

}

#endif  // SRC_NODE_AGENT_H_
