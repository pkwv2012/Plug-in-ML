// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#include <string>
#include <fstream>

#include "src/communication/zmq_communicator.h"
#include "src/master/master.h"
#include "src/util/logging.h"
#include "master.h"


namespace rpscc {

DEFINE_int32(heartbeat_timeout, 30, "The maximum time to decide "
  "whether the node is offline");
DEFINE_int32(listen_port, 16666, "The listening port of cluster.");


void Master::WaitForClusterReady() {
}

void Master::Initialize(const int16 &listen_port) {
  this->sender_.reset(new ZmqCommunicator());
  this->sender_->Initialize(16, true, listen_port);
  this->receiver_.reset(new ZmqCommunicator());
  this->receiver_->Initialize(16, false, listen_port);
}

void Master::MainLoop() {
  std::cout << "Main loop";
  bool terminated = false;
  while (! terminated) {
    std::string msg_str;
    LOG(INFO) << "Receiving";
    int32_t len = receiver_->Receive(&msg_str);
    std::fstream fs;
    fs.open("master_msg.txt", std::fstream::app);
    fs << msg_str << std::endl;
    fs.close();
    LOG(INFO) << "Master||Receive " << len << " bytes" << std::endl;
    LOG(INFO) << "Receive finish";
    Message msg;
    msg.ParseFromString(msg_str);
    LOG(INFO) << msg.DebugString() << std::endl;
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
          LOG(INFO) << "Cluster ready!";
          config_.GeneratePartition();
          for (auto pr: config_.get_id_to_addr()) {
            sender_->AddIdAddr(pr.first, pr.second);
          }
          DeliverConfig();
        }
        LOG(INFO) << "worker_num=" << config_.worker_num()
                  << "server_num=" << config_.server_num()
                  << "worker_cur_num=" << config_.agent_id().size()
                  << "server_cur_num=" << config_.server_id().size()
                  << std::endl;
        break;
      }
      case Message_MessageType_terminate: {
        terminated_node_.insert(msg.send_id());
        if (terminated_node_.size() == config_.worker_num()) {
          terminated = true;
        }
        break;
      }
    }
  }
}

Master::Master() {
  LOG(INFO) << "Master initialization" << std::endl;
  config_.Initialize(""/*config file name*/);
}

bool Master::DeliverConfig() {
  Message* msg = new Message();
  msg->set_send_id(0); // Id of master
  msg->set_message_type(Message_MessageType_config);
  msg->set_allocated_config_msg(config_.ToMessage());
  LOG(INFO) << "config right" << std::endl;
  for (int32_t i = 0; i < config_.get_node_ip().size(); ++ i) {
    msg->set_recv_id(i);
    LOG(INFO) << i << std::endl;
    auto send_byte = sender_->Send(i, msg->SerializeAsString());
    LOG(INFO) << "Send to " << i << " config of " << send_byte;
  }
  delete msg;
  return true;
}

void Master::ProcessRegisterMsg(Message *msg) {
  auto register_msg = msg->register_msg();
  bool is_server = register_msg.is_server();
  std::string ip = register_msg.ip();
  int32_t port = register_msg.port();
  LOG(INFO) << "Master " << is_server << "  port" << port << std::endl;
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
  std::cout << "Start Initialize";
  this->sender_.reset(new ZmqCommunicator());
  LOG(INFO) << "Create sender" << std::endl;
  this->sender_->Initialize(16, true, FLAGS_listen_port);
  LOG(INFO) << "Sender finish" << std::endl;
  this->receiver_.reset(new ZmqCommunicator());
  this->receiver_->Initialize(16, false, FLAGS_listen_port);
  LOG(INFO) << "Master init finish." << "count = " << count << std::endl;
  return count;
}

}
