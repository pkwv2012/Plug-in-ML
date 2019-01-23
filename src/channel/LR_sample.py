import numpy as np
from sklearn import linear_model
import mmap
import contextlib
import struct
import os

def transfer2Agent(values, keys, w):
    w.seek(0)
    w.write(struct.pack('i', len(values)))
    for v in values:
        w.write(struct.pack('f', v))
    w.seek(4*(101)) #________________________________________________________
    for k in keys:
        w.write(struct.pack('i', k))

def readFromAgent(m):
    m.seek(0)
    c = m.read(4)
    sz = struct.unpack('i', c)[0]
    coef = []
    keys = []
    for i in range(sz):
        c = m.read(4)
        coef.append(struct.unpack('f', c)[0])
    for i in range(100-sz):
        c = m.read(4)
    for i in range(sz):
        c = m.read(4)
        keys.append(struct.unpack('i', c)[0])
    return coef, keys
if __name__ == '__main__':

    read_fd = os.open('/dev/shm/test_sharedMemory_sample1', os.O_RDWR | os.O_SYNC | os.O_CREAT)
    write_fd = os.open('/dev/shm/test_sharedMemory_sample2', os.O_RDWR | os.O_SYNC | os.O_CREAT)
    read_fifo_path = '/tmp/test_fifo_sample1'
    write_fifo_path = '/tmp/test_fifo_sample2'

    read_fifo = os.open(read_fifo_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)
    write_fifo = os.open(write_fifo_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)

    m = mmap.mmap(read_fd, 804, access=mmap.ACCESS_READ)
    w = mmap.mmap(write_fd, 804, mmap.MAP_SHARED, mmap.PROT_WRITE)

    # start of compute
    n_samples, n_features = 1000, 5
    np.random.seed(0)
    y = np.random.randn(n_samples)
    X = np.random.randn(n_samples, n_features)
    clf = linear_model.SGDRegressor(max_iter=1000, tol=1e-3)
    clf.fit(X[0:200, :], y[0:200])
    coef = clf.coef_
    intercept = clf.intercept_
    keys = [i for i in range(5)]
    # print coef
    for i in range(0, 5):
        transfer2Agent([], keys, w)
        os.write(write_fifo, struct.pack('i', 0))
        os.read(read_fifo, 4);
        coef, keys = readFromAgent(m)
        clf.fit(X[i*200:(i+1)*200,:], y[i*200:(i+1)*200], coef_init=coef, intercept_init=intercept);
        coef = clf.coef_ - coef
        #intercept = clf.intercept_
        transfer2Agent(coef, keys, w)
        os.write(write_fifo, struct.pack('i', 1))

    os.write(write_fifo, struct.pack('i', 2))
    #print X[0:100, :]
    #print clf.predict(X[0:100, :])
