// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

#include "src/agent/agent.h"

DEFINE_int32(listen_port, 5555, "The listening port of agent");
DEFINE_string(master_ip_port, "", "The master's ip & port.");

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
  rpscc::Agent* agent = new rpscc::Agent();
  std::string para_memory_name = "/test_sharedMemory_sample1";
  std::string grad_memory_name = "/test_sharedMemory_sample2";
  std::string para_fifo_name = "/tmp/test_fifo_sample1";
  std::string grad_fifo_name = "/tmp/test_fifo_sample2";
  
  agent->Initialize(para_fifo_name, grad_fifo_name, 
                    para_memory_name, grad_memory_name, 
                    FLAGS_master_ip_port, FLAGS_listen_port);
  agent->Start();
  agent->Finalize();
  return 0;
}

