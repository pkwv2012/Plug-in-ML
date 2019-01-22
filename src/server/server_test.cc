// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include <deque>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "src/message/message.pb.h"
#include "src/server/key_value_list.h"
#include "src/server/pull_info.h"
#include "src/server/server.h"
#include "src/util/common.h"

using namespace rpscc;

class TestServer : public Server {
 public:
  bool TestRespondToAll();
  void TestUpdateParameter();
  void TestServePull(uint32, const Message_RequestMessage &);
  void TestServePush(uint32, const Message_RequestMessage &);
  bool TestInitialize(uint32);
  void TestStart(std::queue<std::string>*);
};

bool TestServer::TestInitialize(uint32 bound) {
  // Initialization of server fields
  local_id_ = 0;
  bottom_version_ = 0;
  consistency_bound_ = bound;
  agent_num_ = 3;
  server_num_ = 1;
  start_key_ = 0;
  parameter_length_ = 10;

  // Initialize map from worker ID to version buffer index
  for (uint32 i = 0; i < agent_num_; ++i)
    id_to_index_[i] = i;

  // By default, all parameters are initialized to be zero
  for (uint32 i = 0; i < parameter_length_; ++i)
    parameters_.push_back(0.0f);

  // Initialize the deque finish_count to be zeros, it's length should be
  // equal to consistency bound. To maintain finish_count, It's length must
  // stay unchanged throughout the program.
  for (uint32 i = 0; i < consistency_bound_; ++i)
    finish_count_.push_back(0);

  for (uint32 i = 0; i < agent_num_; ++i)
    version_buffer_.push_back(std::queue<KeyValueList>());

  return true;
}

bool TestServer::TestRespondToAll() {
  while (pull_request_.empty() == false) {
    PullInfo request = pull_request_.front();
    pull_request_.pop();
    uint32 len = request.Length();
    printf("reply from server to worker %d:\n", request.get_id());
    for (uint32 i = 0; i < len; ++i) {
      printf("index:%d value:%f\n", request.Key(i),
      parameters_[request.Key(i) - start_key_]);
    }
    printf("reply end\n");
  }
}

void TestServer::TestUpdateParameter() {
  std::vector<float> update(parameter_length_, 0.0f);
  for (uint32 i = 0; i < agent_num_; ++i) {
    KeyValueList update_i = version_buffer_[i].front();
    version_buffer_[i].pop();
    uint32 len = update_i.Length();
    for (uint32 j = 0; j < len; ++j)
      update[update_i.Key(j) - start_key_] += update_i.Value(j);
  }
  for (uint32 i = 0; i < parameter_length_; ++i) {
    parameters_[i] += update[i] / agent_num_;
  }
  bottom_version_++;
}

void TestServer::TestStart(std::queue<std::string>* msg_queue) {
  printf("TestStart-begin\n");
  while (msg_queue->empty() == false) {
    std::string recv_str = msg_queue->front();
    msg_queue->pop();
    Message msg_recv;
    msg_recv.ParseFromString(recv_str);
    uint32 sender_id = msg_recv.send_id();
    printf("request from worker %d\n", sender_id);

    if (msg_recv.message_type() == Message_MessageType_request) {
      Message_RequestMessage request = msg_recv.request_msg();
      if (request.request_type()
        == Message_RequestMessage_RequestType_key_value) {
        // Push request:
        TestServePush(sender_id, request);
      } else if (request.request_type()
        == Message_RequestMessage_RequestType_key) {
        // Pull request:
        TestServePull(sender_id, request);
      }
    }
  }
}

void TestServer::TestServePush(uint32 sender_id,
  const Message_RequestMessage &request) {
  finish_count_[version_buffer_[id_to_index_[sender_id]].size()]++;
  KeyValueList worker_update;
  for (uint32 i = 0; i < request.keys_size(); ++i)
    worker_update.AddPair(request.keys(i), request.values(i));
  version_buffer_[id_to_index_[sender_id]].push(worker_update);

  printf("ServePush for worker %d\n", sender_id);

  if (finish_count_[0] == agent_num_) {
    finish_count_.pop_front();
    finish_count_.push_back(0);
    TestUpdateParameter();
    TestRespondToAll();
  }
}

