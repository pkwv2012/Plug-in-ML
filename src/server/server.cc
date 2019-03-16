// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include <string>

#include "gflags/gflags.h"
#include "src/server/server.h"
#include "src/util/logging.h"
#include "src/util/network_util.h"

namespace rpscc {

DEFINE_string(net_interface, "",
  "Name of the net interface used by the node.");

// note that server heartbeat port equals "server_port" + 1
DEFINE_int32(server_port, 8888, "Port used by the server receiver.");
DEFINE_int32(ring_size, 64, "Size of communicator's message queue.");
DEFINE_int32(buffer_size, 2048, "Size of each message's buffer.");
DEFINE_string(master_ip_port, "", "IP and Port of the first master node.");


// In Initialize() the server configures itself by sending its IP to the
// master and receiving related configuration information.
bool Server::Initialize() {
  // First initialize server communicators
  sender_.reset(new ZmqCommunicator());
  receiver_.reset(new ZmqCommunicator());
  receiver_heatbeat_.reset(new ZmqCommunicator());
  sender_->Initialize(FLAGS_ring_size, true, 1024, FLAGS_buffer_size);
  sender_->AddIdAddr(0, FLAGS_master_ip_port);
  receiver_->Initialize(FLAGS_ring_size, false, FLAGS_server_port,
    FLAGS_buffer_size);
  receiver_heatbeat_->Initialize(FLAGS_ring_size, false, FLAGS_server_port + 1,
    FLAGS_buffer_size);

  // Send the server local ip to master and receive config information
  std::string ip = "";
  GetIP(FLAGS_net_interface, &ip);
  if (ip == "") {
    LOG(ERROR) << "Cannot find IP from the interface provided.";
    return false;
  }
  local_address_ = ip;
  Message* msg_send = new Message;
  Message msg_recv;
  Message_RegisterMessage* reg_msg = new Message_RegisterMessage;
  Message_ConfigMessage config_msg;
  std::string reg_str;
  std::string config_str;
  reg_msg->set_ip(local_address_);
  reg_msg->set_is_server(true);
  reg_msg->set_port(FLAGS_server_port);
  msg_send->set_message_type(Message_MessageType_register_);
  msg_send->set_recv_id(0);
  msg_send->set_send_id(-1);
  msg_send->set_allocated_register_msg(reg_msg);
  msg_send->SerializeToString(&reg_str);
  delete msg_send;

  // TODO(Song Xu): to improve robustness multiple attemptions can be made
  // before we return false.
  // TODO(Song Xu): the configuration message should be checked
  // (type and content) before we use the information in it.
  // If the message is fault or corrupted
  // the server should try again to get another message.
  if (sender_->Send(0, reg_str) == -1) {
    LOG(ERROR) << "Failed to send register information to master.";
    return false;
  }
  if (receiver_->Receive(&config_str) == -1) {
    LOG(ERROR) << "Failed to receive configuration information from master.";
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

  // Initialization of sender's id mapping to ip-ports, where the id 0 is
  // already added.
  for (uint32 i = 1; i < config_msg.node_ip_port_size(); ++i) {
    sender_->AddIdAddr(i, config_msg.node_ip_port(i));
  }

  // Initialize server ids, and record local parameter range from
  // config_msg.partition.
  // Note that config_msg.partition should be a array of length #server + 1.
  // It's first element must be 0 and the last must be config_msg.key_range.
  bool found_local = false;
  for (uint32 i = 0; i < server_num_; ++i) {
    uint32 server_i_id = config_msg.server_id(i);
    server_ids_.push_back(server_i_id);
    if (server_i_id == local_id_) {
      start_key_ = config_msg.partition(i);
      parameter_length_ = config_msg.partition(i + 1) - start_key_;
      found_local = true;
    }
  }
  if (found_local == false) {
    LOG(ERROR) << "Nothing is found to be assigned to the server.";
    return false;
  }

  // Initialize agent id set
  for (uint32 i = 0; i < config_msg.worker_id_size(); ++i) {
    agent_ids_.insert(config_msg.worker_id(i));
  }

  // Initialize master ids.
  for (uint32 i = 0; i < config_msg.master_id_size(); ++i) {
    master_ids_.push_back(config_msg.master_id(i));
  }

  // Initialize map from worker ID to version buffer index
  for (uint32 i = 0; i < agent_num_; ++i)
    id_to_index_[config_msg.worker_id(i)] = i;

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
bool Server::RespondToAll() {
  while (pull_request_.empty() == false) {
    std::string reply_str;
    PullInfo request = pull_request_.front();
    pull_request_.pop();
    Message* msg_send = new Message;
    Message_RequestMessage* reply_msg = new Message_RequestMessage;
    reply_msg->set_request_type(Message_RequestMessage_RequestType_key_value);
    uint32 len = request.Length();
    for (uint32 i = 0; i < len; ++i) {
      reply_msg->add_keys(request.Key(i));
      reply_msg->add_values(parameters_[request.Key(i) - start_key_]);
    }
    msg_send->set_send_id(local_id_);
    msg_send->set_recv_id(request.get_id());
    msg_send->set_message_type(Message_MessageType_request);
    msg_send->set_allocated_request_msg(reply_msg);
    msg_send->SerializeToString(&reply_str);
    delete msg_send;

    // TODO(Song Xu): we'd better try more times before give up replying, and
    // if we decide to give up for one agent, we shoule send a message to warn
    // it about the situation.
    if (sender_->Send(request.get_id(), reply_str) == -1) {
      LOG(ERROR) << "Failed to respond to worker " << request.get_id()
                 << "'s pull request which is blocked before";
    }
  }
}

// After the server receives update of one version from all agents,
// UpdateParameter is called to merge the updates to current parameter.
// Simple implementation -- average.
void Server::UpdateParameter() {
  std::vector<float> update(parameter_length_, 0.0f);
  for (uint32 i = 0; i < agent_num_; ++i) {
    KeyValueList update_i = version_buffer_[i].front();
    version_buffer_[i].pop();
    uint32 len = update_i.Length();
    for (uint32 j = 0; j < len; ++j)
      update[update_i.Key(j) - start_key_] += update_i.Value(j);
  }
  for (uint32 i = 0; i < parameter_length_; ++i) {
    parameters_[i] += update[i] / agent_num_;
  }
  bottom_version_++;
}

// In Start(), the server repeatedly receive message from agents, and
// handle the requests according to their type. It is here that
// UpdateParameter() and ResponseAll() will be called.
void Server::Start() {
  // first initialize heartbeat handling thread
  pthread_create(&heartbeat_, NULL, HeartBeat, reinterpret_cast<void*>(this));

  while (true) {
    std::string recv_str;
    receiver_->Receive(&recv_str);
    Message msg_recv;
    msg_recv.ParseFromString(recv_str);
    uint32 sender_id = msg_recv.send_id();

    if (msg_recv.message_type() == Message_MessageType_request) {
      Message_RequestMessage request = msg_recv.request_msg();
      if (request.request_type()
        == Message_RequestMessage_RequestType_key_value) {
        // Push request:
        ServePush(sender_id, request);
      } else if (request.request_type()
        == Message_RequestMessage_RequestType_key) {
        // Pull request:
        ServePull(sender_id, request);
      }
    }

    if (msg_recv.message_type() == Message_MessageType_config) {
      Message_ConfigMessage config_msg = msg_recv.config_msg();
      Reconfigurate(config_msg);
    }
  }
}

// Serve push request. Since consistency will be handled by ServePull(),
// we can always insert the push update in the queue.
// If a round of version update is finished after the push, UpdateParameter()
// and RespondToAll() will be called to return the new version of parameters
// to the blocked workers.
void Server::ServePush(uint32 sender_id,
  const Message_RequestMessage &request) {
  if (agent_ids_.find(sender_id) == agent_ids_.end()) {
    LOG(ERROR) << "Got push request from worker " << sender_id
               << ", which is unknown by the server.";
    return;
  }
  finish_count_[version_buffer_[id_to_index_[sender_id]].size()]++;
  KeyValueList worker_update;
  for (uint32 i = 0; i < request.keys_size(); ++i)
    worker_update.AddPair(request.keys(i), request.values(i));
  version_buffer_[id_to_index_[sender_id]].push(worker_update);

  // Acknowledgement from server
  std::string send_str;
  Message_RequestMessage* ack_msg = new Message_RequestMessage;
  Message* msg_send = new Message;
  ack_msg->set_request_type(Message_RequestMessage_RequestType_ack);
  msg_send->set_message_type(Message_MessageType_request);
  msg_send->set_allocated_request_msg(ack_msg);
  msg_send->set_send_id(local_id_);
  msg_send->set_recv_id(sender_id);
  msg_send->SerializeToString(&send_str);
  delete msg_send;
  if (sender_->Send(sender_id, send_str) == -1) {
    LOG(ERROR) << "Failed to respond to worker " << sender_id
               << "'s push request.";
  }

  // Update of the bottom version is done
  // We can call UpdateParameter() when finish_count_[0] is larger than
  // m*agent_num_/n, as an expedience between speed and consistency.
  if (finish_count_[0] == agent_num_) {
    finish_count_.pop_front();
    finish_count_.push_back(0);
    UpdateParameter();
    RespondToAll();
  }
}

// ServePull() will handle version consistency by checking the number of
// updates that the worker has already committed but is not yet processed by
// the server. If the number of updates in version_buffer is too large, the
// pull request will be blocked.
void Server::ServePull(uint32 sender_id,
  const Message_RequestMessage &request) {
  if (id_to_index_.find(sender_id) == id_to_index_.end()) {
    LOG(ERROR) << "Got pull request from worker " << sender_id
      << ", which is unknown by the server.";
    return;
  }
  // Blocked when enough update is pushed but not yet processed
  // A block message will be sent to the sender agent
  if (version_buffer_[id_to_index_[sender_id]].size()
    >= consistency_bound_) {
    PullInfo blocked_request;
    blocked_request.set_id(sender_id);
    for (uint32 i = 0; i < request.keys_size(); ++i)
      blocked_request.AddKey(request.keys(i));
    pull_request_.push(blocked_request);

    std::string send_str;
    Message_RequestMessage* block_msg = new Message_RequestMessage;
    Message* msg_send = new Message;
    block_msg->set_request_type(Message_RequestMessage_RequestType_block);
    msg_send->set_message_type(Message_MessageType_request);
    msg_send->set_allocated_request_msg(block_msg);
    msg_send->set_send_id(local_id_);
    msg_send->set_recv_id(sender_id);
    msg_send->SerializeToString(&send_str);
    delete msg_send;
    if (sender_->Send(sender_id, send_str) == -1) {
      LOG(ERROR) << "Failed to send block message for worker " << sender_id
        << "'s pull request.";
    }
  } else {
    std::string reply_str;
    Message* msg_send = new Message;
    Message_RequestMessage* reply_msg = new Message_RequestMessage;
    reply_msg->set_request_type(
      Message_RequestMessage_RequestType_key_value);
    for (uint32 i = 0; i < request.keys_size(); ++i) {
      reply_msg->add_keys(request.keys(i));
      reply_msg->add_values(parameters_[request.keys(i) - start_key_]);
    }
    msg_send->set_message_type(Message_MessageType_request);
    msg_send->set_allocated_request_msg(reply_msg);
    msg_send->set_send_id(local_id_);
    msg_send->set_recv_id(sender_id);
    msg_send->SerializeToString(&reply_str);
    delete msg_send;
    if (sender_->Send(sender_id, reply_str) == -1) {
      LOG(ERROR) << "Failed to respond to worker " << sender_id
        << "'s pull request.";
    }
  }
}

// Heartbeat function receives liveness check from master and reply as a
// heartbeat. Server won't terminate itself or change to a new master if
// current master is not heard for a long time. Instead, it waits for
// another master to send live check to it.
void* Server::HeartBeat(void* arg) {
  Server* server = reinterpret_cast<Server*>(arg);
  Message send_msg, recv_msg;
  Message_HeartbeatMessage* hb_msg = new Message_HeartbeatMessage();
  std::string send_str, recv_str;

  hb_msg->set_is_live(true);
  send_msg.set_message_type(Message_MessageType_heartbeat);
  send_msg.set_send_id(server->local_id_);

  while (1) {
    if (server->receiver_heatbeat_->Receive(&recv_str) == -1) {
      LOG(ERROR) << "Error in receiving heartbeat from master";
    }
    recv_msg.ParseFromString(recv_str);

    // Config the send_msg
    send_msg.set_recv_id(recv_msg.send_id());
    send_msg.set_allocated_heartbeat_msg(hb_msg);
    send_msg.SerializeToString(&send_str);

    if (server->sender_->Send(0, send_str) == -1) {
      LOG(ERROR) << "Cannot send a heartbeat to master";
    }
  }
}

// when server receives a re-config order from masterm it will
// change the system meta data stored.
void Server::Reconfigurate(const Message_ConfigMessage &config_msg) {
  agent_num_ = config_msg.worker_num();
  server_num_ = config_msg.server_num();

  // Reconfiguration of sender's id mapping to ip-ports, where the id 0 is
  // already added.
  for (uint32 i = 1; i < config_msg.node_ip_port_size(); ++i) {
    sender_->AddIdAddr(i, config_msg.node_ip_port(i));
  }

  // Reconfigurate server ids, and record local parameter range from
  // config_msg.partition.
  // Note that config_msg.partition should be a array of length #server + 1.
  // It's first element must be 0 and the last must be config_msg.key_range.
  bool found_local = false;
  server_ids_.clear();
  for (uint32 i = 0; i < server_num_; ++i) {
    uint32 server_i_id = config_msg.server_id(i);
    server_ids_.push_back(server_i_id);
    if (server_i_id == local_id_) {
      start_key_ = config_msg.partition(i);
      parameter_length_ = config_msg.partition(i + 1) - start_key_;
      found_local = true;
    }
  }
  if (found_local == false) {
    LOG(ERROR) << "Nothing is found to be assigned to the server.";
    exit(1);
  }

  // if an agent recorded in the server is not in the new configuration,
  // the agent is considered to be out of service. Therefore, server need
  // to first remove the agent's last updates.
  __gnu_cxx::hash_set<uint32>::iterator begin = agent_ids_.begin(),
  end = agent_ids_.end(), iter;
  for (iter = begin; iter != end; iter++) {
    bool found = false;
    for (uint32 i = 0; i < config_msg.worker_id_size(); ++i) {
      if (config_msg.worker_id(i) == *iter) {
        found = true;
        break;
      }
    }
    if (found == false) {
      for (uint32 i = 0; i < version_buffer_[id_to_index_[*iter]].size();
        ++i)
        finish_count_[i]--;
    }
  }
  // refresh agent id set
  agent_ids_.clear();
  for (uint32 i = 0; i < config_msg.worker_id_size(); ++i) {
    agent_ids_.insert(config_msg.worker_id(i));
  }

  // refresh master ids.
  master_ids_.clear();
  for (uint32 i = 0; i < config_msg.master_id_size(); ++i) {
    master_ids_.push_back(config_msg.master_id(i));
  }

  // refresh map from worker ID to version buffer index
  for (uint32 i = 0; i < agent_num_; ++i) {
    if (id_to_index_.find(config_msg.worker_id(i)) == id_to_index_.end()) {
      id_to_index_[config_msg.worker_id(i)] = version_buffer_.size();
      version_buffer_.push_back(std::queue<KeyValueList>());
    }
  }
}

}  // namespace rpscc
