// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Created by PeikaiZheng on 2018/12/8.

#include <thread>

#include "gtest/gtest.h"
#include "src/communication/zmq_communicator.h"
#include "src/master/master.h"
#include "src/message/message.pb.h"
#include "src/util/logging.h"

using namespace rpscc;

void AgentSimulator(std::string master_addr, int32_t agent_port) {
  rpscc::Communicator* sender = new rpscc::ZmqCommunicator();
  sender->Initialize(16, true, /* this port is useless */ 100, 128);
  sender->AddIdAddr(0, master_addr);
  rpscc::Message msg;
  msg.set_send_id(-1);
  msg.set_recv_id(0);
  msg.set_message_type(rpscc::Message_MessageType_register_);
  Message_RegisterMessage* register_msg = new Message_RegisterMessage();
  std::string agent_ip = "127.0.0.1";
  register_msg->set_ip(agent_ip);
  register_msg->set_is_server(false);
  LOG(INFO) << "Agent port " << agent_port << std::endl;
  register_msg->set_port(agent_port);
  msg.set_allocated_register_msg(register_msg);

  rpscc::Communicator* receiver = new rpscc::ZmqCommunicator();
  receiver->Initialize(16, false, agent_port, 128);
  std::string msg_str = msg.SerializeAsString();
  LOG(INFO) << "Agent sending register msg." << std::endl;
  sender->Send(0, msg_str);
  LOG(INFO) << "Agent send register msg done" << std::endl;
  std::string msg_got;
  receiver->Receive(&msg_got);
  LOG(INFO) << "Agent receive msg done." << std::endl;
  Message msg_recv;
  msg_recv.ParseFromString(msg_got);
  EXPECT_EQ(msg_recv.send_id(), 0);
  int32_t agent_id = msg_recv.recv_id();
  EXPECT_STREQ(msg_recv.config_msg().node_ip_port(agent_id).c_str(),
               (agent_ip + ":" + std::to_string(agent_port)).c_str());

  msg.clear_register_msg();
  msg.set_message_type(Message_MessageType_terminate);
  sender->Send(0, msg.SerializeAsString());

  delete sender;
  delete receiver;
}

void ServerSimulator(std::string master_addr, int32_t server_port) {
  rpscc::Communicator* sender = new rpscc::ZmqCommunicator();
  sender->Initialize(16, true, /* this port is useless */ 100, 128);
  sender->AddIdAddr(0, master_addr);
  rpscc::Message msg;
  msg.set_send_id(-1);
  msg.set_recv_id(0);
  msg.set_message_type(rpscc::Message_MessageType_register_);
  Message_RegisterMessage* register_msg = new Message_RegisterMessage();
  std::string agent_ip = "127.0.0.1";
  register_msg->set_ip(agent_ip);
  register_msg->set_is_server(true);
  register_msg->set_port(server_port);
  msg.set_allocated_register_msg(register_msg);

  rpscc::Communicator* receiver = new rpscc::ZmqCommunicator();
  receiver->Initialize(16, false, server_port, 128);
  std::string msg_str = msg.SerializeAsString();
  LOG(INFO) << "Server sending register msg." << std::endl;
  sender->Send(0, msg_str);
  LOG(INFO) << "Server send register msg done" << std::endl;
  std::string msg_got;
  receiver->Receive(&msg_got);
  LOG(INFO) << "Server receive msg done." << std::endl;
  Message msg_recv;
  msg_recv.ParseFromString(msg_got);
  EXPECT_EQ(msg_recv.send_id(), 0);
  int32_t agent_id = msg_recv.recv_id();
  EXPECT_STREQ(msg_recv.config_msg().node_ip_port(agent_id).c_str(),
               (agent_ip + ":" + std::to_string(server_port)).c_str());

  msg.clear_register_msg();
  msg.set_message_type(Message_MessageType_terminate);
  sender->Send(0, msg.SerializeAsString());

  delete sender;
  delete receiver;
}

// Create one master thread, one agent thread, one server thread.
TEST(Master, RegisterTest) {
  FLAGS_listen_port = 16666;
  FLAGS_worker_num = 1;
  FLAGS_server_num = 1;
  rpscc::Master* master = rpscc::Master::Get();
  //master->Initialize(FLAGS_master_listen_port);
  std::string master_addr = "127.0.0.1:16666";
  int32_t agent_port = 15555, server_port = 17777;
  master->Initialize(master_addr);
  using namespace std::placeholders;
  std::thread master_thread(std::bind(&Master::MainLoop, master));
  std::thread agent_thread(std::bind(&AgentSimulator, master_addr, agent_port));
  std::thread server_thread(std::bind(&ServerSimulator, master_addr, server_port));
  master_thread.join();
  agent_thread.join();
  server_thread.join();
}
