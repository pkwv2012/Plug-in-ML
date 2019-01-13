// Copyright (c) 2019 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#include "src/channel/fifo.h"
#include "src/channel/shared_memory.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace std;
using namespace rpscc;

const string filename1 = "/tmp/test_fifo1";
const string filename2 = "/tmp/test_fifo2.dat";
string ipc_name1 = "/tmp/test_sharedMemory1";
string ipc_name2 = "/tmp/test_sharedMemory2";
const int kflag = 0666;

// notice must run these three test one by one
// cause these test use the same fifo and shared
// memory may cause the result wrong.
// && these test must run in Linux, Mac OS is not
// support now.


TEST(ChannelTest, Fifo) {
  mkfifo(filename1.c_str(), kflag);
  int pid = fork();
  if(pid == 0) {
    Fifo fifo1;
    fifo1.Initialize(filename1, true);
    fifo1.Open();
    fifo1.Wait();
    cout << "Read process complete." << endl;
  } else {
    sleep(1);
    Fifo fifo2;
    fifo2.Initialize(filename1, false);
    fifo2.Open();
    fifo2.Signal();
    cout << "Write process complete." << endl;
  }
}

TEST(ChannelTest, SharedMemory) {
  mkfifo(filename1.c_str(), kflag);
  int pid = fork();
  if(pid == 0) {
    Fifo fifo_reader;
    fifo_reader.Initialize(filename1, true);
    fifo_reader.Open();

    SharedMemory read_mem;
    read_mem.Initialize(ipc_name1.c_str());

    fifo_reader.Wait();

    shmstruct *data = read_mem.Read();
    cout << "read complete." << endl;
    EXPECT_EQ(data->size, 1);
    EXPECT_EQ(data->keys[0], 1111);
    EXPECT_EQ(data->values[0], 13.5);
  } else {
    sleep(1);
    Fifo fifo_writer;
    fifo_writer.Initialize(filename1, false);
    fifo_writer.Open();

    SharedMemory write_mem;
    write_mem.Initialize(ipc_name1.c_str());

    shmstruct store_data;
    store_data.values[0] = 13.5;
    store_data.keys[0] = 1111L;
    store_data.size = 1;
    write_mem.Write(&store_data);
    fifo_writer.Signal();
    cout << "write complete." << endl;
  }
}

TEST(ChannelTest, FifoWithPython) {
  // run this test first && then run python_test.py
  // maybe following add the part of auto run python_test.py
  mkfifo(filename1.c_str(), kflag);
  Fifo read_fifo;
  read_fifo.Initialize(filename1, true);
  read_fifo.Open();
  read_fifo.Wait();
  cout << "read complete." << endl;
}

// write to shared memory and then the python process will
// read the data add increase 1 to every keys and values
// and write to another shared memory and the c++ process read
// the data and make gtest to make sure.
// maybe chmod 777 /dev/shm/test_sharedMemory2 is needed
// also need to run channel_test_with_python.py first
TEST(ChannelTest, ChannelTestWithPython) {
  Fifo fifo_read, fifo_write;
  mkfifo(filename1.c_str(), kflag);
  mkfifo(filename2.c_str(), kflag);
  fifo_read.Initialize(filename2.c_str(), true);
  fifo_write.Initialize(filename1.c_str(), false);
  fifo_read.Open();
  fifo_write.Open();
  SharedMemory read_mem, write_mem;
  read_mem.Initialize(ipc_name2.c_str());
  write_mem.Initialize(ipc_name1.c_str());
  shmstruct store_data;
  store_data.size = 100;
  memset(store_data.values, 0, sizeof store_data.values);
  memset(store_data.keys, 0, sizeof store_data.keys);
  for(int i=0; i<100; i++) {
    store_data.values[i] = i+0.5;
    store_data.keys[i] = i;
  }
  write_mem.Write(&store_data);
  fifo_write.Signal();
  fifo_read.Wait();

  shmstruct* data_read = read_mem.Read();
  EXPECT_EQ(data_read->size, 100);
  for(int i=0; i<100; i++) {
    EXPECT_EQ(data_read->values[i], i+1.5);
    EXPECT_EQ(data_read->keys[i], i+1);
  }
  exit(0);
}