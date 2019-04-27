// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include <cstring>
#include "src/communication/zmq_communicator.h"
#include "src/util/logging.h"

namespace rpscc {

bool ZmqCommunicator::Initialize(int32 ring_size, bool is_sender,
                            int16 listenport, int32 buffer_size) {
  fifo_ring_.Initialize(ring_size);
  send_recv_.Initialize(is_sender, listenport);
  buffer_size_ = buffer_size;

  if (is_sender) {
    pthread_create(&add_fetch_, NULL, Consume, reinterpret_cast<void*>(this));
  } else {
    pthread_create(&add_fetch_, NULL, Produce, reinterpret_cast<void*>(this));
  }

  return true;
}
void ZmqCommunicator::Finalize() {
  return;
}

int32 ZmqCommunicator::Send(int32 dst_id, const char* const message,
                            int32 len) {
  char* mix_message = new char[buffer_size_];
  snprintf(mix_message, buffer_size_, "%d,", dst_id);
  int32 offset = strlen(mix_message);
  memcpy(mix_message + offset, message, len);
  len += offset;
  
  fifo_ring_.Add(mix_message, len);
  return len;
}
int32 ZmqCommunicator::Send(int32 dst_id, const std::string& message) {
  int len = Send(dst_id, message.c_str(), message.size());
  return len;
}

int32 ZmqCommunicator::Receive(char* message, const int32 max_size) {
  int len = fifo_ring_.Fetch(message, max_size);
  return len;
}
int32 ZmqCommunicator::Receive(std::string* message) {
  char* char_message = new char[buffer_size_];
  int len = fifo_ring_.Fetch(char_message, buffer_size_);
  message->assign(char_message, len);
  delete char_message;
  return len;
}

// This is a static function.
void* ZmqCommunicator::Produce(void* arg) {
  ZmqCommunicator* zc = reinterpret_cast<ZmqCommunicator*>(arg);
  static char* message = new char[zc->buffer_size_];
  static int32 len;

  while (1) {
    len = zc->send_recv_.Receive(message, zc->buffer_size_);
    zc->fifo_ring_.Add(message, len);
    sleep(1);
  }
}

// This is a static function.
void* ZmqCommunicator::Consume(void* arg) {
  ZmqCommunicator* zc = reinterpret_cast<ZmqCommunicator*>(arg);
  static char *mix_message = new char[zc->buffer_size_];
  static char *message = new char[zc->buffer_size_];
  static int32 len;
  static int32 dst_id;

  while (1) {
    // It is necessary for sender to push its dst_id in the message,
    // because the send_recv_ should know the dst_id to send message.
    len = zc->fifo_ring_.Fetch(mix_message, zc->buffer_size_);
    // Extract the dst_id and message from the mix_message
    // Calculate the true message's length
    sscanf(mix_message, "%d,", &dst_id);
    for (int32 i = 0; i < 12; i++)
      if (mix_message[i] == ',') {
        len = len - i - 1;
        memcpy(message, mix_message + i + 1, len);
        break;
      }
    // Refer to the id_to_addr_ for dst_addr_
    std::map<int32, std::string>::iterator iter =
                                  zc->id_to_addr_.find(dst_id);
    if (iter == zc->id_to_addr_.end()) {
      LOG(ERROR) << "Destination Node id error: dst_id = " << dst_id;
      LOG(ERROR) << "Can not send this message to destination.";
      continue;
    }
    zc->send_recv_.Send(iter->second, message, len);
    sleep(1);
  }
}

bool ZmqCommunicator::AddIdAddr(int32 id, std::string addr) {
  if (id_to_addr_.find(id) != id_to_addr_.end()) {
    LOG(INFO) << "This id already exists in the id_to_addr_";
    return false;
  } else {
    id_to_addr_.insert(std::make_pair(id, addr));
    LOG(INFO) << "AddIdAddr: id = " << id << " | addr = " << addr.c_str();
    return true;
  }
}

bool ZmqCommunicator::DeleteId(int32 id) {
  bool res = true;
  if (id_to_addr_.find(id) != id_to_addr_.end()) {
    LOG(INFO) << "DeleteId: " << id;
    res = send_recv_.CloseSocket(id_to_addr_[id]) == 0;
    id_to_addr_.erase(id);
  } else {
    LOG(INFO) << "This id doesn't exist in the id_to_addr_";
    res = false;
  }
  return res;
}

bool ZmqCommunicator::CheckIdAddr(int32 id, std::string addr) {
  if (id_to_addr_.find(id) == id_to_addr_.end()) {
    return false;
  } else {
    return id_to_addr_[id] == addr;
  }
}

}  // namespace rpscc


