<<<<<<< HEAD
// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#ifndef SRC_MASTER_MASTER_H_
#define SRC_MASTER_MASTER_H_

#include <mutex>

#include "src/message/message.pb.h"
#include "src/master/task_config.h"

namespace rpscc {

class Master {
 public:
  Master();

  // wait for servers & agents ready
  void WaitForClusterReady();

  bool DeliverConfig();

  void MainLoop();

 private:
  std::mutex config_mutex_;
  TaskConfig config_;
};

}  // namespace rpscc

#endif  // SRC_MASTER_MASTER_H_
||||||| merged common ancestors
=======
// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#ifndef SRC_MASTER_MASTER_H_
#define SRC_MASTER_MASTER_H_

#include <mutex>

#include "src/communication/communicator.h"
#include "src/message/message.pb.h"
#include "src/master/task_config.h"

namespace rpscc {

class Master {
 public:
  Master();

  void Initialize(const int16& listen_port=12018);

  // wait for servers & agents ready
  void WaitForClusterReady();

  bool DeliverConfig();

  void MainLoop();

 private:
  std::mutex config_mutex_;
  TaskConfig config_;
  Communicator* sender_;
  Communicator* receiver_;
};

}  // namespace rpscc

#endif  // SRC_MASTER_MASTER_H_
>>>>>>> 6dedbc8a57e71447e87f5c32925e962e1e870b68
