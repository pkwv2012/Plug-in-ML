// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#include <string>

#include "src/communication/zmq_communicator.h"
#include "src/master/master.h"
#include "src/util/logging.h"
#include "master.h"


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
        CHECK(msg.has_heartbeat_msg());
        auto heartbeat_msg = msg.heartbeat_msg();
        CHECK(heartbeat_msg.is_live());
        auto send_id = msg.send_id();
        time_t cur_time = time(NULL);
        //CHECK(alive_node_.find(send_id) != alive_node_.end());
        alive_node_[send_id] = cur_time;
        LOG (INFO) << "Heartbeat from " << send_id << ", ip = "
                   << config_.GetIp(send_id);
        break;
      }

      case Message_MessageType_register_: {
        // Master node should response to all cluster node,
        // after master received the entire register message.
        ProcessRegisterMsg(&msg);
        if (config_.Ready()) {
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

  return false;
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

void Master::ProcessHeartbeatMsg(Message *msg) {

}

}
