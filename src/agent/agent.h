// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#ifndef SRC_AGENT_AGENT_H_
#define SRC_AGENT_AGENT_H_

#include <stdio.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <utility>

#include "src/agent/partition.h"
#include "src/channel/fifo.h"
#include "src/channel/shared_memory.h"
#include "src/communication/communicator.h"
#include "src/message/message.pb.h"
#include "src/util/common.h"


namespace rpscc {

// Agent is on the same host with worker. It provide agency service for worker.
// Think of it this way. Agent works as a middleman between servers and worker.
// Agent will get gradients from worker, then push it to servers. On the other 
// hand, agent will also pull parameters from servers, and submit them to
// the worker.
class Agent {
 public:
  Agent() {}
  ~Agent() {}
  // Initialzie the agent
  // Parameters:
  // parame_fifo_name and grad_fifo_name is the fifo files' names, the fifo
  // files are used for communicating with worker. parame_memory_name and
  // grad_memory_name are shared memory files' names, the shared memory files
  // work as data channels between agent and worker, and the data in the shared
  // memory files is parameter data or gradient data about parameters.
  bool Initialize(std::string para_fifo_name,
                  std::string grad_fifo_name,
                  std::string para_memory_name,
                  std::string grad_memory_name,
                  std::string master_addr,
                  const int16& listen_port=5555);
  // Finalize the agent
  void Finalize();
  // To start the agent
  bool Start();

 private:
  // The Agent's information, local_port_ indicate the port for which this
  // agent is listening.
  int32 local_id_;
  std::string local_ip_;
  int16 listen_port_;
  // Global information about rpscc
  int32 agent_num_;
  int32 server_num_;
  int32 key_range_;
  int32 epoch_num_;
  std::vector<int32> server_id_list_;

  // Sender and Receiver for agent.
  std::unique_ptr<Communicator> sender_;
  std::unique_ptr<Communicator> receiver_;

  // Fifo for communication with worker
  std::string para_fifo_name_;
  std::string grad_fifo_name_;
  Fifo para_fifo_;
  Fifo grad_fifo_;

  // Shared memory for transfering data with worker
  std::string para_memory_name_;
  std::string grad_memory_name_;
  SharedMemory para_memory_;
  SharedMemory grad_memory_;

  // Key-value list for pushing
  std::vector<int32> keys_;
  std::vector<float32> values_;

  // gradients_ is read from worker, and it will be pushed to servers
  shmstruct gradients_;

  // parameters_ is pulled from servers, and it will be sent to worker
  shmstruct parameters_;

  // Partition message to server
  Partition partition_;

  // Thread for heartbeat
  pthread_t heartbeat_;

  // Messages for reconfiguration
  Message* reconfig_msg_;

  // Mutex for reconfiguration
  std::mutex reconfig_mutex_;

  bool AgentWork();
  bool Push();
  bool Pull();

  // HeartBeat with master
  static void* HeartBeat(void* arg);

  // To sort the key list and value list
  void SortKeyValue(int32* keys, float32* values, int32 size);

  // Reconfigigurate the agent when the master send a ConfigMessage
  // to agent not for the first time
  void Reconfigurate();
};

}  // namespace rpscc

#endif  // SRC_AGENT_AGENT_H_
