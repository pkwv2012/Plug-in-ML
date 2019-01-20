#include "src/communication/zmq_communicator.h"

using namespace rpscc;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("There should be 3 arguments.\n");
    return 1;
  }
  ZmqCommunicator comm;
  char message[128];
  int max_size = 128;
  int port;
  sscanf(argv[2], "%d", &port);
  if (argv[1][0] == 'r') {
    printf("You create a receiver\n");
    comm.Initialize(16, false, port, 128);
    while (1) {
      comm.Receive(message, max_size);
      printf("Get message %s\n", message);
      sleep(1);
    }
  } else {
    printf("You create a sender\n");
    comm.Initialize(16, true, port, 128);
    comm.AddIdAddr(1, "172.17.0.1:5555");
    for (int i = 0; i < 10; i++) {
      sprintf(message, "Hello %d", i);
      comm.Send(1, message, strlen(message)); 
      printf("Send message %s\n", message);
      sleep(1);
    }
    sleep(1);
  }
  return 0;
}
