// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include "src/communication/zmq_communicator.h"

namespace rpscc {

bool ZmqCommunicator::Initialize(int32 ring_size, bool is_sender,
                            int16 listenport, int32 buffer_size) {
  fifo_ring_.Initialize(ring_size);
  send_recv_.Initialize(is_sender, listenport);
  buffer_size_ = buffer_size;

  if (is_sender) {
    printf("Is sender\n");
    pthread_create(&add_fetch_, NULL, Consume, (void*)this);
  } else {
    printf("Is receiver\n");
    pthread_create(&add_fetch_, NULL, Produce, (void*)this);
  }

  return true;
}
void ZmqCommunicator::Finalize() {
  return;
}

int32 ZmqCommunicator::Send(int32 dst_id, const char* const message,
                            int32 len) {
  char mix_message[buffer_size_];
  snprintf(mix_message, buffer_size_, "%d,%s", dst_id, message);
  for (int i = 0; i < 12; i++) {
    if (mix_message[i] == ',') {
      len = len + i + 1;
      break;
    }
  }
  fifo_ring_.Add(mix_message, len);
  printf("Add: %s, %d\n", mix_message, len);
  return len;
}
int32 ZmqCommunicator::Send(int32 dst_id, const std::string& message) {
  int len = Send(dst_id, message.c_str(), message.size());
  return len;
}

int32 ZmqCommunicator::Receive(char* message, const int32 max_size) {
  int len = fifo_ring_.Fetch(message, max_size);
  printf("Fetch: %s, %d\n", message, len);
  return len;
}
int32 ZmqCommunicator::Receive(std::string* message) {
  char* char_message = new char[buffer_size_];
  int len = fifo_ring_.Fetch(char_message, buffer_size_);
  message->assign(char_message);
  delete char_message;
  return len;
}

void* ZmqCommunicator::Produce(void* arg) {
  ZmqCommunicator* zc = (ZmqCommunicator*) arg;
  static char* message = new char[zc->buffer_size_];
  static int32 len;
  printf("Start receiving.\n");

  while (1) {
    len = zc->send_recv_.Receive(message, zc->buffer_size_);
    zc->fifo_ring_.Add(message, len);
    printf("Receive: %s, %d\n", message, len);
    sleep(1);
  }
}
void* ZmqCommunicator::Consume(void* arg) {
  ZmqCommunicator* zc = (ZmqCommunicator*) arg;
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
    printf("dst_id = %d\n", dst_id);
    for (int32 i = 0; i < 12; i++)
      if (mix_message[i] == ',') {
        len = len - i - 1;
        memcpy(message, mix_message + i + 1, len);
        break;
      }
    // Refer to the id_to_addr_ for dst_addr_
    map<int32, string>::iterator iter = zc->id_to_addr_.find(dst_id);
    if (iter == zc->id_to_addr_.end()) {
      printf("Destination Node id error %d\n", dst_id);
      printf("Can not send this message to destination.");
      continue;
    }
    zc->send_recv_.Send(iter->second, message, len);
    printf("Send: %s, %d\n", message, len);
    sleep(1);
  }
}

bool ZmqCommunicator::AddIdAddr(int32 id, string addr) {
  if (id_to_addr_.find(id) != id_to_addr_.end()) {
    printf("This id already exists in the id_to_addr_\n");
    return false;
  } else {
    id_to_addr_.insert(std::make_pair(id, addr));
    printf("AddIdAddr: %d, %s\n", id, addr.c_str());
    return true;
  }
}

bool ZmqCommunicator::DeleteId(int32 id) {
  if (id_to_addr_.find(id) != id_to_addr_.end()) {
    printf("DeleteId: %d\n", id);
    id_to_addr_.erase(id);
    return true;
  } else {
    printf("This id doesn't exist in the id_to_addr_\n");
    return false;
  }
}

}  // namespace rpscc

