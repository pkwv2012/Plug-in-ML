/// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Created by PeikaiZheng on 2018/12/8.

#include "gtest/gtest.h"
#include "src/communication/zmq_communicator.h"
#include "src/master/master.h"
#include "src/message/message.pb.h"

void AgentSimulator(std::string master_addr, int32_t agent_port) {
  rpscc::Communicator sender = new rpscc::ZmqCommunicator();
  sender.Initialize(16, true, /* this port is useless */ 100);
  sender.AddIdAddr(0, master_addr);
  rpscc::Message msg;
  msg.set_send_id(-1);
  msg.set_recv_id(0);
  msg.set_message_type(rpscc::Message_MessageType_register_);
  rpscc::Message_RegisterMessage register_msg;
  std::string agent_ip = "162.105.146.128";
  register_msg.set_ip(ip);
  register_msg.set_is_server(false);
  register_msg.set_port(agent_port);
  msg.set_allocated_register_msg(&register_msg);

  rpscc::Communicator receiver = new rpscc::ZmqCommunicator();
  receiver.Initialize(16, false, agent_port);
  std::string msg_str = msg.SerializeAsString();
  sender.Send(0, msg_str);
  std::string msg_got;
  receiver.Receive(msg_got);

}

// Create one master thread, one agent thread, one server thread.
TEST(Master, Register) {
  FLAGS_listen_port = 16666;
  rpscc::Master* master = rpscc::Master::Get();
  //master->Initialize(FLAGS_master_listen_port);
  master->Initialize("162.105.146.128:16666");
  std::bind(master, master->MainLoop());
  master->MainLoop();
}
