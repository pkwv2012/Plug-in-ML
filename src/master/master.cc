<<<<<<< HEAD
// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#include "src/master/master.h"

namespace rpscc {

void Master::WaitForClusterReady() {
}

}

||||||| merged common ancestors
=======
// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#include "src/communication/zmq_communicator.h"
#include "src/master/master.h"

namespace rpscc {

void Master::WaitForClusterReady() {
}

void Master::Initialize(const int16 &listen_port) {
  this->sender_ = new ZmqCommunicator();
  this->sender_->Initialize(16, true, listen_port);
  this->receiver_ = new ZmqCommunicator();
  this->receiver_->Initialize(16, false, listen_port);
}

}

>>>>>>> 6dedbc8a57e71447e87f5c32925e962e1e870b68
