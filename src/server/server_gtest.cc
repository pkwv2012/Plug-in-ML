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

void SimulMaster() {
  ZmqCommunicator sender;
  ZmqCommunicator receiver;
  int16 master_port = 5000;

  Message msg_send;
  Message msg_recv;

  Message_RegisterMessage reg_msg;
  Message_ConfigMessage* config_msg = new Message_ConfigMessage();

  string reg_str;    // receive from server
  string config_str; // send to agent or server

  sender.Initialize(64/* ring_size */, true, 1024/* listen_port */);
  receiver.Initialize(64, false, master_port);

  cout << "Master: Wait for server's registration" << endl;
  receiver.Receive(&reg_str);
  msg_recv.ParseFromString(reg_str);
  reg_msg = msg_recv.register_msg();
  cout << "Master: Server's ip = " << reg_msg.ip() << " port = "
       << reg_msg.port() << " is_server = " << reg_msg.is_server() << endl;

  string agent_addr = reg_msg.ip() + ":" + to_string(reg_msg.port());
  sender.AddIdAddr(2, "127.0.0.1:5005");

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

  cout << "Master: Master send config string to server" << endl;
  sender.Send(2, config_str);
  sleep(10);
}

class ServerTest : public ::testing::Test {
 public:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  Server server;
  string master_addr = "127.0.0.1:5000";
  string agent_addr = "127.0.0.1:5555";
  string server_addr = "127.0.0.1:5005";
  string server_addr2 = "127.0.0.1:5006";
  string server_addr3 = "127.0.0.1:5007";
};

TEST_F(ServerTest, TestServer) {
  cout << "This is TestInitialize" << endl;
  if (fork() == 0) {
    SimulMaster();
  } else {
    FLAGS_master_ip_port = master_addr;
    FLAGS_server_port = 5005;
    FLAGS_net_interface = "lo";
    server.Initialize();
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