// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include "src/server/pull_info.h"

namespace rpscc {

// Everytime only one key is added to its vector
void PullInfo::AddKey(uint32 key) {
  keys_.push_back(key);
  length_++;
}

uint32 PullInfo::Key(uint32 index) {
  return keys_[index];
}

uint32 PullInfo::Length() {
  return length_;
}

}  // namespace rpscc

