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
  LOG(INFO) << "Agent: Agent send push request string to server";
  sleep(1);
  sender.Send(2, request_str);
  sleep(10);
}

void SimulServer(int server_id, int16 listen_port, int start_key, int param_len) {
  LOG(INFO) << "Helper Server: " << server_id << " " << listen_port << " " << start_key
            << param_len;
  ZmqCommunicator sender;
  ZmqCommunicator receiver;

  Message msg_send;
  Message msg_recv;
  Message_RequestMessage* msg_reply;

  string msg_str;

  sender.Initialize(64/* ring_size */, true, 1024/* listen_port */);
  sender.AddIdAddr(2, "127.0.0.1:5005");
  receiver.Initialize(64, false, listen_port);

  vector<float> parameters;
  for (int i = 0; i < param_len; i++)
    parameters.push_back(i + start_key * 10);

  while (true) {
    LOG(INFO) << "Helper Server: Wait for server's backup_request";
    receiver.Receive(&msg_str);
    LOG(INFO) << "Helper Server: Get server's request";
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
    LOG(INFO) << "Helper Server: Sent respond to server";

    sleep(1);
    LOG(INFO) << "Helper Server: Sent backup_request to server";
    msg_send.clear_request_msg();
    msg_send.SerializeToString(&msg_str);
    if (sender.Send(2, msg_str) == -1) {
      LOG(ERROR) << "Failed to request to server" << 2;
    }
    LOG(INFO) << "Helper Server: Sent backup request to server 2";
    LOG(INFO) << "Helper Server: Wait for server's backup_request";
    receiver.Receive(&msg_str);
    LOG(INFO) << "Helper Server: Get server's request";
    msg_recv.ParseFromString(msg_str);
    EXPECT_EQ(2, msg_recv.send_id());
    EXPECT_EQ(server_id, msg_recv.recv_id());
    for (int32 i = 0; i < msg_recv.request_msg().values_size(); i++) {
      LOG(INFO) << "Get params " << msg_recv.request_msg().keys(i) << " "
                << msg_recv.request_msg().values(i);
    }
  }

}

//void Server::RequestBackup() {
//  LOG(INFO) << "RequestBackup";
//
//  std::unordered_set<int32> wait_list;
//  std::string str;
//  Message* msg = new Message;
//  Message_RequestMessage request;
//  int32 server_id, server_index;
//
//  for (int32 i = 1; i <= backup_size_; i++) {
//    int32 target = (local_index_ - i + server_num_) % server_num_;
//    server_id = server_ids_[target];
//    wait_list.insert(server_id);
//    msg->set_send_id(local_id_);
//    msg->set_recv_id(server_id);
//    msg->SerializeToString(&str);
//    if (sender_->Send(server_id, str) == -1) {
//      LOG(ERROR) << "Failed to send request to server: " << server_id;
//    }
//  }
//  // TODO: Read bottom_version_ from the message
//  while (!wait_list.empty()) {
//    if (receiver_->Receive(&str) == -1) {
//      LOG(ERROR) << "Failed to receive configuration information from master.";
//      return;
//    }
//    msg->ParseFromString(str);
//    server_id = msg->send_id();
//    server_index = (local_index_ - 1 - servers_[server_id] + server_num_) % server_num_;
//    request = msg->request_msg();
//
//    // Reset the backup_parameters_'s size if necessary
//    if (backup_parameters_[server_index].size() != request.values_size()) {
//      backup_parameters_[server_index] = std::vector<float32>(request.values_size());
//    }
//    for (int32 i = 0; i < request.values_size(); i++) {
//      backup_parameters_[server_index][i] = request.values(i);
//    }
//    wait_list.erase(server_id);
//  }
//
//  delete msg;
//};


TEST(ServerTest, TestServer) {
  Server server;
  string master_addr = "127.0.0.1:5000";
  string agent_addr = "127.0.0.1:5555";
  string server_addr = "127.0.0.1:5005";
  string server_addr2 = "127.0.0.1:5006";
  string server_addr3 = "127.0.0.1:5007";

  LOG(INFO) << "This is TestInitialize";
  if (fork() == 0) {
    if (fork() == 0) {
      if (fork() == 0) {
        SimulServer(4, 5007, 7, 3);
      } else {
        SimulServer(3, 5006, 5, 2);
      }
    } else {
      SimulOuter();
    }
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