import mmap
import contextlib
import struct
import os

read_fd = open('/dev/shm/test_sharedMemory1', 'rb')
#write_fd = open('/dev/shm/test_sharedMemory2', 'wb')
write_fd = os.open('/dev/shm/test_sharedMemory2', os.O_RDWR | os.O_SYNC | os.O_CREAT)
read_fifo_path = '/tmp/test_fifo1'
write_fifo_path = '/tmp/test_fifo2'

read_fifo = os.open(read_fifo_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)
write_fifo = os.open(write_fifo_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)

sig = os.read(read_fifo, 4)
print struct.unpack('i', sig)[0]
s = []
with contextlib.closing(mmap.mmap(read_fd.fileno(), 804, access=mmap.ACCESS_READ)) as m:
    m.seek(0)
    c = m.read(4)
    s.append(struct.unpack('i', c)[0])
    for i in range(100):
        c = m.read(4)
        s.append(struct.unpack('f', c)[0])
    for i in range(100):
        c = m.read(4)
        s.append(struct.unpack('i', c)[0])

    #print s
# add 1 to every keys and values
with contextlib.closing(mmap.mmap(write_fd, 804, mmap.MAP_SHARED, mmap.PROT_WRITE)) as w:
    w.seek(0)
    w.write(struct.pack('i', s[0]))
    for i in range(1, 101):
        w.write(struct.pack('f', s[i]+1))
    for i in range(101, 201):
        w.write(struct.pack('i', s[i]+1))

os.write(write_fifo, struct.pack('i', 5))
