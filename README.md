[![Build Status](https://travis-ci.com/purkyston/Plug-in-ML.svg?branch=master)](https://travis-ci.com/purkyston/Plug-in-ML)

## 简介
通过Plug-in ML框架，用户不需要重新编写并行程序，而是将已有的，使用任何编程语言编写的串行机器学习程序，
通过接口直接与我们所设计的计算框架对接，从而快速完成机器学习算法并行化的任务。
除此之外，该框架针对分布式机器学习应用的特点进行各种优化，并且在对用户透明的前提下提供完善的错误恢复机制。


本项目由北京大学网络所云计算组开发，项目受到国家自然科学基金（项目号：61572044）的支持和肖臻研究员的指导。
云计算组研究方向包括分布式计算和机器学习，更多信息请访问[云计算组主页](http://zhenxiao.com/)

## Installation

### Install Protobuf
Please see the [installation guide](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md)

```sh
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.7.1/protobuf-cpp-3.7.1.tar.gz
cd protobuf-3.7.1
./configure
make -j8
make check
sudo make install
sudo ldconfig
```

### Install Zookeeper [optional]

```sh
# download the tar from this page
# http://zookeeper.apache.org/

# compile the cpp client
# according to this page: 
# https://github.com/apache/zookeeper/tree/release-3.4.14/zookeeper-client/zookeeper-client-c

cd zookeeper-x.x.x/zookeeper-client/zookeeper-client-c
sudo apt-get install libcppunit-dev
# cppunit.m4 may be installed in the /usr/share/aclocal
# or in the /usr/local/share/aclocal
# use the script below to generate the configure file
ACLOCAL="aclocal -I /usr/local/share/aclocal" autoreconf -if

make [run-check | check]
sudo make install
```

If using the zookeeper component, please add the 'USE_ZOOKEEPER' defination 
into the compiler.


### Compile RPSCC
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
