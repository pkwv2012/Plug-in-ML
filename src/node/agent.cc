// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#include "src/node/agent.h"

#include <string>

#include "src/util/common.h"
#include "src/channel/fifo.h"

namespace rpscc {

bool Agent::Initialize(int32 agent_id) {
  agent_id_ = agent_id;
  std::string filename = "/tmp/fifo.agentworker";
  unlink(filename.c_str());
  if (mkfifo(filename, 0666) != 0) {
    //LOG(FATAL) << "Create fifo file failed.";
  }
  fifo_reader_.Initialize(filename, true);
  fifo_reader_.Open();
}

bool Agent::Start() {
  if (!AgentWork()) {
    //LOG(ERROR) << "agent work failed."
    return false;
  }
  return true;
}

bool Agent::AgentWork() {
  while (true) {
    fifo_reader_.Wait();
//    read from shared memory.
//  read from memory.
  }
}

void Agent::Terminate() {

}

}  // namespace rpscc
