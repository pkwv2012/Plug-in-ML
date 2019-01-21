#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include "src/agent/agent.h"
#include "src/communication/zmq_communicator.h"
#include "src/message/message.pb.h"

using namespace rpscc;
using namespace std;

// In this test, the <ip>:<port> list is as follow

void Init() {
  Agent agent;
  string para_fifo_name = "cute_para_fifo";
  string grad_fifo_name = "cute_grad_fifo";
  string para_memory_name = "cute_para_mem";
  string grad_memory_name = "cute_grad_mem";
  int32 shared_memory_size = 1024;
  string master_addr = "127.0.0.1:5000";
  agent.Initialize(para_fifo_name, grad_fifo_name,
                   para_memory_name, grad_memory_name,
                   shared_memory_size, master_addr);
}

void Mast() {
  ZmqCommunicator sender;
  ZmqCommunicator receiver;
  int16 master_port = 5000;
  
  Message msg_send;
  Message msg_recv;
  
  Message_RegisterMessage reg_msg;
  Message_ConfigMessage config_msg;
  
  string reg_str;    // receive from agent or server
  string config_str; // send to agent or server
  
  sender.Initialize(64/* ring_size */, true, 1024/* listen_port */);
  receiver.Initialize(64, false, master_port);
  
  while (1) {
    receiver.Receive(&reg_str);
    printf("Get message %s\n", reg_str.c_str());
    msg_recv.ParseFromString(reg_str);
    reg_msg = msg_recv.register_msg();
    cout << "Agent's ip = " << reg_msg.ip() << " port = " << reg_msg.port()
         << " is_server = " << reg_msg.is_server() << endl;
    
    string agent_addr = reg_msg.ip() + ":" + to_string(reg_msg.port());
    sender.AddIdAddr(1, agent_addr);
    
    config_msg.set_worker_num(1);
    config_msg.set_server_num(1);
    config_msg.set_key_range(100);
    
    config_msg.add_node_ip_port("127.0.0.1:5000");
    config_msg.add_node_ip_port(agent_addr);
    config_msg.add_node_ip_port("127.0.0.1:5005");

    config_msg.add_partition(0);
    config_msg.add_partition(100);
    config_msg.add_server_id(2);
    config_msg.add_worker_id(1);
    config_msg.add_master_id(0);
    config_msg.set_bound(1);
    
    msg_send.set_message_type(Message_MessageType_config);
    msg_send.set_recv_id(1);
    msg_send.set_send_id(0);
    msg_send.set_allocated_config_msg(&config_msg);
    msg_send.SerializeToString(&config_str);
    
    cout << "config_str.size() = " << config_str.size() << endl;
    cout << "Master send config string to agent" << endl;
    sender.Send(1, config_str);
    
    sleep(10);
  }
}

void Serve() {
  
}

void Work() {

}

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  
  // Test initialization
  int pid = fork();
  if (pid == 0) {
    Init();
    return 0;
  }
  int ppid = fork();
  if (ppid == 0) {
    Mast();
    return 0;
  }
  
  // Test AgentWork
  int pppid = fork();
  if (pppid == 0) {
    Serve();
  }
  
  
  cout << "done" << endl;
  return 0;
}
