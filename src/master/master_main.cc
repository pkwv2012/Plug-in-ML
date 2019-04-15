// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

#include "src/master/master.h"

DEFINE_int32(master_listen_port, 12018, "The listening port of master");
DEFINE_string(master_ip_port, "", "The master list of ip & port.");

void SegHandler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char** argv) {
  // 1. install signal handler
  signal(SIGSEGV, SegHandler);
  signal(SIGINT, SegHandler);
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  rpscc::Master* master = rpscc::Master::Get();
  //master->Initialize(FLAGS_master_listen_port);
  master->Initialize(FLAGS_master_ip_port);
  master->MainLoop();
  return 0;
}