#include <iostream>
#include <time.h>

#include "gtest/gtest.h"
#include "src/agent/agent.h"
#include "src/util/logging.h"

using std::string;

TEST (Agent, Initialize) {
  rpscc::Agent agent;
  string para_fifo_name = "cute_para_fifo";
  string grad_fifo_name = "cute_grad_fifo";
  string para_memory_name = "cute_para_mem";
  string grad_memory_name = "cute_grad_mem";
  string master_addr = "127.0.0.1:5000";
  agent.Initialize(para_fifo_name, grad_fifo_name,
                   para_memory_name, grad_memory_name,
                   master_addr);
  LOG(INFO) << "Agent: Start" << std::endl;
  sleep(10000);

}
