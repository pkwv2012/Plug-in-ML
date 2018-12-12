// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#ifndef SRC_COMMUNICATION_ZMQ_COMMUNICATOR_H_
#define SRC_COMMUNICATION_ZMQ_COMMUNICATOR_H_

#include <string>

#include "src/communication/communicator.h"
#include "src/communication/fifo_ring.h"
#include "src/communication/zmq_sendrecv.h"
#include "src/util/common.h"

namespace rpscc {

class ZmqCommunicator : public Communicator {
 public:
  ZmqCommunicator() {}
  virtual ~ZmqCommunicator() {}

  // Initialize the communicator
  // Return:
  // true : Init successfully
  // false : Init failed
  bool Initialize(int32 ring_size, bool is_sender, 
                  int16 listen_port, int32 buffer_size);
  void Finalize();

  // Send a message from one node to another node,
  // or send a message from one node to some nodes.
  // Return:
  // > 0 : bytes send
  // - 1 : error
  int32 Send(int32 dst_id, const char* const message, int32 len);
  int32 Send(int32 dst_id, const std::string& message);

  // Receive a message from any node
  // Return:
  // > 0 : bytes received
  // - 1 : error
  int32 Receive(char* message, const int32 max_size);
  int32 Receive(std::string* message);
  
  // Function for adder
  static void* Produce(void* arg);
  // Function for fetcher
  static void* Consume(void* arg);
  
  // Add an <id, addr> to the id_to_addr_
  bool AddIdAddr(int32 id, string addr);
  // Delete an <id, addr> from id_to_addr_
  bool DeleteId(int32 id);
  
 private:
  // Max size of a buffer block
  int32 buffer_size_;
  // The fifo ring in the communicator.
  FifoRing fifo_ring_;
  // Zmq sender or receiver in the communicator.
  ZmqSendRecv send_recv_;
  // Adder or fetcher thread for the fifo ring.
  pthread_t add_fetch_;
  // Map from <node id> to <ip>:<port>
  map<int32, string> id_to_addr_;
};

}  // namespace rpscc

#endif  // SRC_COMMUNICATION_ZMQ_COMMUNICATOR_H_

