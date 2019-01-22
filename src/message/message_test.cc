//
// Created by pkwv on 1/21/19.
//

#include <iostream>

#include "src/message/message.pb.h"

int main() {
  rpscc::Message msg;
  msg.set_recv_id(2);
  msg.set_send_id(1);
  msg.set_message_type(rpscc::Message_MessageType_config);
  auto msg_str = msg.SerializeAsString();
  std::cout << msg.DebugString() << std::endl;
  assert(msg_str.c_str() != nullptr);
  return 0;
}

