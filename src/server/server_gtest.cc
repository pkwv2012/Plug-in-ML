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
string server_addr = "127.0.0.1:5500";
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
      EXPECT_EQ(reg_msg.port(), 5500);
      EXPECT_EQ(reg_msg.is_server(), 1);

      sender.AddIdAddr(2, "127.0.0.1:5500");
  }

  {
    config_msg->set_worker_num(1);
    config_msg->set_server_num(3);
    config_msg->set_key_range(10);

    config_msg->add_node_ip_port("127.0.0.1:5000");  // Master 0
    config_msg->add_node_ip_port("127.0.0.1:5555");  // Agent  1
    config_msg->add_node_ip_port("127.0.0.1:5500");  // Server 2
    config_msg->add_node_ip_port("127.0.0.1:5006");  // Server 3
    config_msg->add_node_ip_port("127.0.0.1:5007");  // Server 4

    config_msg->add_partition(0);
    config_msg->add_partition(3);
    config_msg->add_partition(5);

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
  LOG(INFO) << "Agent: Agent send push request string to server";
  sender.Send(2, request_str);

  sleep(1);
  // Then, let's send heartbeats to server
  sender.DeleteId(2);
  sender.AddIdAddr(2, "127.0.0.1:5501");
  Message send_msg, recv_msg;
  Message_HeartbeatMessage* hb_msg = new Message_HeartbeatMessage();
  std::string send_str, recv_str;

  hb_msg->set_is_live(true);
  send_msg.set_message_type(Message_MessageType_heartbeat);
  send_msg.set_send_id(0);
  send_msg.set_allocated_heartbeat_msg(hb_msg);
  send_msg.set_recv_id(2);
  send_msg.SerializeToString(&send_str);

  for (int i = 0; i < 5; i++) {
    sleep(1);
    // Config the send_msg
    if (sender.Send(2, send_str) == -1) {
      LOG(ERROR) << "Cannot send a heartbeat to master";
    }
    LOG(INFO) << "Master: Send heartbeat message to master";

    if (master_receiver.Receive(&recv_str) == -1) {
      LOG(ERROR) << "Error in receiving heartbeat from master";
    }
    recv_msg.ParseFromString(recv_str);
    LOG(INFO) << "Master: Receive heartbeat message from master with id "
              << recv_msg.send_id();
  }
  sleep(1);

  {
    sender.DeleteId(2);
    sender.AddIdAddr(2, "127.0.0.1:5500");
    config_msg = new Message_ConfigMessage();
    config_msg->set_worker_num(1);
    config_msg->set_server_num(2);
    config_msg->set_key_range(10);

    config_msg->add_node_ip_port("127.0.0.1:5000");  // Master 0
    config_msg->add_node_ip_port("127.0.0.1:5555");  // Agent  1
    config_msg->add_node_ip_port("127.0.0.1:5500");  // Server 2
    config_msg->add_node_ip_port("127.0.0.1:5006");  // Server 3
    // config_msg->add_node_ip_port("127.0.0.1:5007");  // Server 4
    // config_msg->add_node_ip_port("127.0.0.1:5008");  // Server 5

    config_msg->add_partition(3);
    config_msg->add_partition(5);
    // config_msg->add_partition(7);
    // config_msg->add_partition(10);

    config_msg->add_server_id(3);
    config_msg->add_server_id(2);
    // config_msg->add_server_id(4);
    // config_msg->add_server_id(5);

    config_msg->add_worker_id(1);
    config_msg->add_master_id(0);
    config_msg->set_bound(1);

    msg_send.set_message_type(Message_MessageType_config);
    msg_send.set_recv_id(2);
    msg_send.set_send_id(0);
    msg_send.set_allocated_config_msg(config_msg);
    msg_send.SerializeToString(&config_str);

    LOG(INFO) << "Master: Master send Reconfig string to server";
    sender.Send(2, config_str);
  }
  sleep(20);
}

