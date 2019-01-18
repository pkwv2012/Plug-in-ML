// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#include "src/master/master.h"

DEFINE_int32(master_listen_port, 12018, "The listening port of master");
DEFINE_string(master_ip_port, "", "The master list of ip & port.");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  rpscc::Master* master = rpscc::Master::Get();
  //master->Initialize(FLAGS_master_listen_port);
  master->Initialize(FLAGS_master_ip_port);
  master->MainLoop();
  return 0;
}