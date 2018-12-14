// Copyright (c) 2018 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#ifndef RPSCC_SHAREDMEMORY_H
#define RPSCC_SHAREDMEMORY_H

namespace rpscc {

class SharedMemory {
 public:
  SharedMemory() {}
  ~SharedMemory() {}

  bool Initialize(char* ipc_name, bool is_reader);

  void Open();

  void Write();

  void Read();
};

}

#endif //RPSCC_SHAREDMEMORY_H
