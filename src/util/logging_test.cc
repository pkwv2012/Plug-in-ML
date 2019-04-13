// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/19.
//

#include "chrono"
#include <thread>

#include "src/util/logging.h"
#include "gtest/gtest.h"

using namespace rpscc;

TEST(Logging, Logger) {
  rpscc::InitLogging("UselessString");
  LOG(INFO) << "Log info";
  LOG(ERROR) << "Log error";
  // LOG(FATAL) << "Log fatal";
}

TEST(Logging, MultiThreadLogger) {
  const int NUM = 100;
  auto print_hello = [&] () {
    for (int i = 0 ; i < NUM; ++ i) {
      LOG(INFO) << "hello";
    }
  };
  auto print_world = [&] () {
    for (int i = 0; i < NUM; ++ i) {
      LOG(INFO) << "world!";
    }
  };
  std::thread hello(print_hello);
  std::thread world(print_world);
  hello.join();
  world.join();
}


