// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#ifndef SRC_COMMUNICATION_FIFO_RING_H_
#define SRC_COMMUNICATION_FIFO_RING_H_

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstring>

#include "src/util/common.h"

namespace rpscc {

// FifoRing is a implemention of round-robin queue.
// This ring now support one producer and one consumer to work at the same
// time, and it is thread-safe.
class FifoRing {
 public:
  FifoRing() {}
  ~FifoRing() {}
  // To initialize the ring
  bool Initialize(int32 ring_size);
  // To finalize the ring
  void Finalize();
  // Add or Fetch a message from the ring, return the size of message added
  // or fetched.
  int32 Add(const char* const message, int32 len);
  int32 Fetch(char* message, const int32 max_size);

 private:
  // Size of the ring.
  int32 ring_size_;
  // The body of the ring
  char** ring_;
  // data_sizes_ stores all data blocks' size information.
  int* data_sizes_;
  // These pointers point to the place where producer or consumer start to
  // produce or consume data in the ring.
  int32 produce_point_;
  int32 consume_point_;
  // empty_sem_ is used for consumer, which indicates that whether there is
  // data in the ring.
  sem_t empty_sem_;
  // full_sem_ is used for producer, which indicates that whether there is
  // space in the ring.
  sem_t full_sem_;
};

}  // namespace rpscc

#endif  // SRC_COMMUNICATION_FIFO_RING_H_

