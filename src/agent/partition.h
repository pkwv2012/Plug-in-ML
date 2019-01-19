// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#ifndef SRC_AGENT_PARTITION_H_
#define SRC_AGENT_PARTITION_H_

#include <vector>

#include "src/util/common.h"

namespace rpscc {

// Partition is a class to support consistent hash algorithm. It store a vector
// whose elements are a list of hash value. When an agent wants to push its 
// data to servers, it should refer to a Partiton's instance for the server_id 
// list. This is because parameters are distributed among servers.
class Partition {
 public:
  Partition() {}
  ~Partition() {}
  bool Initialize(int32 key_range, int32 server_num, 
                  std::vector<int>& part_vec);
  bool Initialize(int32 key_range, int32 server_num, int32* part_vec);
  void Finalize();
  // Binary search in part_vec_ for key
  int32 GetServerByKey(int32 key);
  // Get 'end' point for the 'strat'. They will satisfies that keys lie in
  // [start, end) will belong to the same server, but key 'end' belongs to 
  // the next server or is greater than key_range_. By the way, the keys as
  // parameters should be sorted.
  int32 NextEnding(std::vector<int32> keys, int32 start, int32& server_id);
 private:
  // Number of keys in the system
  int32 key_range_;
  // Number of servers
  int32 server_num_;
  // The partition vector, whose size is 'server_num_ + 1'.
  // The elements in this vector indicates the hash values in consistent hash
  // algorithm. Take server_i as an example, the keys corresponding to it are
  // those between [part_vec_[(i >> 1) - 1], part_vec_[i >> 1] ).
  // Last but not least, servers' ids are 2, 4, 6, 8..., master's id is 0,
  // agents' ids are 1, 3, 5, 7... And this way of numbering results in the
  // array addressing above.
  std::vector<int32> part_vec_;
};

}  // namespace rpscc

#endif  // SRC_AGENT_PARTITION_H_
