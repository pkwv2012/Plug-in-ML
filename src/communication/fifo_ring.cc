// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include "src/communication/fifo_ring.h"

namespace rpscc {
  bool FifoRing::Initialize(int32 ring_size) {
    ring_size_ = ring_size;
    produce_point_ = 0;
    consume_point_ = 0;
    sem_init(&empty_sem_, 0, ring_size_);
    sem_init(&full_sem_, 0, 0);
    ring_ = new char*[ring_size_];
    data_sizes_ = new int[ring_size_];
    return true;
  }
  void FifoRing::Finalize() {
    sem_destroy(&empty_sem_);
    sem_destroy(&full_sem_);
    delete ring_;
    delete data_sizes_;
    // Maybe I will add empty_sem_ processing method in the future
  }
  int32 FifoRing::Add(const char* const message, int32 len) {
    sem_wait(&empty_sem_);
    
    ring_[produce_point_] = new char[len];
    memcpy(ring_[produce_point_], message, len);
    data_sizes_[produce_point_] = len;
    
    sem_post(&full_sem_);
    produce_point_++;
    produce_point_ %= ring_size_;
    
    return len;
  }
  int32 FifoRing::Fetch(char* message, const int32 max_size) {
    sem_wait(&full_sem_);
    
    if (ring_[consume_point_] == NULL) {
      return -1;
    }
    int32 len = data_sizes_[consume_point_];
    memcpy(message, ring_[consume_point_], len);
    
    sem_post(&empty_sem_);
    consume_point_++;
    consume_point_ %= ring_size_;
    return len;
  }
  
}  // namespace rpscc
