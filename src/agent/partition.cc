// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include <stdio.h>

#include <algorithm>

#include "src/agent/partition.h"
#include "src/util/logging.h"

namespace rpscc {
  
bool Partition::Initialize(int32 key_range, int32 server_num, 
                           std::vector<int>& part_vec) {
  key_range_ = key_range;
  server_num_ = server_num;
  // Check the part_vec
  // sort(part_vec.begin(), part_vec.end());
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
bool Partition::Initialize(int32 key_range, int32 server_num, int* part_vec) {
  key_range_ = key_range;
  server_num_ = sever_num;
 
  // Check the part_vec
  // sort(part_vec, part_vec + server_num + 1);
  if (part_vec[0] != 0 || part_vec[server_num] != key_range) {
    printf("part_vec's content is wrong");
    return false;
  }
  
  part_vec_.clear();
  for (int32 i = 0; i <= server_num; i++) {
    part_vec_.push_back(part_vec[i]);
  }
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
    return server_num_ - 1;
  // Binary search
  int32 left, right, mid;
  left = 0;
  right = server_num_;
  while (left + 1 < right) {
    mid = (left + right) / 2;
    if (key >= part_vec_[mid]) {
      left = mid;
    } else {
      right = mid;
    }
  }
  return left;
}

int32 Partition::NextEnding(std::vector<int32>& keys, int32 start, 
                            int32& server_id) {
  server_id = GetServerByKey(keys[start]);
  int32 upper_bound = part_vec_[server_id + 1];
  int32 left, right, mid;
  
  if (server_id == server_num_ - 1)
    return keys.size();
  
  left = start;
  right = keys.size();
  
  while (left < right) {
    mid = (left + right) / 2;
    if (keys[mid] < upper_bound) {
      left = mid + 1;
    } else if (keys[mid] > upper_bound) {
      right = mid;
    } else {
      left = mid;
      break;
    }
  }
  
  return left;
}

}  // namespace rpscc
