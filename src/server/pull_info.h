// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)
#ifndef SRC_SERVER_PULL_INFO_H_
#define SRC_SERVER_PULL_INFO_H_

#include <vector>

#include "/src/util/common.h"

namespace rpscc {

// PullInfo maintains a pull request which is blocked for consistency
class PullInfo {
 public:
  PullInfo() {
    length = 0;
  }
  inline uint32 Length();
  inline void AddKey(uint32 key);
  inline uint32 Key(uint32 index);

 private:
  uint32 length;

  std::vector<uint32> keys;
};

}  // namespace rpscc

#endif  // SRC_SERVER_PULL_INFO_H_
