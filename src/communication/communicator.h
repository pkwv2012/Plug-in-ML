// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#ifndef SRC_COMMUNICATION_COMMUNICATOR_H_
#define SRC_COMMUNICATION_COMMUNICATOR_H_

#include <string>
#include <vector>

namespace rpscc {

// Communicator is a abstract class, which will be implemented by real
// communicators, such as MPI, ZMQ or unix socket.
class Communicator {
 public:
  virtual ~Communicator() { }

  // Send a message from one node to another node,
  // or send a message from one node to some nodes.
  // Return:
  // > 0 : bytes send
  // - 1 : error
  virtual int32 Send(int32 dst_id, const char* const message,
                     int32 len) = 0;
  virtual int32 Send(int32 dst_id, const std::string& message) = 0;
  virtual int32 Send(const vector<int32>& dst_ids,
                     const char* const message, int32 len) = 0;
  virtual int32 Send(const vector<int32>& dst_ids,
                     const std::string& message) = 0;

  // Receive
  virtual int32 Receive(char* message, const int max_size) = 0;
  virtual int32 Receive(std::string* message) = 0;
};

}  // namespace rpscc

#endif  // SRC_COMMUNICATION_COMMUNICATOR_H_

