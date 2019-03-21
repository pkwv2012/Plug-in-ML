// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#ifndef SRC_COMMUNICATION_ZMQ_SENDRECV_H_
#define SRC_COMMUNICATION_ZMQ_SENDRECV_H_

#include <zmq.h>
#include <cstring>
#include <string>
#include <map>


#include "src/util/common.h"

namespace rpscc {

// ZmqSendRecv is a simple wrapper around zmq.
// An ZmqSendRecv can be only a sender or a receiver.
// A receiver will bind a listen_port to it, then it will just wait for data.
// A sender will maintain a list of sending sockets. In a sending transaction,
// the sender will fetch a socket or create one to send message.
class ZmqSendRecv {
 public:
  ZmqSendRecv() {}
  ~ZmqSendRecv() {}

  // Initialize the communicator
  // Parameter:
  // listen_port : indicates the <port> of this node, it should be 0
  // if this node is a sender.
  // Return:
  // true : Init successfully
  // false : Init failed
  bool Initialize(bool is_sender, int16 listen_port);
  void Finalize();

  // Send data using zmq
  // Parameter: dst_addr is in the form of <ip>:<port>
  // Return : bytes send to remote
  int32 Send(std::string dst_addr, const char* const message, int len);

  // Receive data using zmq
  // Return : bytes received
  int32 Receive(char* message, const int max_size);

  // Close conncetion
  // Return: 0 if it is closed successfully, -1 else
  int32 CloseSocket(std::string dst_addr);

 private:
  bool is_sender_;
  int16 listen_port_;
  // Context for zmq
  void* context_;
  // A socket for sending or receiving
  void* sendrecv_;
  // Number of sending sockets
  int32 num_senders_;
  // Map for sender, each pair is < (<ip>:<port>), (<sender_>) >
  std::map<std::string, void*> mapper_;
};

}  // namespace rpscc

#endif  // SRC_COMMUNICATION_ZMQ_SENDRECV_H_

