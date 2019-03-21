// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include "src/communication/zmq_sendrecv.h"
#include "src/util/logging.h"

namespace rpscc {

bool ZmqSendRecv::Initialize(bool is_sender, int16 listen_port) {
  is_sender_ = is_sender;
  listen_port_ = listen_port;
  // Create a new zmq context
  context_ = zmq_ctx_new();
  // If it is a sender
  if (is_sender_) {
    num_senders_ = 0;
  } else {
    // If it is a receiver
    // Initialize the socket for receiving
    // ZmqSendRecv uses the PUSH-PULL model
    sendrecv_ = zmq_socket(context_, ZMQ_PULL);
    // Bind the socket to assigned port
    char str[16];
    snprintf(str, sizeof(str), "tcp://*:%d", listen_port_);
    zmq_bind(sendrecv_, str);
  }
  return true;
}
void ZmqSendRecv::Finalize() {
  if (is_sender_) {
    std::map<std::string, void*>::iterator i;
    for (i = mapper_.begin(); i != mapper_.end(); i++) {
      zmq_close(i->second);
    }
    mapper_.clear();
  } else {
    zmq_close(sendrecv_);
  }
  zmq_ctx_destroy(context_);
}

int32 ZmqSendRecv::Send(std::string dst_addr, const char* const message,
                      int len) {
  // If there is not a open socket for assigend address, create one socket
  std::map<std::string, void*>::iterator iter = mapper_.find(dst_addr);
  if (iter == mapper_.end()) {
    // LOG(INFO) << "Create a new socket." << std::endl;
    void *sender_ = zmq_socket(context_, ZMQ_PUSH);
    char str[32];
    snprintf(str, sizeof(str), "tcp://%s", dst_addr.c_str());
    // LOG(INFO) << "str = " << str << std::endl;
    zmq_connect(sender_, str);
    // LOG(INFO) << "Connection established." << std::endl;
    mapper_.insert(std::make_pair(dst_addr, sender_));
    sendrecv_ = sender_;
  } else {
    printf("Use existed socket.\n");
    sendrecv_ = iter->second;
  }
  // Start sending message
  //printf("message = %s, len = %d\n", message, len);
  int rc = zmq_send(sendrecv_, message, len, 0);
  // LOG(INFO) << "ZmqSend bytes = " << rc << std::endl;
  //printf("Send_rc = %d\n", rc);
  // I will add a error handler in the future.
  return len;
}

int32 ZmqSendRecv::Receive(char* message, const int32 max_size) {
  // Start receiving message
  int32 msg_size = zmq_recv(sendrecv_, message, max_size, 0);
  message[msg_size] = '\0';
  //printf("Receive message : %s, size = %d\n", message, msg_size);
  return msg_size;
}

int32 ZmqSendRecv::CloseSocket(std::string dst_addr) {
  int32 res = 0;
  std::map<std::string, void*>::iterator iter = mapper_.find(dst_addr);
  if (iter == mapper_.end()) {
    printf("Close a nonexistent socket\n");
    res = -1;
  } else {
    printf("Try to close socket\n");
    res = zmq_close(iter->second);
    mapper_.erase(dst_addr);
  }
  return res;
}

}  // namespace rpscc


