// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include "/src/server/key_value_list.h"

namespace rpscc {

// Everytime only one key-value pair is added to their respective vector
inline void KeyValueList::AddPair(uint32 key, float value) {
  keys_.push_back(key);
  values_.push_back(value);
  length_++;
}

inline uint32 KeyValueList::Key(uint32 index) {
  return keys_[index];
}

inline float KeyValueList::Value(uint32 index) {
  return values_[index];
}

inline uint32 KeyValueList::Length() {
  return length_;
}

}  // namespace rpscc
