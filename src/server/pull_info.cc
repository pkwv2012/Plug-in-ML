// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include "/src/server/pull_info.h"

namespace rpscc {

// Everytime only one key is added to its vector
inline void PullInfo::AddKey(uint32 key) {
  keys.push_back(key);
  length++;
}

inline uint32 PullInfo::Key(uint32 index) {
  return keys[index];
}

inline uint32 PullInfo::Length() {
  return length;
}

}  // namespace rpscc

