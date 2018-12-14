// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#ifndef SRC_COMMUNICATION_COMMUNICATOR_H_
#define SRC_COMMUNICATION_COMMUNICATOR_H_

#include <string>
#include <vector>

#include "src/util/common.h"

namespace rpscc {

// Communicator is a abstract class, which will be implemented by real
// communicators, such as MPI, ZMQ or unix socket.
class Communicator {
 public:
  Communicator() {}
  virtual ~Communicator() {}
  virtual void Finalize() = 0;

  // Initialize the communicator
  // The buffer_size is used in receiving.
  // Return:
  // true : Init successfully
  // false : Init failed
  virtual bool Initialize(int32 ring_size, bool is_sender,
                  int16 listen_port, int32 buffer_size=2048) = 0;

  // Send a message from one node to another node,
  // Return:
  // > 0 : bytes send
  // - 1 : error
  virtual int32 Send(int32 dst_id, const char* const message,
                     int32 len) = 0;
  virtual int32 Send(int32 dst_id, const std::string& message) = 0;
  // Receive a message from any node
  // Return:
  // > 0 : bytes received
  // - 1 : error
  virtual int32 Receive(char* message, const int max_size) = 0;
  virtual int32 Receive(std::string* message) = 0;

  // Add an <id, addr> to the id_to_addr_
  virtual bool AddIdAddr(int32 id, std::string addr);
  // Delete an <id, addr> from id_to_addr_
  virtual bool DeleteId(int32 id);
};

}  // namespace rpscc

#endif  // SRC_COMMUNICATION_COMMUNICATOR_H_



