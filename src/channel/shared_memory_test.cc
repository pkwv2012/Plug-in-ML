//
// Created by pkwv on 4/7/19.
//

#include <string>
#include <Python.h>

#include "src/channel/shared_memory.h"

#include "gtest/gtest.h"

TEST(SharedMemory, PythonCpp) {
  std::string reader_name = "/python_cpp";
  rpscc::SharedMemory reader;
  reader.Initialize(reader_name.c_str());;
  // Invoke the python program.
  std::string python_prog = "write_to_shared_memory";
  wchar_t *program = Py_DecodeLocale(python_prog.c_str(), NULL);
  Py_SetProgramName(program);  /* optional but recommended */
  Py_Initialize();
  PyRun_SimpleString("import os\n"
                     "import mmap\n"
                     "write_fd = os.open('/dev/shm/python_cpp', os.O_RDWR | os.O_SYNC)\n"
                     "w = mmap.mmap(write_fd, 804, mmap.MAP_SHARED, mmap.PROT_WRITE)\n"
                     "import struct\n"
                     "w.seek(0)\n"
                     "w.write(struct.pack('i', 1))\n"
                     "for i in range(90):\n"
                     "    w.write(struct.pack('f', i))\n"
                     "w.seek(101 * 4)\n"
                     "for i in range(90):\n"
                     "    w.write(struct.pack('i', i))\n"
                     );
  PyMem_RawFree(program);
  rpscc::shmstruct* data = reader.Read();
  EXPECT_EQ(data->size, 1);
  for (int i = 0; i < 90; ++ i)
    EXPECT_FLOAT_EQ(data->values[i], i);
  for (int i = 0; i < 90; ++ i)
    EXPECT_EQ(data->keys[i], i);
}


