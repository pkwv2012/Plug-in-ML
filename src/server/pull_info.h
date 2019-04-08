// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)
#ifndef SRC_SERVER_PULL_INFO_H_
#define SRC_SERVER_PULL_INFO_H_

#include <vector>

#include "src/util/common.h"

namespace rpscc {

// PullInfo maintains a pull request which is blocked for consistency
class PullInfo {
 public:
  PullInfo() {
    length_ = 0;
    id_ = 0;
  }
  int32 Length();
  void AddKey(int32 key);
  int32 Key(int32 index);
  int32 get_id() {
    return id_;
  }
  void set_id(int32 id) {
    id_ = id;
  }

 private:
  int32 length_;
  int32 id_;

  std::vector<int32> keys_;
};

}  // namespace rpscc

#endif  // SRC_SERVER_PULL_INFO_H_
