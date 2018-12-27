// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Chenbin Zhang (zcbin@pku.edu.cn)

#include "src/communication/fifo_ring.h"
#include "stdio.h"
#include <pthread.h>

using namespace rpscc;

FifoRing fifo_ring;

void* Produce(void* arg) {
  char input[20];
  for (int i = 0; i < 100; i++) {
    sprintf(input, "Hello %d", i);
    fifo_ring.Add(input, strlen(input));
    printf("Add: %s\n", input);
    sleep(1);
  }
}

void* Consume(void* arg) {
  char output[20];
  for (int i = 0; i < 100; i++) {
    fifo_ring.Fetch(output, 20);
    printf("Fetch: %s\n", output);
    sleep(10);
  }
}
// This is a tester for fifo_ring
int main() {
  fifo_ring.Initialize(4);
  
  pthread_t P, C;
  pthread_create(&P, NULL, Produce, NULL);
  pthread_create(&C, NULL, Consume, NULL);
  pthread_join(P, NULL);
  pthread_join(C, NULL);
  
  fifo_ring.Finalize();
}
