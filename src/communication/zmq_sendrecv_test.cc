// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "src/communication/zmq_sendrecv.h"

using namespace rpscc;

// A tester for the ZmqSendRecv class.
// argv[1] indicates the process is a sender or receiver.

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("There should be exactly 3 arguments!\n");
    return 1;
  }
  if (argv[1][0] == 'r') {
    printf("You create a receiver\n");
    ZmqSendRecv receiver;
    int port;
    sscanf(argv[2], "%d", &port);
    receiver.Initialize(false, port);
    const int max_size = 100;
    char message[max_size];
    while (1) {
      int size = receiver.Receive(message, max_size);
      printf("message = %s, size = %d\n", message, size);
    }
    receiver.Finalize();
  }
  else {
    printf("You create a sender\n");
    ZmqSendRecv sender;
    sender.Initialize(true, 0);
    char message[32];
    for (int i = 0; i < 10; i++) {
      sprintf(message, "Hello World, %d", i);
      sender.Send(argv[2], message, strlen(message));
      sleep(1);
    }
    sender.Finalize();
  }
  return 0;
}
