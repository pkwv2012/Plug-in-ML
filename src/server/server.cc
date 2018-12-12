// Copyright 2018 The RPSCC Authors. All Rights Reserved.
// Author : Xu Song (sazel.sekibanki@gmail.com)

#include <string>

#include "/src/message/message.pb.h"
#include "/src/server/server.h"

namespace rpscc {

// In Initialize() the server configures itself by sending its IP to the
// master and receiving related configuration information.
bool Server::Initialize() {
  // First initialize server communicators
  sender.Initialize(/* size */, true, 1024, /* size */);
  sender.AddIdAddr(0, /* master address and port */);
  receiver.Initialize(/* size */, false, /* port */, /* size */);

  // Send the server local ip to master and receive config information
  local_ip = /* find a way to get server ip */;
  Message_RegisterMessage reg_msg();
  Message_ConfigMessage config_msg();
  std::string reg_str;
  std::string config_str;
  reg_msg.set_ip(local_ip);
  reg_msg.SerializeToString(&reg_str);
  if (sender.Send(0, reg_str) == -1) {
    /* handle */
  }
  if (receiver.Receive(&config_str) == -1) {
    /* handle */
  }
  config_msg.ParseFromString(config_str);

  // Initialization of server fields
  bottom_version = 0;
  consistency_bound = config_msg.bound();
  agent_num = config_msg.worker_num();
  server_num = config_msg.server_num();

  // Initialize server ips and ids,
  // at the same time find local_id from corresponding ip list
  bool found_local
  for (uint32 i = 0; i < server_num; ++i) {
    std::string server_i_ip = config_msg.server_ip(i);
    uint32 server_i_id = config_msg.server_id(i);
    server_ips.push_back(server_i_ip);
    server_ids.push_back(server_i_id);
    if (server_i_ip == local_ip) {
      local_id = server_i_id;
      found_local = true;
    }
  }
  if (found_local == false) {
    /* handle */
  }

  start_key;
  parameter_length;
  std:vecotr<float> parameters;
  std:vector<std : queue<std : unique_ptr<KeyValueList> > > version_buffer
  std : deque<uint32> finish_count;

}

}  // namespace rpscc