// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include <stdio.h>

#include <algorithm>
#include "src/agent/partition.h"

namespace rpscc {
  
bool Partition::Initialize(int32 key_range, int32 server_num, 
                           std::vector<int>& part_vec) {
  key_range_ = key_range;
  server_num_ = server_num;
  // Check the part_vec
  sort(part_vec.begin(), part_vec.end());
  if (part_vec.size() != (server_num + 1)) {
    printf("part_vec's size is wrong");
    return false;
  }
  if (part_vec[0] != 0 || part_vec[server_num] != key_range) {
    printf("part_vec's content is wrong");
    return false;
  }
  
  part_vec_ = part_vec;
  return true;
}
void Partition::Finalize() {
  part_vec_.clear();
}

int32 Partition::GetServerByKey(int32 key) {
  // Check if the key falls in the [0, key_range_]
  if (key < 0 || key > key_range_)
    return -1;
  if (key == key_range_)
    return server_num_ << 1;
  for (int32 i = 0; i < server_num_ - 1; i++) {
    if (key >= part_vec_[i] && key < part_vec_[i + 1])
      return (i + 1) << 1;
  }
  return -1;
}
  
}  // namespace rpscc
