// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include <stdio.h>

#include <algorithm>
#include <set>

#include "src/agent/agent.h"
#include "src/communication/zmq_communicator.h"
#include "src/message/message.pb.h"


namespace rpscc {

// This is a sorter for key list and value list stroed in the agent. During the
// sorting, keys and values will keep thier relative positions.
void Agent::SortKeyValue(std::vector<int32> keys, 
                         std::vector<float32> values) {
  std::map<int32, int32> key_sub;
  std::vector<float32> tmp_values(values.begin(), values.end());

  int32 size = keys.size();
  for (int32 i = 0; i < size; i++) key_sub.insert(std::make_pair(keys[i], i));
  size = 0;
  for (auto i : key_sub) {
    keys[size] = i.first;
    values[size++] = tmp_values[i.second];
  }
  tmp_values.clear();
  key_sub.clear();
}

// To Initialize the agent.
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
  
  // 3 Exchange messages with master
  // 3_1.Send this agent's local_ip_ to master, and receive config information
  local_ip_ = ""; /* find a way to get agent ip */
  listen_port_ = "5555";
  Message msg_send;
  Message msg_recv;
  // Send register message to master, and receive config_msg.
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
  if (receiver_->Receive(&config_str) == -1) {
    printf("Error in receiving message from master\n");
    return false;
  }
  msg_recv.ParseFromString(config_str);
  config_msg = msg_recv.config_msg();
  
  // 3_2.Initialization of agent fields
  local_id_ = msg_recv.recv_id();
  agent_num_ = config_msg.worker_num();
  server_num_ = config_msg.server_num();
  key_range_  = config_msg.key)range();
  
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
  int32* part_vec = new int32[server_num_ + 1];
  config_msg.mutable_partition->ExtractSubrange(0, server_num_ + 1, part_vec);
  partition_.Initialize(key_range_, server_num_, part_vec);
  delete[] part_vec;
  
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
  para_fifo_.Open();
  grad_fifo_.Open();
  if (!AgentWork()) {
    printf("Agent work failed.\n");
    return false;
  }
  return true;
}

bool Agent::AgentWork() {
  while (true) {
    // Wait for worker's signal,
    grad_fifo_.Wait();
    gradients_ = grad_memory_.Read();
    Push();
    // Set the key_list_ for pulling
    // PS: At present, agent just request for all keys from servers
    parameters_.keys.clear();
    for (int32 i = 0; i < key_range_; i++) {
      parameters_.keys.push_back(i);
    }
    Pull();
    para_memory_.Write(&parameters_);
    para_fifo_.Signal();
  }
  return true;
}

bool Agent::Push() {
  int32 start, end, server_id, size;
  Message msg_send;
  Message_RequestMessage request_msg;
  std::string request_str;
  
  // Sort the key_value_list_ by the key, and then send them by blocks.
  SortKeyValue(gradients_.keys, gradients_.values);
  
  // Set the message type
  request_msg.set_request_type(Message_RequestMessage_RequestType_key_value);
  msg_send.set_message_type(Message_MessageType_request);
  
  // Set send_id
  msg_send.set_send_id(local_id_);
  
  // Divide key list and value list and send them to different serverss
  start = 0;
  size = gradients_.keys.size();
  while (start < size) {
    end = NextEnding(gradients_.keys, start, server_id);
    request_msg.clear_keys();
    request_msg.clear_values();
    for (int32 i = start; i < end; i++) {
      request_msg.add_keys(gradients_.keys[i]);
      request_msg.add_values(gradients_.values[i]);
    }
    msg_send.set_allocated_request_msg(request_msg);
    msg_send.set_recv_id(server_id);
    msg_send.SerializeToString(&request_str);
    if (sender_->Send(server_id, request_str) == -1) {
      printf("Cannot send push message to server:%d\n", server_id);
    }
    
    start = end;
  }
}

bool Agent::Pull() {
  // Agent will sort the key_list_, and send pull request to servers by blocks.
  // Then it will wait until it has received all the replies from the servers 
  // the agent have requested to
  int32 start, end, server_id, size;
  Message msg_send_recv;
  Message_RequestMessage request_msg;
  std::string msg_str;
  std::set<int32> server_set; 
  
  // Sort the key_list_
  sort(parameters_.keys.begin(), parameters_.keys.end());
  
  // Set the message type
  request_msg.set_request_type(Message_RequestMessage_RequestType_key);
  msg_send_recv.set_message_type(Message_MessageType_request);
  
  // Set send_id
  msg_send_recv.set_send_id(local_id_);
  
  // Divide key list and send them to different servers
  start = 0;
  size = parameters_.keys.size();
  while (start < size) {
    end = NextEnding(parameters_.keys, start, server_id);
    server_set.insert(server_id);
    request_msg.clear_keys();
    for (int32 i = start; i < end; i++) {
      request_msg.add_keys(gradients_.keys[i]);
    }
    msg_send_recv.set_allocated_request_msg(request_msg);
    msg_send_recv.set_recv_id(server_id);
    msg_send_recv.SerializeToString(&msg_str);
    if (sender_->Send(server_id, msg_str) == -1) {
      printf("Cannot send pull message to server:%d\n", server_id);
    }
    
    start = end;
  }
  
  // Receive parameters from servers
  // PS: Maybe I will add a timer for this loop. Beacuse I want to avoid
  // infinite loop caused by carshed server or servers.
  parameters_.keys.clear();
  parameters_.values.clear();
  while (!server_list.empty()) {
    if (receiver_->Receive(&msg_str) == -1) {
      printf("Error in receiving message from servers\n");
    } else {
      msg_send_recv.ParseFromString(msg_str);
      
      // Ignore wrong messages
      // Check the message type
      if (msg_send_recv.message_type() != Message_MessageType_request) {
        printf("Agent receives a message with wrong message_type\n");
        continue;
      }
      // Check the message content
      if (!msg_send_recv.has_request_message()) {
        printf("Agent receives a message without request_message\n");
        continue;
      }
      // Check the message's send_id
      if (server_set.find(msg_send_recv.send_id()) == server_set.end()) {
        printf("Agent receives a message from an unknown sender\n");
        continue;
      }
      // Check the message's recv_id
      if (msg_send_recv.recv_id() != local_id_) {
        printf("Agent receives a message with a wrong recv_id\n");
        continue;
      }
      
      // Parse the request_msg
      // Check the request_msg's type
      request_msg = msg_send_recv.request_msg();
      // PS: Maybe the agent will send a feedback message to server in the 
      // future
      if (request_msg.request_type() != 
          Message_RequestMessage_RequestType_key_value) {
        printf("Agent receives a message with wrong request_type\n");
        continue;
      }
      server_set.erase(msg_send_recv.send_id());
      size = request_msg.keys_size();
      // PS: Maybe I will allocate stable space for part_keys and part_values,
      // if I know the maximal number of key-value pairs
      // Extract parameter keys from the message
      int32* part_keys = new int32[size];
      request_msg.mutable_keys->ExtractSubrange(0, size, part_keys);
      parameters_.keys.insert(parameters_keys.end(), 
                              part_keys, part_keys + size);
      delete[] part_keys;
      // Extract parameter values from the message
      float32* part_values = new float32[size];
      request_msg.mutable_values->ExtractSubrange(0, size, part_values);
      parameters_.values.insert(paramters_values.end(),
                                part_values, part_values + size);
      delete[] part_values;
    }
  }
}

}  // namespace rpscc