void SimulServer(int server_id, int16 listen_port, int start_key, int param_len) {
  LOG(INFO) << "Helper Server " << server_id << ": " << server_id << " " << listen_port << " " << start_key
            << param_len;
  ZmqCommunicator sender;
  ZmqCommunicator receiver;

  Message msg_send;
  Message msg_recv;
  Message_RequestMessage* msg_reply;

  string msg_str;

  sender.Initialize(64/* ring_size */, true, 1024/* listen_port */);
  sender.AddIdAddr(2, "127.0.0.1:5500");
  receiver.Initialize(64, false, listen_port);

  vector<float> parameters;
  for (int i = 0; i < param_len; i++)
    parameters.push_back(i + start_key * 10);

  while (true) {
    LOG(INFO) << "Helper Server " << server_id << ": Wait for server's backup_request";
    receiver.Receive(&msg_str);
    LOG(INFO) << "Helper Server " << server_id << ": Get server's request";
    msg_recv.ParseFromString(msg_str);
    EXPECT_EQ(msg_recv.recv_id(), server_id);
    EXPECT_EQ(msg_recv.send_id(), 2);
    msg_reply = new Message_RequestMessage;
    msg_reply->set_request_type(Message_RequestMessage_RequestType_key_value);
    for (int32 i = 0; i < param_len; ++i) {
      msg_reply->add_keys(start_key + i);
      msg_reply->add_values(parameters[i]);
    }
    msg_send.set_message_type(Message_MessageType_request);
    msg_send.set_allocated_request_msg(msg_reply);
    msg_send.set_send_id(server_id);
    msg_send.set_recv_id(2);
    msg_send.SerializeToString(&msg_str);
    if (sender.Send(2, msg_str) == -1) {
      LOG(ERROR) << "Failed to respond to server " << 2
                 << "'s pull request.";
    }
    LOG(INFO) << "Helper Server " << server_id << ": Sent respond to server";

    sleep(1);
    LOG(INFO) << "Helper Server " << server_id << ": Try to send backup_request to server";
    msg_send.clear_request_msg();
    msg_send.SerializeToString(&msg_str);
    if (sender.Send(2, msg_str) == -1) {
      LOG(ERROR) << "Failed to request to server" << 2;
    }
    LOG(INFO) << "Helper Server " << server_id << ": Sent backup request to server 2";
    LOG(INFO) << "Helper Server " << server_id << ": Wait for server's backup_respond";
    receiver.Receive(&msg_str);
    LOG(INFO) << "Helper Server " << server_id << ": Get server's request";
    msg_recv.ParseFromString(msg_str);
    EXPECT_EQ(2, msg_recv.send_id());
    EXPECT_EQ(server_id, msg_recv.recv_id());
    for (int32 i = 0; i < msg_recv.request_msg().values_size(); i++) {
      LOG(INFO) << "Helper Server " << server_id <<  ": Get params " << msg_recv.request_msg().keys(i) << " "
                << msg_recv.request_msg().values(i);
    }
  }

}

TEST(ServerTest, TestServer) {
  Server server;
  string master_addr = "127.0.0.1:5000";
  string agent_addr = "127.0.0.1:5555";
  string server_addr = "127.0.0.1:5500";
  string server_addr2 = "127.0.0.1:5006";
  string server_addr3 = "127.0.0.1:5007";

  LOG(INFO) << "This is TestInitialize";
  int server3_id, server4_id, server2_id;
  server3_id = fork();
  if (server3_id == 0) {
    SimulServer(3, 5006, 3, 2);
    return;
  }

  server4_id = fork();
  if (server4_id == 0) {
    SimulServer(4, 5007, 5, 5);
    return;
  }

  server2_id = fork();
  if (server2_id == 0) {
    sleep(3);
    FLAGS_master_ip_port = master_addr;
    FLAGS_server_port = 5500;
    FLAGS_net_interface = "lo";
    server.Initialize();
    server.Start();
  }

  SimulOuter();

  kill(server2_id, SIGKILL);
  kill(server3_id, SIGKILL);
  kill(server4_id, SIGKILL);
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