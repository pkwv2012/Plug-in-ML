// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/19.
//

#include "src/util/logging.h"
#include "gtest/gtest.h"

TEST(Logging, Logger) {
  rpscc::InitLogging("UselessString");
  LOG(INFO) << "Log info";
  LOG(ERROR) << "Log error";
  LOG(FATAL) << "Log fatal";
}