void TestServer::TestServePull(uint32 sender_id,
  const Message_RequestMessage &request) {
  // Blocked when enough update is pushed but not yet processed
  // A block message will be sent to the sender agent
  if (version_buffer_[id_to_index_[sender_id]].size()
    >= consistency_bound_) {
    PullInfo blocked_request;
    blocked_request.set_id(sender_id);
    for (uint32 i = 0; i < request.keys_size(); ++i)
      blocked_request.AddKey(request.keys(i));
    pull_request_.push(blocked_request);

    printf("worker %d's pull request is blocked\n", sender_id);
  } else {
    printf("reply from server to worker %d\n", sender_id);
    for (uint32 i = 0; i < request.keys_size(); ++i) {
      printf("index:%d value:%f\n", request.keys(i),
      parameters_[request.keys(i) - start_key_]);
    }
    printf("reply end\n");
  }
}

// type 0 for push, 1 for pull
std::string Generate(uint32 from, uint32 type, uint32 length,
  uint32 index[], float value[] = NULL) {
  std::string msg_str;
  if (type == 0) {
    Message* msg_send = new Message;
    Message_RequestMessage* msg = new Message_RequestMessage;
    msg->set_request_type(
      Message_RequestMessage_RequestType_key_value);
    for (uint32 i = 0; i < length; ++i) {
      msg->add_keys(index[i]);
      msg->add_values(value[i]);
    }
    msg_send->set_message_type(Message_MessageType_request);
    msg_send->set_allocated_request_msg(msg);
    msg_send->set_send_id(from);
    msg_send->set_recv_id(0);
    msg_send->SerializeToString(&msg_str);
    delete msg_send;
    return msg_str;
  } else if (type == 1) {
    Message* msg_send = new Message;
    Message_RequestMessage* msg = new Message_RequestMessage;
    msg->set_request_type(
      Message_RequestMessage_RequestType_key);
    for (uint32 i = 0; i < length; ++i) {
      msg->add_keys(index[i]);
    }
    msg_send->set_message_type(Message_MessageType_request);
    msg_send->set_allocated_request_msg(msg);
    msg_send->set_send_id(from);
    msg_send->set_recv_id(0);
    msg_send->SerializeToString(&msg_str);
    delete msg_send;
    return msg_str;
  }
}

int main() {
  TestServer server;
  server.TestInitialize(1);
  std::queue<std::string> msg_queue;
  uint32* keys;
  float* values;
  // pull 2
  keys = new uint32[3]{1, 2, 3};
  msg_queue.push(Generate(2, 1, 3, keys));
  delete [] keys;
  // push 1
  keys = new uint32[3]{1, 2, 3};
  values = new float[3]{1, 2, 3};
  msg_queue.push(Generate(1, 0, 3, keys, values));
  delete [] keys;
  delete [] values;
  // push2
  keys = new uint32[3]{4, 5, 6};
  values = new float[3]{4, 5, 6};
  msg_queue.push(Generate(2, 0, 3, keys, values));
  delete [] keys;
  delete [] values;
  // pull3 should be granted
  keys = new uint32[3]{2, 3, 4};
  msg_queue.push(Generate(3, 1, 3, keys));
  delete [] keys;
  // pull1 should be blocked
  keys = new uint32[3]{1, 2, 3};
  msg_queue.push(Generate(1, 1, 3, keys));
  delete [] keys;
  // pull2 should be blocked
  keys = new uint32[3]{4, 5, 6};
  msg_queue.push(Generate(2, 1, 3, keys));
  delete [] keys;
  // push3
  keys = new uint32[4]{7, 8, 9, 0};
  values = new float[4]{7, 8, 9, 0};
  msg_queue.push(Generate(3, 0, 4, keys, values));
  delete [] keys;
  delete [] values;
  // pull3
  keys = new uint32[10]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  msg_queue.push(Generate(3, 1, 10, keys));
  delete [] keys;
  server.TestStart(&msg_queue);
  printf("request list processed successfully!\n");
}
