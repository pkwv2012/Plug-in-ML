#include <iostream>
#include <time.h>

#include "gtest/gtest.h"
#include "gflags/gflags.h"
#include "src/server/server.h"
#include "src/util/common.h"
#include "src/util/logging.h"
#include "src/server/key_value_list.h"
#include "src/server/pull_info.h"
#include "src/communication/zmq_communicator.h"
#include "src/message/message.pb.h"


using namespace std;
using namespace rpscc;

/*
string master_addr = "127.0.0.1:5000";
string agent_addr = "127.0.0.1:5555";
string server_addr = "127.0.0.1:5005";
string server_addr2 = "127.0.0.1:5006";
string server_addr3 = "127.0.0.1:5007";
*/

// This function will simulate the master and agent
void SimulOuter() {
  ZmqCommunicator sender;
  ZmqCommunicator master_receiver, agent_receiver;
  int16 master_port = 5000;
  int16 agent_port = 5555;

  Message msg_send;
  Message msg_recv;

  Message_RegisterMessage reg_msg;
  Message_ConfigMessage* config_msg = new Message_ConfigMessage();
  Message_RequestMessage* request_msg;

  string reg_str;       // receive from server
  string config_str;    // send to agent or server
  string request_str;   // request sent to server

  sender.Initialize(64/* ring_size */, true, 1024/* listen_port */);
  master_receiver.Initialize(64, false, master_port);
  agent_receiver.Initialize(4, false, agent_port);

  {
      LOG(INFO) << "Master: Wait for server's registration";
      master_receiver.Receive(&reg_str);
      msg_recv.ParseFromString(reg_str);
      reg_msg = msg_recv.register_msg();
      LOG(INFO) << "Master: Server's ip = " << reg_msg.ip() << " port = "
           << reg_msg.port() << " is_server = " << reg_msg.is_server();
      EXPECT_EQ(reg_msg.ip(), "127.0.0.1");
      EXPECT_EQ(reg_msg.port(), 5005);
      EXPECT_EQ(reg_msg.is_server(), 1);

      sender.AddIdAddr(2, "127.0.0.1:5005");
  }

  {
    config_msg->set_worker_num(1);
    config_msg->set_server_num(3);
    config_msg->set_key_range(10);

    config_msg->add_node_ip_port("127.0.0.1:5000");  // Master 0
    config_msg->add_node_ip_port("127.0.0.1:5555");  // Agent  1
    config_msg->add_node_ip_port("127.0.0.1:5005");  // Server 2
    config_msg->add_node_ip_port("127.0.0.1:5006");  // Server 3
    config_msg->add_node_ip_port("127.0.0.1:5007");  // Server 4

    config_msg->add_partition(0);
    config_msg->add_partition(3);
    config_msg->add_partition(5);
    config_msg->add_partition(7);
    config_msg->add_partition(10);

    config_msg->add_server_id(2);
    config_msg->add_server_id(3);
    config_msg->add_server_id(4);
    config_msg->add_worker_id(1);
    config_msg->add_master_id(0);
    config_msg->set_bound(1);

    msg_send.set_message_type(Message_MessageType_config);
    msg_send.set_recv_id(2);
    msg_send.set_send_id(0);
    msg_send.set_allocated_config_msg(config_msg);
    msg_send.SerializeToString(&config_str);

    LOG(INFO) << "Master: Master send config string to server";
    sender.Send(2, config_str);
  }
  sleep(1);

  // Then I will act as an agent
  request_msg = new Message_RequestMessage();
  request_msg->set_request_type(Message_RequestMessage_RequestType_key_value);
  request_msg->add_keys(0);
  request_msg->add_keys(1);
  request_msg->add_keys(2);
  request_msg->add_values(0.1);
  request_msg->add_values(0.2);
  request_msg->add_values(0.3);

  msg_send.set_message_type(Message_MessageType_request);
  msg_send.set_recv_id(2);
  msg_send.set_send_id(1);
  msg_send.set_allocated_request_msg(request_msg);
  msg_send.SerializeToString(&request_str);
  sender.Send(2, request_str);
  sleep(1);
  LOG(INFO) << "Agent: Agent send push request string to server";
  sleep(10);
}

TEST(ServerTest, TestServer) {
  Server server;
  string master_addr = "127.0.0.1:5000";
  string agent_addr = "127.0.0.1:5555";
  string server_addr = "127.0.0.1:5005";
  string server_addr2 = "127.0.0.1:5006";
  string server_addr3 = "127.0.0.1:5007";

  LOG(INFO) << "This is TestInitialize";
  if (fork() == 0) {
    SimulOuter();
  } else {
    sleep(3);
    FLAGS_master_ip_port = master_addr;
    FLAGS_server_port = 5005;
    FLAGS_net_interface = "lo";
    server.Initialize();
    server.Start();

  }
}

//int main(int argc, char **argv) {
//  gflags::ParseCommandLineFlags(&argc, &argv, true);
//  ::testing::InitGoogleTest(&argc, argv);
//  if (fork() == 0) {
//    // SimulMaster();
//    return 0;
//  } else {
//    return RUN_ALL_TESTS();
//  }
//}