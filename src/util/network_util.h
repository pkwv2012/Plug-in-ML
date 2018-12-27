// Copyright 2018 The RPSCC Authors. All Rights Reserved.
//
// Created by PeikaiZheng on 2018/12/19.
//

#ifndef SRC_UTIL_NETWORK_UTIL_H_
#define SRC_UTIL_NETWORK_UTIL_H_

#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string>

namespace rpscc {

void GetIP(const std::string& interface, std::string* ip) {
  struct ifaddrs * ifAddrStruct = NULL;
  struct ifaddrs * ifa = NULL;
  void * tmpAddrPtr = NULL;

  getifaddrs(&ifAddrStruct);
  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL) continue;
    if (ifa->ifa_addr->sa_family == AF_INET) {
      // is a valid IP4 Address
      tmpAddrPtr = &(reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr))->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      if (strncmp(ifa->ifa_name,
                  interface.c_str(),
                  interface.size()) == 0) {
        *ip = addressBuffer;
        break;
      }
    }
  }
  if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
}


//
// return the IP address and Interface the first interface which is not
// loopback
//
// only support IPv4
//
void GetAvailableInterfaceAndIP(
  std::string* interface, std::string* ip) {

  struct ifaddrs * ifAddrStruct = nullptr;
  struct ifaddrs * ifa = nullptr;

  interface->clear();
  ip->clear();
  getifaddrs(&ifAddrStruct);
  for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
    if (nullptr == ifa->ifa_addr) continue;

    if (AF_INET == ifa->ifa_addr->sa_family &&
        0 == (ifa->ifa_flags & IFF_LOOPBACK)) {
      char address_buffer[INET_ADDRSTRLEN];
      void* sin_addr_ptr = &(reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr))->sin_addr;
      inet_ntop(AF_INET, sin_addr_ptr, address_buffer, INET_ADDRSTRLEN);

      *ip = address_buffer;
      *interface = ifa->ifa_name;

      break;
    }
  }
  if (nullptr != ifAddrStruct) freeifaddrs(ifAddrStruct);
  return;
}


// return an available port on local machine
//
// only support IPv4
// return 0 on failure

int GetAvailablePort() {
  struct sockaddr_in addr;
  addr.sin_port = htons(0);  // have system pick up a random port available for me
  addr.sin_family = AF_INET;  // IPV4
  addr.sin_addr.s_addr = htonl(INADDR_ANY);  // set our addr to any interface

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (0 != bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
    perror("bind():");
    return 0;
  }
  socklen_t addr_len = sizeof(struct sockaddr_in);

  if (0 != getsockname(sock, (struct sockaddr*)&addr, &addr_len)) {
    perror("getsockname():");
    return 0;
  }

  int ret_port = ntohs(addr.sin_port);
  close(sock);
  return ret_port;
}

}  // namespace rpscc

#endif  // SRC_UTIL_NETWORK_UTIL_H_
