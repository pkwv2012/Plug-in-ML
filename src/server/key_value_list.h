// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)
#ifndef SRC_SERVER_KEY_VALUE_LIST_H_
#define SRC_SERVER_KEY_VALUE_LIST_H_

#include <vector>

#include "/src/util/common.h"

namespace rpscc {

// The KeyValueList class saves the update data in the form of two lists.
// The first list represents keys, and the second represents data.
class KeyValueList {
 public:
  KeyValueList() {
    length_ = 0;
  }

  inline void AddPair(uint32 key, float value);
  inline uint32 Key(uint32 index);
  inline float Value(uint32 index);
  inline uint32 Length();

 private:
  uint32 length_;

  std::vector<uint32> keys_;
  std::vector<float> parameters_;
};

}  // namespace rpscc

#endif  // SRC_SERVER_KEY_VALUE_LIST_H_
