# Gradient descent algorithm for linear regression
from numpy import *
import os
import struct
import mmap
from sklearn.datasets import load_boston

# read from shared memory
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

# write to shared memory
def transfer2Agent(values, keys, w):
    w.seek(0)
    w.write(struct.pack('i', len(values)))
    # print values
    for v in values:
        w.write(struct.pack('f', v))
    w.seek(4*(101)) #________________________________________________________
    for k in keys:
        w.write(struct.pack('i', k))

# minimize the "sum of squared errors". This is how we calculate and correct our error
def compute_error_for_line_given_points(b, m, points):
    totalError = 0      #sum of square error formula
    for i in range (0, len(points)):
        x = points[i, 0]
        y = points[i, 1]
        totalError += (y-(m*x + b)) ** 2
    return totalError/ float(len(points))

def y_hat(parameters, x):
    y = 0
    i = 0
    for it in x:
        y += it * parameters[i]
        i = i + 1
    y += parameters[i]
    return y

def step_gradient(read_fifo, write_fifo, m, w, p, X, Y, learning_rate):
    #gradient descent
    grad = [0 for i in range(len(p))]
    N = float(len(X))
    for i in range(0, len(X)):
        x = X[i, :]
        y = Y[i]
        y_h = y_hat(p, x)
        grad[0] += -(2/N) * (y - y_h)
        for j in range(1, len(grad)):
            grad[j] += -(2/N) * x[j-1] * (y - y_h)
        print grad
    transfer2Agent(grad, [i for i in range(len(p))], w)
    os.write(write_fifo, struct.pack('i', 1))
    os.read(read_fifo, 4);
    coef, keys = readFromAgent(m)
    return coef

def gradient_descent_runner(read_fifo, write_fifo, m, w, X, Y, param, learning_rate, num_iteartions):
    p = param
    for i in range(num_iteartions):
       p = step_gradient(read_fifo, write_fifo, m, w, p, X, Y, learning_rate)
    return p

def run(read_fifo, write_fifo, m, w):
    # Step 1: Collect the data
    boston_data = load_boston()
    X = boston_data.data[0:300, :]
    Y = boston_data.target[0:300, :]
    # Step 2: Define our Hyperparameters
    learning_rate = 0.0001 #how fast the data converge
    # write the key the data have
    transfer2Agent([0 for i in range(14)], [i for i in range(14)], w)
    # signal to the agent
    os.write(write_fifo, struct.pack('i', 0))
    # wait agent
    os.read(read_fifo, 4)
    # read the initalize parameters from agent
    coef, keys = readFromAgent(m)
    # y=mx+b (Slope formule)
    init_param = coef
    num_iterations = 1000

    print("Running...")
    p = gradient_descent_runner(read_fifo, write_fifo, m, w, X, Y, init_param, learning_rate, num_iterations)
    print p
    os.write(write_fifo, struct.pack('i', 2))

# main function
if __name__ == "__main__":
    read_fd = os.open('/dev/shm/bh_shm_w_name', os.O_RDWR | os.O_SYNC | os.O_CREAT)
    write_fd = os.open('/dev/shm/bh_shm_r_name', os.O_RDWR | os.O_SYNC | os.O_CREAT)

    read_fifo_path = '/tmp/bh_w_fifo'
    write_fifo_path = '/tmp/bh_r_fifo'

    read_fifo = os.open(read_fifo_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)
    write_fifo = os.open(write_fifo_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)

    m = mmap.mmap(read_fd, 804, access=mmap.ACCESS_READ)
    w = mmap.mmap(write_fd, 804, mmap.MAP_SHARED, mmap.PROT_WRITE)

    run(read_fifo, write_fifo, m, w)
