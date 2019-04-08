// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include "src/server/key_value_list.h"

namespace rpscc {

// Everytime only one key-value pair is added to their respective vector
void KeyValueList::AddPair(int32 key, float value) {
  keys_.push_back(key);
  values_.push_back(value);
  length_++;
}

int32 KeyValueList::Key(int32 index) {
  return keys_[index];
}

float KeyValueList::Value(int32 index) {
  return values_[index];
}

int32 KeyValueList::Length() {
  return length_;
}

}  // namespace rpscc
