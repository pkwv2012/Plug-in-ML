// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#ifndef SRC_AGENT_AGENT_H_
#define SRC_AGENT_AGENT_H_

#include <stdio.h>
#include <pthread.h>

#include "src/agent/partition.h"
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
  // in-memory file to store the updates
  File update_channel_;
  // in-memory file to store the parameters
  File parameter_channel_:
  // Receive thread, receive message from server
  pthread_t recv_thread_;
  // Partition message to server
  Partition partition_;
  
  bool AgentWork();
  void Push();
  void Pull();
  static void ReceiveWork(Agent* pagent);
  
  void GetUpdateChannelFile();
  void GetParameterChannelFile();
};

}  // namespace rpscc

#endif  // SRC_AGENT_AGENT_H_
