// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#include <string>

#include "src/communication/zmq_communicator.h"
#include "src/master/master.h"
#include "src/util/logging.h"
#include "master.h"


DEFINE_int32(heartbeat_timeout, 30, "The maximum time to decide "
  "whether the node is offline");

namespace rpscc {

void Master::WaitForClusterReady() {
}

void Master::Initialize(const int16 &listen_port) {
  this->sender_.reset(new ZmqCommunicator());
  this->sender_->Initialize(16, true, listen_port);
  this->receiver_.reset(new ZmqCommunicator());
  this->receiver_->Initialize(16, false, listen_port);
}

void Master::MainLoop() {
  while (true) {
    std::string* msg_str;
    receiver_->Receive(msg_str);
    Message msg;
    msg.ParseFromString(*msg_str);
    switch (msg.message_type()) {
      case Message_MessageType_config:
      case Message_MessageType_request:
        LOG (INFO) << "Invalid message type";
        break;

      case Message_MessageType_heartbeat: {
        ProcessHeartbeatMsg(msg);
        break;
      }

      case Message_MessageType_register_: {
        // Master node should response to all cluster node,
        // after master received the entire register message.
        ProcessRegisterMsg(&msg);
        if (config_.Ready()) {
          config_.GeneratePartition();
          DeliverConfig();
        }
        break;
      }
    }
  }
}

Master::Master() {

}

bool Master::DeliverConfig() {
  Message* msg = new Message();
  msg->set_send_id(0); // Id of master
  msg->set_message_type(Message_MessageType_config);
  msg->set_allocated_config_msg(config_.ToMessage());
  for (int32_t i = 0; i < config_.get_node_ip().size(); ++ i) {
    msg->set_recv_id(i);
    int32_t* buf_size = nullptr;
    char* buf = nullptr;
    *buf_size = msg->ByteSize();
    buf = new char[*buf_size + 1];
    msg->SerializeToArray(buf, *buf_size);
    auto send_byte = sender_->Send(i, buf, *buf_size);
    LOG(INFO) << "Send to " << i << " config of " << send_byte;
  }
  return true;
}

void Master::ProcessRegisterMsg(Message *msg) {
  auto register_msg = msg->register_msg();
  bool is_server = register_msg.is_server();
  std::string ip = register_msg.ip();
  int32_t port = register_msg.port();
  if (is_server) {
    config_.AppendServer(ip, port);
  } else {
    config_.AppendAgent(ip, port);
  }
}

void Master::ProcessHeartbeatMsg(const Message& msg) {
  CHECK(msg.has_heartbeat_msg());
  auto heartbeat_msg = msg.heartbeat_msg();
  CHECK(heartbeat_msg.is_live());
  auto send_id = msg.send_id();
  time_t cur_time = time(NULL);
  //CHECK(alive_node_.find(send_id) != alive_node_.end());
  alive_node_[send_id] = cur_time;
  LOG (INFO) << "Heartbeat from " << send_id << ", ip = "
             << config_.GetIp(send_id);
}

std::vector<int> Master::GetDeadNode() {
  std::vector<int> dead_node;
  auto cur_time = time(NULL);
  for (const auto& pr: alive_node_) {
    if (pr.second + FLAGS_heartbeat_timeout < cur_time) {
      dead_node.push_back(pr.first);
    }
  }
  return dead_node;
}

int32_t Master::Initialize(const std::string &master_ip_port) {
  std::stringstream ss(master_ip_port);
  int count = 0;
  while (ss.good()) {
    std::string ip_port;
    std::getline(ss, ip_port, ',');
    if (ip_port.size() > 0) {
      config_.AppendMaster(ip_port);
      ++ count;
    }
  }
  return count;
}

}
