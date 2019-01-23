// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include <src/util/logging.h>
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

    int32_t index = -1;
    {
      std::lock_guard<std::mutex> guard(produce_mutex);
      index = produce_point_;
      produce_point_++;
      produce_point_ %= ring_size_;
    }

    CHECK_NE(index, -1) << "Fifo add error." << std::endl;

    ring_[index] = new char[len];
    memcpy(ring_[index], message, len);
    data_sizes_[index] = len;

    sem_post(&full_sem_);

    return len;
  }
  int32 FifoRing::Fetch(char* message, const int32 max_size) {
    sem_wait(&full_sem_);

    int32_t index = -1;
    {
      std::lock_guard<std::mutex> guard(consume_mutex);
      index = consume_point_;
      consume_point_ ++;
      consume_point_ %= ring_size_;
    }
    CHECK_NE(index, -1) << "Fetch error." << std::endl;

    if (ring_[index] == NULL) {
      return -1;
    }
    int32 len = data_sizes_[index];
    memcpy(message, ring_[index], len);

    sem_post(&empty_sem_);
    return len;
  }

}  // namespace rpscc

