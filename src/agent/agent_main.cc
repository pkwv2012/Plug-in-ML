// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include "src/agent/agent.h"

DEFINE_int32(listen_port, 5555, "The listening port of agent");
DEFINE_string(master_ip_port, "", "The master's ip & port.");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  rpscc::Agent* agent = new rpscc::Agent();
  std::string para_fifo_name = "/dev/shm/test_sharedMemory_sample1";
  std::string grad_fifo_name = "/dev/shm/test_sharedMemory_sample2";
  std::string para_memory_name = "/tmp/test_fifo_sample1";
  std::string grad_memory_name = "/tmp/test_fifo_sample2";
  
  agent->Initialize(para_fifo_name, grad_fifo_name, 
                    para_memory_name, grad_memory_name, 
                    FLAGS_master_ip_port, FLAGS_listen_port);
  agent->Start();
  agent->Finalize();
  return 0;
}

