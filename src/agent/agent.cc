// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include <stdio.h>

#include <algorithm>
#include <set>

#include "src/agent/agent.h"
#include "src/communication/zmq_communicator.h"
#include "src/message/message.pb.h"
#include "src/util/logging.h"
#include "src/util/network_util.h"

using namespace std;

namespace rpscc {

// Macro for getting the Agent's IP address
DEFINE_string(net_interface, "",
              "Name of the net interface used by the node.");

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
                       int32 shared_memory_size,
                       std::string master_addr) {
  // 1.Initialize sender

  cout << "1.Initialize sender" << endl;

  sender_.reset(new ZmqCommunicator());
  if (sender_.get() == NULL) {
    printf("Initialize sender failed.");
    return false;
  }
  sender_->Initialize(64/* ring_size */, true, 1024/* listen_port */);

  // 2.Initialize receiver

  cout << "2.Initialize receiver" << endl;

  receiver_.reset(new ZmqCommunicator());
  if (receiver_.get() == NULL) {
    printf("Initialize receiver failed.");
    return false;
  }
  receiver_->Initialize(64/* ring_size */, false, 5555/* listen_port */);

  // 3 Exchange messages with master
  // 3_1.Send this agent's local_ip_ to master, and receive config information
  std::string ip = "";
  GetIP(FLAGS_net_interface, &ip);
  if (ip == "") {
    LOG(ERROR) << "Cannot find IP from the interface provided.";
    return false;
  }

  cout << "3_1 Agent's ip is " << ip << endl;

  local_ip_ = ip;
  listen_port_ = 5555;
  Message msg_send;
  Message msg_recv;
  // Send register message to master, and receive config_msg.
  Message_RegisterMessage reg_msg;
  Message_ConfigMessage config_msg;
  std::string reg_str;
  std::string config_str;

  reg_msg.set_ip(local_ip_);
  reg_msg.set_port(listen_port_);
  reg_msg.set_is_server(false);
  msg_send.set_message_type(Message_MessageType_register_);
  msg_send.set_recv_id(0);
  msg_send.set_send_id(-1);
  msg_send.set_allocated_register_msg(&reg_msg);
  msg_send.SerializeToString(&reg_str);
  cout << "3_1 Sending register string to master" << endl;
  sender_->AddIdAddr(0, master_addr);
  if (sender_->Send(0, reg_str) == -1) {
    LOG(ERROR) << "Cannot send register message to master";
    return false;
  }
  cout << "3_2 Receiving config string from master" << endl;
  if (receiver_->Receive(&config_str) == -1) {
    LOG(ERROR) << "Error in receiving message from master";
    return false;
  }

  cout << "config_str.size() = " << config_str.size() << endl;
  msg_recv.ParseFromString(config_str);
  config_msg = msg_recv.config_msg();
  // 3_2.Initialization of agent fields 
  local_id_ = msg_recv.recv_id();
  agent_num_ = config_msg.worker_num();
  server_num_ = config_msg.server_num();
  key_range_  = config_msg.key_range();

  cout << "3_2 Initialization " << "local_id = " << local_id_
       << " agent_num_ = " << agent_num_ << " server_num_ = "
       << server_num_ << " key_range_ = " << key_range_ << endl;

  // 3_3.Add <server_id, server_addr> pairs to the sender_'s <Id, Addr> map.
  for (int32 i = 0; i < server_num_; i++) {
    int32 server_i_id = config_msg.server_id(i);
    cout << "server_id = " << server_i_id;
    std::string server_i_ip_port = config_msg.node_ip_port(server_i_id);
    
    cout << " ip_port = " << server_i_ip_port
         << endl;
    
    sender_->AddIdAddr(server_i_id, server_i_ip_port);
  }

  cout << "3_3.Add <server_id, server_addr> pairs to the sender_'s"
       << " <Id, Addr> map." << endl;

  // 3_4.Initialize the partition_.

  cout << "3_4.Initialize the partition_." << endl;

  int32* part_vec = new int32[server_num_ + 1];
  config_msg.mutable_partition()->ExtractSubrange(0, server_num_ + 1,
                                                  part_vec);
  partition_.Initialize(key_range_, server_num_, part_vec);
  delete[] part_vec;

  // 4.Initialize the fifo and shared memory
  cout << "4.Initialize the fifo and shared memory" << endl;
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
  grad_memory_.Initialize(grad_memory_name_.c_str());

  cout << "Agent's initialization is done" << endl;
  return true;
}

void Agent::Finalize() {
  sender_->Finalize();
  receiver_->Finalize();
  partition_.Finalize();
}

bool Agent::Start() {
  para_fifo_.Open();
  grad_fifo_.Open();
  if (!AgentWork()) {
    LOG(ERROR) << "Agent work failed.";
    return false;
  }
  return true;
}

