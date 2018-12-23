// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include <stdio.h>

#include "src/agent/agent.h"
#include "src/communication/zmq_communicator.h"
#include "src/message/message.pb.h"
#include "src/server/key_value_list.h"
#include "src/server/pull_info.h"


namespace rpscc {

bool Agent::Initialize(std::string para_fifo_name, 
                       std::string grad_fifo_name,
                       std::string para_memory_name,
                       std::string grad_memory_name,
                       int32 shared_memory_size) {
  // 1.Initialize sender
  sender_.reset(new ZmqCommunicator());
  if (sender_.get() == NULL) {
    printf("Initialize sender failed.");
    return false;
  }
  sender_->Initialize(/* ring_size */, true, /* listen_port */);
  
  // 2.Initialize receiver
  receiver_.reset(new ZmqCommunicator());
  if (receiver_.get() == NULL) {
    printf("Initialize receiver failed.");
    return false;
  }
  receiver_->Initialize(/* ring_size */, false, /* listen_port */);
  
  // 3.Interact with master
  // 3_1.Send this agent's local_ip_ to master, and receive config information
  local_ip_ = ""; /* find a way to get agent ip */
  listen_port_ = "5555";
  Message msg_send;
  Message msg_recv;
  // Send reg_msg to master, and receive config_msg.
  Message_RegisterMessage reg_msg;
  Message_ConfigMessage config_msg;
  std::string reg_str;
  std::string config_str;
  
  reg_msg.set_ip(local_ip_);
  msg_send.set_message_type(Message_MessageType_register_);
  msg_send.set_recv_id(0);
  msg_send.set_send_id(-1);
  msg_send.set_allocated_register_msg(&reg_msg);
  msg_send.SerializeToString(&reg_str);
  if (sender_.Send(0, reg_str) == -1) {
    printf("Cannot send register message to master\n");
    return false;
  }
  if (receiver_->Receive(&config_str_) == -1) {
    printf("Error in receiving message from master\n");
    return false;
  }
  msg_recv.ParseFromString(config_str);
  config_msg = msg_recv.config_msg();
  
  // 3_2.Initialization of agent fields
  local_id_ = msg_recv.recv_id();
  agent_num_ = config_msg.worker_num();
  server_num_ = config_msg.server_num();
  
  // 3_3.Add <server_id, server_addr> pairs to the sender_'s <Id, Addr> map.
  for (int32 i = 0; i < server_num_; i++) {
    std::string server_i_ip = config_msg.server_ip(i);
    int32 server_i_id = config_msg.server_id(i);
    sender_.AddIdAddr(server_i_id, server_i_ip);
  }
  std::vector<int32> part_vec;
  for (int32 i = 0; i < server_num_ + 1; i++) {
    part_vec.push_back(config_msg.partition(i));
  }
  
  // 3_4.Initialize the partition_.
  partition_.Initialize(part_vec[part_vec.size() - 1], server_num_, part_vec);
  
  // 4.Initialize the fifo and shared memory
  para_fifo_name_ = para_fifo_name;
  grad_fifo_name_ = grad_fifo_name;
  para_memory_name_ = para_memory_name;
  grad_memory_name_ = grad_memory_name;
  shared_memory_size_ = shared_memory_size;
  
  int32 fd = shm_open(para_memory_name_.c_str(), O_RDWR | O_CREAT, FILE_MODE);
  ftruncate(fd, sizeof(struct shmstruct));
  close(fd);
  fd = shm_open(grad_memory_name_.c_str(), O_RDWR | O_CREAT, FILE_MODE);
  ftruncate(fd, sizeof(struct shmstruct));
  close(fd);
  
  // Agent is reader for parameters and writer for gradients
  para_fifo_.Initialize(para_fifo_name_, true);
  grad_fifo_.Initialize(grad_fifo_name_, false);
  para_memory_.Initialize(para_memory_name_.c_str());
  grad_memory_.Initialzie(grad_memory_name_.c_str());
  
  return true;
}

bool Agent::Finalize() {
  sender_.Finalize();
  receiver_.Finalize();
  partition_.Finalize();
}

bool Agent::Start() {
  if (!AgentWork()) {
    printf("Agent work failed.\n");
    return false;
  }
  return true;
}

bool Agent::AgentWork() {
  while (true) {
    grad_fifo_.Wait();
    
  }
}

bool Agent::Push() {
    
}

bool Agent::Pull() {
  
}

}  // namespace rpscc
