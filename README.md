## Introduction

RPSCC project is a robust parameter server with configurable consistency.

## Installation

```sh
git clone https://github.com/purkyston/rpscc
cd rpscc
git submodule init
git submodule update
mkdir build & cd build
cmake ..
make
```

## Usage

```sh
./server_main --master_ip_port=162.105.146.128:16666 --server_port=17777
./agent_main --net_interface=eno1 --listen_port=15555 --master_ip_port=162.105.146.128:16666
python Linear_Regression.py
./master_main --worker_num=1 --server_num=1 --listen_port=16666 --master_ip_port=162.105.146.128:16666 --key_range=100 --bound=1
```

This scripts will automatically generate protobuf files.

## TODO List

Please follow our [TODO.md](https://github.com/purkyston/rpscc/blob/master/TODO.md)
