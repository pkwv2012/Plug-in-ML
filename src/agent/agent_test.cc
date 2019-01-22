#include <iostream>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include "src/agent/agent.h"
#include "src/communication/zmq_communicator.h"
#include "src/message/message.pb.h"
#include "src/channel/fifo.h"
#include "src/channel/shared_memory.h"

using namespace rpscc;
using namespace std;

// In this test, the <id>:<ip>:<port> list is as follows:
// Master 0 : 127.0.0.1       : 5000
// Agent  1 : 127.0.0.1       : 5555
// Server 2 : 127.0.0.1       : 5005

void Init() {
  Agent agent;
  string para_fifo_name = "/tmp/cute_para_fifo";
  string grad_fifo_name = "/tmp/cute_grad_fifo";
  string para_memory_name = "/cute_para_mem";
  string grad_memory_name = "/cute_grad_mem";
  int32 shared_memory_size = 1024;
  string master_addr = "127.0.0.1:5000";
  agent.Initialize(para_fifo_name, grad_fifo_name,
                   para_memory_name, grad_memory_name,
                   shared_memory_size, master_addr);
  cout << "Agent: Start" << endl;
  agent.Start();
}

void Mast() {
  cout << "Master: Start" << endl;
  ZmqCommunicator sender;
  ZmqCommunicator receiver;
  int16 master_port = 5000;
  
  Message msg_send;
  Message msg_recv;
  
  Message_RegisterMessage reg_msg;
  Message_ConfigMessage* config_msg = new Message_ConfigMessage();
  
  string reg_str;    // receive from agent or server
  string config_str; // send to agent or server
  
  sender.Initialize(64/* ring_size */, true, 1024/* listen_port */);
  receiver.Initialize(64, false, master_port);
  
  while (1) {
    receiver.Receive(&reg_str);
    msg_recv.ParseFromString(reg_str);
    reg_msg = msg_recv.register_msg();
    cout << "Agent's ip = " << reg_msg.ip() << " port = " << reg_msg.port()
         << " is_server = " << reg_msg.is_server() << endl;
    
    string agent_addr = reg_msg.ip() + ":" + to_string(reg_msg.port());
    sender.AddIdAddr(1, agent_addr);
    
    config_msg->set_worker_num(1);
    config_msg->set_server_num(1);
    config_msg->set_key_range(100);
    
    config_msg->add_node_ip_port("127.0.0.1:5000");
    config_msg->add_node_ip_port(agent_addr);
    config_msg->add_node_ip_port("127.0.0.1:5005");

    config_msg->add_partition(0);
    config_msg->add_partition(100);
    config_msg->add_server_id(2);
    config_msg->add_worker_id(1);
    config_msg->add_master_id(0);
    config_msg->set_bound(1);
    
    msg_send.set_message_type(Message_MessageType_config);
    msg_send.set_recv_id(1);
    msg_send.set_send_id(0);
    msg_send.set_allocated_config_msg(config_msg);
    msg_send.SerializeToString(&config_str);

    cout << "Master: Master send config string to agent" << endl;
    sender.Send(1, config_str);
    break;
  }
  while (true) {
    sleep(10);
    return;
  }
}

void Serve() {
  cout << "Server: Start" << endl;
  ZmqCommunicator sender;
  ZmqCommunicator receiver;
  int16 server_port = 5005;
  
  Message msg;
  
  Message_RequestMessage* req_msg;
  
  string msg_str;
  
  sender.Initialize(64, true, 1024);
  receiver.Initialize(64, false, server_port);
  sender.AddIdAddr(1, "127.0.0.1:5555");
  while (true) {
    cout << "Server: Wait for agent's push request" << endl;
    int rc = receiver.Receive(&msg_str);
    cout << "Server: Receive " << rc << " bytes from agent" << endl;
    msg.ParseFromString(msg_str);
    cout << "Server: Receive " << msg.DebugString() << endl;
    
    cout << "Server: Wait for agents' pull request" << endl;
    rc = receiver.Receive(&msg_str);
    cout << "Server: Receive " << rc << " bytes from agent" << endl;
    msg.ParseFromString(msg_str);
    cout << "Server: Receive " << msg.DebugString() << endl;
    
    cout << "Server: Try to return parameters to agent" << endl;
    req_msg = new Message_RequestMessage();
    req_msg->set_request_type(Message_RequestMessage_RequestType_key_value);
    req_msg->add_keys(0);
    req_msg->add_keys(1);
    req_msg->add_keys(2);
    req_msg->add_values(20);
    req_msg->add_values(30);
    req_msg->add_values(40);
    msg.clear_request_msg();
    
    msg.set_allocated_request_msg(req_msg);
    msg.set_send_id(2);
    msg.set_recv_id(1);
    msg.set_message_type(Message_MessageType_request);
    
    msg.SerializeToString(&msg_str);
    cout << "Server: Return parameters to agent" << endl;
    sender.Send(1, msg_str);
    sleep(1);
  }
}

void Work() {
  cout << "Worker: Press any key to start" << endl;
  string str;
  cin >> str;
  
  Fifo para_fifo, grad_fifo;
  SharedMemory para_memory, grad_memory;
  shmstruct parameters, gradients;
  
  string para_fifo_name = "/tmp/cute_para_fifo";
  string grad_fifo_name = "/tmp/cute_grad_fifo";
  string para_memory_name = "/cute_para_mem";
  string grad_memory_name = "/cute_grad_mem";
  int32 shared_memory_size = 1024;
  
  int fd = shm_open(para_memory_name.c_str(), O_RDWR | O_CREAT, FILE_MODE);
  ftruncate(fd, sizeof(struct shmstruct));
  fd = shm_open(grad_memory_name.c_str(), O_RDWR | O_CREAT, FILE_MODE);
  ftruncate(fd, sizeof(struct shmstruct));
  close(fd);
  
  para_fifo.Initialize(para_fifo_name, true);
  grad_fifo.Initialize(grad_fifo_name, false);
  para_memory.Initialize(para_memory_name.c_str());
  grad_memory.Initialize(grad_memory_name.c_str());
  para_fifo.Open();
  grad_fifo.Open();
  gradients.size = 5;
  for (int i = 0; i < 5; i++) {
    gradients.keys[i] = i;
    gradients.values[i] = i + 10;
  }
  
  while (true) {
    cout << "Worker: Prepare gradients for agent" << endl;
    cin >> str;
    cout << "Worker: Transfer " << gradients.size << " gradients to agent"
         << endl;
    cout << "(key, value)s are as follows" << endl;
    for (int i = 0; i < gradients.size; i++) {
      cout << "(" << gradients.keys[i] << ", " << gradients.values[i]
           << ")" << ", ";
    }
    cout << endl;
    grad_memory.Write(&gradients);
    grad_fifo.Signal();
    cout << "Worker: Wait for agent's parameters" << endl;
    para_fifo.Wait();
    cout << "Worker: Get parameters from agent" << endl;
    parameters = *para_memory.Read();
    cout << "Worker: Get " << parameters.size << " parameters from agent"
         << endl;
    cout << "(key, value)s are as follows" << endl;
    for (int i = 0; i < parameters.size; i++) {
      cout << "(" << parameters.keys[i] << ", " << parameters.values[i]
           << ")" << ", ";
    }
    cout << endl;
    cout << "Worker: A loop is done" << endl;
  }
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
    sleep(2);
    Serve();
    return 0;
  }
  Work();
  
  cout << "done" << endl;
  return 0;
}
