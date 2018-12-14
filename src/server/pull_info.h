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
    length_ = 0;
    id_ = 0;
  }
  inline uint32 Length();
  inline void AddKey(uint32 key);
  inline uint32 Key(uint32 index);
  inline uint32 get_id() {
    return id_;
  }
  inline void set_id(uint32 id) {
    id_ = id;
  }

 private:
  uint32 length_;
  uint32 id_;

  std::vector<uint32> keys_;
};

}  // namespace rpscc

#endif  // SRC_SERVER_PULL_INFO_H_
