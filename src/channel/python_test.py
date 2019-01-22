import os
import struct

write_path = "/tmp/python_fifo"

wf = os.open(write_path, os.O_RDWR)

len_send = os.write(wf, struct.pack('i', 2))

len_send = os.write(wf, struct.pack('i', 3))

os.close(wf)
