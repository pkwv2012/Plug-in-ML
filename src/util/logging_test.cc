// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/19.
//

#include "src/util/logging.h"

int main() {
  rpscc::InitLogging("log.out");
  CHECK_EQ(1, 2) << "1 != 2";
  return 0;
}

