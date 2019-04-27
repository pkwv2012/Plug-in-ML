// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include <execinfo.h>
#include <signal.h>
#include "unistd.h"

#include "gflags/gflags.h"
#include "src/util/logging.h"
#include "src/server/server.h"

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

// server_main defines the main function for a server process.
int main(int argc, char **argv) {
  // 1. install signal handler
  signal(SIGSEGV, SegHandler);
  signal(SIGINT, SegHandler);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  rpscc::Server server;
  if (server.Initialize() == false) {
    LOG(ERROR) << "Failed to initialize server.";
    return 1;
  }
  server.Start();
}
