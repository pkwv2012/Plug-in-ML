// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#ifndef SRC_AGENT_AGENT_H_
#define SRC_AGENT_AGENT_H_

#include <stdio.h>

#include <memory>
#include <string>

#include "src/agent/partition.h"
#include "src/channel/fifo.h"
#include "src/channel/sharedmemory.h"
#include "src/communication/communicator.h"
#include "src/util/common.h"

namespace rpscc {

// In-memory file class. This class define the structure of update_channel_ 
// and parameter_channel_
struct File {
  FILE* fp_;
  std::string filename_;
  File() {}
  File(FILE* fp, std::string filename): fp_(fp), filename_(filename) {}
}

// Agent is on the same host with worker. It provide agency service for worker.
// Think of it this way. Agent works as a middleman between servers and worker.
// Agent will get update message from worker, then push it to servers. On the 
// other hand, agent will also pull parameters from servers, and submit them to
// the worker.
class Agent {
 public:
  Agent() {}
  // Initialzie the agent
  bool Initialize();
  // Finalize the agent
  void Finalize();
  // To start the agent
  bool Start();
  
 private:
  // The Agent's information, local_port_ indicate the port for which this 
  // agent is listening.
  int32 local_id_;
  std::string local_ip_;
  std::string listen_port_;
  // Global information about rpscc
  int32 agent_num_;
  int32 server_num_;
  
  // Sender and Receiver for agent.
  std::unique_ptr<Communicator> sender_;
  std::unique_ptr<Communicator> receiver_;
  
  // in-memory file to store the updates
  File update_channel_;
  // in-memory file to store the parameters
  File parameter_channel_:
  // Partition message to server
  Partition partition_;
  
  bool AgentWork();
  void Push();
  void Pull();
  
};

}  // namespace rpscc

#endif  // SRC_AGENT_AGENT_H_