bool Agent::AgentWork() {
  while (true) {
    // Wait for worker's signal,
    grad_fifo_.Wait();
    gradients_ = *grad_memory_.Read();
    Push();
    // Set the key_list_ for pulling
    // PS: At present, agent just request for all keys from servers
    for (int32 i = 0; i < key_range_; i++) {
      parameters_.keys[i] = i;
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
  // SortKeyValue(gradients_.keys, gradients_.values);

  // Set the message type
  request_msg.set_request_type(Message_RequestMessage_RequestType_key_value);
  msg_send.set_message_type(Message_MessageType_request);

  // Set send_id
  msg_send.set_send_id(local_id_);

  // Divide key list and value list and send them to different serverss
  start = 0;
  size = gradients_.size;
  while (start < size) {
    // *@&(#&(@&$(&@)!*$)@*!)$)!@$)!@$)@)
    end = partition_.NextEnding(std::vector<int>(parameters_.keys,
                              parameters_.keys + 100), start, server_id);
    request_msg.clear_keys();
    request_msg.clear_values();
    for (int32 i = start; i < end; i++) {
      request_msg.add_keys(gradients_.keys[i]);
      request_msg.add_values(gradients_.values[i]);
    }
    msg_send.set_allocated_request_msg(&request_msg);
    msg_send.set_recv_id(server_id);
    msg_send.SerializeToString(&request_str);
    if (sender_->Send(server_id, request_str) == -1) {
      LOG(ERROR) << "Cannot send push message to server:" << server_id;
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
  // sort(parameters_.keys.begin(), parameters_.keys.end());

  // Set the message type
  request_msg.set_request_type(Message_RequestMessage_RequestType_key);
  msg_send_recv.set_message_type(Message_MessageType_request);

  // Set send_id
  msg_send_recv.set_send_id(local_id_);

  // Divide key list and send them to different servers
  start = 0;
  size = parameters_.size;
  while (start < size) {
    // ^&(@#&@)#*)@!*)#*@!)#*)!@*)*)
    end = partition_.NextEnding(std::vector<int>(parameters_.keys,
                              parameters_.keys + 100), start, server_id);
    server_set.insert(server_id);
    request_msg.clear_keys();
    for (int32 i = start; i < end; i++) {
      request_msg.add_keys(gradients_.keys[i]);
    }
    msg_send_recv.set_allocated_request_msg(&request_msg);
    msg_send_recv.set_recv_id(server_id);
    msg_send_recv.SerializeToString(&msg_str);
    if (sender_->Send(server_id, msg_str) == -1) {
      LOG(ERROR) << "Cannot send pull message to server:" << server_id;
    }

    start = end;
  }

  // Receive parameters from servers
  // PS: Maybe I will add a timer for this loop. Beacuse I want to avoid
  // infinite loop caused by carshed server or servers.

  int32 cur = 0;
  parameters_.size = 0;

  while (!server_set.empty()) {
    if (receiver_->Receive(&msg_str) == -1) {
      LOG(ERROR) << "Error in receiving message from servers";
    } else {
      msg_send_recv.ParseFromString(msg_str);

      // Ignore wrong messages
      // Check the message type
      if (msg_send_recv.message_type() != Message_MessageType_request) {
        LOG(ERROR) << "Agent receives a message with wrong message_type";
        continue;
      }
      // Check the message content
      if (!msg_send_recv.has_request_msg()) {
        LOG(ERROR) << "Agent receives a message without request_message";
        continue;
      }
      // Check the message's send_id
      if (server_set.find(msg_send_recv.send_id()) == server_set.end()) {
        LOG(ERROR) << "Agent receives a message from an unknown sender";
        continue;
      }
      // Check the message's recv_id
      if (msg_send_recv.recv_id() != local_id_) {
        LOG(ERROR) << "Agent receives a message with a wrong recv_id";
        continue;
      }

      // Parse the request_msg
      // Check the request_msg's type
      request_msg = msg_send_recv.request_msg();
      // PS: Maybe the agent will send a feedback message to server in the
      // future
      if (request_msg.request_type() !=
          Message_RequestMessage_RequestType_key_value) {
        LOG(ERROR) << "Agent receives a message with wrong request_type";
        continue;
      }
      server_set.erase(msg_send_recv.send_id());
      size = request_msg.keys_size();
      // PS: Maybe I will allocate stable space for part_keys and part_values,
      // if I know the maximal number of key-value pairs
      // Extract parameter keys from the message
      int32* part_keys = new int32[size];
      request_msg.mutable_keys()->ExtractSubrange(0, size, part_keys);
      memcpy(parameters_.keys + cur, part_keys, size);

      delete[] part_keys;
      // Extract parameter values from the message
      float32* part_values = new float32[size];
      request_msg.mutable_values()->ExtractSubrange(0, size, part_values);
      memcpy(parameters_.values + cur, part_values, size);
      delete[] part_values;

      cur += size;
    }
  }
}

}  // namespace rpscc

