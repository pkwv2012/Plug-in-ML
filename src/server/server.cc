// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include <string>

#include "src/message/message.pb.h"
#include "src/server/server.h"

namespace rpscc {

// In Initialize() the server configures itself by sending its IP to the
// master and receiving related configuration information.
bool Server::Initialize() {
  // First initialize server communicators
  sender_.reset(new ZmqCommunicator());
  receiver_.reset(new ZmqCommunicator());
  sender_->Initialize(/* size */, true, 1024, /* size */);
  sender_->AddIdAddr(0, /* master address and port */);
  receiver_->Initialize(/* size */, false, /* port */, /* size */);

  // Send the server local ip to master and receive config information
  local_ip_ = /* find a way to get server ip */;
  Message msg_send();
  Message msg_recv();
  Message_RegisterMessage reg_msg();
  Message_ConfigMessage config_msg();
  std::string reg_str;
  std::string config_str;
  reg_msg.set_ip(local_ip_);
  msg_send.set_message_type(Message_MessageType_register);
  msg_send.set_recv_id(0);
  msg_send.set_send_id(-1);
  msg_send.set_allocated_register_msg(&reg_msg);
  msg_send.SerializeToString(&reg_str);
  if (sender_->Send(0, reg_str) == -1) {
    /* handle */
    return false;
  }
  if (receiver_->Receive(&config_str) == -1) {
    /* handle */
    return false;
  }
  msg_recv.ParseFromString(config_str);
  config_msg = msg_recv.config_msg();

  // Initialization of server fields
  local_id_ = msg_recv.recv_id();
  bottom_version_ = 0;
  consistency_bound_ = config_msg.bound();
  agent_num_ = config_msg.worker_num();
  server_num_ = config_msg.server_num();

  // Initialize server ips and ids,
  // and record local parameter range from config_msg.partition.
  // Note that config_msg.partition should be a array of length #server + 1.
  // It's first element must be 0 and the last must be config_msg.key_range.
  bool found_local = false;
  for (uint32 i = 0; i < server_num_; ++i) {
    std::string server_i_ip = config_msg.server_ip(i);
    uint32 server_i_id = config_msg.server_id(i);
    server_ips_.push_back(server_i_ip);
    server_ids_.push_back(server_i_id);
    if (server_i_id == local_id_) {
      start_key_ = config_msg.partition(i);
      parameter_length_ = config_msg.partition(i + 1) - start_key_;
      found_local = true;
    }
  }
  if (found_local == false) {
    /* handle */
    return false;
  }

  // By default, all parameters are initialized to be zero
  for (uint32 i = 0; i < parameter_length_; ++i)
    parameters_.push_back(0.0f);

  // Initialize the deque finish_count to be zeros, it's length should be
  // equal to consistency bound. To maintain finish_count, It's length must
  // stay unchanged throughout the program.
  for (uint32 i = 0; i < consistency_bound_; ++i)
    finish_count_.push_back(0);

  for (uint32 i = 0; i < agent_num_; ++i)
    version_buffer_.push_back(std::queue<KeyValueList>());

  return true;
}

// ResponseAll is invoked once an update is applied to the parameters.
// The server use this function to reply to the blocked pull requests.
bool Server::ResponseAll() {
  while (pull_request_.empty() == false) {
    std::string reply_str;
    PullInfo request = pull_request_.pop();
    Message msg_send();
    Message_RequestMessage reply_msg();
    reply_msg.set_request_type(Message_RequestMessage_RequestType_key_value);
    uint32 len = request.Length();
    for (uint32 i = 0; i < len; ++i) {
      reply_msg.add_keys(request.Key(i));
      reply_msg.add_values(parameters[request.Key(i) - start_key_]);
    }
    msg_send.set_send_id(local_id_);
    msg_send.set_recv_id(request.get_id());
    msg_send.set_message_type(Message_MessageType_request);
    msg_send.set_allocated_request_msg(reply_msg);
    msg_send.SerializeToString(&reply_str);
    if (sender_->Send(request.get_id(), reply_str) == -1) {
      /* handle */
    }
  }
}

// After the server receives update of one version from all agents,
// UpdateParameter is called to merge the updates to current parameter.
// Simple implementation -- average.
void Server::UpdateParameter() {
  std::vector<float> update(parameter_length_, 0.0f);
  for (uint32 i = 0; i < agent_num_; ++i) {
    KeyValueList update_i = version_buffer_[i].pop();
    uint32 len = update_i.Length();
    for (uint32 j = 0; j < len; ++j)
      update[update_i.Key(j) - start_key_] += update_i.Value(j);
  }
  for (uint32 i = 0; i < parameter_length_; ++i) {
    parameters_[i] += update[i] / agent_num_;
  }
}

// In Start(), the server repeatedly receive message from agents, and
// handle the requests according to their type. It is here that
// UpdateParameter() and ResponseAll() will be called.
void Server::Start() {
  while (true) {
    std:string recv_str;
    receiver_->Receive(&recv_str);
    Message msg_recv();
    msg_recv.ParseFromString(recv_str);
    uint32 sender_id = msg_recv.send_id();
    if (msg_recv.message_type() == Message_MessageType_request) {
      Message_RequestMessage request = msg_recv.request_msg();
      // Push request:
      if (request.request_type()
          == Message_RequestMessage_RequestType_key_value) {
        
      }
    }
  }
}

}  // namespace rpscc
