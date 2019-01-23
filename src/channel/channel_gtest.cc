// Copyright (c) 2019 The RPSCC Authors. All rights reserved.
// Author : Zhen Lee (lz.askey@gmail.com)

#include "src/channel/fifo.h"
#include "src/channel/shared_memory.h"

#include "gtest/gtest.h"

#include <iostream>
#include <string>

using namespace std;
using namespace rpscc;

const string filename1 = "/tmp/test_fifo1";
const string filename2 = "/tmp/test_fifo2";
string ipc_name1 = "/test_sharedMemory1";
string ipc_name2 = "/test_sharedMemory2";
const int kflag = 0666;

// notice must run these three test one by one
// cause these test use the same fifo and shared
// memory may cause the result wrong.
// && these test must run in Linux, Mac OS is not
// support now.


TEST(ChannelTest, Fifo) {
  string fifoname = "/tmp/Fifo_Test1";
  mkfifo(fifoname.c_str(), kflag);
  int pid = fork();
  if(pid == 0) {
    Fifo fifo1;
    fifo1.Initialize(fifoname, true);
    fifo1.Open();
    int sig = fifo1.Wait();
    EXPECT_EQ(sig, 2);
    cout << "Read process complete." << endl;
    sig = fifo1.Wait();
    EXPECT_EQ(sig, 3);
  } else {
    sleep(1);
    Fifo fifo2;
    fifo2.Initialize(fifoname, false);
    fifo2.Open();
    fifo2.Signal(2);
    cout << "Write process complete." << endl;
    fifo2.Signal(3);
  }
}

TEST(ChannelTest, SharedMemory) {
  string fifoname = "/tmp/Fifo_Test2";
  string ipc_name_in = "/shm_test_in";
  mkfifo(fifoname.c_str(), kflag);
  int pid = fork();
  if(pid == 0) {
    Fifo fifo_reader;
    fifo_reader.Initialize(fifoname, true);
    fifo_reader.Open();

    SharedMemory read_mem;
    read_mem.Initialize(ipc_name_in.c_str());
    int sig_read = fifo_reader.Wait();
    //std::cout << sig_read << std::endl;

    shmstruct *data = read_mem.Read();
    cout << "read complete." << endl;
    EXPECT_EQ(data->size, 1);
    EXPECT_EQ(data->keys[0], 1111);
    EXPECT_EQ(data->values[0], 13.5);
  } else {
    sleep(1);
    Fifo fifo_writer;
    fifo_writer.Initialize(fifoname, false);
    fifo_writer.Open();

    SharedMemory write_mem;
    write_mem.Initialize(ipc_name_in.c_str());

    shmstruct store_data;
    store_data.values[0] = 13.5;
    store_data.keys[0] = 1111L;
    store_data.size = 1;
    write_mem.Write(&store_data);
    fifo_writer.Signal(1);
    cout << "write complete." << endl;
  }
}


// write to shared memory and then the python process will
// read the data add increase 1 to every keys and values
// and write to another shared memory and the c++ process read
// the data and make gtest to make sure.
// maybe chmod 777 /dev/shm/test_sharedMemory2 is needed

TEST(ChannelTest, ChannelTestWithPython) {
  system("python channel_test_with_python.py &");
  Fifo fifo_read, fifo_write;
  mkfifo(filename1.c_str(), kflag);
  mkfifo(filename2.c_str(), kflag);
  fifo_read.Initialize(filename2.c_str(), true);
  fifo_write.Initialize(filename1.c_str(), false);
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
  fifo_write.Open();
  fifo_write.Signal(1);
  fifo_read.Open();
  int sig = fifo_read.Wait();
  EXPECT_EQ(sig, 5);
  shmstruct* data_read = read_mem.Read();
  EXPECT_EQ(data_read->size, 100);
  for(int i=0; i<100; i++) {
    EXPECT_EQ(data_read->values[i], i+1.5);
    EXPECT_EQ(data_read->keys[i], i+1);
  }
  exit(0);
}

