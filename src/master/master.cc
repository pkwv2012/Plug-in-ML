// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/8.
//

#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#include "src/communication/zmq_communicator.h"
#include "src/master/master.h"
#include "src/util/logging.h"


namespace rpscc {

DEFINE_int32(heartbeat_timeout, 30, "The maximum time(seconds) to decide "
                                    "whether the node is offline.");
DEFINE_int32(listen_port, 16666, "The listening port of cluster.");
DEFINE_int32(heartbeat_gap, 5, "The heartbeat gap(seconds).");
DEFINE_int32(detect_dead_node, 10, "The time(seconds) gap to detect dead node.");
DEFINE_int32(max_cluster_node, 10000, "The maximum cluster number.");
DEFINE_string(zookeeper_hosts, "127.0.0.1:2181", ""
              "Comma separated host:port pairs, "
              "each corresponding to a zkserver. e.g. "
              "'127.0.0.1:3000,127.0.0.1:3001,127.0.0.1:3002'");
DEFINE_int32(client_id, 0, "The client id used in zookeeper.");
DEFINE_string(task_name, "ml_task", "The name used in zookeeper node.");

void Master::WaitForClusterReady() {
}

#ifdef USE_ZOOKEEPER
// return 0, if create success (means this master is the lead master),
// otherwise failed, which means this master is not the lead master.
static int CreateZookeeperNode(zhandle_t* zh) {
  struct ACL CREATE_ONLY_ACL[] = {{ZOO_PERM_ALL, ZOO_ANYONE_ID_UNSAFE}};
  struct ACL_vector CREATE_ONLY = {1, CREATE_ONLY_ACL};
  char buffer[512];
  std::string node_name = "/" + FLAGS_task_name;
  std::string node_value = "master_ip";
  int rc = zoo_create(zh,
                      node_name.c_str() ,
                      node_value.c_str(),
                      5,
                      &CREATE_ONLY,
                      ZOO_EPHEMERAL,  // create ephemeral node
                                      // for leader selection
                      buffer,
                      sizeof(buffer)-1);
  LOG(INFO) << "zoo_create: " << rc << std::endl;
  struct Stat stat;
  // register a wathcer.
  int zoo_state = zoo_exists(zh, node_name.c_str(), 1, &stat);
  LOG(INFO) << "here" << std::endl;
  LOG(INFO) << "zoo_exists: " << zoo_state << std::endl;
  return rc;
}

static void ZkCallback(zhandle_t* zh, int type, int state,
                       const char* path, void* watchCtx) {
  LOG(INFO) << "Callback|type=" << type
            << "|state=" << state
            << "|path=" << path << std::endl;
  std::string node_name = "/" + FLAGS_task_name;
  std::string spath(path);
  if (spath == node_name) {
    auto rc = CreateZookeeperNode(zh);
    if (rc == 0) {
      // TODO(peikai): Do what master lead should do.
      Master::Get()->set_is_lead(true);
      Master::Get()->MainLoop();
    }  // Otherwise, do nothing
  }
}

void Master::init_zookeeper() {
  // 1. connect to zookeeper,
  zh_ = zookeeper_init(FLAGS_zookeeper_hosts.c_str(),
                           ZkCallback,
                           100,
                           0,
                           nullptr,
                           0);
  // 2. create zookeeper ephemeral node;
  int rc = CreateZookeeperNode(zh_);
  if (rc == 0) {
    // TODO(peikai): Do what master lead should do.
    is_lead_ = true;
  }   // Otherwise, do nothing.
}

#endif

void Master::MainLoop() {
  LOG(INFO) << "This master is "
            << (is_lead_? "" : "not ")
            << "the lead master" << std::endl;
  if (!is_lead_) {
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    return;
  }
  bool terminated = false;
  while (!terminated) {
    std::string msg_str;
    LOG(INFO) << "Receiving";
    int32_t len = receiver_->Receive(&msg_str);
    std::fstream fs;
    fs.open("master_msg.txt", std::fstream::app);
    fs << msg_str << std::endl;
    fs.close();
    LOG(INFO) << "Master||Receive " << len << " bytes" << std::endl;
    LOG(INFO) << "Receive finish";
    Message msg;
    msg.ParseFromString(msg_str);
    LOG(INFO) << msg.DebugString() << std::endl;
    switch (msg.message_type()) {
      case Message_MessageType_config:
      case Message_MessageType_request:
        LOG(INFO) << "Invalid message type";
        break;

      case Message_MessageType_heartbeat: {
        ProcessHeartbeatMsg(msg);
        break;
      }

      case Message_MessageType_register_: {
        // Master node should response to all cluster node,
        // after master received the entire register message.
        ProcessRegisterMsg(&msg);
        if (config_.Ready()) {
          LOG(INFO) << "Cluster ready!";
          config_.GeneratePartition();
          for (auto pr : config_.get_id_to_addr()) {
            sender_->AddIdAddr(pr.first, pr.second);
            std::stringstream ss(pr.second);
            std::string ip, port;
            std::getline(ss, ip, ':');
            std::getline(ss, port, ':');
            sender_->AddIdAddr(pr.first + FLAGS_max_cluster_node,
                ip + ":" + std::to_string(std::stoi(port) + 1));
          }
          DeliverConfig();
          heartbeat_ = std::make_unique<std::thread>(
              std::bind(&Master::DeliverHeartbeatLoop, this));
          auto cur_time = std::chrono::system_clock::now();
          for (int i = 0; i < config_.get_node_ip().size(); ++i) {
            alive_node_[i] = cur_time;
          }
          detect_dead_node_ = std::make_unique<std::thread>(
              std::bind(&Master::DetectDeadNode, this));
        }
        LOG(INFO) << "worker_num=" << config_.worker_num()
                  << "server_num=" << config_.server_num()
                  << "worker_cur_num=" << config_.agent_id().size()
                  << "server_cur_num=" << config_.server_id().size()
                  << std::endl;

        break;
      }
      case Message_MessageType_terminate: {
        terminated_node_.insert(msg.send_id());
        if (terminated_node_.size() == config_.worker_num()) {
          terminated = true;
        }
        break;
      }
      default:
        LOG(ERROR) << "Unknown message type";
    }
  }
}

Master::Master() {
  LOG(INFO) << "Master initialization" << std::endl;
  config_.Initialize(""/*config file name*/);
  is_lead_ = true;
}

bool Master::DeliverConfig() {
  Message* msg = new Message();
  msg->set_send_id(0);  // Id of master
  msg->set_message_type(Message_MessageType_config);
  msg->set_allocated_config_msg(config_.ToMessage());
  LOG(INFO) << "config right" << std::endl;
  for (int32_t i = 0; i < config_.get_node_ip().size(); ++i) {
    msg->set_recv_id(i);
    LOG(INFO) << i << std::endl;
    auto send_byte = sender_->Send(i, msg->SerializeAsString());
    LOG(INFO) << "Send to " << i << " config of " << send_byte;
  }
  delete msg;
  return true;
}

void Master::ProcessRegisterMsg(Message *msg) {
  auto register_msg = msg->register_msg();
  bool is_server = register_msg.is_server();
  std::string ip = register_msg.ip();
  int32_t port = register_msg.port();
  LOG(INFO) << "Master " << is_server << "  port" << port << std::endl;
  if (is_server) {
    config_.AppendServer(ip, port);
  } else {
    config_.AppendAgent(ip, port);
  }
}

void Master::ProcessHeartbeatMsg(const Message& msg) {
  CHECK(msg.has_heartbeat_msg());
  auto heartbeat_msg = msg.heartbeat_msg();
  CHECK(heartbeat_msg.is_live());
  auto send_id = msg.send_id();
  auto cur_time = std::chrono::system_clock::now();
  // CHECK(alive_node_.find(send_id) != alive_node_.end());
  alive_node_[send_id] = cur_time;
  LOG(INFO) << "Heartbeat from " << send_id << ", ip = "
             << config_.GetIp(send_id);
}

std::vector<int> Master::GetDeadNode() {
  std::vector<int> dead_node;
  auto cur_time = std::chrono::system_clock::now();
  for (const auto& pr : alive_node_) {
    if (pr.second
        + std::chrono::seconds(FLAGS_heartbeat_timeout)
        < cur_time) {
      dead_node.push_back(pr.first);
    }
  }
  return dead_node;
}

int32_t Master::Initialize(const std::string &master_ip_port) {
#ifdef USE_ZOOKEEPER
  // 1. Init zookeeper node, finish leader election.
  init_zookeeper();
#endif
  std::stringstream ss(master_ip_port);
  int count = 0;
  while (ss.good()) {
    std::string ip_port;
    std::getline(ss, ip_port, ',');
    if (ip_port.size() > 0) {
      config_.AppendMaster(ip_port);
      ++count;
    }
  }
  std::cout << "Start Initialize";
  this->sender_.reset(new ZmqCommunicator());
  LOG(INFO) << "Create sender" << std::endl;
  this->sender_->Initialize(16, true, FLAGS_listen_port);
  LOG(INFO) << "Sender finish" << std::endl;
  this->receiver_.reset(new ZmqCommunicator());
  this->receiver_->Initialize(16, false, FLAGS_listen_port);
  LOG(INFO) << "Master init finish." << "count = " << count << std::endl;
  return count;
}

// Master should deliver the heartbeat message to a new port of node.
// So that we should deliver message to recv_id + FLAGS_max_cluster_node,
// because the zmq_sender map <id> to <ip>:<port>.
void Master::DeliverHeartbeat() {
  Message* msg = new Message();
  msg->set_send_id(0);  // Id of master
  msg->set_message_type(Message_MessageType_heartbeat);
  Message_HeartbeatMessage* heartbeat_msg = new Message_HeartbeatMessage();
  heartbeat_msg->set_is_live(true);
  msg->set_allocated_heartbeat_msg(heartbeat_msg);
  if (config_.config_changed()) {
    msg->set_allocated_config_msg(config_.ToMessage());
    config_.set_config_changed(false);
  }
  for (auto id : config_.agent_id()) {
    msg->set_recv_id(id);
    auto send_byte = sender_->Send(id + FLAGS_max_cluster_node,
                                   msg->SerializeAsString());
    LOG(INFO) << "Send to agent " << id << " heartbeat of " << send_byte;
  }
  for (auto id : config_.server_id()) {
    msg->set_recv_id(id);
    auto send_byte = sender_->Send(id, msg->SerializeAsString());
    LOG(INFO) << "Send to server" << id << " heartbeat of " << send_byte;
  }
  delete msg;
}

void Master::DeliverHeartbeatLoop() {
  while (1) {
    // Wait for agent start heartbeat thread.
    std::this_thread::sleep_for(
      std::chrono::seconds(FLAGS_heartbeat_gap));
    DeliverHeartbeat();
  }
}

void Master::DetectDeadNode() {
  while (1) {
    // If there are dead_node, master should restart the node.
    std::this_thread::sleep_for(
      std::chrono::seconds(FLAGS_detect_dead_node));
    auto dead_node = GetDeadNode();
    // Reconfig the cluster
    if (dead_node.size() > 0) {
      // Log dead node.
      auto& logger = LOG(INFO);
      for (auto node : dead_node) {
        logger << node << ":" << config_.GetIp(node) << " ";
      }
      config_.FixConfig(dead_node);
    }
  }
}

}  // namespace rpscc
