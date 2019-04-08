// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)
#ifndef SRC_SERVER_KEY_VALUE_LIST_H_
#define SRC_SERVER_KEY_VALUE_LIST_H_

#include <vector>

#include "src/util/common.h"

namespace rpscc {

// The KeyValueList class saves the update data in the form of two lists.
// The first list represents keys, and the second represents data.
class KeyValueList {
 public:
  KeyValueList() {
    length_ = 0;
  }

  void AddPair(int32 key, float value);
  int32 Key(int32 index);
  float Value(int32 index);
  int32 Length();

 private:
  int32 length_;

  std::vector<int32> keys_;
  std::vector<float> values_;
};

}  // namespace rpscc

#endif  // SRC_SERVER_KEY_VALUE_LIST_H_