// Test with Liner regression
TEST(ChannelTest, LinearRegressionTest) {
  system("python Linear_Regression.py &");
  string wf_name = "/tmp/lr_w_fifo";
  string rf_name = "/tmp/lr_r_fifo";
  string shm_w_name = "/shm_w_name";
  string shm_r_name = "/shm_r_name";
  int32 fd = shm_open(shm_w_name.c_str(), O_RDWR | O_CREAT, FILE_MODE);
  ftruncate(fd, sizeof(struct shmstruct));
  close(fd);
  fd = shm_open(shm_r_name.c_str(), O_RDWR | O_CREAT, FILE_MODE);
  ftruncate(fd, sizeof(struct shmstruct));
  close(fd);

  mkfifo(wf_name.c_str(), 0777);
  mkfifo(rf_name.c_str(), 0777);
  Fifo wf, rf;
  wf.Initialize(wf_name, false);
  rf.Initialize(rf_name, true);
  SharedMemory rm, wm;
  rm.Initialize(shm_r_name.c_str());
  wm.Initialize(shm_w_name.c_str());

  wf.Open(); rf.Open();
  double b = 0, m = 0;
  int32 sig;
  shmstruct parameters, grad;
  float32 learning_rate = 0.0001;
  while(true) {
    sig = rf.Wait();
    if(sig == 0) {
      parameters.size = 2;
      parameters.keys[0] = 0;
      parameters.keys[1] = 1;
      parameters.values[0] = 0;
      parameters.values[1] = 0;
      wm.Write(&parameters);
      wf.Signal(2);
    } else if(sig == 1) {
      grad = *rm.Read();
      // cout << "c++" << endl;
      // co      //   cout << grad.keys[i] << endl;
      // }ut << grad.size << endl;
      // for(int i=0; i<grad.size; i++) {
      //   cout << grad.values[i] << endl;
      //   cout << grad.keys[i] << endl;
      // }x
      b = b - (learning_rate * grad.values[0]);
      m = m - (learning_rate * grad.values[1]);
      parameters.size = 2;
      parameters.keys[0] = 0;
      parameters.keys[1] = 1;
      parameters.values[0] = b;
      parameters.values[1] = m;
      wm.Write(&parameters);
      wf.Signal(2);
    } else {
      break;
    }
  }
  cout << b << " " << m << endl;
  exit(0);
}

// Test with Liner regression for boston housing dataset
// TEST(ChannelTest, BostonHousingTest) {
//   system("python boston_housing.py &");
//   string wf_name = "/tmp/bh_w_fifo";
//   string rf_name = "/tmp/bh_r_fifo";
//   string shm_w_name = "/bh_shm_w_name";
//   string shm_r_name = "/bh_shm_r_name";
//   int32 fd = shm_open(shm_w_name.c_str(), O_RDWR | O_CREAT, FILE_MODE);
//   ftruncate(fd, sizeof(struct shmstruct));
//   close(fd);
//   fd = shm_open(shm_r_name.c_str(), O_RDWR | O_CREAT, FILE_MODE);
//   ftruncate(fd, sizeof(struct shmstruct));
//   close(fd);

//   mkfifo(wf_name.c_str(), 0777);
//   mkfifo(rf_name.c_str(), 0777);
//   Fifo wf, rf;
//   wf.Initialize(wf_name, false);
//   rf.Initialize(rf_name, true);
//   SharedMemory rm, wm;
//   rm.Initialize(shm_r_name.c_str());
//   wm.Initialize(shm_w_name.c_str());

//   wf.Open(); rf.Open();
//   double param[100];
//   memset(param, 0, sizeof param);
//   int32 sig;
//   shmstruct parameters, grad;
//   float32 learning_rate = 0.0001;
//   int32 sz;
//   while(true) {
//     sig = rf.Wait();
//     if(sig == 0) {
//       grad = *rm.Read();
//       parameters.size = grad.size;
//       for(int i=0; i<grad.size; i++) {
//         parameters.keys[i] = i;
//         parameters.values[i] = param[i];
//       }
//       wm.Write(&parameters);
//       wf.Signal(2);
//     } else if(sig == 1) {
//       grad = *rm.Read();
//       // b = b - (learning_rate * grad.values[0]);
//       // m = m - (learning_rate * grad.values[1]);
//       sz = grad.size;
//       for(int i=0; i<sz; i++) {
//         param[i] = param[i] - (learning_rate * grad.values[i]);
//       }

//       parameters.size = sz;
//       for(int i=0; i<sz; i++) {
//         parameters.keys[i] = i;
//         parameters.values[i] = param[i];
//       }
//       wm.Write(&parameters);
//       wf.Signal(2);
//     } else {
//       break;
//     }
//   }
//   for(int i=0; i<sz; i++) {
//     cout << param[i] << " ";
//   }
//   cout << endl;
//   exit(0);
// }
