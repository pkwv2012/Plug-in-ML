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
// Server 3 : 129.0.0.1       : 5006

void Init() {
  Agent agent;
  string para_fifo_name = "/tmp/cute_para_fifo";
  string grad_fifo_name = "/tmp/cute_grad_fifo";
  string para_memory_name = "/cute_para_mem";
  string grad_memory_name = "/cute_grad_mem";
  string master_addr = "127.0.0.1:5000";
  agent.Initialize(para_fifo_name, grad_fifo_name,
                   para_memory_name, grad_memory_name,
                   master_addr);
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
  
  while (true) {
    cout << "Master: Wait for agent's registration" << endl;
    receiver.Receive(&reg_str);
    msg_recv.ParseFromString(reg_str);
    reg_msg = msg_recv.register_msg();
    cout << "Master: Agent's ip = " << reg_msg.ip() << " port = " 
         << reg_msg.port() << " is_server = " << reg_msg.is_server() << endl;
    
    string agent_addr = reg_msg.ip() + ":" + to_string(reg_msg.port());
    sender.AddIdAddr(1, agent_addr);
    
    config_msg->set_worker_num(1);
    config_msg->set_server_num(2);
    config_msg->set_key_range(5);
    
    config_msg->add_node_ip_port("127.0.0.1:5000");
    config_msg->add_node_ip_port(agent_addr);
    config_msg->add_node_ip_port("127.0.0.1:5005");
    config_msg->add_node_ip_port("127.0.0.1:5006");

    config_msg->add_partition(0);
    config_msg->add_partition(3);
    config_msg->add_partition(5);
    
    config_msg->add_server_id(2);
    config_msg->add_server_id(3);
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
    cout << "Master: Wait for agent's terminate request" << endl;
    receiver.Receive(&reg_str);
    msg_recv.ParseFromString(reg_str);
    cout << "Master: Receive " << msg_recv.DebugString() << endl;
  }
}

// In the future test, the port and key_value will be the parameters of Serve.
// So I can emit the function Serve3().
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
    cout << "Server: Wait for agent's request" << endl;
    int rc = receiver.Receive(&msg_str);
    cout << "Server: Receive " << rc << " bytes from agent" << endl;
    msg.ParseFromString(msg_str);
    cout << "Server: Receive " << msg.DebugString() << endl;
    
    if (msg.request_msg().request_type() == 
        Message_RequestMessage_RequestType_key) {
      cout << "Server: Get pull request from agent" << endl;
      cout << "Server: Try to return parameters to agent" << endl;
      
      req_msg = new Message_RequestMessage();
      req_msg->set_request_type(Message_RequestMessage_RequestType_key_value);
      req_msg->add_keys(0);
      req_msg->add_keys(1);
      req_msg->add_keys(2);
      req_msg->add_values(0);
      req_msg->add_values(10);
      req_msg->add_values(20);
      msg.clear_request_msg();
      
      msg.set_allocated_request_msg(req_msg);
      msg.set_send_id(2);
      msg.set_recv_id(1);
      msg.set_message_type(Message_MessageType_request);
      
      msg.SerializeToString(&msg_str);
      cout << "Server: Return parameters to agent" << endl;
      sender.Send(1, msg_str);
    } else {
      cout << "Server: Get push request from agent" << endl;
      cout << "Server: I just do nothing ╮(╯▽╰)╭" << endl;
    }
  }
}

void Serve3() {
  cout << "Server3: Start" << endl;
  ZmqCommunicator sender;
  ZmqCommunicator receiver;
  int16 server_port = 5006;
  
  Message msg;
  
  Message_RequestMessage* req_msg;
  
  string msg_str;
  
  sender.Initialize(64, true, 1024);
  receiver.Initialize(64, false, server_port);
  sender.AddIdAddr(1, "127.0.0.1:5555");
  while (true) {
    cout << "Server3: Wait for agent's request" << endl;
    int rc = receiver.Receive(&msg_str);
    cout << "Server3: Receive " << rc << " bytes from agent" << endl;
    msg.ParseFromString(msg_str);
    cout << "Server3: Receive " << msg.DebugString() << endl;
    
    if (msg.request_msg().request_type() == 
        Message_RequestMessage_RequestType_key) {
      cout << "Server3: Get pull request from agent" << endl;
      cout << "Server3: Try to return parameters to agent" << endl;
      
      req_msg = new Message_RequestMessage();
      req_msg->set_request_type(Message_RequestMessage_RequestType_key_value);
      req_msg->add_keys(3);
      req_msg->add_keys(4);
      req_msg->add_values(30);
      req_msg->add_values(40);
      msg.clear_request_msg();
      
      msg.set_allocated_request_msg(req_msg);
      msg.set_send_id(3);
      msg.set_recv_id(1);
      msg.set_message_type(Message_MessageType_request);
      
      msg.SerializeToString(&msg_str);
      cout << "Server3: Return parameters to agent" << endl;
      sender.Send(1, msg_str);
    } else {
      cout << "Server3: Get push request from agent" << endl;
      cout << "Server3: I just do nothing ╮(╯▽╰)╭" << endl;
    }
  }
}

void Work() {
  sleep(5);
  cout << "Worker: Start" << endl;
  std::string str;
  Fifo para_fifo, grad_fifo;
  SharedMemory para_memory, grad_memory;
  shmstruct parameters, gradients;
  
  string para_fifo_name = "/tmp/cute_para_fifo";
  string grad_fifo_name = "/tmp/cute_grad_fifo";
  string para_memory_name = "/cute_para_mem";
  string grad_memory_name = "/cute_grad_mem";
  
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
    gradients.keys[i] = 4 - i;
    gradients.values[i] = 4 - i + 10;
  }
  
  while (true) {
    cout << "Worker: Choose one way to signal Agent: 0/1/2" << endl;
    cin >> str;
    if (str == "0") {
      cout << "Worker: Pull request to agent" << endl;
      cout << "(key)s are as follows" << endl;
      for (int i = 0; i < gradients.size; i++) cout << gradients.keys[i] << " ";
      cout << endl;
      grad_memory.Write(&gradients);
      grad_fifo.Signal(0);
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
      cout << "Worker: A pull request's loop is done" << endl;
    } else if (str == "1") {
      cout << "Worker: Push request to agent" << endl;
      cout << "Worker: Transfer " << gradients.size << " gradients to agent"
         << endl;
      grad_memory.Write(&gradients);
      grad_fifo.Signal(1);
      cout << "Worker: A push request is done" << endl;
    } else {
      cout << "Worker: Terminate request to agent" << endl;
      grad_fifo.Signal(2);
      cout << "Worker: A terminate request is done" << endl;
    }
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
    int ppppid = fork();
    if (ppppid == 0)
      Serve();
    else
      Serve3();
    return 0;
  }
  Work();
  
  cout << "done" << endl;
  return 0;
}
