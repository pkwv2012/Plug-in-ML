// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include "gflags/gflags.h"
#include "src/util/logging.h"
#include "src/server/server.h"

// server_main defines the main function for a server process.
int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  rpscc::Server server;
  if (server.Initialize() == false) {
    LOG(ERROR) << "Failed to initialize server.";
    return 1;
  }
  server.Start();
}
